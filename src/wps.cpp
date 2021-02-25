#include <Arduino.h>
#include <esp_wps.h>

#include "config.h"

#define ESP_WPS_MODE      WPS_TYPE_PBC
#define ESP_MANUFACTURER  "ESPRESSIF"
#define ESP_MODEL_NUMBER  "ESP32"
#define ESP_MODEL_NAME    "ESPRESSIF IOT"
#define ESP_DEVICE_NAME   "ESP STATION"

esp_wps_config_t wpsconfig;


void wpsInit()
{
  wpsconfig.crypto_funcs = &g_wifi_default_wps_crypto_funcs;
  wpsconfig.wps_type = ESP_WPS_MODE;
  strcpy(wpsconfig.factory_info.manufacturer, ESP_MANUFACTURER);
  strcpy(wpsconfig.factory_info.model_number, ESP_MODEL_NUMBER);
  strcpy(wpsconfig.factory_info.model_name, ESP_MODEL_NAME);
  strcpy(wpsconfig.factory_info.device_name, ESP_DEVICE_NAME);
  esp_wifi_wps_enable(&wpsconfig);
  esp_wifi_wps_start(0);
  enterState(STATE_WPS);
}


