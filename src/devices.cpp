#include <ArduinoJson.h>
#include <LITTLEFS.h>

#include "devices.h"
#include "config.h"

const char *KEY_ID = "id";
const char *KEY_PIN = "pin";
const char *KEY_SWITCH = "switch";
const char *KEY_PIN_R = "pinR";
const char *KEY_PIN_G = "pinG";
const char *KEY_PIN_B = "pinB";
const char *KEY_PIN_S1 = "pinSpd1";
const char *KEY_PIN_S2 = "pinSpd2";
const char *KEY_PIN_D1 = "pinDir1";
const char *KEY_PIN_D2 = "pinDir2";
const char *KEY_ADDR = "addr";
const char *KEY_SWITCHED = "switched";

/*
String conf = R"!(
{
    "irrcv":{
        "id":"irctlr",
        "pin":34
    },
    "irled":[{
        "id":"irled",
        "pin":4
    }],
    "indicator":[{
        "id":"indicator",
        "pinR":2,
        "pinG":12,
        "pinB":15
    }],
    "lamp":[{
        "id":"lamp",
        "pin":13
    }],
    "switch":[{
        "id":"sw1",
        "pin":19,
        "switched":[{"id":"lamp"}]
    }],
    "fan":[{
        "id":"fan",
        "pinSpd1":25,
        "pinSpd2":33,
        "pinDir1":26,
        "pinDir2":27
    }],
    "ldr":[{
        "id":"ldr",
        "pin":35
    }],
    "bme":[{
        "id":"bme",
        "addr":"0x76"
    }]
}
)!";
*/

void Devices::buildIRController(JsonObject obj)
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
    serr.printf("irctlr %s set pin to %d\n", id.c_str(), pin);

    irctlr = new IRController(id.c_str(), pin);
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
        serr.printf("irled %s on pin %d\n", id.c_str(), pin);
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
        serr.printf("indicator %s on pin %d, %d, %d\n", id.c_str(), pinRed, pinGreen, pinBlue);
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
        serr.printf("lamp %s on pin %d\n", id.c_str(), pin);
        Lamp *lamp = new Lamp(id, pin);
        lamps.push_back(*lamp);
    }
}

void Devices::buildSwitch(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int pin = 0;
        vector<String> swdevs;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_PIN)
                pin = kv.value().as<int>();
            else if (kv.key() == KEY_SWITCHED)
            {
                for (JsonObject swobj : kv.value().as<JsonArray>())
                {
                    for (JsonPair kvs : swobj)
                    {
                        if (kvs.key() == KEY_ID)
                        {
                            swdevs.push_back(kvs.value().as<String>());
                        }
                        else
                            serr.printf("  %s: %s\n", kvs.key().c_str(), kvs.value().as<String>().c_str());
                    }
                }
            }
            else
                serr.printf("  %s: %s", kv.key().c_str(), kv.value().as<String>().c_str());
        }
        Switch *sw = new Switch(id, pin);
        for (String s : swdevs)
        {
            for (Lamp &l : lamps)
            {
                if (l.getid() == s)
                {
                    sw->addDevice(l);
                    break;
                }
            }
        }
        switches.push_back(*sw);
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
        serr.printf("Fan %s on pins %d, %d, %d, %d\n", id.c_str(), pinD1, pinD2, pinS1, pinS2);
        Fan *fan = new Fan(id, pinD1, pinD2, pinS1, pinS2);
        fans.push_back(*fan);
    }
}

void Devices::buildLDR(JsonArray list)
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
        serr.printf("LDR %s on pin %d\n", id.c_str(), pin);
        LDR *ldr = new LDR(id, pin);
        ldrs.push_back(*ldr);
    }
}

void Devices::buildBME(JsonArray list)
{
    for (JsonObject obj : list)
    {
        String id;
        int addr = 0;

        for (JsonPair kv : obj)
        {
            if (kv.key() == KEY_ID)
                id = kv.value().as<String>();
            else if (kv.key() == KEY_ADDR)
            {
                String addrstr = kv.value().as<String>();
                sscanf(addrstr.c_str(), "%x", &addr);
            }
            else
                serr.printf("  %s: %s\n", kv.key().c_str(), kv.value().as<String>().c_str());
        }
        serr.printf("BME %s on address %x\n", id.c_str(), addr);

        BME *bme = new BME(id, addr);
        bmes.push_back(*bme);
    }
}

bool Devices::build(const char *fileName)
{
    bool result = true;

    File devfile = LITTLEFS.open(fileName, "r");
    if (devfile)
    {
        StaticJsonDocument<1024> doc;

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
                if (kvroot.key() == "irrcv")
                {
                    buildIRController(kvroot.value().as<JsonObject>());
                }
                else if (kvroot.key() == "irled")
                {
                    buildIRLed(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == "indicator")
                {
                    buildIndicator(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == "lamp")
                {
                    buildLamp(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == "switch")
                {
                    buildSwitch(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == "fan")
                {
                    buildFan(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == "ldr")
                {
                    buildLDR(kvroot.value().as<JsonArray>());
                }
                else if (kvroot.key() == "bme")
                {
                    buildBME(kvroot.value().as<JsonArray>());
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
    switchTask = new Switches(&switches);
    /*
    if (irctlr == NULL) // Emergency Dummy IR Controller to give things 
    {
        irctlr = new IRController("DummyIR", 34); 
    }
    */
    
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

    Wire.begin();

    switchTask->start(5);

    if (irctlr != NULL)
    {
        irctlr->registerMQTT(mqtt);
        irctlr->start(4);
    }
    else
    {
        serr.println("No IR Controller");
    }

    eventlogger.registerMQTT(mqtt);
    eventlogger.start(0);

    p2sys.registerIR(irctlr);
    p2sys.registerMQTT(mqtt);

    for (Fan &fan : fans)
    {
        fan.registerIR(irctlr);
        fan.registerMQTT(mqtt);
    }
    for (Lamp &lamp : lamps)
    {
        lamp.registerIR(irctlr);
        lamp.registerMQTT(mqtt);
        // lamp.start(5);
    }
    for (IRLed &irled : irleds)
    {
        irled.registerMQTT(mqtt);
    }
    for (BME &bme : bmes)
    {
        bme.registerMQTT(mqtt);
        if (!bme.start(0x76, &Wire))
        {
            serr.println("Could not find a valid BME280 sensor on dev 0x76");
        }
    }
    // configurator.registerIR(irctlr);
    webServer.init();
}