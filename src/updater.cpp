#include <ArduinoJson.h>

#include "config.h"
#include "devices.h"
#include "p2system.h"

extern IndicatorLed::Colour indicateUpdate;

void updateStarted()
{
  Event e;
  e.enqueue("Update started");
  dev.p2sys.enterState(P2System::STATE_UPDATE);
  dev.toSecure();
}

void updateCompleted()
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

void updateNone()
{
  Event e;
  e.enqueue("No update available");
  dev.p2sys.revertState();
}

void updateFail()
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
  ready = false;
}

Updater::~Updater()
{
}

void prog(size_t completed, size_t total)
{
  extern AsyncEventSource events;
  static int oldPhase = 1;
  int progress = (completed * 100) / total;

  int phase = (progress / 5) % 2; // report at 5% intervals

  if (phase != oldPhase)
  {
    if (dev.indicators.size() > 0)
    {
      if (phase)
        dev.indicators[0].off();
      else
        dev.indicators[0].setColour(indicate_update, true);
    }
    serr.printf("Progress: %d%% (%d/%d)\n", progress, completed, total);
    dev.webServer.event("progress", (String(progress) + "%").c_str());
    oldPhase = phase;
  }
}

t_httpUpdate_return Updater::systemUpdate(const String &s, const uint16_t p, const String &i, bool r)
{
  setRemote(s, p, i, r);
  return systemUpdate();
}

t_httpUpdate_return Updater::systemUpdate()
{
  t_httpUpdate_return ret = HTTP_UPDATE_NO_UPDATES;
  if (ready)
  {
    ready = false;
    WiFiClient client;

    if (startCallback != NULL)
    {
      startCallback();
    }

    httpUpdate.rebootOnUpdate(false);

    Update.onProgress(prog);

    t_httpUpdate_return ret = httpUpdate.update(client, server, port, image);

    switch (ret)
    {
    case HTTP_UPDATE_FAILED:
      serr.printf("HTTP_UPDATE_FAILED Error (%d): %s\n", httpUpdate.getLastError(), httpUpdate.getLastErrorString().c_str());
      if (failCallback != NULL)
        failCallback();
      break;

    case HTTP_UPDATE_NO_UPDATES:
      serr.println("HTTP_UPDATE_NO_UPDATES");
      if (nullCallback != NULL)
        nullCallback();
      break;

    case HTTP_UPDATE_OK:
      serr.println("HTTP_UPDATE_OK");
      if (endCallback != NULL)
        endCallback();
      dev.webServer.event("progress", "Complete, reset device to install");
      break;
    }
  }
  return ret;
}

void Updater::onStart(void (*callback)())
{
  startCallback = callback;
}

void Updater::onEnd(void (*callback)())
{
  endCallback = callback;
}

void Updater::onNone(void (*callback)())
{
  nullCallback = callback;
}

void Updater::onFail(void (*callback)())
{
  failCallback = callback;
}