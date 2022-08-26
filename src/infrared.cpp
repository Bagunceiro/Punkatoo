#include <Arduino.h>
#include <ArduinoJson.h>

#include "config.h"
#include "infrared.h"
#include "eventlog.h"

const int IRDEBOUNCE = 200; // Number of milliseconds to leave fallow between IR messages

unsigned long irDebounce(unsigned long then, unsigned long debounceTime)
{
  unsigned long when = millis();
  if (when - then > debounceTime)
    return when;
  else
    return 0;
}

IRController::IRController() : P2Task("irctlr", 2500), MQTTClientDev("irctlr")
{
}

IRController::~IRController()
{
  delete _receiver;
}

void IRController::setPin(int pin)
{
  pinMode(pin, INPUT_PULLUP);
  if (_receiver != NULL)
    delete _receiver;
  _receiver = new IRrecv(pin, kCaptureBufferSize, kTimeout, true);
  _receiver->enableIRIn();
}

bool IRController::operator()()
{
  if (_receiver != NULL)
  {
    decode_results IRDecodeResults;
    if (_receiver->decode(&IRDecodeResults))
    {
      static unsigned long then = 0;
      unsigned long when;
      uint64_t val = IRDecodeResults.value;
      if (val != ~0)
      {
        Event e;

        if ((when = irDebounce(then, IRDEBOUNCE)))
        {
          Serial.printf("IRMsg %s\n", uint64ToString(val, HEX).c_str());
          // Inform anyone who's interested
          String payload = R"--({"source":")--" + config[controllername_n] + R"--(","code":")--" + uint64ToString(val, HEX) + R"--("})--";
          mqttPublish(MQTT_TPC_RECDIRCODE, payload);
          Event e;
          e.enqueue("IRMsg " + uint64ToString(val, HEX));
          dev.switchTask.irMessage(val);
          then = when;
        }
      }
      _receiver->resume();
    }
    delay(20); // should this be yield?
    return true;
  }
  return false;
}

IRLed::IRLed(const String &name, uint8_t pin) : MQTTClientDev(name)
{
  lpin = pin;
  pinMode(lpin, OUTPUT);
  irsend = new IRsend(lpin);
  off();
}

IRLed::~IRLed()
{
  off();
}

void IRLed::on()
{
  digitalWrite(lpin, HIGH);
}

void IRLed::off()
{
  digitalWrite(lpin, LOW);
}

void IRLed::subscribeToMQTT()
{
  pmqttctlr->subscribe(this, MQTT_TPC_SENDIRCODE);
  mqttSendStatus();
}

void IRLed::txCode(String type, long code, int bits)
{
  Event e;
  char buffer[32];
  snprintf(buffer, sizeof(buffer) - 1, "IR TX: %s, %lx, %d\n", type.c_str(), code, bits);

  e.enqueue(buffer);
  if (type == "NEC")
  {
    irsend->sendNEC(code);
  }
}

void IRLed::mqttMsgRecd(const String &topic, const String &msg)
{
  if (topic == MQTT_TPC_SENDIRCODE)
  {
    String type;
    String code;
    int bits = 32;
    int rpt = 1;

    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, msg.c_str());

    if (error)
    {
      Serial.printf("Config deserialization error (%d)\n", error.code());
    }
    else
    {
      JsonObject root = doc.as<JsonObject>();
      for (JsonPair kv : root)
      {
        if (kv.key() == "type")
        {
          type = (const char *)kv.value();
        }
        else if (kv.key() == "code")
        {
          code = (const char *)kv.value();
        }
        else if (kv.key() == "bits")
        {
          bits = (const int)kv.value();
        }
        else if (kv.key() == "repeat")
        {
          rpt = (const int)kv.value();
        }
      }
    }
    if ((type != "") && (code != ""))
    {
      for (int i = 0; i < rpt; i++)
      {
        txCode(type, strtoll(code.c_str(), 0, 16), bits);
      }
    }
  }
}
