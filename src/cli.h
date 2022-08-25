#pragma once

#include <Arduino.h>
#include <WiFi.h>
#include <vector>
#include <LittleFS.h>

#include "p2task.h"

typedef std::vector<String> stringArray; /* An array of Strings. The command line arguments are passed in this form */

/**
 * @class CLITask
 * @brief A networked mini-shell - listens by default on port 1685
 *
 * Runs as a seperate task.
 * There should only be one instance of this class.
 *
 */
class CLITask : public P2Task
{
public:
    CLITask(const char *name);
    virtual ~CLITask();
    /**
     * @brief service shell request
     *
     * If there is a connection request pending runs the shell. otherwise returns
     */
    bool operator()() override;
    /**
     *  @brief Start up the listener
     * @return Always returns true
     */
    void init();
    /**
     * @brief Report system update progress
     * @param completed The number of bytes so far received
     * @param total The total number of bytes expected
     * @param interval Interval (in percentage points) between report updates
     */
    void reportProgress(size_t completed, size_t total, int interval = 10);
    /**
     * @brief Callback from system update, gets context for object method reportProgress()
     * @param completed The number of bytes so far received
     * @param total The total number of bytes expected
     */
    static void reportProgressCB(size_t completed, size_t total);

private:
    WiFiServer cliServer;
    WiFiClient cliClient;

    /**
     * @brief Read command line
     * @return String containg the raw command line
     */
    String getCommand();
    /**
     * @brief Parse command line
     *
     * @param line Raw command line
     * @param argv parsed command line arguments ([0] is command)
     */
    int parse(const char *line, stringArray &argv);
    /** @brief Interpret command line and call the appropriate execution function */
    int execute(stringArray argv);

    /** @brief Retrieve a file given its URL */
    int wget(stringArray argv);
    /** @brief Show contents of a file */
    int cat(stringArray argv);
    /** @brief Update the system from a firmware image given its URL */
    int sysupdate(stringArray argv);
    /** @brief Load a filesystem image given its URL */
    int fsupdate(stringArray argv);
    /** @brief Remove a file */
    int rm(stringArray argv);
    /** @brief Make a directory */
    int mkdir(stringArray argv);
    /** @brief Dump the directory tree structure */
    int tree(stringArray argv);
    /** @brief Reset the processor */
    int reboot(stringArray argv);
    /** @brief Query state of any PIRs */
    int pir(stringArray argv);
    /** @brief Worker function called by tree(). Recursively decends tree structure */
    void treeRec(File f);
    /** @brief Decoded error return to report to client */
    String error;

    /** @brief Context pointer for  unique instance of this class (for callbacks) */
    static CLITask *pThis;
};