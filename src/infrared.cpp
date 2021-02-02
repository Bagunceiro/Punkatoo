#include <Arduino.h>
#include "config.h"
#include "infrared.h"

const int IRDEBOUNCE = 200; // Number of milliseconds to leave fallow between IR messages

IRControlled* IRControlled::list = NULL;
IRController irctlr;

IRControlled::IRControlled()
{
  next = NULL;

  IRControlled** ptr = &list;
  while (*ptr != NULL)
  {
    ptr = &((*ptr)->next);
  }
  *ptr = this;
}

IRControlled::~IRControlled()
{
  IRControlled** ptr = &list;
  while (*ptr != NULL)
  {
    if (*ptr == this)
    {
      (*ptr = next);
      break;
    }
    ptr = &((*ptr)->next);
  }
}

void IRControlled::irmsgRecd(uint32_t code)
{

}

void IRControlled::irmsgScanDevices(uint32_t code)
{
  IRControlled* ptr = list;
  while (ptr != NULL)
  {
    ptr->irmsgRecd(code);
    ptr = ptr->next;
  }
}

unsigned long irDebounce(unsigned long then, unsigned long debounceTime)
{
  unsigned long when = millis();
  if (when - then > debounceTime) return when;
  else return 0;
}

// IRrecv irrecv(IR_DETECTOR_PIN, kCaptureBufferSize, kTimeout, true);

IRController::IRController() : IRrecv(IR_DETECTOR_PIN, kCaptureBufferSize, kTimeout, true)
{
  pinMode(IR_DETECTOR_PIN, INPUT_PULLUP);
  enableIRIn();
}

IRController::~IRController()
{

}


void IRController::poll()
{
  decode_results IRDecodeResults;
  if (decode(&IRDecodeResults)) {
    static unsigned long then = 0;
    unsigned long  when;
    uint64_t val = IRDecodeResults.value;
    serialPrintUint64(val, HEX);
    Serial.println("");

    if ((when = irDebounce(then)))
    {
      IRControlled::irmsgScanDevices(val);
      then = when;
    }
    resume();
  }
}

/*
void pollIR()
{
  decode_results IRDecodeResults;
  if (irctlr.decode(&IRDecodeResults)) {
    static unsigned long then = 0;
    unsigned long  when;
    uint64_t val = IRDecodeResults.value;
    serialPrintUint64(val, HEX);
    Serial.println("");

    if ((when = irDebounce(then)))
    {
      IRControlled::irmsgScanDevices(val);
      then = when;

    }
    irctlr.resume();
  }
}
*/

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
