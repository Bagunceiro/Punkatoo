#include "cli.h"
#include "config.h"
#include "url.h"
#include <HTTPUpdate.h>

CLITask *CLITask::pThis = NULL;

CLITask::CLITask(const char *name) : P2Task(name, 3500), cliServer(1685)
{
    pThis = this;
}

CLITask::~CLITask() {}

void CLITask::init()
{
    cliServer.begin();
    start(1);
}

bool CLITask::operator()()
{
    cliClient = cliServer.available();
    while (cliClient)
    {
        String s = getCommand();
        stringArray argv;
        parse(s.c_str(), argv);
        int result = execute(argv);
        if (result < 0)
        {
            cliClient.printf("%s: error %d (%s)\n", argv[0].c_str(), result, error.c_str());
        }
    }
    return true;
}

String CLITask::getCommand()
{
    String result;
    cliClient.printf("%s> ", config[controllername_n].c_str());

    while (true)
    {
        if (cliClient.available() > 0)
        {
            int c = (cliClient.read());
            if (c == '\n')
                break;
            else if (c == '\r')
                ;
            else
            {
                if (isprint(c))
                    result += (char)c;
                else
                {
                }
            }
        }
        else if (!cliClient.connected())
        {
            cliClient.stop();
            result = "";
            break;
        }
        delay(50);
    }
    return result;
}

int CLITask::parse(const char *line, stringArray &argv)
{
    String arg;
    bool quoting = false;
    bool quotechar = false;

    int len = strlen(line);
    for (int i = 0; i < len; i++)
    {
        char c = line[i];

        if (c == '"' && !quotechar)
        {
            quoting = !quoting;
            continue;
        }
        else if (!quoting && !quotechar)
        {
            if (c == '\\')
            {
                quotechar = true;
                continue;
            }
            else if (isspace(c))
            {
                if (arg.length() > 0)
                    argv.push_back(arg);
                arg = "";
                continue;
            }
        }
        quotechar = false;
        arg += c;
    }
    if (arg.length() > 0)
        argv.push_back(arg);

    return 0;
}

void CLITask::progress(size_t completed, size_t total)
{
    static unsigned long then = 0;
    unsigned long now = millis();
    if ((now - then) > 100)
    {
        then = now;
        cliClient.printf("%lu %u%% (%u/%u)\n", now, (100 * completed) / total, completed, total);
    }
}

void CLITask::progcb(size_t completed, size_t total, void *ptr)
{
    CLITask *thecli = reinterpret_cast<CLITask *>(ptr);
    thecli->progress(completed, total);
}

int CLITask::wget(stringArray argv)
{
    int result = -1;
    if ((argv.size() >= 2) && (argv.size() < 4))
    {
        String url = argv[1];
        String target;
        if (!url.startsWith("http://"))
            url = "http://" + url;
        if (argv.size() == 3)
        {
            target = argv[2];
        }
        else
        {
            int index = url.lastIndexOf("/");
            target = url.substring(index);
        }
        if (!target.startsWith("/"))
            target = "/" + target;

        HTTPClient http;

        // http.begin(url); using this form adds 150K odd to the size of the image!
        // So instead:

        Url u(url.c_str());
        uint16_t port = u.getPort();
        if (port == 0)
            port = 80;
        http.begin(u.getHost(), port, u.getPath());

        int httpCode = http.GET();

        if (httpCode > 0)
        {
            if (httpCode == HTTP_CODE_OK)
            {
                File f = LITTLEFS.open("/upload.tmp", "w+");
                if (f)
                {
                    Stream &s = http.getStream();
                    uint8_t buffer[128];
                    size_t total = http.getSize();
                    size_t sofar = 0;
                    size_t l;
                    while ((l = s.readBytes(buffer, sizeof(buffer) - 1)))
                    {
                        f.write(buffer, l);
                        reportProgress(sofar, total);
                        sofar += l;
                    }
                    f.close();
                    if (LITTLEFS.rename("/upload.tmp", target))
                    {
                        serr.println("Complete");
                        cliClient.println("Complete");
                        result = 0;
                    }
                    else
                        cliClient.println("Couldn't create file");
                }
                else
                    cliClient.println("Couldn't create temp file");
            }
            else
                cliClient.printf("Upload failed %d\n", httpCode);
        }
        else
            cliClient.printf("Get failed %s\n", http.errorToString(httpCode).c_str());
    }
    else
        error = "upload SERVER PORT SOURCE TARGET";
    return result;
}

void CLITask::reportProgress(size_t completed, size_t total, int interval)
{
    static int oldPhase = 1;
    int progress = (completed * 100) / total;

    int phase = (progress / interval) % 2; // report at 5% intervals

    if (phase != oldPhase)
    {
        serr.printf("%3d%% (%d/%d)\n", progress, completed, total);
        cliClient.printf("%3d%% (%d/%d)\n", progress, completed, total);
        oldPhase = phase;
    }
}

void CLITask::reportProgressCB(size_t completed, size_t total)
{
    pThis->reportProgress(completed, total);
}

int CLITask::sysupdate(stringArray argv)
{
    int result = -1;
    if (argv.size() == 2)
    {
        String url = argv[1];
        if (!url.startsWith("http://"))
            url = "http://" + url;

        WiFiClient httpclient;

        httpUpdate.rebootOnUpdate(false);

        Update.onProgress(reportProgressCB);
        serr.println("System update started");

        t_httpUpdate_return ret = httpUpdate.update(httpclient, url);

        switch (ret)
        {
        case HTTP_UPDATE_FAILED:
            cliClient.printf("Update fail error (%d): %s\n",
                             httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
            break;

        case HTTP_UPDATE_NO_UPDATES:
            cliClient.println("No update file available");
            break;

        case HTTP_UPDATE_OK:
            serr.println("System update available - reseting");
            cliClient.println("System update available - reseting");
            cliClient.stop();
            delay(1000);
            ESP.restart();
            result = 0;
            break;
        }
    }
    else
    {
        error = "sysupdate URL";
    }
    return result;
}

void CLITask::treeRec(File dir)
{
    if (dir)
    {
        dir.size();
        if (dir.isDirectory())
        {
            cliClient.printf("%s :\n", dir.name());
            while (File f = dir.openNextFile())
            {
                treeRec(f);
                f.close();
            }
        }
        else
            cliClient.printf(" %6.d %s\n", dir.size(), dir.name());

        dir.close();
    }
}

int CLITask::rm(stringArray argv)
{
    for (int i = 1; i < argv.size(); i++)
    {
        if (!((LITTLEFS.remove(argv[i]) || LITTLEFS.rmdir(argv[i]))))
        {
            cliClient.printf("Could not remove %s\n", argv[i].c_str());
        }
    }
    return 0;
}

int CLITask::tree(stringArray argv)
{
    File dir = LITTLEFS.open("/");
    treeRec(dir);
    dir.close();
    return 0;
}

int CLITask::mkdir(stringArray argv)
{
    for (int i = 1; i < argv.size(); i++)
    {
        if (!LITTLEFS.mkdir(argv[i]))
        {
            cliClient.printf("Could not make %s\n", argv[i].c_str());
        }
    }
    return 0;
}

int CLITask::cat(stringArray argv)
{
    for (int i = 1; i < argv.size(); i++)
    {
        File f = LITTLEFS.open(argv[i].c_str());
        if (f)
        {
            if (f.isDirectory())
            {
                cliClient.printf("%s is a directory\n", argv[i].c_str());
                break;
            }
            else
            {
                int c;
                while ((c = f.read()) >= 0)
                    cliClient.write(c);
            }
            f.close();
        }
        else
        {
            cliClient.printf("Could not open %s\n", argv[i].c_str());
        }
    }
    return 0;
}

int CLITask::reboot(stringArray argv)
{
    cliClient.println("Rebooting");
    cliClient.flush();
    delay(100);
    cliClient.stop();
    delay(500);
    ESP.restart();
    return 0;
}

int CLITask::execute(stringArray argv)
{
    int result = -1;
    if (argv.size() >= 1)
    {

        if (argv[0] == "sysupdate")
        {
            result = sysupdate(argv);
        }
        else if (argv[0] == "wget")
        {
            result = wget(argv);
        }
        else if (argv[0] == "tree")
        {
            result = tree(argv);
        }
        else if (argv[0] == "rm")
        {
            result = rm(argv);
        }
        else if (argv[0] == "mkdir")
        {
            result = mkdir(argv);
        }
        else if (argv[0] == "cat")
        {
            result = cat(argv);
        }
        else if (argv[0] == "reboot")
        {
            result = reboot(argv);
        }
        else if (argv[0] == "help")
        {
            cliClient.println("cat FILE");
            cliClient.println("rm FILE ...");
            cliClient.println("mkdir DIR ...");
            cliClient.println("sysupdate URL");
            cliClient.println("tree");
            cliClient.println("wget URL [TARGET]");
            cliClient.println("reboot");
            result = true;
        }
        else if (argv[0] == "exit")
        {
            cliClient.stop();
        }
        else
        {
            error = "Command not recognised";
        }
    }
    return result;
}