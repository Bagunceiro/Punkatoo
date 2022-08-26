#include <ArduinoJson.h>
#include <LittleFS.h>

#include "devices.h"
#include "config.h"

const char *KEY_ID = "id";
const char *KEY_PIN = "pin";
const char *KEY_IR = "ir";
const char *KEY_PARM = "parm";
const char *KEY_FAN = "fan";
const char *KEY_IRRCV = "irrcv";
const char *KEY_IRLED = "irled";
const char *KEY_SWITCH = "switch";
const char *KEY_INDICATOR = "indicator";
const char *KEY_PIN_R = "pinR";
const char *KEY_PIN_G = "pinG";
const char *KEY_PIN_B = "pinB";
const char *KEY_PIR = "pir";
const char *KEY_PIN_S1 = "pinSpd1";
const char *KEY_PIN_S2 = "pinSpd2";
const char *KEY_PIN_D1 = "pinDir1";
const char *KEY_PIN_D2 = "pinDir2";
const char *KEY_ADDR = "addr";
const char *KEY_INTERVAL = "interval";
const char *KEY_SWITCHED = "switched";
const char *KEY_KILL = "kill";
const char *KEY_TIMEOUT = "timeout";
const char *KEY_BME = "bme";
const char *KEY_LAMP = "lamp";

void Devices::buildIRController(JsonObject obj)
{
    String id;
    for (JsonPair kv : obj)
    {
        if (kv.key() == KEY_ID)
            id = kv.value().as<String>();
        else if (kv.key() == KEY_PIN)
        {
            int pin = kv.value().as<int>();
            irctlr.setPin(pin);
        }
        else
            serr.printf("  %s: %s\n", kv.key().c_str(), kv.value().as<String>().c_str());
    }
}

void Devices::buildIRLed(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pin = 0;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_PIN)
                pin = kv.value().as<int>();
            else
                serr.printf("  %s: %s\n", kv.key().c_str(), kv.value().as<String>().c_str());
        }
        IRLed *irled = new IRLed(id, pin);
        irleds.push_back(*irled);
    }
}

void Devices::buildIndicator(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pinRed = 0;
        int pinGreen = 0;
        int pinBlue = 0;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_PIN_R)
                pinRed = kv.value().as<int>();
            else if (kv.key() == KEY_PIN_G)
                pinGreen = kv.value().as<int>();
            else if (kv.key() == KEY_PIN_B)
                pinBlue = kv.value().as<int>();
            else
                serr.printf("  %s: %s\n", kv.key().c_str(), kv.value().as<String>().c_str());
        }
        IndicatorLed *indicator = new IndicatorLed(id, pinRed, pinGreen, pinBlue);
        indicators.push_back(*indicator);
    }
}

void Devices::buildLamp(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pin = 0;
        for (JsonPair kvl : obj)
        {
            if (kvl.key() == KEY_ID)
                id = kvl.value().as<String>();
            else if (kvl.key() == KEY_PIN)
                pin = kvl.value().as<int>();
            else
                serr.printf("  %s: %s\n", kvl.key().c_str(), kvl.value().as<String>().c_str());
        }
        // serr.printf("lamp %s on pin %d\n", id.c_str(), pin);
        Lamp *lamp = new Lamp(id.c_str(), pin);
        lamps.push_back(*lamp);
    }
}

struct internalIRMsg
{
    String dev;
    String parm;
};

void Devices::buildSwitch(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pin = -1;
        String ircode;
        vector<internalIRMsg> swdevs;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_PIN)
                pin = kv.value().as<int>();
            else if (kv.key() == KEY_IR)
                ircode = kv.value().as<String>();
            else if (kv.key() == KEY_SWITCHED)
            {
                for (JsonObject swobj : kv.value().as<JsonArray>())
                {
                    internalIRMsg iirm;
                    for (JsonPair kvs : swobj)
                    {

                        if (kvs.key() == KEY_ID)
                        {
                            iirm.dev = kvs.value().as<String>();
                        }
                        else if (kvs.key() == KEY_PARM)
                        {
                            iirm.parm = kvs.value().as<String>();
                        }
                        else
                            serr.printf("  Unk %s: %s\n", kvs.key().c_str(), kvs.value().as<String>().c_str());
                    }
                    swdevs.push_back(iirm);
                }
            }
            else
                serr.printf("  %s: %s", kv.key().c_str(), kv.value().as<String>().c_str());
        }
        Switch *sw;
        if (pin >= 0)
        {
            sw = new PhysSwitch(id.c_str(), pin);
        }
        else if (ircode.length() > 0)
        {
            sw = new IRSwitch(id.c_str(), ircode.c_str());
        }
        else
        {
            serr.println("Unknown switch type");
            continue;
        }
        for (internalIRMsg &s : swdevs)
        {
            for (Lamp &l : lamps)
            {
                if ((String)l.getid() == s.dev)
                {
                    sw->addDevice(l);
                    sw->addParm(s.parm.c_str());
                    break;
                }
            }
            for (Fan &f : fans)
            {
                if ((String)f.getid() == s.dev)
                {
                    sw->addDevice(f);
                    sw->addParm(s.parm.c_str());
                    break;
                }
            }
        }
        switchTask.addSwitch(sw);
    }
}

void Devices::buildFan(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pinS1 = 0;
        int pinS2 = 0;
        int pinD1 = 0;
        int pinD2 = 0;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_PIN_S1)
                pinS1 = kv.value().as<int>();
            else if (kv.key() == KEY_PIN_S2)
                pinS2 = kv.value().as<int>();
            else if (kv.key() == KEY_PIN_D1)
                pinD1 = kv.value().as<int>();
            else if (kv.key() == KEY_PIN_D2)
                pinD2 = kv.value().as<int>();
            else
                serr.printf("  %s: %s\n", kv.key().c_str(), kv.value().as<String>().c_str());
        }
        // serr.printf("Fan %s on pins %d, %d, %d, %d\n", id.c_str(), pinD1, pinD2, pinS1, pinS2);
        Fan *fan = new Fan(id.c_str(), pinD1, pinD2, pinS1, pinS2);
        fans.push_back(*fan);
    }
}

void Devices::buildPIR(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pin = 0;
        int timeout = 0;
        vector<String> lampids;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_PIN)
            {
                pin = kv.value().as<int>();
            }
            else if (kv.key() == KEY_TIMEOUT)
            {
                timeout = kv.value().as<int>();
            }
            else if (kv.key() == KEY_KILL)
            {
                for (JsonObject swobj : kv.value().as<JsonArray>())
                {
                    for (JsonPair kvs : swobj)
                    {
                        if (kvs.key() == KEY_ID)
                        {
                            String lid = kvs.value().as<String>();
                            lampids.push_back(lid);
                        }
                        else
                            serr.printf("  Unk %s: %s\n", kvs.key().c_str(), kvs.value().as<String>().c_str());
                    }
                }
            }
            else
                serr.printf("  %s: %s\n", kv.key().c_str(), kv.value().as<String>().c_str());
        }

        PIR newpir(id.c_str(), pin);
        pirs.push_back(newpir);
        PIR &pir = pirs.back();

        for (String lid : lampids)
        {
            for (Lamp &l : lamps)
            {
                if (lid == l.getid())
                {
                    pir.addLamp(l);
                }
            }
        }
        pir.setTimeoutSecs(timeout);
        // pir.setTimeoutSecs(60);
    }
}

bool Devices::build(const char *fileName)
{
    bool result = true;

    File devfile = LittleFS.open(fileName, "r");
    if (devfile)
    {
        StaticJsonDocument<2000> doc;

        DeserializationError error = deserializeJson(doc, devfile);
        if (error)
        {
            serr.printf("Device file deserialization error (%d)\n", error.code());
        }
        else
        {
            JsonObject root = doc.as<JsonObject>();

            for (JsonPair kvroot : root)
            {
                Serial.printf("Build - %s\n", kvroot.key().c_str());
                if (kvroot.key() == KEY_IRRCV)
                {
                    buildIRController(kvroot.value().as<JsonObject>());
                }
                else if (kvroot.key() == KEY_IRLED)
                {
                    buildIRLed(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == KEY_INDICATOR)
                {
                    buildIndicator(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == KEY_LAMP)
                {
                    buildLamp(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == KEY_SWITCH)
                {
                    buildSwitch(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == KEY_FAN)
                {
                    buildFan(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == KEY_PIR)
                {
                    buildPIR(kvroot.value().as<JsonArray>());
                }
            }
        }
        devfile.close();
    }
    else
    {
        perror("");
        serr.println("Device file open for read failed");
    }

    return result;
}

void Devices::toSecure()
{
    for (Lamp &lamp : lamps)
    {
        lamp.sw(0);
    }
    for (Fan &fan : fans)
    {
        fan.setSpeed(0);
    }
    for (IRLed &led : irleds)
    {
        led.off();
    }
}

void Devices::start()
{
    toSecure();

    switchTask.start(5);

    if (irctlr)
    {
        irctlr.registerMQTT(mqtt);
        irctlr.start(4);
    }
    else
    {
        serr.println("No IR Controller");
    }

    eventlogger.registerMQTT(mqtt);
    eventlogger.start(0);

    for (Fan &fan : fans)
    {
        fan.registerMQTT(mqtt);
    }
    for (Lamp &lamp : lamps)
    {
        lamp.registerMQTT(mqtt);
    }
    for (IRLed &irled : irleds)
    {
        irled.registerMQTT(mqtt);
    }
    webServer.init();
}

void Devices::poll()
{
    for (PIR &pir : pirs)
    {
        pir.routine();
    }
}

// A lamp calls this to indicate change of state - used to trigger relevant PIR(s)
void Devices::lampState(const Lamp *l, const int val)
{
    for (PIR &pir : pirs)
    {
        pir.lampState(l, val);
    }
}