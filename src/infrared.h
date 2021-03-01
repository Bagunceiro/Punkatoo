#pragma once

#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>
#include <map>
#include <vector>

#include "ptask.h"
#include "mqtt.h"

const uint8_t kTimeout = 15;
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
  const uint32_t IRREMOTE_1     = 0xffa25d;
  const uint32_t IRREMOTE_2     = 0xff629d;
  const uint32_t IRREMOTE_3     = 0xffe21d;
  const uint32_t IRREMOTE_4     = 0xff22dd;
  const uint32_t IRREMOTE_5     = 0xff02fd;
  const uint32_t IRREMOTE_6     = 0xffc23d;
  const uint32_t IRREMOTE_7     = 0xffe01f;
  const uint32_t IRREMOTE_8     = 0xffa857;
  const uint32_t IRREMOTE_9     = 0xff906f;
  const uint32_t IRREMOTE_0     = 0xff9867;
  const uint32_t IRREMOTE_STAR  = 0xff6897;
  const uint32_t IRREMOTE_HASH  = 0xffb04f;
  const uint32_t IRREMOTE_UP    = 0xff18e7;
  const uint32_t IRREMOTE_DOWN  = 0xff4ab5;
  const uint32_t IRREMOTE_LEFT  = 0xff10ef;
  const uint32_t IRREMOTE_RIGHT = 0xff5aa5;
  const uint32_t IRREMOTE_OK    = 0xff38c7;
*/

/*
   Fan controller remote
*/
const uint32_t IRREMOTE_FAN_ONOFF   = 0xff20df;
const uint32_t IRREMOTE_FAN_FASTER  = 0xffe01f;
const uint32_t IRREMOTE_FAN_SLOWER  = 0xff58a7;
const uint32_t IRREMOTE_FAN_REVERSE = 0xff10ef;
const uint32_t IRREMOTE_FAN_TIMER   = 0xffd827;

const uint32_t IRREMOTE_LIGHT_ONOFF = 0xff609f;
const uint32_t IRREMOTE_LIGHT_UP    = 0xff906f;
const uint32_t IRREMOTE_LIGHT_DOWN  = 0xff38c7;
const uint32_t IRREMOTE_LIGHT_TIMER = 0xffa05f;

// extern const int IRDEBOUNCE; // Number of milliseconds to leave fallow between IR messages

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


class IRController : public IRrecv, public PTask
{
public:
  IRController(const String& name);
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

  void sendCode(String type, long code, int bits = 32);
  void on();
  void off();

private:
  uint8_t lpin;
  IRsend* irsend;
};
