#include "confblk.h"
#include <ArduinoJson.h>
#include <LittleFS.h>

#include <wifiserial.h>

#include "crypt.h"

// ConfBlk::ConfBlk(const String& fileName)
ConfBlk::ConfBlk(const char *fileName)
{
    Serial.printf("ConfBlk::ConfBlk(%s)\n", fileName);
    _fileName = fileName;
}

void ConfBlk::dump(Stream &s) const
{
    for (auto iterator : *this)
    {
        s.printf("%s  = %s\n", iterator.first.c_str(), iterator.second.c_str());
    }
}

bool ConfBlk::writeFile() const
{
    bool result = false;

    File configFile = LittleFS.open(_fileName, "w");
    if (!configFile)
    {
        perror("");
        serr.println("Config file open for write failed");
        result = false;
    }
    else
    {
        StaticJsonDocument<512> doc;
        for (auto iterator : *this)
        {
            String k = "e."+ iterator.first;
            String v = pencrypt64(iterator.second.c_str());
            Serial.printf("write %s=%s\n", k.c_str(),v.c_str());
            // doc[iterator.first] = iterator.second;
            doc[k] = v;
        }
        serializeJson(doc, configFile);
        configFile.close();
        result = true;
    }

    return result;
}

bool ConfBlk::readFile()
{
    Serial.printf("Confblk::readFile %s\n", _fileName.c_str());
    bool result = false;
    bool unencoded = false; // indicates that we have an in the clear parameter and need to encode it

    File configFile = LittleFS.open(_fileName, "r");
    if (!configFile)
    {
        serr.println("Config file open for read failed");
    }
    else
    {
        StaticJsonDocument<512> doc;
        Serial.printf("deserializing\n");

        DeserializationError error = deserializeJson(doc, configFile);
        if (error)
        {
            serr.printf("Config deserialization error (%d)\n", error.code());
            result = false;
        }
        else
        {
            JsonObject root = doc.as<JsonObject>();
            for (JsonPair kv : root)
            {
                String key(kv.key().c_str());
                String val = kv.value();
                // (*this)[kv.key().c_str()] = (const char*)kv.value();
                if (key.startsWith("e."))
                {
                    key = key.substring(2);
                    val = pdecrypt64(val.c_str());
                }
                else
                {
                    unencoded = true;
                }
                Serial.printf("Conf: %s=%s\n", key.c_str(), val.c_str());
                (*this)[key.c_str()] = val.c_str();
            }
        }
        configFile.close();
        if (unencoded)
        {
            Serial.println("Rewriting file");
            writeFile();
        }
        result = true;
    }
    return result;
}