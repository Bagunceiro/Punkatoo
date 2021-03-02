#ifndef FC_LAMP_H
#define FC_LAMP_H

#include <vector>

#include "mqtt.h"
#include "infrared.h"

typedef std::vector<int> SwitchPinList;

struct Switch
{
  int spin;
  int switchState;
  int debounce;
};

typedef std::vector<Switch> SwitchList;

class Lamp: public MQTTClientDev, public IRControlled, public P2Task
{
  public:
    Lamp(String devName);

    void init(const SwitchPinList inpList, int out);
    void sw(int toState);
    void toggle();
    virtual bool operator()();
    const int blip(const int t = 500);
    void blip(const int number, const int length);

    virtual ~Lamp();
    virtual String getStatus();
    virtual void mqttMsgRecd(const String &topic, const String &msg);
    
    virtual void irmsgRecd(IRMessage msg);
    int switchstate();

  private:
    int lpin; // goes to the control relay (active low)
    virtual void subscribeToIR();
    virtual void subscribeToMQTT();

    SwitchList swList;
};

#endif
