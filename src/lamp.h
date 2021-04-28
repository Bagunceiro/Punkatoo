#ifndef FC_LAMP_H
#define FC_LAMP_H

#include <vector>

#include "mqtt.h"
#include "infrared.h"
#include "switch.h"

class Lamp: public MQTTClientDev, public IRClientDev, public SwitchedDev
{
  public:
    Lamp(const char* devName, const int relayPin);

    const int getStatus() const;
    void sw(int toState);
    void toggle();

    const int blip(const int t = 500);
    void blip(const int number, const int length);

    virtual ~Lamp();
    virtual String mqttGetStatus() override;
    virtual void mqttMsgRecd(const String &topic, const String &msg) override;
    
    virtual void irmsgRecd(IRMessage msg) override;
    virtual void switchTo(const int state) { toggle(); }

  private:
    int lpin; // goes to the control relay (active low)
    virtual void subscribeToIR() override;
    virtual void subscribeToMQTT() override;
};

#endif
