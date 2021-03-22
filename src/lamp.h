#ifndef FC_LAMP_H
#define FC_LAMP_H

#include <vector>

#include "mqtt.h"
#include "infrared.h"
#include "switch.h"

// typedef std::vector<int> SwitchPinList;



// typedef std::vector<Switch> SwitchList;

class Lamp: public MQTTClientDev, public IRControlled, public SwitchedDev
{
  public:
    Lamp(String devName, const int relayPin);

    // void init(const SwitchPinList inpList, int out);
    void sw(int toState);
    void toggle();
    // virtual bool operator()();
    const int blip(const int t = 500);
    void blip(const int number, const int length);

    virtual ~Lamp();
    virtual String mqttGetStatus();
    virtual void mqttMsgRecd(const String &topic, const String &msg);
    
    virtual void irmsgRecd(IRMessage msg);
    // int switchstate();
    virtual void switchTo(const int state) { toggle(); }

  private:
    int lpin; // goes to the control relay (active low)
    virtual void subscribeToIR();
    virtual void subscribeToMQTT();

    // SwitchList swList;
};

#endif
