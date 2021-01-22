#ifndef FC_INFRARED_H
#define FC_INFRARED_H

#include <IRrecv.h>
#include <IRsend.h>
#include <IRremoteESP8266.h>
#include <IRutils.h>

const uint8_t kTimeout = 15;
const uint16_t kMinUnknownSize = 12;
const uint16_t kCaptureBufferSize = 1024;

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

const uint32_t IRREMOTE_CONFIGURATOR_START = IRREMOTE_FAN_FASTER;
const uint32_t IRREMOTE_CONFIGURATOR_STOP  = IRREMOTE_FAN_SLOWER;

extern const int IRDEBOUNCE; // Number of milliseconds to leave fallow between IR messages

/*
   LG TV remote
*/
// const uint32_t IRREMOTE_LG_ONOFF   = 0x20df10ef;

class IRControlled
{
    public:
    IRControlled();
    virtual ~IRControlled();
    virtual void irmsgRecd(uint32_t code);
    static void irmsgScanDevices(uint32_t code);
    private:
    IRControlled* next;
    static IRControlled* list;
};

extern unsigned long irDebounce(unsigned long then, unsigned long debounceTime = IRDEBOUNCE);
extern void pollIR();
extern IRrecv irrecv;

#endif
