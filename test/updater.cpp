#include <ArduinoJson.h>
#include <LITTLEFS.h>

#include "config.h"
#include "devices.h"
#include "p2system.h"

extern IndicatorLed::Colour indicateUpdate;
Updater *Updater::pThis = NULL;

Updater::Updater(const String &devName) : P2Task(devName, 8000)
{
  clear();
  pThis = this;
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

bool Updater::operator()()
{
  switch (uType)
  {
  case UPD_SYS:
    systemUpdate();
    clear();
    break;
  case UPD_CONF:
    configUpdate();
    clear();
    break;
  default:
    break;
  }
  return true;
}

void Updater::systemUpdate()
{
  WiFiClient client;

  if (startCallback != NULL)
  {
    startCallback("Upload started", startcbdata);
  }

  httpUpdate.rebootOnUpdate(false);

  Update.onProgress(progcb);

  Event e;
  e.enqueue("Update started");
  dev.p2sys.enterState(P2System::STATE_UPDATE);
  dev.toSecure();

  t_httpUpdate_return ret = httpUpdate.update(client, server, port, source);

  switch (ret)
  {
  case HTTP_UPDATE_FAILED:
    snprintf(messageBuffer, sizeof(messageBuffer) - 1, "Update failed (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
    e.enqueue(messageBuffer);
    if (failCallback != NULL)
      failCallback(messageBuffer, failcbdata);
    break;

  case HTTP_UPDATE_NO_UPDATES:
    serr.println("HTTP_UPDATE_NO_UPDATES");
    e.enqueue("No updates available");
    if (nullCallback != NULL)
      nullCallback("No Updates Available", nullcbdata);
    break;

  case HTTP_UPDATE_OK:
    serr.println("HTTP_UPDATE_OK");
    e.enqueue("Update complete");
    if (endCallback != NULL)
      endCallback("OK", endcbdata);
    break;
  }
  dev.p2sys.revertState();
}

bool Updater::configUpdate()
{
  bool result = false;

  HTTPClient http;
  http.begin(server, port, source);
  int httpCode = http.GET();

  if (httpCode > 0)
  {
    if (httpCode == HTTP_CODE_OK)
    {
      File f = LITTLEFS.open("/upload.tmp", "w+");
      if (f)
      {
        Stream &s = http.getStream();
        uint8_t buffer[128];
        size_t total = http.getSize();
        size_t sofar = 0;
        size_t l;
        doprogcb(sofar, total);
        while ((l = s.readBytes(buffer, sizeof(buffer) - 1)))
        {
          doprogcb(sofar, total);
          sofar += l;
          f.write(buffer, l);
        }
        f.close();
        doprogcb(sofar, total);
        if (LITTLEFS.rename("/upload.tmp", target))
        {
          if (endCallback != NULL)
            endCallback("Complete", endcbdata);
          result = true;
        }
        else
        {
          if (failCallback != NULL)
            failCallback("Could not create file", failcbdata);
          // Serial.println("Couldn't create file");
        }
      }

      else
      {
        if (failCallback != NULL)
          failCallback("Could not open temporary file", failcbdata);
        // Serial.println("Couldn't create temp file");
      }
    }
    else
    {
      snprintf(messageBuffer, sizeof(messageBuffer) - 1, "Upload failed (%d)", httpCode);
      if (failCallback != NULL)
        failCallback(messageBuffer, failcbdata);
      // Serial.printf("Upload failed %d\n", httpCode);
    }
  }
  else
  {
    snprintf(messageBuffer, sizeof(messageBuffer) - 1, "GET failed, error: %s", http.errorToString(httpCode).c_str());
    if (failCallback != NULL)
      failCallback(messageBuffer, failcbdata);
       // Serial.printf("Get failed %s", http.errorToString(httpCode).c_str());
  }
  return result;
}

void Updater::onStart(void (*callback)(const char *, void *), void *d)
{
  startCallback = callback;
  startcbdata = d;
}

void Updater::onEnd(void (*callback)(const char *, void *), void *d)
{
  endCallback = callback;
  endcbdata = d;
}

void Updater::onNone(void (*callback)(const char *, void *), void *d)
{
  nullCallback = callback;
  nullcbdata = d;
}

void Updater::onFail(void (*callback)(const char *, void *), void *d)
{
  failCallback = callback;
  failcbdata = d;
}

void Updater::onProgress(void (*callback)(size_t completed, size_t total, void *), void *d)
{
  progCallback = callback;
  progcbdata = d;
}

void Updater::clear()
{
  startCallback = NULL;
  endCallback = NULL;
  nullCallback = NULL;
  failCallback = NULL;
  progCallback = NULL;
  startcbdata = NULL;
  endcbdata = NULL;
  nullcbdata = NULL;
  failcbdata = NULL;
  progcbdata = NULL;
  messageBuffer[0] = '\0';

  uType = UPD_NONE;
}