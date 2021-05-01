#include "cli.h"

CLITask::CLITask(const char *name) : P2Task(name, 2048), cliServer(1685)
{
}

CLITask::~CLITask() {}

void CLITask::init()
{
    cliServer.begin();
    start(0);
}

bool CLITask::operator()()
{
    cliClient = cliServer.available();
    bool conn = false;
    while (cliClient)
    {
        if (!conn)
        {
            Serial.println("Client connected");
            conn = true;
        }
        std::vector<String> args;

        cliClient.print("Punkatoo>");
        String s = getCommand();
        cliClient.printf("Command: %s\n", s.c_str());
    }
    if (conn)
        Serial.println("Client disconnected");
    return true;
}

String CLITask::getCommand()
{
    String result;

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