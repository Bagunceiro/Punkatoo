#pragma once

#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <map>
#include <vector>

#include "p2task.h"
#include "mqtt.h"

const uint8_t  kTimeout = 15;
const uint16_t kMinUnknownSize = 12;
const uint16_t kCaptureBufferSize = 1024;

class IRControlled;

typedef uint32_t IRCode;
typedef String IRMessage;
typedef std::vector<IRControlled *> DevList;
typedef std::vector<IRMessage> MsgList;
typedef std::map<IRMessage, DevList> SubscriptionList;
typedef std::map<IRCode, MsgList> DecodeList;
/*
  Miniature test remote
*/
/*
  const IRCode IRREMOTE_1     = 0xffa25d;
  const IRCode IRREMOTE_2     = 0xff629d;
  const IRCode IRREMOTE_3     = 0xffe21d;
  const IRCode IRREMOTE_4     = 0xff22dd;
  const IRCode IRREMOTE_5     = 0xff02fd;
  const IRCode IRREMOTE_6     = 0xffc23d;
  const IRCode IRREMOTE_7     = 0xffe01f;
  const IRCode IRREMOTE_8     = 0xffa857;
  const IRCode IRREMOTE_9     = 0xff906f;
  const IRCode IRREMOTE_0     = 0xff9867;
  const IRCode IRREMOTE_STAR  = 0xff6897;
  const IRCode IRREMOTE_HASH  = 0xffb04f;
  const IRCode IRREMOTE_UP    = 0xff18e7;
  const IRCode IRREMOTE_DOWN  = 0xff4ab5;
  const IRCode IRREMOTE_LEFT  = 0xff10ef;
  const IRCode IRREMOTE_RIGHT = 0xff5aa5;
  const IRCode IRREMOTE_OK    = 0xff38c7;
*/

/*
   Fan controller remote
*/
const IRCode IRREMOTE_FAN_ONOFF   = 0xff20df;
const IRCode IRREMOTE_FAN_FASTER  = 0xffe01f;
const IRCode IRREMOTE_FAN_SLOWER  = 0xff58a7;
const IRCode IRREMOTE_FAN_REVERSE = 0xff10ef;
const IRCode IRREMOTE_FAN_TIMER   = 0xffd827;

const IRCode IRREMOTE_LIGHT_ONOFF = 0xff609f;
const IRCode IRREMOTE_LIGHT_UP    = 0xff906f;
const IRCode IRREMOTE_LIGHT_DOWN  = 0xff38c7;
const IRCode IRREMOTE_LIGHT_TIMER = 0xffa05f;

// Internal (decoded) messages

extern const IRMessage IR_LAMP_OFF;
extern const IRMessage IR_LAMP_ON;
extern const IRMessage IR_LAMP_TOGGLE;

extern const IRMessage IR_FAN_TOGGLE;
extern const IRMessage IR_FAN_REVERSE;
extern const IRMessage IR_FAN_FASTER;
extern const IRMessage IR_FAN_SLOWER;

extern const IRMessage IR_CONFIGURATOR_START;
extern const IRMessage IR_CONFIGURATOR_STOP;

extern const IRMessage IR_SEND_IRCODE;

extern const IRMessage IR_RESET;


class IRController : public IRrecv, public P2Task
{
public:
  IRController(const String& name = "irctlr");
  ~IRController();
  void poll();
  void newpoll();
  bool subscribe(IRControlled *, IRMessage);
  virtual bool operator()();

private:
  const String dec(const IRCode c);

  SubscriptionList subList;
  static DecodeList decList;
};

class IRControlled
{
public:
  IRControlled(const String& n);
  virtual ~IRControlled();
  /*
   Take action on receipt of an IR message. This function should be implemented by the derived class.
  */
  virtual void irmsgRecd(const IRMessage msg);
  /*
   Register with the IR controller.
   Also subscribes to messages of interest (via the subscribeToIR function for the derived class)
  */
  void registerIR(IRController &c);
  /*
    Called by registerIR to allow derived class to subscribe to messages of interest
  */
  virtual void subscribeToIR() = 0;
  String getName() { return name; }

protected:
  bool subscribe(IRMessage m);

private:
  IRController *ctlr;
  String name;
};

class IRLed : public MQTTClientDev
{
public:
  IRLed(const String& name, uint8_t pin);
  virtual ~IRLed();
  virtual void subscribeToMQTT();
  virtual void mqttMsgRecd(const String &topic, const String &msg);

  void txCode(String type, long code, int bits = 32);
  void on();
  void off();

private:
  uint8_t lpin;
  IRsend* irsend;
};
