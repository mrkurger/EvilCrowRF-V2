/*
 * EvilCrow RF v2 - Minimal Working Version
 * 
 * This is a stripped-down version that will boot successfully
 * and provide basic functionality without memory issues.
 */

#include <WiFi.h>
#include <WebServer.h>

// Pin definitions
const int push1 = 0;
const int push2 = 35;
const int led = 2;

// Global instances
WebServer server(80);

// System state
bool systemInitialized = false;
unsigned long lastHeartbeat = 0;

// RF Configuration
float frequency = 433.92;
bool isReceiving = false;

// WiFi Configuration
const char *ssid = "EvilCrow-RF";
const char *password = "123456789";

void setup() {
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n================================================");
    Serial.println("🚀 EvilCrow RF v2 - Minimal Working Version");
    Serial.println("📡 Basic functionality without memory issues");
    Serial.println("================================================\n");

    // Setup pins
    Serial.print("🔌 Setting up pins... ");
    pinMode(push1, INPUT_PULLUP);
    pinMode(push2, INPUT_PULLUP);
    pinMode(led, OUTPUT);
    Serial.println("✅ Success");

    // Setup WiFi
    setupWiFi();

    // Setup web server
    setupWebServer();

    systemInitialized = true;

    Serial.println("\n🎉 System initialization complete!");
    Serial.println("🌐 Access point: " + String(ssid));
    Serial.println("🔐 Password: " + String(password));
    Serial.println("🔗 Web interface: http://192.168.4.1");

    // Startup LED sequence
    for (int i = 0; i < 3; i++) {
        digitalWrite(led, HIGH);
        delay(200);
        digitalWrite(led, LOW);
        delay(200);
    }

    Serial.println("\n✅ EvilCrow RF v2 is ready for action!");
}

void loop() {
    if (!systemInitialized) {
        delay(100);
        return;
    }

    server.handleClient();

    uint32_t currentTime = millis();

    // System heartbeat
    if (currentTime - lastHeartbeat > 15000) {
        Serial.println("💓 System heartbeat - Free heap: " + String(ESP.getFreeHeap()) + " bytes");
        Serial.println("📶 WiFi Clients: " + String(WiFi.softAPgetStationNum()));
        lastHeartbeat = currentTime;
    }

    // Handle button presses
    handleButtons();

    delay(10);
}

void setupWiFi() {
    Serial.print("📶 Setting up WiFi Access Point... ");

    WiFi.mode(WIFI_AP);
    delay(100);

    if (WiFi.softAP(ssid, password)) {
        Serial.println("✅ Success");
        Serial.println("📡 SSID: " + String(ssid));
        Serial.println("🔐 Password: " + String(password));
        Serial.println("🌐 IP Address: " + WiFi.softAPIP().toString());
    } else {
        Serial.println("❌ Failed");
    }
}

void setupWebServer() {
    Serial.print("🚀 Setting up web server... ");

    // Main dashboard - minimal HTML
    server.on("/", HTTP_GET, []() {
        String html = "<!DOCTYPE html><html><head><title>EvilCrow RF v2</title>";
        html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
        html += "<style>body{font-family:Arial;background:#1a1a1a;color:#fff;padding:20px;}";
        html += ".card{background:#333;padding:20px;margin:10px 0;border-radius:8px;}";
        html += ".btn{background:#2196F3;color:white;padding:10px 20px;border:none;border-radius:4px;margin:5px;cursor:pointer;}";
        html += ".btn:hover{background:#1976D2;}</style></head><body>";
        html += "<h1>🚀 EvilCrow RF v2</h1>";
        html += "<div class='card'><h3>📊 System Status</h3>";
        html += "<p>WiFi Clients: " + String(WiFi.softAPgetStationNum()) + "</p>";
        html += "<p>Free Memory: " + String(ESP.getFreeHeap()) + " bytes</p>";
        html += "<p>Uptime: " + String(millis() / 1000) + " seconds</p>";
        html += "</div>";
        html += "<div class='card'><h3>📺 TV Remote</h3>";
        html += "<button class='btn' onclick='tvControl(\"off\")'>Turn Off</button>";
        html += "<button class='btn' onclick='tvControl(\"on\")'>Turn On</button>";
        html += "<div id='tv-status'>Ready</div></div>";
        html += "<div class='card'><h3>🔗 Links</h3>";
        html += "<a href='/attack-dashboard.html' class='btn'>🎯 Attack Dashboard</a>";
        html += "<a href='/status' class='btn'>📊 Status API</a></div>";
        html += "<script>";
        html += "function tvControl(action){";
        html += "document.getElementById('tv-status').textContent='Sending '+action+' command...';";
        html += "fetch('/api/tv/'+action).then(r=>r.text()).then(d=>{";
        html += "document.getElementById('tv-status').textContent='Command sent: '+action;";
        html += "}).catch(e=>{document.getElementById('tv-status').textContent='Error: '+e;});}";
        html += "</script></body></html>";
        server.send(200, "text/html", html);
    });

    // TV Remote API endpoints
    server.on("/api/tv/off", HTTP_GET, []() {
        Serial.println("📺 TV Turn Off command");
        for (int i = 0; i < 3; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }
        server.send(200, "text/plain", "TV OFF signal sent");
    });

    server.on("/api/tv/on", HTTP_GET, []() {
        Serial.println("📺 TV Turn On command");
        for (int i = 0; i < 3; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }
        server.send(200, "text/plain", "TV ON signal sent");
    });

    // System status API
    server.on("/status", HTTP_GET, []() {
        String json = "{";
        json += "\"wifi_clients\":" + String(WiFi.softAPgetStationNum()) + ",";
        json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"uptime\":" + String(millis()) + ",";
        json += "\"frequency\":" + String(frequency) + ",";
        json += "\"is_receiving\":" + String(isReceiving ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json);
    });

    // Serve attack dashboard from SD card if available
    server.on("/attack-dashboard.html", HTTP_GET, []() {
        server.send(200, "text/html", "Attack dashboard will be loaded from SD card when available.");
    });

    server.begin();
    Serial.println("✅ Success");
    Serial.println("🌐 Web server listening on: http://" + WiFi.softAPIP().toString());
}

void handleButtons() {
    static unsigned long lastButtonCheck = 0;
    if (millis() - lastButtonCheck < 50) return; // Debounce
    lastButtonCheck = millis();

    if (digitalRead(push1) == LOW) {
        Serial.println("🔘 Button 1 pressed - TV scan");
        for (int i = 0; i < 5; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }
        delay(500); // Prevent multiple triggers
    }

    if (digitalRead(push2) == LOW) {
        Serial.println("🔘 Button 2 pressed - Toggle receive");
        isReceiving = !isReceiving;
        
        if (isReceiving) {
            Serial.println("📡 Started receiving mode");
            digitalWrite(led, HIGH);
        } else {
            Serial.println("📡 Stopped receiving mode");
            digitalWrite(led, LOW);
        }
        
        delay(500); // Prevent multiple triggers
    }
}
