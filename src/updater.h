#include <HTTPUpdate.h>

#include "mqtt.h"

class Updater
{
public:
    Updater(const String& devName);
    virtual ~Updater();
    t_httpUpdate_return systemUpdate(const String& server, const uint16_t port, const String& image);
    void onStart(void(*callback)());
    void onEnd(void(*callback)());
    void onNone(void(*callback)());
    void onFail(void(*callback)());
    private:
    void(*startCallback)();
    void(*endCallback)();
    void(*nullCallback)();
    void(*failCallback)();
};