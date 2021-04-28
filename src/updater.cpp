#include <ArduinoJson.h>
#include <LITTLEFS.h>

#include "config.h"
#include "devices.h"
#include "p2system.h"

extern IndicatorLed::Colour indicateUpdate;
Updater *Updater::pThis = NULL;

void updateStarted(void *)
{
  Event e;
  e.enqueue("Update started");
  dev.p2sys.enterState(P2System::STATE_UPDATE);
  dev.toSecure();
}

void updateCompleted(void *)
{
  Event e;
  e.enqueue("Update complete");
  dev.p2sys.revertState();

  /*
  time_t now = timeClient.getEpochTime();
  config[updateTime_n] = String(now);
  config.writeFile();
*/
}

/*
void updateNone(void *)
{
  Event e;
  e.enqueue("No update available");
  dev.p2sys.revertState();
}

void updateFail(void *)
{
  Event e;
  e.enqueue("Update failed");
  dev.p2sys.revertState();
}
*/

Updater::Updater(const String &devName) : P2Task(devName, 8000)
{
  startCallback = NULL;
  endCallback = NULL;
  nullCallback = NULL;
  failCallback = NULL;
  progCallback = NULL;
  uType = UPD_NONE;
  startcbdata = NULL;
  endcbdata = NULL;
  nullcbdata = NULL;
  failcbdata = NULL;
  pThis = this;
  messageBuffer[0] = '\0';
}

Updater::~Updater()
{
}

void Updater::doprogcb(size_t completed, size_t total)
{
  if (progCallback != NULL)
  {
    progCallback(completed, total, progcbdata);
  }
}

/*
void endcb(void *)
{
  dev.webServer.event("progress", "Complete. Reseting, please wait");
  delay(1000);
  ESP.restart();
}
*/

/*
void Updater::systemUpdate(const String &s, const uint16_t p, const String &i, updateType t)
{
  setRemote(s, p, i, t);
  ();
}
*/
bool Updater::operator()()
{
  switch (uType)
  {
  case UPD_SYS:
    systemUpdate();
    break;
  case UPD_CONF:
    configUpdate();
    break;
  default:
    break;
  }
  uType = UPD_NONE;
  return true;
}

void Updater::systemUpdate()
{
  WiFiClient client;

  if (startCallback != NULL)
  {
    startCallback("", startcbdata);
  }

  httpUpdate.rebootOnUpdate(false);

  Update.onProgress(progcb);

  t_httpUpdate_return ret = httpUpdate.update(client, server, port, source);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    snprintf(messageBuffer, sizeof(messageBuffer)-1, "HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    if (failCallback != NULL)
      failCallback(messageBuffer, failcbdata);
    break;

  case HTTP_UPDATE_NO_UPDATES:
    serr.println("HTTP_UPDATE_NO_UPDATES");
    if (nullCallback != NULL)
      nullCallback("No Updates Available", nullcbdata);
    break;

  case HTTP_UPDATE_OK:
    serr.println("HTTP_UPDATE_OK");
    if (endCallback != NULL)
      endCallback("OK", endcbdata);
    break;
  }
}

void Updater::configUpdate()
{
  HTTPClient http;
  http.begin(server, port, source);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      File f = LITTLEFS.open(target, "w+");
      if (f)
      {
        Stream &s = http.getStream();
        uint8_t buffer[32];
        size_t l;
        while ((l = s.readBytes(buffer, sizeof(buffer) - 1)))
        {
          f.write(buffer, l);
        }
        f.close();
        snprintf(messageBuffer, sizeof(messageBuffer) -1, "File uploaded");
        if (endCallback != NULL) endCallback(messageBuffer, endcbdata);
      }
      else
        snprintf(messageBuffer, sizeof(messageBuffer) -1, "Could not open file");
        if (failCallback != NULL) failCallback(messageBuffer, failcbdata);
    }
    else
    {
      snprintf(messageBuffer, sizeof(messageBuffer) -1, "Upload failed (%d)", httpCode);
      if (failCallback != NULL) failCallback(messageBuffer, failcbdata);
    }
  }
  else
  {
    snprintf(messageBuffer, sizeof(messageBuffer) -1, "GET failed, error: %s", http.errorToString(httpCode).c_str());
    if (failCallback != NULL) failCallback(messageBuffer, failcbdata);
  }
}

void Updater::onStart(void (*callback)(const char*, void *), void *d)
{
  startCallback = callback;
  startcbdata = d;
}

void Updater::onEnd(void (*callback)(const char*, void *), void *d)
{
  endCallback = callback;
  endcbdata = d;
}

void Updater::onNone(void (*callback)(const char*, void *), void *d)
{
  nullCallback = callback;
  nullcbdata = d;
}

void Updater::onFail(void (*callback)(const char*, void *), void *d)
{
  failCallback = callback;
  failcbdata = d;
}

void Updater::onProgress(void (*callback)(size_t completed, size_t total, void *), void *d)
{
  progCallback = callback;
  progcbdata = d;
}