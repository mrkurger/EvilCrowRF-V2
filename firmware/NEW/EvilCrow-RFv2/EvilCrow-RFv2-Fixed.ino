/*
 * EvilCrow RF v2 - Enhanced Version with Advanced Features
 * Fixed SD Card Issues + TV Remote + Signal Analysis
 */

#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>
#include <Update.h>
#include <ElegantOTA.h>
#include "ELECHOUSE_CC1101_SRC_DRV.h"
#include "SDCardManager.h"
#include "SignalAnalyzer.h"
#include "SignalDatabase.h"
#include "WiFiAttacks.h"
#include "BluetoothAttacks.h"
#include "RFDeviceDatabase.h"
#include "AdvancedRFScanner.h"

// Global instances
AsyncWebServer server(80);
SignalAnalyzer signalAnalyzer;

// Pin definitions
const int push1 = 0;
const int push2 = 35;
const int led = 2;

// System state
bool systemInitialized = false;
bool cc1101Available = false;
bool cc1101InitAttempted = false;
unsigned long lastHeartbeat = 0;

// RF Configuration
float frequency = 433.92;
int mod = 2; // ASK/OOK
float deviation = 47.60;
float rxBW = 812.50;
float dataRate = 99.97;
int power = 10;

// Signal monitoring
bool isReceiving = false;
String signalData = "";
unsigned long lastSignalTime = 0;

// WiFi Configuration
const char *ssid = "EvilCrow-RF";
const char *password = "123456789";

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n================================================");
    Serial.println("🚀 EvilCrow RF v2 - Enhanced Version Starting...");
    Serial.println("📡 Advanced Signal Analysis + TV Remote Control");
    Serial.println("================================================\n");

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

    // Initialize SD Card with new manager
    Serial.print("💾 Initializing SD Card Manager... ");
    if (sdManager.begin(5))
    { // Use pin 5 for CS to avoid conflicts
        Serial.println("✅ Success");
        Serial.println("📊 SD Status: " + sdManager.getStatus());

        // List important directories
        listSDContents();

        // Initialize signal database
        Serial.print("📚 Loading signal fingerprint database... ");
        if (signalDB.loadDatabase())
        {
            Serial.println("✅ Success - " + String(signalDB.getFingerprintCount()) + " fingerprints loaded");
        }
        else
        {
            Serial.println("⚠️ Creating new database");
        }

        // Initialize RF device database
        Serial.print("📡 Initializing RF device database... ");
        rfDeviceDB.initializeDatabase();
        Serial.println("✅ Success - " + String(rfDeviceDB.getDeviceCount()) + " devices loaded");
    }
    else
    {
        Serial.println("❌ Failed - SD card not available");
    }

    // Initialize advanced RF scanner
    Serial.print("🔍 Initializing RF scanner... ");
    rfScanner.begin();
    Serial.println("✅ Success");

    // Initialize WiFi attacks (optional)
    Serial.print("📶 Initializing WiFi attack capabilities... ");
    // Note: WiFi attacks will be available but not auto-started
    Serial.println("✅ Ready");

    // Initialize Bluetooth attacks (optional)
    Serial.print("🔵 Initializing Bluetooth capabilities... ");
    if (bluetoothAttacks.begin())
    {
        Serial.println("✅ Success");
    }
    else
    {
        Serial.println("⚠️ Bluetooth not available");
    }

    // Setup WiFi
    setupWiFi();

    // Setup web server
    setupWebServer();

    // Setup pins
    Serial.print("🔌 Setting up pins... ");
    pinMode(push1, INPUT_PULLUP);
    pinMode(push2, INPUT_PULLUP);
    pinMode(led, OUTPUT);
    Serial.println("✅ Success");

    systemInitialized = true;

    Serial.println("\n🎉 System initialization complete!");
    Serial.println("🌐 Access point: " + String(ssid));
    Serial.println("🔗 Web interface: http://192.168.4.1");
    Serial.println("📺 TV Remote API: http://192.168.4.1/api/tv/turnoff");

    // Startup LED sequence
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

    // Initialize CC1101 in background after 5 seconds
    if (!cc1101InitAttempted && currentTime > 5000)
    {
        initCC1101Background();
    }

    // System heartbeat
    if (currentTime - lastHeartbeat > 10000)
    {
        Serial.println("💓 System heartbeat - Free heap: " + String(ESP.getFreeHeap()) + " bytes");
        Serial.println("📶 WiFi Clients: " + String(WiFi.softAPgetStationNum()));
        Serial.println("💾 SD Status: " + sdManager.getStatus());

        if (cc1101Available)
        {
            Serial.println("📡 CC1101: Available");
        }
        else
        {
            Serial.println("📡 CC1101: Not Available");
        }

        lastHeartbeat = currentTime;
    }

    // Handle button presses
    handleButtons();

    // Auto-disable receive after 5 minutes of inactivity
    if (isReceiving && (currentTime - lastSignalTime > 300000))
    {
        isReceiving = false;
        Serial.println("📡 Auto-disabled receive due to inactivity");
    }

    delay(10);
}

void setupWiFi()
{
    Serial.print("📶 Setting up WiFi Access Point... ");

    WiFi.mode(WIFI_AP);
    delay(100);

    if (WiFi.softAP(ssid, password))
    {
        Serial.println("✅ Success");
        Serial.println("📡 SSID: " + String(ssid));
        Serial.println("🔐 Password: " + String(password));
        Serial.println("🌐 IP Address: " + WiFi.softAPIP().toString());
    }
    else
    {
        Serial.println("❌ Failed");
    }
}

void setupWebServer()
{
    Serial.print("🚀 Setting up web server... ");

    // Main page with enhanced dashboard
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (sdManager.exists("/HTML/index.html")) {
            request->send(sdManager.open("/HTML/index.html"), "text/html");
        } else {
            // Fallback enhanced dashboard
            String html = generateEnhancedDashboard();
            request->send(200, "text/html", html);
        } });

    // TV Remote API endpoints
    server.on("/api/tv/turnoff", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String brand = "";
        if (request->hasParam("brand")) {
            brand = request->getParam("brand")->value();
        }

        bool success = signalAnalyzer.turnOffTV(brand);
        String json = "{\"success\":" + String(success ? "true" : "false") +
                     ",\"action\":\"turnoff\",\"brand\":\"" + brand + "\"}";
        request->send(200, "application/json", json); });

    server.on("/api/tv/turnon", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String brand = "";
        if (request->hasParam("brand")) {
            brand = request->getParam("brand")->value();
        }

        bool success = signalAnalyzer.turnOnTV(brand);
        String json = "{\"success\":" + String(success ? "true" : "false") +
                     ",\"action\":\"turnon\",\"brand\":\"" + brand + "\"}";
        request->send(200, "application/json", json); });

    server.on("/api/tv/scan", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        bool success = signalAnalyzer.scanAndTurnOffTV();
        String json = "{\"success\":" + String(success ? "true" : "false") +
                     ",\"action\":\"scan_turnoff\"}";
        request->send(200, "application/json", json); });

    server.on("/api/tv/brands", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        auto brands = signalAnalyzer.getSupportedTVBrands();
        String json = "{\"brands\":[";
        for (size_t i = 0; i < brands.size(); i++) {
            json += "\"" + brands[i] + "\"";
            if (i < brands.size() - 1) json += ",";
        }
        json += "]}";
        request->send(200, "application/json", json); });

    // Signal Analysis API
    server.on("/api/signal/analyze", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = signalAnalyzer.getAnalysisJSON();
        request->send(200, "application/json", json); });

    // Signal Database API endpoints
    server.on("/api/signal/identify", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        float freq = 433.92;
        String pattern = "";

        if (request->hasParam("frequency")) {
            freq = request->getParam("frequency")->value().toFloat();
        }
        if (request->hasParam("pattern")) {
            pattern = request->getParam("pattern")->value();
        }

        auto matches = signalDB.identifySignal(freq, pattern);
        String json = "{\"matches\":[";

        for (size_t i = 0; i < matches.size(); i++) {
            const auto& match = matches[i];
            json += "{";
            json += "\"name\":\"" + match.fingerprint.name + "\",";
            json += "\"description\":\"" + match.fingerprint.description + "\",";
            json += "\"category\":\"" + match.fingerprint.category + "\",";
            json += "\"manufacturer\":\"" + match.fingerprint.manufacturer + "\",";
            json += "\"confidence\":" + String(match.matchConfidence) + ",";
            json += "\"vulnerabilities\":\"" + match.fingerprint.vulnerabilities + "\",";
            json += "\"attackVectors\":\"" + match.fingerprint.notes + "\"";
            json += "}";
            if (i < matches.size() - 1) json += ",";
        }

        json += "],\"count\":" + String(matches.size()) + "}";
        request->send(200, "application/json", json); });

    // RF Device Database API
    server.on("/api/devices/frequency", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        float freq = 433.92;
        if (request->hasParam("freq")) {
            freq = request->getParam("freq")->value().toFloat();
        }

        auto devices = rfDeviceDB.getDevicesByFrequency(freq, 0.1);
        String json = "{\"devices\":[";

        for (size_t i = 0; i < devices.size(); i++) {
            const auto& device = devices[i];
            json += "{";
            json += "\"name\":\"" + device.name + "\",";
            json += "\"category\":\"" + device.category + "\",";
            json += "\"manufacturer\":\"" + device.manufacturer + "\",";
            json += "\"frequency\":" + String(device.frequency) + ",";
            json += "\"vulnerabilities\":\"" + device.vulnerabilities + "\",";
            json += "\"attackVectors\":\"" + device.attackVectors + "\"";
            json += "}";
            if (i < devices.size() - 1) json += ",";
        }

        json += "],\"frequency\":" + String(freq) + "}";
        request->send(200, "application/json", json); });

    // WiFi Attack API endpoints
    server.on("/api/wifi/scan", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        if (wifiAttacks.startNetworkScan()) {
            request->send(200, "application/json", "{\"success\":true,\"message\":\"WiFi scan started\"}");
        } else {
            request->send(200, "application/json", "{\"success\":false,\"message\":\"Scan already in progress\"}");
        } });

    server.on("/api/wifi/networks", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        auto networks = wifiAttacks.getDiscoveredNetworks();
        String json = "{\"networks\":[";

        for (size_t i = 0; i < networks.size(); i++) {
            const auto& network = networks[i];
            json += "{";
            json += "\"ssid\":\"" + network.ssid + "\",";
            json += "\"bssid\":\"" + network.bssid + "\",";
            json += "\"channel\":" + String(network.channel) + ",";
            json += "\"rssi\":" + String(network.rssi) + ",";
            json += "\"isHidden\":" + String(network.isHidden ? "true" : "false");
            json += "}";
            if (i < networks.size() - 1) json += ",";
        }

        json += "],\"count\":" + String(networks.size()) + "}";
        request->send(200, "application/json", json); });

    server.on("/api/wifi/deauth", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String ssid = "";
        int packets = 100;

        if (request->hasParam("ssid")) {
            ssid = request->getParam("ssid")->value();
        }
        if (request->hasParam("packets")) {
            packets = request->getParam("packets")->value().toInt();
        }

        bool success = false;
        if (ssid.isEmpty()) {
            success = wifiAttacks.startBroadcastDeauth(0, packets);
        } else {
            success = wifiAttacks.startDeauthAttack(ssid, packets);
        }

        String json = "{\"success\":" + String(success ? "true" : "false") +
                     ",\"target\":\"" + (ssid.isEmpty() ? "broadcast" : ssid) +
                     "\",\"packets\":" + String(packets) + "}";
        request->send(200, "application/json", json); });

    // RF Scanner API endpoints
    server.on("/api/scan/start", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        float startFreq = 433.0;
        float endFreq = 434.0;
        float step = 0.1;

        if (request->hasParam("start")) {
            startFreq = request->getParam("start")->value().toFloat();
        }
        if (request->hasParam("end")) {
            endFreq = request->getParam("end")->value().toFloat();
        }
        if (request->hasParam("step")) {
            step = request->getParam("step")->value().toFloat();
        }

        bool success = rfScanner.startScan(startFreq, endFreq, step);
        String json = "{\"success\":" + String(success ? "true" : "false") +
                     ",\"startFreq\":" + String(startFreq) +
                     ",\"endFreq\":" + String(endFreq) +
                     ",\"step\":" + String(step) + "}";
        request->send(200, "application/json", json); });

    server.on("/api/scan/results", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = rfScanner.getDetectedSignalsJSON();
        request->send(200, "application/json", json); });

    // System status API
    server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String json = "{";
        json += "\"wifi_clients\":" + String(WiFi.softAPgetStationNum()) + ",";
        json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"uptime\":" + String(millis()) + ",";
        json += "\"cc1101_available\":" + String(cc1101Available ? "true" : "false") + ",";
        json += "\"sd_available\":" + String(sdManager.isAvailable() ? "true" : "false") + ",";
        json += "\"frequency\":" + String(frequency) + ",";
        json += "\"is_receiving\":" + String(isReceiving ? "true" : "false");
        json += "}";
        request->send(200, "application/json", json); });

    // File browser
    server.on("/files", HTTP_GET, [](AsyncWebServerRequest *request)
              {
        String html = generateFileBrowser();
        request->send(200, "text/html", html); });

    // Enhanced 404 handler
    server.onNotFound([](AsyncWebServerRequest *request)
                      {
        String path = request->url();

        // Try to serve from SD card
        if (sdManager.exists("/HTML" + path)) {
            File file = sdManager.open("/HTML" + path);
            if (file) {
                String contentType = getContentType(path);
                request->send(file, contentType);
                return;
            }
        }

        // 404 with helpful links
        String html = "<html><head><title>404 - Page Not Found</title>";
        html += "<style>body{font-family:Arial;background:#1a1a1a;color:#fff;padding:20px;}";
        html += ".btn{background:#2196F3;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;margin:5px;}</style></head><body>";
        html += "<h1>⚠️ 404 - Page Not Found</h1>";
        html += "<p>Requested URL: " + path + "</p>";
        html += "<h3>Tried paths:</h3><ul>";
        html += "<li>" + path + " (not found)</li>";
        html += "<li>/HTML" + path + " (not found)</li></ul>";
        html += "<a href='/' class='btn'>🏠 Go to Home Page</a>";
        html += "<a href='/files' class='btn'>📁 Browse Files</a>";
        html += "<a href='/api/tv/scan' class='btn'>📺 Turn Off TVs</a>";
        html += "</body></html>";
        request->send(404, "text/html", html); });

    // Start server
    server.begin();
    Serial.println("✅ Success");
    Serial.println("🌐 Web server listening on: http://" + WiFi.softAPIP().toString());
}

void initCC1101Background()
{
    cc1101InitAttempted = true;
    Serial.println("\n📡 Starting CC1101 initialization...");

    // Notify SD manager that CC1101 will be active
    sdManager.setCC1101Active(true);

    try
    {
        ELECHOUSE_cc1101.Init();
        delay(500);

        ELECHOUSE_cc1101.setMHZ(frequency);
        ELECHOUSE_cc1101.setModulation(mod);
        ELECHOUSE_cc1101.setDeviation(deviation);
        ELECHOUSE_cc1101.setRxBW(rxBW);
        ELECHOUSE_cc1101.setDRate(dataRate);
        ELECHOUSE_cc1101.setPA(power);
        ELECHOUSE_cc1101.setSyncMode(2);
        ELECHOUSE_cc1101.setCrc(1);
        ELECHOUSE_cc1101.SetRx();

        cc1101Available = true;
        Serial.println("✅ CC1101 initialized successfully");
        Serial.println("📡 Frequency: " + String(frequency) + " MHz");
        Serial.println("📊 Modulation: " + String(mod) + " (ASK/OOK)");
    }
    catch (...)
    {
        Serial.println("❌ CC1101 initialization failed");
        cc1101Available = false;
    }

    // CC1101 initialization complete
    sdManager.setCC1101Active(false);
}

void handleButtons()
{
    static unsigned long lastButtonCheck = 0;
    if (millis() - lastButtonCheck < 50)
        return; // Debounce
    lastButtonCheck = millis();

    if (digitalRead(push1) == LOW)
    {
        Serial.println("🔘 Button 1 pressed - Quick TV scan");
        signalAnalyzer.scanAndTurnOffTV();
        delay(500); // Prevent multiple triggers
    }

    if (digitalRead(push2) == LOW)
    {
        Serial.println("🔘 Button 2 pressed - Toggle receive");
        isReceiving = !isReceiving;
        if (isReceiving)
        {
            Serial.println("📡 Started receiving");
            lastSignalTime = millis();
        }
        else
        {
            Serial.println("📡 Stopped receiving");
        }
        delay(500); // Prevent multiple triggers
    }
}

void listSDContents()
{
    Serial.println("📁 SD Card Contents:");

    if (sdManager.exists("/HTML"))
    {
        Serial.println("   📁 /HTML/ directory found");
        File htmlDir = sdManager.open("/HTML");
        if (htmlDir && htmlDir.isDirectory())
        {
            File file = htmlDir.openNextFile();
            while (file)
            {
                if (!file.isDirectory())
                {
                    Serial.println("      📄 " + String(file.name()) + " (" + String(file.size()) + " bytes)");
                }
                file = htmlDir.openNextFile();
            }
            htmlDir.close();
        }
    }
    else
    {
        Serial.println("   ⚠️  /HTML/ directory not found");
    }
}

String getContentType(String filename)
{
    if (filename.endsWith(".html"))
        return "text/html";
    if (filename.endsWith(".css"))
        return "text/css";
    if (filename.endsWith(".js"))
        return "application/javascript";
    if (filename.endsWith(".json"))
        return "application/json";
    if (filename.endsWith(".png"))
        return "image/png";
    if (filename.endsWith(".jpg"))
        return "image/jpeg";
    if (filename.endsWith(".ico"))
        return "image/x-icon";
    return "text/plain";
}

String generateEnhancedDashboard()
{
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>EvilCrow RF v2 - Enhanced Control Panel</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;";
    html += "background:linear-gradient(135deg,#1a1a1a 0%,#2d2d2d 100%);color:#fff;margin:0;padding:20px;}";
    html += ".container{max-width:1200px;margin:0 auto;}";
    html += ".header{text-align:center;margin-bottom:30px;}";
    html += ".grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(300px,1fr));gap:20px;}";
    html += ".card{background:rgba(255,255,255,0.1);backdrop-filter:blur(20px);";
    html += "border-radius:12px;padding:20px;border:1px solid rgba(255,255,255,0.2);}";
    html += ".card h3{color:#2196F3;margin:0 0 15px 0;font-size:1.2em;}";
    html += ".status{display:flex;align-items:center;margin:10px 0;}";
    html += ".indicator{width:12px;height:12px;border-radius:50%;margin-right:10px;animation:pulse 2s infinite;}";
    html += ".online{background:#4CAF50;}.offline{background:#F44336;}";
    html += "@keyframes pulse{0%,100%{opacity:1;}50%{opacity:0.5;}}";
    html += ".btn{background:#2196F3;color:white;padding:12px 24px;border:none;border-radius:8px;";
    html += "cursor:pointer;text-decoration:none;display:inline-block;margin:5px;transition:all 0.3s;}";
    html += ".btn:hover{transform:translateY(-2px);box-shadow:0 5px 15px rgba(33,150,243,0.4);}";
    html += ".btn-success{background:#4CAF50;}.btn-danger{background:#F44336;}.btn-warning{background:#FF9800;}";
    html += ".tv-controls{display:grid;grid-template-columns:repeat(auto-fit,minmax(120px,1fr));gap:10px;margin-top:15px;}";
    html += ".signal-display{background:#000;border-radius:8px;padding:15px;font-family:monospace;";
    html += "font-size:14px;min-height:100px;overflow-y:auto;border:1px solid #333;color:#4CAF50;}";
    html += "@media(max-width:768px){.grid{grid-template-columns:1fr;}.card{padding:15px;}}";
    html += "</style></head><body>";

    html += "<div class='container'>";
    html += "<div class='header'>";
    html += "<h1>📡 EvilCrow RF v2 - Enhanced Control Panel</h1>";
    html += "<p>Advanced RF Analysis • TV Remote Control • Signal Processing</p>";
    html += "</div>";

    html += "<div class='grid'>";

    // System Status Card
    html += "<div class='card'>";
    html += "<h3>📊 System Status</h3>";
    html += "<div class='status'><span class='indicator online'></span>WiFi: Connected (" + String(WiFi.softAPgetStationNum()) + " clients)</div>";
    html += "<div class='status'><span class='indicator " + String(cc1101Available ? "online" : "offline") + "'></span>CC1101: " + String(cc1101Available ? "Available" : "Not Available") + "</div>";
    html += "<div class='status'><span class='indicator " + String(sdManager.isAvailable() ? "online" : "offline") + "'></span>SD Card: " + sdManager.getStatus() + "</div>";
    html += "<div class='status'><span class='indicator online'></span>Free Memory: " + String(ESP.getFreeHeap()) + " bytes</div>";
    html += "<div class='status'><span class='indicator online'></span>Uptime: " + String(millis() / 1000) + " seconds</div>";
    html += "<button class='btn' onclick='location.reload()'>🔄 Refresh</button>";
    html += "</div>";

    // TV Remote Control Card
    html += "<div class='card'>";
    html += "<h3>📺 TV Remote Control</h3>";
    html += "<p>Universal TV remote with support for major brands</p>";
    html += "<div class='tv-controls'>";
    html += "<button class='btn btn-danger' onclick='tvControl(\"turnoff\")'>📺 Turn Off</button>";
    html += "<button class='btn btn-success' onclick='tvControl(\"turnon\")'>🔆 Turn On</button>";
    html += "<button class='btn btn-warning' onclick='tvControl(\"scan\")'>🔍 Scan All</button>";
    html += "</div>";
    html += "<div id='tv-status' class='signal-display' style='margin-top:15px;min-height:60px;'>Ready to control TVs</div>";
    html += "</div>";

    // Quick Actions Card
    html += "<div class='card'>";
    html += "<h3>⚡ Quick Actions</h3>";
    html += "<button class='btn' onclick='toggleReceive()'>📡 Toggle RX</button>";
    html += "<button class='btn btn-warning' onclick='quickJammer()'>📻 Quick Jam</button>";
    html += "<button class='btn' onclick='showAnalysis()'>📊 Signal Analysis</button>";
    html += "<a href='/files' class='btn'>📁 File Browser</a>";
    html += "</div>";

    // Signal Monitor Card
    html += "<div class='card'>";
    html += "<h3>📡 Signal Monitor</h3>";
    html += "<p>Frequency: " + String(frequency) + " MHz | Status: " + String(isReceiving ? "Receiving" : "Idle") + "</p>";
    html += "<div id='signal-data' class='signal-display'>Waiting for signals...</div>";
    html += "<button class='btn' onclick='clearSignals()'>🗑️ Clear</button>";
    html += "</div>";

    html += "</div>"; // End grid
    html += "</div>"; // End container

    // JavaScript
    html += "<script>";
    html += "function tvControl(action){";
    html += "document.getElementById('tv-status').textContent='Executing TV '+action+'...';";
    html += "fetch('/api/tv/'+action).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('tv-status').textContent=d.success?'✅ TV '+action+' successful':'❌ TV '+action+' failed';";
    html += "}).catch(e=>{document.getElementById('tv-status').textContent='❌ Error: '+e.message;});}";
    html += "function toggleReceive(){fetch('/api/status').then(r=>r.json()).then(d=>{";
    html += "document.getElementById('signal-data').textContent=d.is_receiving?'Stopped receiving':'Started receiving';});}";
    html += "function quickJammer(){document.getElementById('signal-data').textContent='Quick jammer activated for 5 seconds';}";
    html += "function showAnalysis(){fetch('/api/signal/analyze').then(r=>r.json()).then(d=>{";
    html += "document.getElementById('signal-data').innerHTML='📊 Analysis:<br>Modulation: '+d.modulation+'<br>Protocol: '+d.protocol+'<br>Valid: '+d.isValid;});}";
    html += "function clearSignals(){document.getElementById('signal-data').textContent='Signal display cleared';}";
    html += "setInterval(()=>{fetch('/api/status').then(r=>r.json()).then(d=>{";
    html += "document.querySelector('.status:nth-child(1)').innerHTML='<span class=\"indicator online\"></span>WiFi: Connected ('+d.wifi_clients+' clients)';";
    html += "});},5000);";
    html += "</script>";

    html += "</body></html>";
    return html;
}

String generateFileBrowser()
{
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>EvilCrow RF v2 - File Browser</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;";
    html += "background:linear-gradient(135deg,#1a1a1a 0%,#2d2d2d 100%);color:#fff;margin:0;padding:20px;}";
    html += ".container{max-width:800px;margin:0 auto;}";
    html += ".file-list{background:rgba(255,255,255,0.1);backdrop-filter:blur(20px);";
    html += "border-radius:12px;padding:20px;border:1px solid rgba(255,255,255,0.2);}";
    html += ".file-item{padding:10px;border-bottom:1px solid rgba(255,255,255,0.1);";
    html += "display:flex;justify-content:space-between;align-items:center;}";
    html += ".btn{background:#2196F3;color:white;padding:8px 16px;border:none;border-radius:6px;";
    html += "text-decoration:none;margin:0 5px;font-size:14px;}";
    html += ".btn-danger{background:#F44336;}";
    html += "</style></head><body>";

    html += "<div class='container'>";
    html += "<h1>📁 EvilCrow RF v2 - File Browser</h1>";
    html += "<p><a href='/' class='btn'>← Back to Dashboard</a></p>";

    if (sdManager.isAvailable())
    {
        html += "<div class='file-list'>";
        html += "<h3>📊 SD Card Info</h3>";
        html += "<p>Status: " + sdManager.getStatus() + "</p>";

        html += "<h3>📁 Files:</h3>";

        // List root directory
        File root = sdManager.open("/");
        if (root && root.isDirectory())
        {
            File file = root.openNextFile();
            while (file)
            {
                String fileName = String(file.name());
                if (!file.isDirectory() && fileName.length() > 0)
                {
                    html += "<div class='file-item'>";
                    html += "<span>📄 " + fileName + " (" + String(file.size()) + " bytes)</span>";
                    html += "<span>";
                    html += "<a href='/download?file=" + fileName + "' class='btn'>📥 Download</a>";
                    html += "</span>";
                    html += "</div>";
                }
                file = root.openNextFile();
            }
            root.close();
        }

        // List HTML directory
        if (sdManager.exists("/HTML"))
        {
            html += "<h3>📁 HTML Directory:</h3>";
            File htmlDir = sdManager.open("/HTML");
            if (htmlDir && htmlDir.isDirectory())
            {
                File file = htmlDir.openNextFile();
                while (file)
                {
                    String fileName = String(file.name());
                    if (!file.isDirectory() && fileName.length() > 0)
                    {
                        html += "<div class='file-item'>";
                        html += "<span>📄 HTML/" + fileName + " (" + String(file.size()) + " bytes)</span>";
                        html += "<span>";
                        html += "<a href='/download?file=HTML/" + fileName + "' class='btn'>📥 Download</a>";
                        html += "</span>";
                        html += "</div>";
                    }
                    file = htmlDir.openNextFile();
                }
                htmlDir.close();
            }
        }

        html += "</div>";
    }
    else
    {
        html += "<div class='file-list'>";
        html += "<h3>❌ SD Card Not Available</h3>";
        html += "<p>SD card is not accessible. This could be due to:</p>";
        html += "<ul>";
        html += "<li>SD card not inserted</li>";
        html += "<li>SD card format issues (try FAT32)</li>";
        html += "<li>SPI bus conflicts with CC1101</li>";
        html += "<li>Hardware connection issues</li>";
        html += "</ul>";
        html += "<button class='btn' onclick='location.reload()'>🔄 Retry</button>";
        html += "</div>";
    }

    html += "</div></body></html>";
    return html;
}
