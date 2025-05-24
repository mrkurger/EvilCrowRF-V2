// EvilCrow RF v2 - Minimal Working Version
// Simplified version that compiles successfully

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebSrv.h>
#include <SPIFFS.h>
#include <SD.h>
#include <SPI.h>
#include <FS.h>
#include <Update.h>
#include <ElegantOTA.h>
#include <ESP32-targz.h>
#include "ELECHOUSE_CC1101_SRC_DRV.h"

// Global instances
ELECHOUSE_CC1101 ELECHOUSE_cc1101;
AsyncWebServer controlserver(80);
SPIClass sdspi(VSPI);

// Pin definitions
const int push1 = 0;
const int push2 = 35;
const int led = 4;

// System state
bool systemInitialized = false;
uint32_t lastHeartbeat = 0;

// Legacy variables
String raw_rx = "0";
String jammer_tx = "0";
float frequency = 433.92;
int mod = 2;
float deviation = 47.60;
int datarate = 99;
float setrxbw = 812;
int transmissions = 1;
int samplepulse = 400;

// Sample arrays
const int samplesize = 2000;
long sample[samplesize];
long samplesmooth[samplesize];
int samplecount = 0;
long lastTime = 0;

// File handling
File logs;

// Function declarations
void setupWiFi();
void setupWebServer();
void enableReceive();
bool checkReceived();
void printReceived();
void signalanalyse();
void appendFile(fs::FS &fs, const char *path, const char *message1, const char *message2);
void appendFileLong(fs::FS &fs, const char *path, long value);
void deleteFile(fs::FS &fs, const char *path);
IRAM_ATTR void receiver();

void setup()
{
  Serial.begin(115200);
  Serial.println("EvilCrow RF v2 - Minimal Version Starting...");

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("SPIFFS Mount Failed");
  }

  // Initialize SD card
  sdspi.begin(18, 19, 23, 22);
  if (!SD.begin(22, sdspi))
  {
    Serial.println("SD Card Mount Failed");
  }

  // Initialize CC1101
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(433.92);
  ELECHOUSE_cc1101.SetRx();

  // Setup WiFi
  setupWiFi();

  // Setup web server
  setupWebServer();

  // Start web server
  controlserver.begin();

  // Setup pins
  pinMode(push1, INPUT_PULLUP);
  pinMode(push2, INPUT_PULLUP);
  pinMode(led, OUTPUT);

  systemInitialized = true;
  Serial.println("System initialization complete!");
}

void loop()
{
  if (!systemInitialized)
  {
    delay(100);
    return;
  }

  // System heartbeat
  uint32_t currentTime = millis();
  if (currentTime - lastHeartbeat > 5000)
  {
    Serial.println("System heartbeat");
    lastHeartbeat = currentTime;
  }

  // Check for received signals
  if (checkReceived())
  {
    printReceived();
    signalanalyse();
  }

  delay(10);
}

void setupWiFi()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAP("EvilCrow-RF", "123456789");
  Serial.println("WiFi AP started");
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());
}

void setupWebServer()
{
  // Basic web routes
  controlserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(200, "text/html", "<h1>EvilCrow RF v2</h1><p>System running</p>"); });

  controlserver.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(200, "text/plain", "OK"); });

  // OTA update
  ElegantOTA.begin(&controlserver);
}

void enableReceive()
{
  ELECHOUSE_cc1101.SetRx();
  samplecount = 0;
  attachInterrupt(digitalPinToInterrupt(2), receiver, CHANGE);
}

bool checkReceived()
{
  if (samplecount >= 10 && micros() - lastTime > 100000)
  {
    detachInterrupt(digitalPinToInterrupt(2));
    return true;
  }
  return false;
}

void printReceived()
{
  Serial.print("Received signal - Count: ");
  Serial.println(samplecount);
  for (int i = 1; i < samplecount && i < 50; i++)
  {
    Serial.print(sample[i]);
    Serial.print(",");
  }
  Serial.println();
}

void signalanalyse()
{
  // Basic signal analysis
  Serial.println("Analyzing signal...");
  // Implementation would go here
}

IRAM_ATTR void receiver()
{
  const long time = micros();
  const unsigned int duration = time - lastTime;

  if (duration > 100000)
  {
    samplecount = 0;
  }

  if (duration >= 100 && samplecount < samplesize)
  {
    sample[samplecount++] = duration;
  }

  lastTime = time;
}

// File system helper functions
void appendFile(fs::FS &fs, const char *path, const char *message1, const char *message2)
{
  File file = fs.open(path, FILE_APPEND);
  if (!file) return;
  if (message1) file.print(message1);
  if (message2) file.print(message2);
  file.close();
}

void appendFileLong(fs::FS &fs, const char *path, long value)
{
  File file = fs.open(path, FILE_APPEND);
  if (!file) return;
  file.print(value);
  file.close();
}

void deleteFile(fs::FS &fs, const char *path)
{
  fs.remove(path);
}
