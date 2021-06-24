#include "watchdog.h"
#include "config.h"

Watchdog::Watchdog()
{
    bot = NULL;
}

void Watchdog::telegram(const char *botToken, const char *chat)
{
    bot = new UniversalTelegramBot(botToken, secured_client);
    secured_client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
    chatId = (char *)malloc(strlen(chat) + 1);
    strcpy(chatId,chat);
}

Watchdog::~Watchdog()
{
    if (bot) free(bot);
}

void Watchdog::send(const char *msg)
{
    if (config[watchdog_n] == "1")
    {
        if (bot)
        {
            char buff[64];
            sprintf(buff, "%s %s: %s", nowTime(), config[controllername_n].c_str(), msg);
            Serial.printf("Watchdog sends: %s\n", buff);
            bot->sendMessage(chatId, buff, "");
        }
    }
}