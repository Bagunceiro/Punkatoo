#include <Arduino.h>
#include "config.h"
#include "infrared.h"

DecodeList test =
    {
        {0x323323, {(const IRMessage) "v1"}},
        {0x323323, {(const IRMessage) "v2", (const IRMessage) "v3"}}};

const IRMessage IR_LAMP_OFF = (const IRMessage) "LAMP_OFF";
const IRMessage IR_LAMP_ON = (const IRMessage) "LAMP_ON";
const IRMessage IR_LAMP_TOGGLE = (const IRMessage) "LAMP_TOGGLE";

const IRMessage IR_FAN_TOGGLE = (const IRMessage) "FAN_TOGGLE";
const IRMessage IR_FAN_REVERSE = (const IRMessage) "FAN_REV";
const IRMessage IR_FAN_FASTER = (const IRMessage) "FAN_FASTER";
const IRMessage IR_FAN_SLOWER = (const IRMessage) "FAN_SLOWER";

DecodeList IRController::decList =
    {
        {IRREMOTE_LIGHT_ONOFF, {IR_LAMP_TOGGLE}},
        {IRREMOTE_FAN_ONOFF, {IR_FAN_TOGGLE}},
        {IRREMOTE_FAN_REVERSE, {IR_FAN_REVERSE}},
        {IRREMOTE_FAN_FASTER, {IR_FAN_FASTER}},
        {IRREMOTE_FAN_SLOWER, {IR_FAN_SLOWER}}};

const int IRDEBOUNCE = 200; // Number of milliseconds to leave fallow between IR messages

// IRControlled *IRControlled::list = NULL;

IRControlled::IRControlled()
{
}

IRControlled::~IRControlled()
{
}

/*
void IRControlled::irmsgRecd(const IRCode code)
{
}
*/

void IRControlled::irmsgRecd(const IRMessage msg)
{
}

void IRControlled::registerIR(IRController &c)
{
  ctlr = &c;
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

// IRrecv irrecv(IR_DETECTOR_PIN, kCaptureBufferSize, kTimeout, true);

IRController::IRController() : IRrecv(IR_DETECTOR_PIN, kCaptureBufferSize, kTimeout, true), PTask("IRCtlr", 2500)
{
  pinMode(IR_DETECTOR_PIN, INPUT_PULLUP);
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
    serr.println(uint64ToString(val, HEX));
    // serialPrintUint64(val, HEX);
    // Serial.println("");

    if ((when = irDebounce(then)))
    {
      // Decode data from remote
      auto search = decList.find(val);
      if (search != decList.end())
      {
        // for everything that code could mean
        for (IRMessage msg : search->second)
        {
          // Find subscriptions for it
          auto search2 = subList.find(msg);
          if (search2 != subList.end())
          {
            // For each subscription for this message
            for (IRControlled *dev : search2->second)
            {
              // send it to the subscribing device
              dev->irmsgRecd(msg);
            }
          }
        }
      }
      then = when;
    }
    resume();
  }
  return true;
}

bool IRController::subscribe(IRControlled *c, IRMessage m)
{
  auto record = subList.find(m);
  if (record == subList.end())
  {
    DevList newVector;
    newVector.push_back(c);
    subList.insert({m, newVector});
  }
  else
  {
    record->second.push_back(c);
  }
  return true;
}

IRLed::IRLed(uint8_t pin)
{
  lpin = pin;
  pinMode(lpin, OUTPUT);
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
