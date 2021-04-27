#include <ArduinoJson.h>

#include "config.h"
#include "devices.h"
#include "p2system.h"

extern IndicatorLed::Colour indicateUpdate;
Updater* Updater::pThis = NULL;

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

Updater::Updater(const String &devName)
{
  startCallback = updateStarted;
  endCallback = updateCompleted;
  nullCallback = updateNone;
  failCallback = updateFail;
  progCallback = NULL;
  uType = UPD_NONE;
  startcbdata = NULL;
  endcbdata = NULL;
  nullcbdata = NULL;
  failcbdata = NULL;
  pThis = this;
}

Updater::~Updater()
{
}

void Updater::progcb(size_t completed, size_t total)
{
  if (pThis->progCallback != NULL)
  {
    pThis->progCallback(completed, total, pThis->progcbdata);
  }
}

void endcb(void *)
{
  dev.webServer.event("progress", "Complete. Reseting, please wait");
  delay(1000);
  ESP.restart();
}

void Updater::systemUpdate(const String &s, const uint16_t p, const String &i, updateType t)
{
  setRemote(s, p, i, t);
  systemUpdate();
}

void Updater::systemUpdate()
{
  if (uType == UPD_SYS)
  {
    uType = UPD_NONE;
    WiFiClient client;

    if (startCallback != NULL)
    {
      startCallback(startcbdata);
    }

    httpUpdate.rebootOnUpdate(false);

    Update.onProgress(progcb);

    t_httpUpdate_return ret = httpUpdate.update(client, server, port, image);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      serr.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      if (failCallback != NULL)
        failCallback(failcbdata);
      break;

    case HTTP_UPDATE_NO_UPDATES:
      serr.println("HTTP_UPDATE_NO_UPDATES");
      if (nullCallback != NULL)
        nullCallback(nullcbdata);
      break;

    case HTTP_UPDATE_OK:
      serr.println("HTTP_UPDATE_OK");
      if (endCallback != NULL)
        endCallback(endcbdata);
      // dev.webServer.event("progress", "Complete. Reseting, please wait");
      // delay(1000);
      // ESP.restart();
      break;
    }
  }
}

void Updater::onStart(void (*callback)(void *), void *d)
{
  startCallback = callback;
  startcbdata = d;
}

void Updater::onEnd(void (*callback)(void *), void *d)
{
  endCallback = callback;
  endcbdata = d;
}

void Updater::onNone(void (*callback)(void *), void *d)
{
  nullCallback = callback;
  nullcbdata = d;
}

void Updater::onFail(void (*callback)(void *), void *d)
{
  failCallback = callback;
  failcbdata = d;
}

void Updater::onProgress(void (*callback)(size_t completed, size_t total, void *), void *d)
{
  progCallback = callback;
  progcbdata = d;
}