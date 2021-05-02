#include "cli.h"
#include "config.h"
#include <LITTLEFS.h>
#include <HTTPUpdate.h>

CLITask *CLITask::pThis = NULL;

CLITask::CLITask(const char *name) : P2Task(name, 6000), cliServer(1685)
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
    if (cliClient)
    {
        String s = getCommand();
        stringArray argv;
        parse(s.c_str(), argv);
        execute(argv);
    }
    return true;
}

String CLITask::getCommand()
{
    String result;

    while (true)
    {
        if (cliClient.available() > 0)
        {
            result = cliClient.readStringUntil('\n');
            break;
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

int CLITask::upload(stringArray argv)
{
    int result = -1;
    if (argv.size() == 5)
    {
        const char *server = argv[1].c_str();
        int port = argv[2].toInt();
        const char *source = argv[3].c_str();
        const char *target = argv[4].c_str();

        HTTPClient http;
        http.begin(server, port, source);
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
            cliClient.printf("Get failed %s", http.errorToString(httpCode).c_str());
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
        const char *url = argv[1].c_str();

        WiFiClient httpclient;

        httpUpdate.rebootOnUpdate(false);

        Update.onProgress(reportProgressCB);

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

int CLITask::execute(stringArray argv)
{
    int result = -1;
    if (argv[0] == "upload")
    {
        result = upload(argv);
    }
    else if (argv[0] == "sysupdate")
    {
        result = sysupdate(argv);
    }
    else
    {
        error = "Command not recognised";
    }
    return result;
}