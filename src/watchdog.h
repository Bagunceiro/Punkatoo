#pragma once

#include <Arduino.h>

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>

#include "p2task.h"

class Watchdog
{
    public:
    Watchdog();
    virtual ~Watchdog();
    void telegram(const char *botToken, const char *chat);
    void send(const char*);

    private:
    char* chatId;
    WiFiClientSecure secured_client;
    UniversalTelegramBot* bot;
};