#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <LittleFS.h>

#include "p2task.h"

typedef std::vector<String> stringArray;

class CLITask : public P2Task
{
public:
    CLITask(const char *name);
    virtual ~CLITask();
    bool operator()() override;
    void init();
    void reportProgress(size_t completed, size_t total, int interval = 10);
    static void reportProgressCB(size_t completed, size_t total);

private:
    WiFiServer cliServer;
    WiFiClient cliClient;
    String getCommand();
    int parse(const char *line, stringArray &argv);
    int execute(stringArray argv);

    int upload(stringArray argv);
    int wget(stringArray argv);
    int cat(stringArray argv);
    int sysupdate(stringArray argv);
    int fsupdate(stringArray argv);
    int rm(stringArray argv);
    int mkdir(stringArray argv);
    int tree(stringArray argv);
    int reboot(stringArray argv);
    void treeRec(File f);

    void progress(const size_t comp, const size_t total);
    static void progcb(const size_t comp, const size_t total, void *obj);

    String error;
    static CLITask *pThis;
};