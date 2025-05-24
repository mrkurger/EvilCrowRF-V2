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
#include "SignalAnalyzer.h"

// Global instances
extern ELECHOUSE_CC1101 ELECHOUSE_cc1101; // Defined in ELECHOUSE_CC1101_SRC_DRV.cpp
AsyncWebServer controlserver(80);
SPIClass sdspi(VSPI);     // SD card on VSPI
SPIClass cc1101spi(HSPI); // CC1101 on HSPI

// Advanced Signal Analyzer instance
SignalAnalyzer signalAnalyzer;

// SD Card Management Variables
bool sdCardAvailable = false;
int sdInitAttempts = 0;
unsigned long lastSDCheck = 0;

// Pin definitions
const int push1 = 0;
const int push2 = 35;
const int led = 4;

// System state
bool systemInitialized = false;
bool cc1101Available = false;
bool cc1101InitAttempted = false;
uint32_t lastHeartbeat = 0;
uint32_t cc1101InitTime = 0;

// RF Configuration variables
String raw_rx = "0";
String jammer_tx = "0";
float frequency = 433.92;
int mod = 2; // 0=2-FSK, 1=GFSK, 2=ASK/OOK, 3=4-FSK, 4=MSK
float deviation = 47.60;
int datarate = 99;
float setrxbw = 812;
int transmissions = 1;
int samplepulse = 400;
int currentModule = 0; // 0 or 1 for dual CC1101 support

// Signal processing variables
bool isReceiving = false;
bool signalDetected = false;
String lastReceivedSignal = "";
unsigned long lastSignalTime = 0;

// Transmission data
String binaryData = "";
String rawData = "";
long transmitData[2000];
int transmitDataLength = 0;

// Tesla-specific variables
const uint8_t teslaSequence[43] = {
    0x02, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA,
    0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0xAA, 0x52};

// Jammer variables
bool jammerActive = false;
unsigned long jammerStartTime = 0;
int jammerDuration = 10000; // 10 seconds default

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
void disableReceive();
bool checkReceived();
void printReceived();
void signalanalyse();
void transmitRawData();
void transmitBinaryData();
void transmitTeslaSignal();
void startJammer();
void stopJammer();
void processJammer();
void switchModule(int module);
void configureCC1101();
void initCC1101Background();
String getSystemStatus();
String getSignalData();
void appendFile(fs::FS &fs, const char *path, const char *message1, const char *message2);
void appendFileLong(fs::FS &fs, const char *path, long value);
void deleteFile(fs::FS &fs, const char *path);
IRAM_ATTR void receiver();

void setup()
{
  Serial.begin(115200);
  delay(1000); // Give serial time to initialize

  Serial.println("\n================================================");
  Serial.println("🚀 EvilCrow RF v2 - Enhanced Version Starting...");
  Serial.println("================================================");

  // Initialize SPIFFS
  Serial.print("📁 Initializing SPIFFS... ");
  if (SPIFFS.begin(true))
  {
    Serial.println("✅ Success");
  }
  else
  {
    Serial.println("❌ Failed");
  }

  // Initialize SD card with improved SPI management and conflict resolution
  Serial.print("💾 Initializing SD card... ");
  Serial.println("📌 SD Card SPI: SCK=18, MISO=19, MOSI=23, CS=5 (Changed from CS=22)");

  // Use CS=5 instead of CS=22 to avoid conflicts
  sdspi.begin(18, 19, 23, 5); // VSPI pins with CS=5

  // Multiple initialization attempts with progressive delays
  bool sdInitialized = false;
  for (int attempt = 1; attempt <= 5; attempt++)
  {
    Serial.println("🔄 SD Init Attempt " + String(attempt) + "/5");

    // Progressive delay between attempts
    if (attempt > 1)
    {
      delay(attempt * 500);
    }

    // Try different frequencies and settings
    uint32_t frequency = (attempt == 1) ? 4000000 : (attempt == 2) ? 1000000
                                                                   : 400000;

    if (SD.begin(5, sdspi, frequency, "/sd", 10))
    { // Lower frequency, longer timeout
      sdInitialized = true;
      Serial.println("✅ SD Card initialized successfully on attempt " + String(attempt) + "!");
      break;
    }

    Serial.println("❌ Attempt " + String(attempt) + " failed");

    // Try to end and restart SD between attempts
    SD.end();
    delay(200);
    sdspi.end();
    delay(200);
    sdspi.begin(18, 19, 23, 5);
    delay(200);
  }

  if (sdInitialized)
  {
    // Debug SD card contents with error handling
    Serial.println("🔍 SD Card Debug Info:");

    uint64_t cardSize = 0;
    uint64_t usedBytes = 0;

    try
    {
      cardSize = SD.cardSize();
      usedBytes = SD.usedBytes();

      if (cardSize > 0)
      {
        Serial.println("   Card Size: " + String(cardSize / (1024 * 1024)) + " MB");
        Serial.println("   Used Space: " + String(usedBytes / 1024) + " KB");
        Serial.println("   Free Space: " + String((cardSize - usedBytes) / 1024) + " KB");

        // Test SD card read/write capability
        File testFile = SD.open("/test_write.txt", FILE_WRITE);
        if (testFile)
        {
          testFile.println("SD test: " + String(millis()));
          testFile.close();
          Serial.println("✅ SD write test passed");

          // Clean up test file
          SD.remove("/test_write.txt");
        }
        else
        {
          Serial.println("⚠️  SD write test failed - read-only mode");
        }
      }
      else
      {
        Serial.println("⚠️  Card size reported as 0 - possible format issue");
      }
    }
    catch (...)
    {
      Serial.println("⚠️  Error reading SD card info - card may be corrupted");
    }

    // List root directory
    Serial.println("📁 Root directory contents:");
    File root = SD.open("/");
    if (root)
    {
      File file = root.openNextFile();
      while (file)
      {
        if (file.isDirectory())
        {
          Serial.println("   📁 " + String(file.name()) + "/");
        }
        else
        {
          Serial.println("   📄 " + String(file.name()) + " (" + String(file.size()) + " bytes)");
        }
        file = root.openNextFile();
      }
      root.close();
    }

    // Check for HTML folder specifically
    if (SD.exists("/HTML"))
    {
      Serial.println("📁 /HTML/ directory contents:");
      File htmlDir = SD.open("/HTML");
      if (htmlDir)
      {
        File htmlFile = htmlDir.openNextFile();
        while (htmlFile)
        {
          if (!htmlFile.isDirectory())
          {
            Serial.println("   📄 " + String(htmlFile.name()) + " (" + String(htmlFile.size()) + " bytes)");
          }
          htmlFile = htmlDir.openNextFile();
        }
        htmlDir.close();
      }
    }
    else
    {
      Serial.println("❌ /HTML/ directory not found");
    }
  }
  else
  {
    Serial.println("❌ Failed");
  }

  // Setup WiFi FIRST (before CC1101 to ensure it works)
  Serial.println("\n📶 Setting up WiFi...");
  setupWiFi();

  // Setup web server
  Serial.println("\n🌐 Setting up web server...");
  setupWebServer();

  // Start web server
  Serial.print("🚀 Starting web server... ");
  controlserver.begin();
  Serial.println("✅ Success");
  Serial.println("🌐 Web server listening on: http://" + WiFi.softAPIP().toString());
  Serial.println("📄 Available pages:");
  Serial.println("   • http://" + WiFi.softAPIP().toString() + "/ (main page)");
  Serial.println("   • http://" + WiFi.softAPIP().toString() + "/test (test page)");
  Serial.println("   • http://" + WiFi.softAPIP().toString() + "/api/status (JSON status)");

  // Skip CC1101 initialization in setup to prevent blocking
  Serial.println("\n📡 CC1101 initialization will be done in background...");
  Serial.println("🌐 Web interface will be available immediately!");
  cc1101Available = false; // Will be set to true when CC1101 initializes successfully

  // Setup pins
  Serial.print("🔌 Setting up pins... ");
  pinMode(push1, INPUT_PULLUP);
  pinMode(push2, INPUT_PULLUP);
  pinMode(led, OUTPUT);
  Serial.println("✅ Success");

  systemInitialized = true;

  Serial.println("\n================================================");
  Serial.println("✅ System initialization complete!");
  Serial.println("📶 Connect to WiFi: EvilCrow-RF");
  Serial.println("🔑 Password: 123456789");
  Serial.println("🌐 Web interface: http://192.168.4.1");
  Serial.println("================================================\n");

  // Blink LED to indicate successful startup
  for (int i = 0; i < 3; i++)
  {
    digitalWrite(led, HIGH);
    delay(200);
    digitalWrite(led, LOW);
    delay(200);
  }
}

void loop()
{
  if (!systemInitialized)
  {
    delay(100);
    return;
  }

  uint32_t currentTime = millis();

  // Initialize CC1101 in background after 10 seconds (longer delay for SD card stability)
  if (!cc1101InitAttempted && currentTime > 10000)
  {
    cc1101InitAttempted = true;
    cc1101InitTime = currentTime;
    Serial.println("\n📡 Starting background CC1101 initialization...");

    // Check if SD card is present and add extra delay
    bool sdPresent = (SD.cardSize() > 0);
    if (sdPresent)
    {
      Serial.println("⚠️ SD card detected - using careful initialization");
      delay(2000); // Extra delay when SD card is present
    }

    // Try quick initialization with separate SPI bus
    Serial.print("🔧 Quick CC1101 test... ");
    Serial.println("📌 CC1101 SPI: SCK=14, MISO=12, MOSI=13, CS=15");

    // Set timeout for CC1101 initialization
    unsigned long initStart = millis();
    const unsigned long INIT_TIMEOUT = 5000; // 5 second timeout

    ELECHOUSE_cc1101.setSpiPin(14, 12, 13, 15); // HSPI pins
    ELECHOUSE_cc1101.setGDO(2, 4);

    try
    {
      ELECHOUSE_cc1101.Init();
      delay(500); // Allow initialization to complete

      // Check if we're still within timeout
      if (millis() - initStart < INIT_TIMEOUT)
      {
        ELECHOUSE_cc1101.setMHZ(433.92);
        delay(200);
        ELECHOUSE_cc1101.SetRx();
        cc1101Available = true;
        Serial.println("✅ Success!");
      }
      else
      {
        Serial.println("❌ Timeout during initialization");
        cc1101Available = false;
      }
    }
    catch (...)
    {
      Serial.println("❌ Failed - CC1101 not available");
      cc1101Available = false;
    }
  }

  // System heartbeat with WiFi status
  if (currentTime - lastHeartbeat > 5000)
  {
    Serial.println("💓 System heartbeat - Free heap: " + String(ESP.getFreeHeap()) + " bytes");
    Serial.println("📶 WiFi Status - Mode: " + String(WiFi.getMode()) + ", Clients: " + String(WiFi.softAPgetStationNum()));
    Serial.println("📡 AP IP: " + WiFi.softAPIP().toString());
    if (cc1101InitAttempted)
    {
      Serial.println("📻 CC1101 Status: " + String(cc1101Available ? "Available" : "Not Available"));
    }
    lastHeartbeat = currentTime;
  }

  // Process jammer if active
  if (jammerActive)
  {
    processJammer();
  }

  // Check for received signals
  if (isReceiving && checkReceived())
  {
    printReceived();
    signalanalyse();
    signalDetected = true;
    lastSignalTime = currentTime;
  }

  // Auto-disable receive after 30 seconds of inactivity
  if (isReceiving && (currentTime - lastSignalTime > 30000))
  {
    disableReceive();
    Serial.println("Auto-disabled receive due to inactivity");
  }

  delay(10);
}

void setupWiFi()
{
  Serial.println("Starting WiFi setup...");

  // Disconnect any existing connections
  WiFi.disconnect(true);
  delay(1000);

  // Set WiFi mode to Access Point
  WiFi.mode(WIFI_AP);
  delay(100);

  Serial.println("Setting up Access Point...");

  // Configure AP with more explicit parameters
  bool result = WiFi.softAP("EvilCrow-RF", "123456789", 1, 0, 4);

  if (result)
  {
    Serial.println("✅ WiFi AP started successfully!");
    Serial.print("📶 SSID: EvilCrow-RF");
    Serial.print("🔑 Password: 123456789");
    Serial.print("📡 IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.print("📻 Channel: 1");
    Serial.print("👥 Max clients: 4");
  }
  else
  {
    Serial.println("❌ Failed to start WiFi AP!");
    Serial.println("Retrying with different settings...");

    // Try with different parameters
    delay(1000);
    result = WiFi.softAP("EvilCrow-RF");

    if (result)
    {
      Serial.println("✅ WiFi AP started with default settings!");
      Serial.print("📡 IP address: ");
      Serial.println(WiFi.softAPIP());
    }
    else
    {
      Serial.println("❌ WiFi AP failed completely!");
    }
  }

  // Print WiFi status for debugging
  Serial.print("WiFi Mode: ");
  Serial.println(WiFi.getMode());
  Serial.print("AP Status: ");
  Serial.println(WiFi.softAPgetStationNum());

  // Additional debugging
  Serial.print("MAC Address: ");
  Serial.println(WiFi.softAPmacAddress());

  // Try to restart WiFi if it failed
  if (WiFi.getMode() != WIFI_AP)
  {
    Serial.println("⚠️ WiFi mode incorrect, attempting restart...");
    WiFi.mode(WIFI_OFF);
    delay(1000);
    WiFi.mode(WIFI_AP);
    delay(1000);
    WiFi.softAP("EvilCrow-RF", "123456789");
    delay(1000);
    Serial.print("📡 Final IP address: ");
    Serial.println(WiFi.softAPIP());
  }
}

// Enhanced file serving functions
String getContentType(String filename)
{
  if (filename.endsWith(".html"))
    return "text/html";
  else if (filename.endsWith(".css"))
    return "text/css";
  else if (filename.endsWith(".js"))
    return "text/javascript";
  else if (filename.endsWith(".json"))
    return "application/json";
  else if (filename.endsWith(".png"))
    return "image/png";
  else if (filename.endsWith(".jpg") || filename.endsWith(".jpeg"))
    return "image/jpeg";
  else if (filename.endsWith(".gif"))
    return "image/gif";
  else if (filename.endsWith(".ico"))
    return "image/x-icon";
  else if (filename.endsWith(".xml"))
    return "text/xml";
  else if (filename.endsWith(".pdf"))
    return "application/pdf";
  else if (filename.endsWith(".zip"))
    return "application/zip";
  else if (filename.endsWith(".txt"))
    return "text/plain";
  return "text/plain";
}

bool serveFileFromSD(AsyncWebServerRequest *request, String path)
{
  Serial.println("📁 Attempting to serve: " + path);

  if (!SD.exists(path))
  {
    Serial.println("❌ File not found: " + path);
    return false;
  }

  File file = SD.open(path);
  if (!file)
  {
    Serial.println("❌ Cannot open file: " + path);
    return false;
  }

  String contentType = getContentType(path);
  size_t fileSize = file.size();
  file.close();

  Serial.println("✅ Serving file: " + path + " (" + String(fileSize) + " bytes, " + contentType + ")");

  // Add cache headers for static files
  AsyncWebServerResponse *response = request->beginResponse(SD, path, contentType);
  if (path.endsWith(".css") || path.endsWith(".js") || path.endsWith(".png") || path.endsWith(".jpg"))
  {
    response->addHeader("Cache-Control", "public, max-age=3600"); // 1 hour cache
  }
  response->addHeader("Content-Length", String(fileSize));
  request->send(response);
  return true;
}

void setupWebServer()
{
  // Enhanced main page with SD card detection
  controlserver.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📄 Request for main page");

                     // Try to serve enhanced SD card version first
                     if (serveFileFromSD(request, "/HTML/index.html")) {
                       return;
                     }

                     Serial.println("⚠️ SD file not found, serving enhanced built-in page");
                     // Enhanced fallback HTML with SD card status
                     String html = "<!DOCTYPE html><html><head><title>EvilCrow RF v2</title>";
                     html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
                     html += "<style>body{font-family:Arial;margin:40px;background:#1a1a1a;color:white;}";
                     html += ".card{background:#2c3e50;padding:20px;margin:10px;border-radius:10px;}";
                     html += ".btn{background:#3498db;color:white;padding:10px 20px;border:none;border-radius:5px;margin:5px;cursor:pointer;text-decoration:none;display:inline-block;}";
                     html += ".btn:hover{background:#2980b9;}.btn-danger{background:#e74c3c;}.btn-success{background:#27ae60;}";
                     html += ".status-indicator{display:inline-block;width:12px;height:12px;border-radius:50%;margin-right:8px;}";
                     html += ".status-online{background-color:#2ecc71;}.status-offline{background-color:#e74c3c;}</style></head><body>";
                     html += "<h1>🚀 EvilCrow RF v2 - System Online!</h1>";

                     // System Status Card
                     html += "<div class='card'><h3>📊 System Status</h3>";
                     html += "<p><span class='status-indicator status-online'></span>WiFi: Connected</p>";
                     html += "<p><span class='status-indicator status-online'></span>Web Server: Running</p>";
                     html += "<p><span class='status-indicator " + String(cc1101Available ? "status-online" : "status-offline") + "'></span>CC1101: " + String(cc1101Available ? "Available" : "Not Available") + "</p>";
                     html += "<p><span class='status-indicator " + String(SD.exists("/HTML/index.html") ? "status-online" : "status-offline") + "'></span>SD Card: " + String(SD.exists("/HTML/index.html") ? "Files Available" : "No Files") + "</p></div>";

                     // Quick Actions Card
                     html += "<div class='card'><h3>🔧 Quick Actions</h3>";
                     html += "<a href='/api/status' class='btn'>📊 System Status (JSON)</a>";
                     html += "<a href='/test' class='btn'>🧪 Test Page</a>";
                     html += "<a href='/files' class='btn'>📁 File Browser</a>";
                     html += "<a href='/upload' class='btn btn-success'>📤 Upload Files</a></div>";

                     // Debug Info Card
                     html += "<div class='card'><h3>📋 Debug Info</h3>";
                     html += "<p>Free Memory: " + String(ESP.getFreeHeap()) + " bytes</p>";
                     html += "<p>Uptime: " + String(millis()/1000) + " seconds</p>";
                     html += "<p>SD Card Status: " + String(SD.cardSize() > 0 ? "Mounted" : "Not Available") + "</p>";
                     if (SD.cardSize() > 0) {
                       html += "<p>SD Card Size: " + String(SD.cardSize() / (1024 * 1024)) + " MB</p>";
                       html += "<p>SD Used Space: " + String(SD.usedBytes() / 1024) + " KB</p>";
                     }
                     html += "</div>";

                     html += "</body></html>";
                     request->send(200, "text/html", html); });

  // Enhanced static file serving with automatic content type detection
  controlserver.on("/style.css", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/style.css")) {
                       request->send(404, "text/plain", "CSS file not found");
                     } });

  controlserver.on("/lib.js", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/javascript.js")) {
                       request->send(404, "text/plain", "JavaScript file not found");
                     } });

  // File Browser
  controlserver.on("/files", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📁 File browser request");
                     String html = "<!DOCTYPE html><html><head><title>File Browser</title>";
                     html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
                     html += "<style>body{font-family:Arial;margin:20px;background:#1a1a1a;color:white;}";
                     html += ".file-list{background:#2c3e50;padding:20px;border-radius:10px;}";
                     html += ".file-item{padding:10px;border-bottom:1px solid #34495e;display:flex;justify-content:space-between;}";
                     html += ".btn{background:#3498db;color:white;padding:5px 10px;border:none;border-radius:3px;text-decoration:none;}";
                     html += ".btn-danger{background:#e74c3c;}</style></head><body>";
                     html += "<h1>📁 SD Card File Browser</h1>";
                     html += "<p><a href='/' class='btn'>← Back to Home</a></p>";

                     if (SD.cardSize() > 0) {
                       html += "<div class='file-list'>";
                       html += "<h3>Files on SD Card:</h3>";

                       // List root directory files and folders
                       File root = SD.open("/");
                       if (root) {
                         File file = root.openNextFile();
                         while (file) {
                           String fileName = String(file.name());
                           if (file.isDirectory()) {
                             html += "<div class='file-item'>";
                             html += "<span>📁 " + fileName + "/</span>";
                             html += "<span>Directory</span>";
                             html += "</div>";
                           } else {
                             size_t fileSize = file.size();
                             html += "<div class='file-item'>";
                             html += "<span>📄 " + fileName + " (" + String(fileSize) + " bytes)</span>";
                             html += "<span><a href='/download?file=" + fileName + "' class='btn'>📥 Download</a> ";
                             html += "<a href='/delete?file=" + fileName + "' class='btn btn-danger' onclick='return confirm(\"Delete " + fileName + "?\")'>🗑️ Delete</a></span>";
                             html += "</div>";
                           }
                           file = root.openNextFile();
                         }
                         root.close();

                         // List HTML folder contents
                         File htmlDir = SD.open("/HTML");
                         if (htmlDir) {
                           html += "<div class='file-item' style='margin-top:10px;'><span><strong>📁 HTML/ contents:</strong></span><span></span></div>";
                           File htmlFile = htmlDir.openNextFile();
                           while (htmlFile) {
                             String htmlFileName = String(htmlFile.name());
                             if (!htmlFile.isDirectory()) {
                               size_t htmlFileSize = htmlFile.size();
                               html += "<div class='file-item' style='margin-left:20px;'>";
                               html += "<span>📄 HTML/" + htmlFileName + " (" + String(htmlFileSize) + " bytes)</span>";
                               html += "<span><a href='/HTML/" + htmlFileName + "' class='btn'>👁️ View</a></span>";
                               html += "</div>";
                             }
                             htmlFile = htmlDir.openNextFile();
                           }
                           htmlDir.close();
                         }

                         // List URH folder contents if it exists
                         File urhDir = SD.open("/URH");
                         if (urhDir) {
                           html += "<div class='file-item' style='margin-top:10px;'><span><strong>📁 URH/ contents:</strong></span><span></span></div>";
                           File urhFile = urhDir.openNextFile();
                           while (urhFile) {
                             String urhFileName = String(urhFile.name());
                             if (!urhFile.isDirectory()) {
                               size_t urhFileSize = urhFile.size();
                               html += "<div class='file-item' style='margin-left:20px;'>";
                               html += "<span>📄 URH/" + urhFileName + " (" + String(urhFileSize) + " bytes)</span>";
                               html += "<span><a href='/download?file=URH/" + urhFileName + "' class='btn'>📥 Download</a></span>";
                               html += "</div>";
                             }
                             urhFile = urhDir.openNextFile();
                           }
                           urhDir.close();
                         }
                       }
                       html += "</div>";
                     } else {
                       html += "<p>❌ SD Card not available</p>";
                     }

                     html += "</body></html>";
                     request->send(200, "text/html", html); });

  // File Upload Page
  controlserver.on("/upload", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     String html = "<!DOCTYPE html><html><head><title>File Upload</title>";
                     html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
                     html += "<style>body{font-family:Arial;margin:20px;background:#1a1a1a;color:white;}";
                     html += ".upload-form{background:#2c3e50;padding:20px;border-radius:10px;}";
                     html += "input[type=file]{background:#34495e;color:white;padding:10px;border:none;border-radius:5px;width:100%;}";
                     html += "input[type=submit]{background:#27ae60;color:white;padding:10px 20px;border:none;border-radius:5px;cursor:pointer;}";
                     html += ".btn{background:#3498db;color:white;padding:10px 20px;border:none;border-radius:5px;text-decoration:none;}</style></head><body>";
                     html += "<h1>📤 File Upload</h1>";
                     html += "<p><a href='/' class='btn'>← Back to Home</a></p>";
                     html += "<div class='upload-form'>";
                     html += "<h3>Upload File to SD Card</h3>";
                     html += "<form method='POST' action='/upload' enctype='multipart/form-data'>";
                     html += "<p><input type='file' name='file' required></p>";
                     html += "<p><input type='submit' value='📤 Upload File'></p>";
                     html += "</form></div></body></html>";
                     request->send(200, "text/html", html); });

  // File Upload Handler
  controlserver.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request)
                   { request->send(200, "text/html", "<script>alert('Upload complete!'); window.location.href='/files';</script>"); }, [](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final)
                   {
    static File uploadFile;

    if (!index) {
      Serial.println("📤 Starting upload: " + filename);
      String path = "/" + filename;
      uploadFile = SD.open(path, FILE_WRITE);
      if (!uploadFile) {
        Serial.println("❌ Failed to create file: " + path);
        return;
      }
    }

    if (uploadFile) {
      uploadFile.write(data, len);
    }

    if (final) {
      if (uploadFile) {
        uploadFile.close();
        Serial.println("✅ Upload complete: " + filename + " (" + String(index + len) + " bytes)");
      }
    } });

  // File Download Handler
  controlserver.on("/download", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("file")) {
                       String fileName = request->getParam("file")->value();
                       String path = "/" + fileName;

                       // Handle paths that already start with /
                       if (fileName.startsWith("/")) {
                         path = fileName;
                       }

                       if (SD.exists(path)) {
                         Serial.println("📥 Downloading: " + path);
                         String displayName = fileName;
                         if (displayName.indexOf("/") >= 0) {
                           displayName = displayName.substring(displayName.lastIndexOf("/") + 1);
                         }
                         AsyncWebServerResponse *response = request->beginResponse(SD, path, "application/octet-stream");
                         response->addHeader("Content-Disposition", "attachment; filename=" + displayName);
                         request->send(response);
                       } else {
                         Serial.println("❌ File not found: " + path);
                         request->send(404, "text/plain", "File not found: " + path);
                       }
                     } else {
                       request->send(400, "text/plain", "No file specified");
                     } });

  // File Delete Handler
  controlserver.on("/delete", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("file")) {
                       String fileName = request->getParam("file")->value();
                       String path = "/" + fileName;
                       if (SD.exists(path)) {
                         SD.remove(path);
                         Serial.println("🗑️ Deleted: " + fileName);
                         request->send(200, "text/html", "<script>alert('File deleted!'); window.location.href='/files';</script>");
                       } else {
                         request->send(404, "text/plain", "File not found");
                       }
                     } else {
                       request->send(400, "text/plain", "No file specified");
                     } });

  // Enhanced HTML page serving with fallbacks
  controlserver.on("/rxconfig.html", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/rxconfig.html")) {
                       request->send(404, "text/html", "<h1>RX Config page not found</h1><p><a href='/'>← Back to Home</a></p>");
                     } });

  controlserver.on("/txconfig.html", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/txconfig.html")) {
                       request->send(404, "text/html", "<h1>TX Config page not found</h1><p><a href='/'>← Back to Home</a></p>");
                     } });

  controlserver.on("/txbinary.html", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/txbinary.html")) {
                       request->send(404, "text/html", "<h1>TX Binary page not found</h1><p><a href='/'>← Back to Home</a></p>");
                     } });

  controlserver.on("/txtesla.html", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/txtesla.html")) {
                       request->send(404, "text/html", "<h1>Tesla TX page not found</h1><p><a href='/'>← Back to Home</a></p>");
                     } });

  controlserver.on("/jammer.html", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     if (!serveFileFromSD(request, "/HTML/jammer.html")) {
                       request->send(404, "text/html", "<h1>Jammer page not found</h1><p><a href='/'>← Back to Home</a></p>");
                     } });

  // Fix broken navigation links - Add missing .html extensions
  controlserver.on("/btnconfigtesla", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /btnconfigtesla to /HTML/btnconfigtesla.html");
                     if (serveFileFromSD(request, "/HTML/btnconfigtesla.html")) {
                       return;
                     }
                     // Try alternative names
                     if (serveFileFromSD(request, "/HTML/txtesla.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>Tesla Config page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/rxconfig", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /rxconfig to /HTML/rxconfig.html");
                     if (serveFileFromSD(request, "/HTML/rxconfig.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>RX Config page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/txconfig", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /txconfig to /HTML/txconfig.html");
                     if (serveFileFromSD(request, "/HTML/txconfig.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>TX Config page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/jammer", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /jammer to /HTML/jammer.html");
                     if (serveFileFromSD(request, "/HTML/jammer.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>Jammer page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/config", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /config to /HTML/config.html");
                     if (serveFileFromSD(request, "/HTML/config.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>Config page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/about", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /about to /HTML/about.html");
                     if (serveFileFromSD(request, "/HTML/about.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>About page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  // Additional missing routes from HTML navigation
  controlserver.on("/txbinary", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /txbinary to /HTML/txbinary.html");
                     if (serveFileFromSD(request, "/HTML/txbinary.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>TX Binary page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/txtesla", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /txtesla to /HTML/txtesla.html");
                     if (serveFileFromSD(request, "/HTML/txtesla.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>Tesla TX page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/btnconfig", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /btnconfig to /HTML/btnconfigtesla.html");
                     if (serveFileFromSD(request, "/HTML/btnconfigtesla.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>Button Config page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/viewlog", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Serving logs from SD card");
                     if (SD.exists("/logs.txt")) {
                       request->send(SD, "/logs.txt", "text/plain");
                     } else {
                       request->send(404, "text/html", "<h1>No logs found</h1><p><a href='/'>← Back to Home</a></p>");
                     } });

  controlserver.on("/downloadlog", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📥 Downloading logs");
                     if (SD.exists("/logs.txt")) {
                       AsyncWebServerResponse *response = request->beginResponse(SD, "/logs.txt", "application/octet-stream");
                       response->addHeader("Content-Disposition", "attachment; filename=evilcrow_logs.txt");
                       request->send(response);
                     } else {
                       request->send(404, "text/plain", "No logs found");
                     } });

  controlserver.on("/cleanspiffs", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🧹 Cleaning SPIFFS");
                     // Clean SPIFFS implementation would go here
                     request->send(200, "text/html", "<script>alert('SPIFFS cleaned'); window.location.href='/';</script>"); });

  controlserver.on("/txprotocol", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /txprotocol to /HTML/txprotocol.html");
                     if (serveFileFromSD(request, "/HTML/txprotocol.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>TX Protocol page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/listxmlfiles", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📋 Listing XML protocol files");
                     String html = "<!DOCTYPE html><html><head><title>Protocol Files</title></head><body>";
                     html += "<h1>📋 Protocol Files</h1>";
                     html += "<p><a href='/'>← Back to Home</a></p>";
                     // List XML files from URH folder
                     if (SD.exists("/URH")) {
                       File urhDir = SD.open("/URH");
                       if (urhDir) {
                         File file = urhDir.openNextFile();
                         while (file) {
                           String fileName = String(file.name());
                           if (fileName.endsWith(".xml")) {
                             html += "<p>📄 " + fileName + " <a href='/download?file=URH/" + fileName + "'>Download</a></p>";
                           }
                           file = urhDir.openNextFile();
                         }
                         urhDir.close();
                       }
                     }
                     html += "</body></html>";
                     request->send(200, "text/html", html); });

  controlserver.on("/uploadxmlfiles", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /uploadxmlfiles to /HTML/uploadxmlfiles.html");
                     if (serveFileFromSD(request, "/HTML/uploadxmlfiles.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>Upload XML page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/wificonfig", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /wificonfig to /HTML/wificonfig.html");
                     if (serveFileFromSD(request, "/HTML/wificonfig.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>WiFi Config page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  controlserver.on("/updatesd", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔗 Redirecting /updatesd to /HTML/updatesd.html");
                     if (serveFileFromSD(request, "/HTML/updatesd.html")) {
                       return;
                     }
                     request->send(404, "text/html", "<h1>SD Update page not found</h1><p><a href='/'>← Back to Home</a></p>"); });

  // Basic API endpoints
  controlserver.on("/status", HTTP_GET, [](AsyncWebServerRequest *request)
                   { request->send(200, "text/plain", "OK"); });

  // Advanced Signal Analysis API endpoints
  controlserver.on("/api/signal/analyze", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📊 Signal analysis request");
                     String json = signalAnalyzer.getAnalysisJSON();
                     request->send(200, "application/json", json); });

  controlserver.on("/api/signal/report", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📋 Signal analysis report request");
                     String report = signalAnalyzer.getAnalysisReport();
                     request->send(200, "text/plain", report); });

  // TV Remote Control API endpoints
  controlserver.on("/api/tv/turnoff", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📺 TV Turn Off request");
                     String brand = "";
                     if (request->hasParam("brand")) {
                       brand = request->getParam("brand")->value();
                     }

                     bool success = signalAnalyzer.turnOffTV(brand);
                     String json = "{\"success\":" + String(success ? "true" : "false") +
                                  ",\"action\":\"turnoff\",\"brand\":\"" + brand + "\"}";
                     request->send(200, "application/json", json); });

  controlserver.on("/api/tv/turnon", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📺 TV Turn On request");
                     String brand = "";
                     if (request->hasParam("brand")) {
                       brand = request->getParam("brand")->value();
                     }

                     bool success = signalAnalyzer.turnOnTV(brand);
                     String json = "{\"success\":" + String(success ? "true" : "false") +
                                  ",\"action\":\"turnon\",\"brand\":\"" + brand + "\"}";
                     request->send(200, "application/json", json); });

  controlserver.on("/api/tv/scan", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📺 TV Scan and Turn Off request");
                     bool success = signalAnalyzer.scanAndTurnOffTV();
                     String json = "{\"success\":" + String(success ? "true" : "false") +
                                  ",\"action\":\"scan_turnoff\"}";
                     request->send(200, "application/json", json); });

  controlserver.on("/api/tv/brands", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📺 TV Brands list request");
                     auto brands = signalAnalyzer.getSupportedTVBrands();
                     String json = "{\"brands\":[";
                     for (size_t i = 0; i < brands.size(); i++) {
                       json += "\"" + brands[i] + "\"";
                       if (i < brands.size() - 1) json += ",";
                     }
                     json += "]}";
                     request->send(200, "application/json", json); });

  // Attack API endpoints
  controlserver.on("/api/attack/bruteforce", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🔓 Brute force attack request");
                     int protocol = 0;
                     if (request->hasParam("protocol", true)) {
                       protocol = request->getParam("protocol", true)->value().toInt();
                     }

                     bool success = signalAnalyzer.bruteForceAttack((ProtocolType)protocol);
                     String json = "{\"success\":" + String(success ? "true" : "false") +
                                  ",\"protocol\":" + String(protocol) + "}";
                     request->send(200, "application/json", json); });

  controlserver.on("/setrx", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("frequency", true)) {
                       frequency = request->getParam("frequency", true)->value().toFloat();
                       ELECHOUSE_cc1101.setMHZ(frequency);
                       ELECHOUSE_cc1101.SetRx();
                       enableReceive();
                     }
                     request->send(200, "text/plain", "RX Config OK"); });

  controlserver.on("/settx", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("frequency", true)) {
                       frequency = request->getParam("frequency", true)->value().toFloat();
                       ELECHOUSE_cc1101.setMHZ(frequency);
                       ELECHOUSE_cc1101.SetTx();
                     }
                     request->send(200, "text/plain", "TX Config OK"); });

  // Test page
  controlserver.on("/test", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📄 Request for test page");
                     String html = "<!DOCTYPE html><html><head><title>Test Page</title></head><body>";
                     html += "<h1>🧪 EvilCrow RF v2 Test Page</h1>";
                     html += "<p>✅ Web server is working!</p>";
                     html += "<p>📡 CC1101 Status: " + String(cc1101Available ? "Available" : "Not Available") + "</p>";
                     html += "<p><a href='/'>← Back to Home</a></p>";
                     html += "</body></html>";
                     request->send(200, "text/html", html); });

  // Enhanced API endpoints
  controlserver.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📊 API status request");
                     request->send(200, "application/json", getSystemStatus()); });

  controlserver.on("/api/signals", HTTP_GET, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("📡 API signals request");
                     request->send(200, "application/json", getSignalData()); });

  // TX Binary Data
  controlserver.on("/settxbinary", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("frequency", true)) {
                       frequency = request->getParam("frequency", true)->value().toFloat();
                     }
                     if (request->hasParam("binarydata", true)) {
                       binaryData = request->getParam("binarydata", true)->value();
                     }
                     if (request->hasParam("samplepulse", true)) {
                       samplepulse = request->getParam("samplepulse", true)->value().toInt();
                     }

                     configureCC1101();
                     transmitBinaryData();
                     request->send(200, "text/html", "<script>alert('Binary signal transmitted')</script>"); });

  // Tesla Signal
  controlserver.on("/settxtesla", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("frequency", true)) {
                       frequency = request->getParam("frequency", true)->value().toFloat();
                     }

                     configureCC1101();
                     transmitTeslaSignal();
                     request->send(200, "text/html", "<script>alert('Tesla signal transmitted')</script>"); });

  // Jammer Control
  controlserver.on("/startjammer", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     if (request->hasParam("frequency", true)) {
                       frequency = request->getParam("frequency", true)->value().toFloat();
                     }
                     if (request->hasParam("duration", true)) {
                       jammerDuration = request->getParam("duration", true)->value().toInt() * 1000; // Convert to ms
                     }

                     startJammer();
                     request->send(200, "text/html", "<script>alert('Jammer started')</script>"); });

  controlserver.on("/stopjammer", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     stopJammer();
                     request->send(200, "text/html", "<script>alert('Jammer stopped')</script>"); });

  // Tesla Button Configuration Handlers
  controlserver.on("/setbtntesla", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🚗 Tesla button configuration");
                     if (request->hasParam("button", true) && request->hasParam("frequency", true)) {
                       int button = request->getParam("button", true)->value().toInt();
                       frequency = request->getParam("frequency", true)->value().toFloat();
                       Serial.println("Button " + String(button) + " configured for Tesla at " + String(frequency) + " MHz");
                       configureCC1101();
                     }
                     request->send(200, "text/html", "<script>alert('Tesla button configured');</script>"); });

  controlserver.on("/stopbtntesla", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🛑 Stopping Tesla button configuration");
                     request->send(200, "text/html", "<script>alert('Tesla button config stopped');</script>"); });

  // Delete logs handler
  controlserver.on("/delete", HTTP_POST, [](AsyncWebServerRequest *request)
                   {
                     Serial.println("🗑️ Deleting logs");
                     if (SD.exists("/logs.txt")) {
                       SD.remove("/logs.txt");
                       Serial.println("✅ Logs deleted");
                       request->send(200, "text/html", "<script>alert('Logs deleted'); window.location.href='/';</script>");
                     } else {
                       request->send(404, "text/html", "<script>alert('No logs to delete'); window.location.href='/';</script>");
                     } });

  // Enhanced 404 handler with wildcard file serving
  controlserver.onNotFound([](AsyncWebServerRequest *request)
                           {
    String path = request->url();
    Serial.println("🔍 Wildcard request: " + path);

    // Try to serve file from SD card
    if (path.startsWith("/") && SD.exists(path)) {
      Serial.println("📁 Serving wildcard file: " + path);
      String contentType = getContentType(path);
      AsyncWebServerResponse *response = request->beginResponse(SD, path, contentType);

      // Add appropriate headers
      if (path.endsWith(".css") || path.endsWith(".js") || path.endsWith(".png") || path.endsWith(".jpg")) {
        response->addHeader("Cache-Control", "public, max-age=3600");
      }

      request->send(response);
      return;
    }

    // Try with /HTML/ prefix
    String htmlPath = "/HTML" + path;
    if (SD.exists(htmlPath)) {
      Serial.println("📁 Serving from HTML folder: " + htmlPath);
      String contentType = getContentType(htmlPath);
      request->send(SD, htmlPath, contentType);
      return;
    }

    // 404 handler with enhanced debugging
    Serial.println("❌ 404 Not Found: " + path);
    String html = "<!DOCTYPE html><html><head><title>404 Not Found</title>";
    html += "<style>body{font-family:Arial;margin:40px;background:#1a1a1a;color:white;}";
    html += ".btn{background:#3498db;color:white;padding:10px 20px;border:none;border-radius:5px;text-decoration:none;margin:5px;}</style></head><body>";
    html += "<h1>❌ 404 - Page Not Found</h1>";
    html += "<p>Requested URL: <code>" + path + "</code></p>";
    html += "<p>Tried paths:</p><ul>";
    html += "<li>" + path + " (not found)</li>";
    html += "<li>" + htmlPath + " (not found)</li></ul>";
    html += "<p><a href='/' class='btn'>🏠 Go to Home Page</a>";
    html += "<a href='/files' class='btn'>📁 Browse Files</a>";
    html += "<a href='/test' class='btn'>🧪 Test Page</a></p>";
    html += "</body></html>";
    request->send(404, "text/html", html); });

  // OTA update
  ElegantOTA.begin(&controlserver);
}

void enableReceive()
{
  if (!cc1101Available)
  {
    Serial.println("❌ Cannot enable receive - CC1101 not available");
    return;
  }

  ELECHOUSE_cc1101.SetRx();
  samplecount = 0;
  isReceiving = true;
  lastSignalTime = millis();
  attachInterrupt(digitalPinToInterrupt(2), receiver, CHANGE);
  Serial.println("Receive enabled");
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
  // Enhanced signal analysis
  Serial.println("Analyzing signal...");

  // Build raw signal string
  lastReceivedSignal = "";
  for (int i = 1; i < samplecount && i < 200; i++)
  {
    lastReceivedSignal += String(sample[i]);
    if (i < samplecount - 1 && i < 199)
      lastReceivedSignal += ",";
  }

  // Log to SD card
  appendFile(SD, "/logs.txt", "Signal: ", lastReceivedSignal.c_str());
  appendFile(SD, "/logs.txt", "\n", "");

  Serial.println("Signal analysis complete - " + String(samplecount) + " samples");
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
  if (!file)
    return;
  if (message1)
    file.print(message1);
  if (message2)
    file.print(message2);
  file.close();
}

void appendFileLong(fs::FS &fs, const char *path, long value)
{
  File file = fs.open(path, FILE_APPEND);
  if (!file)
    return;
  file.print(value);
  file.close();
}

void deleteFile(fs::FS &fs, const char *path)
{
  fs.remove(path);
}

// Enhanced RF Functions
void disableReceive()
{
  detachInterrupt(digitalPinToInterrupt(2));
  ELECHOUSE_cc1101.setSidle();
  isReceiving = false;
  Serial.println("Receive disabled");
}

void switchModule(int module)
{
  if (module == 0 || module == 1)
  {
    ELECHOUSE_cc1101.setModul(module);
    currentModule = module;
    Serial.println("Switched to module " + String(module));
  }
}

void configureCC1101()
{
  ELECHOUSE_cc1101.Init();
  ELECHOUSE_cc1101.setMHZ(frequency);
  ELECHOUSE_cc1101.setModulation(mod);
  ELECHOUSE_cc1101.setDeviation(deviation);
  ELECHOUSE_cc1101.setDRate(datarate);
  ELECHOUSE_cc1101.setRxBW(setrxbw);
  Serial.println("CC1101 configured - Freq: " + String(frequency) + " MHz");
}

void transmitRawData()
{
  if (rawData.length() == 0)
    return;

  // Parse comma-separated raw data
  transmitDataLength = 0;
  int pos = 0;
  for (int i = 0; i < rawData.length() && transmitDataLength < 2000; i++)
  {
    if (rawData.charAt(i) == ',' || i == rawData.length() - 1)
    {
      String value = rawData.substring(pos, i);
      if (value.length() > 0)
      {
        transmitData[transmitDataLength++] = value.toInt();
      }
      pos = i + 1;
    }
  }

  ELECHOUSE_cc1101.SetTx();
  int txPin = (currentModule == 0) ? 2 : 25;
  pinMode(txPin, OUTPUT);

  for (int t = 0; t < transmissions; t++)
  {
    for (int i = 0; i < transmitDataLength; i += 2)
    {
      if (i + 1 < transmitDataLength)
      {
        digitalWrite(txPin, HIGH);
        delayMicroseconds(transmitData[i]);
        digitalWrite(txPin, LOW);
        delayMicroseconds(transmitData[i + 1]);
      }
    }
    delay(100); // Delay between transmissions
  }

  ELECHOUSE_cc1101.setSidle();
  Serial.println("Raw data transmitted");
}

void transmitBinaryData()
{
  if (binaryData.length() == 0)
    return;

  // Convert binary data to timing array
  transmitDataLength = 0;
  String cleanBinary = binaryData;
  cleanBinary.replace(" ", "");
  cleanBinary.replace("\n", "");

  String lastBit = "1";
  for (int i = 0; i < cleanBinary.length() && transmitDataLength < 1999; i++)
  {
    String currentBit = cleanBinary.substring(i, i + 1);
    if (lastBit != currentBit)
    {
      transmitDataLength++;
      lastBit = currentBit;
    }
    transmitData[transmitDataLength] += samplepulse;
  }

  ELECHOUSE_cc1101.SetTx();
  int txPin = (currentModule == 0) ? 2 : 25;
  pinMode(txPin, OUTPUT);

  for (int i = 0; i < transmitDataLength; i += 2)
  {
    if (i + 1 < transmitDataLength)
    {
      digitalWrite(txPin, HIGH);
      delayMicroseconds(transmitData[i]);
      digitalWrite(txPin, LOW);
      delayMicroseconds(transmitData[i + 1]);
    }
  }

  ELECHOUSE_cc1101.setSidle();
  Serial.println("Binary data transmitted");
}

void transmitTeslaSignal()
{
  ELECHOUSE_cc1101.SetTx();
  int txPin = (currentModule == 0) ? 2 : 25;
  pinMode(txPin, OUTPUT);

  for (int t = 0; t < 5; t++) // Transmit 5 times
  {
    for (int i = 0; i < 43; i++)
    {
      for (int bit = 7; bit >= 0; bit--)
      {
        if (teslaSequence[i] & (1 << bit))
        {
          digitalWrite(txPin, HIGH);
          delayMicroseconds(400);
          digitalWrite(txPin, LOW);
          delayMicroseconds(400);
        }
        else
        {
          digitalWrite(txPin, HIGH);
          delayMicroseconds(200);
          digitalWrite(txPin, LOW);
          delayMicroseconds(200);
        }
      }
    }
    delay(23); // Tesla message distance
  }

  ELECHOUSE_cc1101.setSidle();
  Serial.println("Tesla signal transmitted");
}

void startJammer()
{
  jammerActive = true;
  jammerStartTime = millis();
  ELECHOUSE_cc1101.SetTx();
  Serial.println("Jammer started for " + String(jammerDuration / 1000) + " seconds");
}

void stopJammer()
{
  jammerActive = false;
  ELECHOUSE_cc1101.setSidle();
  Serial.println("Jammer stopped");
}

void processJammer()
{
  if (!jammerActive)
    return;

  // Check if jammer duration exceeded
  if (millis() - jammerStartTime > jammerDuration)
  {
    stopJammer();
    return;
  }

  // Generate noise signal
  int txPin = (currentModule == 0) ? 2 : 25;
  pinMode(txPin, OUTPUT);

  for (int i = 0; i < 100; i++)
  {
    digitalWrite(txPin, HIGH);
    delayMicroseconds(random(50, 500));
    digitalWrite(txPin, LOW);
    delayMicroseconds(random(50, 500));
  }
}

// JSON API Functions
String getSystemStatus()
{
  String json = "{";
  json += "\"status\":\"online\",";
  json += "\"uptime\":" + String(millis()) + ",";
  json += "\"freeHeap\":" + String(ESP.getFreeHeap()) + ",";
  json += "\"cc1101Available\":" + String(cc1101Available ? "true" : "false") + ",";
  json += "\"frequency\":" + String(frequency) + ",";
  json += "\"modulation\":" + String(mod) + ",";
  json += "\"deviation\":" + String(deviation) + ",";
  json += "\"datarate\":" + String(datarate) + ",";
  json += "\"rxBandwidth\":" + String(setrxbw) + ",";
  json += "\"currentModule\":" + String(currentModule) + ",";
  json += "\"isReceiving\":" + String(isReceiving ? "true" : "false") + ",";
  json += "\"jammerActive\":" + String(jammerActive ? "true" : "false") + ",";
  json += "\"signalDetected\":" + String(signalDetected ? "true" : "false") + ",";
  json += "\"lastSignalTime\":" + String(lastSignalTime);
  json += "}";
  return json;
}

String getSignalData()
{
  String json = "{";
  json += "\"sampleCount\":" + String(samplecount) + ",";
  json += "\"signalDetected\":" + String(signalDetected ? "true" : "false") + ",";
  json += "\"lastSignalTime\":" + String(lastSignalTime) + ",";
  json += "\"samples\":[";

  int maxSamples = min(samplecount, 100); // Limit to 100 samples for JSON
  for (int i = 0; i < maxSamples; i++)
  {
    json += String(sample[i]);
    if (i < maxSamples - 1)
      json += ",";
  }

  json += "],";
  json += "\"rawData\":\"" + lastReceivedSignal + "\"";
  json += "}";

  // Reset signal detected flag after reading
  signalDetected = false;

  return json;
}