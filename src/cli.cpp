#include "cli.h"
#include <config.h>
#include <LITTLEFS.h>

CLITask* CLITask::pThis = NULL;

CLITask::CLITask(const char *name) : P2Task(name, 4096), cliServer(1685)
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
        // stringArray args;

        String s = getCommand();
        stringArray argv;
        parse(s.c_str(), argv);
        int result = execute(argv);
        if (result == 0)
            cliClient.println("\nOK");
        else
        {
            cliClient.println(error);
        }
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
    int result = 0;
    if (argv.size() == 5)
    {
        const char *server = argv[1].c_str();
        int port = argv[2].toInt();
        const char *source = argv[3].c_str();
        const char *target = argv[4].c_str();

        // cliClient.printf("upload %s:%d%s to %s\n", server, port, source, target);

        dev.updater.onProgress(progcb, this);
        Serial.println("calling configUpsdate");
        if (!dev.updater.configUpdate(server, port, source, target))
        {
            error = String("Upload failed ") + server + ":" + port + source + " " + target;
            result = -1;
        }
    }
    else
    {
        error = "upload SERVER PORT SOURCE TARGET";
        result = -1;
    }
    return result;
}

void CLITask::reportProgress(size_t completed, size_t total)
{
    static int oldPhase = 1;
    int progress = (completed * 100) / total;

    int phase = (progress / 5) % 2; // report at 5% intervals

    if (phase != oldPhase)
    {
        cliClient.printf("%3d%% (%d/%d)\n", progress, completed, total);
        oldPhase = phase;
    }
}

void CLITask::reportProgressCB(size_t completed, size_t total)
{
    pThis->progress(completed, total);
}

int CLITask::sysupdate(stringArray argv)
{
    int result = -1;
    if (argv.size() == 2)
    {
        const char* url = argv[1].c_str();

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
            cliClient.println("System update available - reboot to load");
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
    if (argv[0] == "sysupdate")
    {
        result = sysupdate(argv);
    }
    else
    {
        error = "Command not recognised";
    }
    return result;
}