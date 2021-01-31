#include <ArduinoJson.h>
#include <WiFiSerial.h>

#include "config.h"
#include "updater.h"

Updater::Updater(const String &devName) : MqttControlled(devName)
{
  startCallback = NULL;
  endCallback = NULL;
}

Updater::~Updater()
{
}

void Updater::mqttaction(const String &topic, const String &msg)
{
  if (topic == MQTT_TPC_UPDATE)
  {
    StaticJsonDocument<512> doc;

    DeserializationError error = deserializeJson(doc, msg);
    if (error)
    {
      WSerial.printf("Config deserialization error (%d)\n", error.code());
    }
    else
    {
      JsonObject root = doc.as<JsonObject>();
      String server;
      int port = 80;
      String image;
      String ver;
      for (JsonPair kv : root)
      {
        if (kv.key() == "server")
          server = (const char *)kv.value();
        else if (kv.key() == "port")
          port = kv.value();
        else if (kv.key() == "image")
          image = (const char *)kv.value();
        else if (kv.key() == "version")
          ver = (const char *)kv.value();
      }
      Serial.printf("systemUpdate(%s, %d, %s, %s)\n", server.c_str(), port, image.c_str(), ver.c_str());
      if (systemUpdate(server, port, image, ver) == HTTP_UPDATE_OK)
      {
        ESP.restart();
      }
    }
  }
}

void Updater::doSubscriptions(PubSubClient &mqttClient)
{
  // Serial.printf("Subscribing to %s\n", (getPrefix() + MQTT_TPC_UPDATE).c_str());
  mqttClient.subscribe((getPrefix() + MQTT_TPC_UPDATE).c_str());
  sendStatus();
}

String Updater::getStatus()
{
  return "";
}

t_httpUpdate_return Updater::systemUpdate(const String &server, const uint16_t port, const String &image, const String& ver)
{
  // ToDo: Check out the version (ver) to see we dont already have it

  WiFiClient client;

  if (startCallback != NULL)
  {
    startCallback();
  }

Serial.println("In systemUpdate");

  httpUpdate.rebootOnUpdate(false);

  t_httpUpdate_return ret = httpUpdate.update(client, server, port, image);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    break;

  case HTTP_UPDATE_NO_UPDATES:
    Serial.println("HTTP_UPDATE_NO_UPDATES");
    break;

  case HTTP_UPDATE_OK:
    Serial.println("HTTP_UPDATE_OK");
    if (endCallback != NULL) endCallback();
    break;
  }
  return ret;
}

void Updater::onStart(void(*callback)())
{
  startCallback = callback;
}

void Updater::onEnd(void(*callback)())
{
  endCallback = callback;
}