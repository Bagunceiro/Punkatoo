#ifndef FC_LAMP_H
#define FC_LAMP_H

#include <vector>

#include "mqtt.h"
#include "infrared.h"

typedef std::vector<int> SwitchList;

struct SwBlk
{
  int spin;
  int switchState;
  int debounce;
};

typedef std::vector<SwBlk> SwBlkList;

class Lamp: public MQTTClientDev, public IRControlled, public PTask
{
  public:
    Lamp(String devName);

    void init(const SwitchList inpList, int out);
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

    SwBlkList swList;
};

#endif
