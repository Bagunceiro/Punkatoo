#ifndef FC_LAMP_H
#define FC_LAMP_H

#include <vector>

#include "mqtt.h"
#include "infrared.h"
#include "switch.h"

class Lamp : public MQTTClientDev, public SwitchedDev
{
public:
  Lamp(const char *devName, const int relayPin);

  typedef void (*LampActionFunc)(Lamp *l, const uint8_t toOn, void *data);

  const int getStatus() const;
  void sw(int toState);
  int toggle();

  const int blip(const int t = 500);
  void blip(const int number, const int length);

  virtual ~Lamp();
  virtual String mqttGetStatus() override;
  virtual void mqttMsgRecd(const String &topic, const String &msg) override;

  virtual int doSwitch(const char *parm, const bool more, const int extra);
  void onAction(LampActionFunc func, void *data)
  {
    LampAction act;
    act._cb = func;
    act._data = data;
    callBacks.push_back(act);
  }

private:
  int lpin; // goes to the control relay (active low)
  virtual void subscribeToMQTT() override;
  struct LampAction
  {
    LampActionFunc _cb;
    void *_data;
  };
  std::vector<LampAction> callBacks;
};

#endif
