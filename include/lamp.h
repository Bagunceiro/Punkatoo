#ifndef FC_LAMP_H
#define FC_LAMP_H

#include "mqtt.h"
#include "infrared.h"

class Lamp: public MqttControlled, IRControlled
{
  public:
    Lamp(String devName);
    virtual ~Lamp();

    void init(int inp, int out);
    void sw(int toState);
    void toggle();
    void pollSwitch();
    char* getStatus(char* buff);
    const int blip(const int t = 500);
    void blip(const int number, const int length);
    void mqttaction(String topic, String msg);
    virtual void doSubscriptions(PubSubClient& mqttclient);
    virtual void irmsgRecd(uint32_t code);

  private:
    int spin; // Switch is connected here
    int lpin; // goes to the control relay (active low)
    int switchState;
    int debounce;
};

#endif
