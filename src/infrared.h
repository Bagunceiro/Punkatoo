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

class IRClientDev;

typedef uint32_t IRCode;
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
/*
const IRCode IRREMOTE_FAN_ONOFF   = 0xff20df;
const IRCode IRREMOTE_FAN_FASTER  = 0xffe01f;
const IRCode IRREMOTE_FAN_SLOWER  = 0xff58a7;
const IRCode IRREMOTE_FAN_REVERSE = 0xff10ef;
const IRCode IRREMOTE_FAN_TIMER   = 0xffd827;
*/
/*
const IRCode IRREMOTE_LIGHT_ONOFF = 0xff609f;
const IRCode IRREMOTE_LIGHT_UP    = 0xff906f;
const IRCode IRREMOTE_LIGHT_DOWN  = 0xff38c7;
const IRCode IRREMOTE_LIGHT_TIMER = 0xffa05f;
*/

class IRController : public IRrecv, public P2Task, public MQTTClientDev
{
public:
  IRController(const char *name, int pin);
  ~IRController();
  void poll();
  virtual bool operator()();

private:
  const String dec(const IRCode c);
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
