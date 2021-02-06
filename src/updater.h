#include <HTTPUpdate.h>

#include "mqtt.h"

class Updater : public MqttControlled
{
public:
    Updater(const String& devName);
    virtual ~Updater();
    virtual void mqttaction(const String& topic, const String& msg);
    virtual void doSubscriptions(PubSubClient& mqttclient);
    virtual String getStatus();
    t_httpUpdate_return systemUpdate(const String& server, const uint16_t port, const String& image, const String& ver);
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