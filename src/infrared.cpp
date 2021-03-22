#include <Arduino.h>
// #include <IRremote.h>
#include <ArduinoJson.h>

#include "config.h"
#include "infrared.h"
#include "eventlog.h"

DecodeList test =
    {
        {0x323323, {(const IRMessage) "v1"}},
        {0x323323, {(const IRMessage) "v2", (const IRMessage) "v3"}}};

const IRMessage IR_LAMP_OFF = (const IRMessage) "LMP_OFF";
const IRMessage IR_LAMP_ON = (const IRMessage) "LMP_ON";
const IRMessage IR_LAMP_TOGGLE = (const IRMessage) "LMP_TOG";

const IRMessage IR_FAN_TOGGLE = (const IRMessage) "FAN_TOG";
const IRMessage IR_FAN_REVERSE = (const IRMessage) "FAN_REV";
const IRMessage IR_FAN_FASTER = (const IRMessage) "FAN_FST";
const IRMessage IR_FAN_SLOWER = (const IRMessage) "FAN_SLW";

const IRMessage IR_CONFIGURATOR_START = (const IRMessage) "CONF_START";
const IRMessage IR_CONFIGURATOR_STOP = (const IRMessage) "CONF_STOP";

const IRMessage IR_RESET = (const IRMessage) "RESET";

DecodeList IRController::decList =
    {
        {IRREMOTE_LIGHT_ONOFF, {IR_LAMP_TOGGLE}},
        {IRREMOTE_FAN_ONOFF, {IR_FAN_TOGGLE}},
        {IRREMOTE_FAN_REVERSE, {IR_FAN_REVERSE}},
        {IRREMOTE_FAN_FASTER, {IR_FAN_FASTER, IR_CONFIGURATOR_START}},
        {IRREMOTE_FAN_SLOWER, {IR_FAN_SLOWER, IR_CONFIGURATOR_STOP}},
        {IRREMOTE_LIGHT_UP, {IR_RESET}},
};

const int IRDEBOUNCE = 200; // Number of milliseconds to leave fallow between IR messages

// IRControlled *IRControlled::list = NULL;

IRControlled::IRControlled(const String &n)
{
  name = n;
}

IRControlled::~IRControlled()
{
}

void IRControlled::irmsgRecd(const IRMessage msg)
{
}

void IRControlled::registerIR(IRController *c)
{
  ctlr = c;
  subscribeToIR();
}

bool IRControlled::subscribe(IRMessage m)
{
  if (ctlr != NULL)
  {
    return ctlr->subscribe(this, m);
  }
  return false;
}

unsigned long irDebounce(unsigned long then, unsigned long debounceTime)
{
  unsigned long when = millis();
  if (when - then > debounceTime)
    return when;
  else
    return 0;
}

IRController::IRController(const String &name, int pin)
    : IRrecv(pin, kCaptureBufferSize, kTimeout, true),
      P2Task(name, 2500),
      MQTTClientDev(name)
{
  pinMode(pin, INPUT_PULLUP);
  enableIRIn();
}

IRController::~IRController()
{
}

bool IRController::operator()()
{
  decode_results IRDecodeResults;
  if (decode(&IRDecodeResults))
  {
    static unsigned long then = 0;
    unsigned long when;
    uint64_t val = IRDecodeResults.value;
    if (val != ~0)
    {
      Event e;

      if ((when = irDebounce(then, IRDEBOUNCE)))
      {
        // Inform anyone who's interested
        String payload = R"--({"source":")--" 
        + persistant[persistant.controllername_n] 
        + R"--(","code":")--"  + uint64ToString(val, HEX) + R"--("})--";
        mqttPublish(MQTT_TPC_RECDIRCODE, payload);

        // Decode data from remote...
        auto search = decList.find(val);
        // ...and if we found a decode
        if (search != decList.end())
        {
          // for everything that code could mean
          for (IRMessage msg : search->second)
          {
            // serr.println(String(" = ") + msg);
            // Find subscriptions for it
            auto search2 = subList.find(msg);
            if (search2 != subList.end())
            {
              // For each subscription for this message
              for (IRControlled *dev : search2->second)
              {
                Event e;
                e.enqueue("IR(" + msg + ") -> " + dev->getName());
                // pass to the subscribing device
                dev->irmsgRecd(msg);
              }
            }
            else // No devices subscribed to this message
            {
              Event e;
              e.enqueue("IR(" + msg + ") no subs");
            }
          }
        }
        else // Message is not in our decode table
        {
          Event e;
          e.enqueue("Unk IRMsg " + uint64ToString(val, HEX));
          serr.println("Unknown IR: " + uint64ToString(val, HEX));
        }

        then = when;
      }
    }
    resume();
  }
  delay(20); // should this be yield?
  return true;
}

bool IRController::subscribe(IRControlled *c, IRMessage m)
{
  auto record = subList.find(m); // Any subscriptions to this message yet?
  if (record == subList.end())   // No, create a new bucket in the subscription list
  {
    DevList newVector;
    newVector.push_back(c);
    subList.insert({m, newVector});
  }
  else                           // Yes, add this sub to the existing bucket
  {
    record->second.push_back(c);
  }
  return true;
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

    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, msg.c_str());

    if (error)
    {
      serr.printf("Config deserialization error (%d)\n", error.code());
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
      }
    }
    if ((type != "") && (code != ""))
    {
      txCode(type, strtoll(code.c_str(), 0, 16), bits);
    }
  }
}
