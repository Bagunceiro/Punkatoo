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

class Lamp: public MqttControlled, public IRControlled, public PTask
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
    virtual void mqttaction(const String& topic, const String& msg);
    virtual void doSubscriptions(PubSubClient& mqttclient);
    // virtual void irmsgRecd(IRCode code);
    virtual void irmsgRecd(IRMessage msg);

  private:
    int lpin; // goes to the control relay (active low)
    virtual void subscribeToIR();

    SwBlkList swList;
};

#endif
