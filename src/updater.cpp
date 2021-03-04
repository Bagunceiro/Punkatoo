#include <ArduinoJson.h>

#include "config.h"
#include "devices.h"
#include "p2state.h"

extern IndicatorLed::Colour indicateUpdate;

void updateStarted()
{
  Event e;
  e.enqueue("Update started");
  p2state.enter(P2State::STATE_UPDATE);
  dev.toSecure();
}

void updateCompleted()
{
  Event e;
  e.enqueue("Update complete");
  p2state.revert();

  time_t now = timeClient.getEpochTime();
  persistant[persistant.updateTime_n] = String(now);
  persistant.writeFile();
}

void updateNone()
{
  Event e;
  e.enqueue("No update available");
  p2state.revert();
}

void updateFail()
{
  Event e;
  e.enqueue("Update failed");
  p2state.revert();
}

Updater::Updater(const String &devName)
{
  startCallback = updateStarted;
  endCallback   = updateCompleted;
  nullCallback  = updateNone;
  failCallback  = updateFail;
}

Updater::~Updater()
{
}

void prog(size_t completed, size_t total)
{
  static int oldPhase = 1;
  int progress = (completed * 100)/total;

  int phase = (progress / 5) % 2; // report at 5% intervals

  if (phase != oldPhase)
  {
      if (phase) dev.indicators[0].off();
      else dev.indicators[0].setColour(indicate_update, true);
      serr.printf("Progress: %d%% (%d/%d)\n", progress, completed, total);
      oldPhase = phase;
  }
}

t_httpUpdate_return Updater::systemUpdate(const String &server, const uint16_t port, const String &image)
{
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
    if (failCallback != NULL) failCallback();
    break;

  case HTTP_UPDATE_NO_UPDATES:
    serr.println("HTTP_UPDATE_NO_UPDATES");
    if (nullCallback != NULL) nullCallback();
    break;

  case HTTP_UPDATE_OK:
    serr.println("HTTP_UPDATE_OK");
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

void Updater::onNone(void(*callback)())
{
  nullCallback = callback;
}

void Updater::onFail(void(*callback)())
{
  failCallback = callback;
}