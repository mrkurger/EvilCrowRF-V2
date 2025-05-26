#ifndef LIB_CONFIG_H
#define LIB_CONFIG_H

// Configure ESP32-targz before including
#define DEST_FS_USES_SD
#define TARGZ_USE_SD
#define TARGZ_USE_SPIFFS
#define TARGZ_NO_PROGRESSIVE_DISPLAY

// Configure SD before including
#define _FS_MUTEX true
#define _FS_REENTRANT

// Configure ElegantOTA before including
#define ELEGANTOTA_USE_ASYNC_WEBSERVER

// Include Arduino.h first to ensure basic types are defined
#include <Arduino.h>

// Then include all external libraries in correct order
#include <WiFi.h>
#include <WiFiClient.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <Update.h>
#define ELEGANTOTA_USE_ASYNC_WEBSERVER 1
#include <ElegantOTA.h>
#include <ESP32-targz.h>

#endif // LIB_CONFIG_H
