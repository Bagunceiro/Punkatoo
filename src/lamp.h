#ifndef FC_LAMP_H
#define FC_LAMP_H

#include <vector>

#include "mqtt.h"
#include "infrared.h"
#include "switch.h"

// typedef std::vector<int> SwitchPinList;



// typedef std::vector<Switch> SwitchList;

class Lamp: public MQTTClientDev, public IRClientDev, public SwitchedDev
{
  public:
    Lamp(String devName, const int relayPin);

    void sw(int toState);
    void toggle();
    const int getStatus();
    
    const int blip(const int t = 500);
    void blip(const int number, const int length);

    virtual ~Lamp();
    virtual String mqttGetStatus();
    virtual void mqttMsgRecd(const String &topic, const String &msg);
    
    virtual void irmsgRecd(IRMessage msg);
    virtual void switchTo(const int state) { toggle(); }

  private:
    int lpin; // goes to the control relay (active low)
    virtual void subscribeToIR();
    virtual void subscribeToMQTT();
};

#endif
