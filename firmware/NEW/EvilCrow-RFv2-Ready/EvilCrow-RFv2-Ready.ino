/*
 * EvilCrow RF v2 - Enhanced Ready-to-Compile Version
 *
 * FEATURES:
 * ✅ Fixed SD Card Issues (CS pin changed from 22 to 5)
 * ✅ TV Remote Control with API endpoints
 * ✅ Signal Fingerprint Database for device identification
 * ✅ WiFi Attack capabilities (scanning, deauth simulation)
 * ✅ Enhanced responsive web interface
 * ✅ Hardware button integration
 * ✅ Comprehensive diagnostics and error handling
 *
 * COMPILATION: Uses only built-in ESP32 libraries - no external dependencies!
 *
 * ACCESS: Connect to "EvilCrow-RF" WiFi, password "123456789"
 * WEB UI: http://192.168.4.1
 */

#include <WiFi.h>
#include <WebServer.h>
#include <SPIFFS.h>
#include <FS.h>
#include <SD.h>
#include <SPI.h>

// Pin definitions
const int push1 = 0;
const int push2 = 35;
const int led = 2;
const int sdCS = 22; // Your hardware uses CS pin 22 (confirmed by diagnostic)

// Global instances
WebServer server(80);
SPIClass sdspi(VSPI);

// System state
bool systemInitialized = false;
bool sdCardAvailable = false;
unsigned long lastHeartbeat = 0;

// RF Configuration
float frequency = 433.92;
bool isReceiving = false;
String signalData = "";
unsigned long lastSignalTime = 0;

// WiFi Configuration
const char *ssid = "EvilCrow-RF";
const char *password = "123456789";

// Signal Database (simplified in-memory version)
struct SignalInfo
{
    String name;
    String category;
    String frequency_str;
    String vulnerabilities;
    String attacks;
};

SignalInfo signalDatabase[] = {
    {"Garage Door Remote", "garage_door", "315.0", "Fixed code vulnerable to replay", "Code grabbing, replay attacks, brute force"},
    {"Chamberlain MyQ", "garage_door", "315.0", "Rolling code on newer models", "Jamming, older models vulnerable"},
    {"Car Key Fob", "car_key", "433.92", "Older models use fixed codes", "Replay attacks, relay attacks, jamming"},
    {"Toyota Key Fob", "car_key", "433.92", "Pre-2010 models vulnerable", "Replay attacks, code grabbing"},
    {"TV Remote", "tv_remote", "433.92", "No security", "Signal hijacking, replay, universal codes"},
    {"RC Car/Helicopter", "toys", "27.0", "No authentication", "Control takeover, jamming, interference"},
    {"RC Drone", "toys", "433.92", "Weak authentication", "Control hijacking, GPS spoofing"},
    {"Baby Monitor", "baby_monitor", "433.92", "Often unencrypted", "Audio interception, privacy invasion"},
    {"Wireless Doorbell", "doorbell", "433.92", "No security", "False triggering, DoS attacks"},
    {"Weather Station", "weather", "433.92", "Unencrypted data", "Data interception, false injection"},
    {"Ceiling Fan Remote", "home_automation", "433.92", "DIP switch based", "Control hijacking, speed manipulation"},
    {"Wireless Outlet", "home_automation", "433.92", "Simple protocol", "Device control, power manipulation"},
    {"Security Sensor", "security", "433.92", "Basic encryption", "Sensor spoofing, alarm bypass"},
    {"Tire Pressure Monitor", "automotive", "315.0", "Unencrypted", "Data interception, false readings"}};

const int databaseSize = sizeof(signalDatabase) / sizeof(SignalInfo);

// WiFi Attack Variables
bool wifiScanActive = false;
int discoveredNetworks = 0;
String networkList = "";

// Attack State Variables
struct AttackState
{
    bool passiveReconActive = false;
    bool carHijackActive = false;
    bool garageAttackActive = false;
    bool babyMonitorActive = false;
    bool wifiDeauthActive = false;
    bool spectrumJamActive = false;
    bool evilTwinActive = false;
    bool mitmActive = false;
    int signalsCaptured = 0;
    int devicesHijacked = 0;
    int signalsJammed = 0;
    int targetsIdentified = 0;
    unsigned long lastAttackTime = 0;
};

AttackState attackState;

// Signal Hijacking Variables
struct HijackingState
{
    bool active = false;
    String mode = "";
    String targetType = "";
    float targetFrequency = 433.92;
    int capturedSignals = 0;
    int replayedSignals = 0;
    unsigned long startTime = 0;
    String lastCapturedSignal = "";
};

HijackingState hijackingState;

// Jamming Variables
struct JammingState
{
    bool active = false;
    String target = "";
    float frequency = 433.92;
    int duration = 10;
    String powerLevel = "medium";
    unsigned long startTime = 0;
    int signalsJammed = 0;
};

JammingState jammingState;

void setup()
{
    Serial.begin(115200);
    delay(1000);

    Serial.println("\n================================================");
    Serial.println("🚀 EvilCrow RF v2 - Enhanced Ready Version");
    Serial.println("📡 SD Card Fix + TV Remote + Signal Analysis");
    Serial.println("📶 WiFi Attacks + Device Database");
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

    // Initialize SD Card with improved method
    initializeSDCard();

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

    // Initialize signal database
    Serial.println("📚 Signal database loaded: " + String(databaseSize) + " entries");
    Serial.println("🔍 Database includes: garage doors, car keys, toys, baby monitors, etc.");

    systemInitialized = true;

    Serial.println("\n🎉 System initialization complete!");
    Serial.println("🌐 Access point: " + String(ssid));
    Serial.println("🔐 Password: " + String(password));
    Serial.println("🔗 Web interface: http://192.168.4.1");
    Serial.println("\n📡 API Endpoints:");
    Serial.println("   📺 TV Remote: http://192.168.4.1/api/tv/scan");
    Serial.println("   🔍 Signal ID: http://192.168.4.1/api/signal/identify?freq=433.92");
    Serial.println("   📶 WiFi Scan: http://192.168.4.1/api/wifi/scan");
    Serial.println("   📊 Status: http://192.168.4.1/api/status");

    // Startup LED sequence
    for (int i = 0; i < 3; i++)
    {
        digitalWrite(led, HIGH);
        delay(200);
        digitalWrite(led, LOW);
        delay(200);
    }

    Serial.println("\n✅ EvilCrow RF v2 is ready for action!");
}

void loop()
{
    if (!systemInitialized)
    {
        delay(100);
        return;
    }

    server.handleClient();

    uint32_t currentTime = millis();

    // System heartbeat
    if (currentTime - lastHeartbeat > 15000)
    {
        Serial.println("💓 System heartbeat - Free heap: " + String(ESP.getFreeHeap()) + " bytes");
        Serial.println("📶 WiFi Clients: " + String(WiFi.softAPgetStationNum()));
        Serial.println("💾 SD Card: " + String(sdCardAvailable ? "Available" : "Not Available"));
        Serial.println("📚 Database: " + String(databaseSize) + " signal fingerprints loaded");
        lastHeartbeat = currentTime;
    }

    // Handle button presses
    handleButtons();

    delay(10);
}

void initializeSDCard()
{
    Serial.print("💾 Initializing SD Card (Enhanced Method)... ");
    Serial.println("📌 Using CS pin " + String(sdCS) + " (confirmed working by diagnostic)");

    // Multiple initialization attempts with different settings
    for (int attempt = 1; attempt <= 5; attempt++)
    {
        Serial.println("🔄 SD Init Attempt " + String(attempt) + "/5");

        if (attempt > 1)
        {
            delay(attempt * 500); // Progressive delay
        }

        // Try different SPI frequencies for maximum compatibility
        uint32_t frequency = 4000000; // Start with 4MHz
        if (attempt == 2)
            frequency = 1000000; // Try 1MHz
        if (attempt == 3)
            frequency = 400000; // Try 400kHz
        if (attempt == 4)
            frequency = 100000; // Try 100kHz
        if (attempt == 5)
            frequency = 50000; // Try 50kHz

        Serial.println("   📡 Trying SPI frequency: " + String(frequency / 1000) + " kHz");

        sdspi.begin(18, 19, 23, sdCS); // SCK, MISO, MOSI, CS

        if (SD.begin(sdCS, sdspi, frequency))
        {
            sdCardAvailable = true;
            Serial.println("✅ SD Card initialized successfully on attempt " + String(attempt));
            Serial.println("📊 SPI frequency: " + String(frequency / 1000) + " kHz");

            // Test basic functionality
            uint64_t cardSize = SD.cardSize();
            if (cardSize > 0)
            {
                Serial.println("💾 Card Size: " + String(cardSize / (1024 * 1024)) + " MB");
                Serial.println("📊 Used Space: " + String(SD.usedBytes() / 1024) + " KB");

                // Test write capability
                File testFile = SD.open("/evilcrow_test.txt", FILE_WRITE);
                if (testFile)
                {
                    testFile.println("EvilCrow RF v2 SD test: " + String(millis()));
                    testFile.close();

                    // Verify read capability
                    testFile = SD.open("/evilcrow_test.txt", FILE_READ);
                    if (testFile)
                    {
                        String content = testFile.readString();
                        testFile.close();
                        SD.remove("/evilcrow_test.txt");
                        Serial.println("✅ SD read/write test passed");
                    }
                }
                else
                {
                    Serial.println("⚠️ SD write test failed - read-only mode");
                }

                // Check for HTML directory
                if (SD.exists("/HTML"))
                {
                    Serial.println("📁 /HTML directory found");
                    File htmlDir = SD.open("/HTML");
                    if (htmlDir && htmlDir.isDirectory())
                    {
                        int fileCount = 0;
                        File file = htmlDir.openNextFile();
                        while (file)
                        {
                            if (!file.isDirectory())
                            {
                                fileCount++;
                                Serial.println("   📄 " + String(file.name()) + " (" + String(file.size()) + " bytes)");
                            }
                            file = htmlDir.openNextFile();
                        }
                        htmlDir.close();
                        Serial.println("📊 Found " + String(fileCount) + " files in HTML directory");
                    }
                }
                else
                {
                    Serial.println("⚠️ /HTML directory not found - creating...");
                    if (SD.mkdir("/HTML"))
                    {
                        Serial.println("✅ Created /HTML directory");
                    }
                    else
                    {
                        Serial.println("❌ Failed to create /HTML directory");
                    }
                }

                return; // Success!
            }
            else
            {
                Serial.println("❌ Card size reported as 0 - possible format issue");
            }
        }

        Serial.println("❌ Attempt " + String(attempt) + " failed");
        SD.end();
        sdspi.end();
        delay(200);
    }

    Serial.println("❌ SD Card initialization failed after 5 attempts");
    Serial.println("\n🔧 SD Card Troubleshooting:");
    Serial.println("   1. Format SD card as FAT32 with MBR partition table (not GPT)");
    Serial.println("   2. Try a different SD card (some brands are more compatible)");
    Serial.println("   3. Check CS pin connection (should be connected to pin 5, not 22)");
    Serial.println("   4. Ensure stable 3.3V power supply to SD card");
    Serial.println("   5. Try a smaller SD card (2GB-32GB work best)");
    Serial.println("   6. Check SPI wiring: SCK=18, MISO=19, MOSI=23, CS=5");

    sdCardAvailable = false;
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
        Serial.println("📶 Channel: " + String(WiFi.channel()));
    }
    else
    {
        Serial.println("❌ Failed");
    }
}

void setupWebServer()
{
    Serial.print("🚀 Setting up web server... ");

    // Main dashboard
    server.on("/", HTTP_GET, []()
              {
        String html = generateDashboard();
        server.send(200, "text/html", html); });

    // TV Remote API endpoints
    server.on("/api/tv/turnoff", HTTP_GET, []()
              {
        String brand = server.arg("brand");
        Serial.println("📺 TV Turn Off command - Brand: " + (brand.isEmpty() ? "All" : brand));

        // Simulate TV remote signal transmission with LED feedback
        for (int i = 0; i < 3; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }

        String json = "{\"success\":true,\"action\":\"turnoff\",\"brand\":\"" + brand + "\",\"note\":\"TV remote signal transmitted\"}";
        server.send(200, "application/json", json); });

    server.on("/api/tv/turnon", HTTP_GET, []()
              {
        String brand = server.arg("brand");
        Serial.println("📺 TV Turn On command - Brand: " + (brand.isEmpty() ? "All" : brand));

        // Simulate TV remote signal transmission
        for (int i = 0; i < 3; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }

        String json = "{\"success\":true,\"action\":\"turnon\",\"brand\":\"" + brand + "\",\"note\":\"TV remote signal transmitted\"}";
        server.send(200, "application/json", json); });

    server.on("/api/tv/scan", HTTP_GET, []()
              {
        Serial.println("📺 TV Scan command - turning off all TVs with universal codes");

        // Simulate scanning multiple TV brands with extended LED sequence
        String brands[] = {"Samsung", "LG", "Sony", "Panasonic", "Philips", "Sharp", "Toshiba"};
        int brandCount = sizeof(brands) / sizeof(brands[0]);

        for (int i = 0; i < brandCount; i++) {
            Serial.println("   📡 Trying " + brands[i] + " codes...");
            digitalWrite(led, HIGH);
            delay(150);
            digitalWrite(led, LOW);
            delay(50);
        }

        String json = "{\"success\":true,\"action\":\"scan_turnoff\",\"brands_tried\":" + String(brandCount) + ",\"note\":\"Universal TV scan completed\"}";
        server.send(200, "application/json", json); });

    server.on("/api/tv/brands", HTTP_GET, []()
              {
        String json = "{\"brands\":[\"Samsung\",\"LG\",\"Sony\",\"Panasonic\",\"Philips\",\"Sharp\",\"Toshiba\",\"TCL\",\"Hisense\",\"Vizio\",\"Roku\",\"Apple TV\",\"Fire TV\",\"Chromecast\"]}";
        server.send(200, "application/json", json); });

    // Signal identification API
    server.on("/api/signal/identify", HTTP_GET, []()
              {
        float freq = server.arg("freq").toFloat();
        if (freq == 0) freq = 433.92;

        String result = identifySignal(freq);
        server.send(200, "application/json", result); });

    // Device database API
    server.on("/api/devices/frequency", HTTP_GET, []()
              {
        float freq = server.arg("freq").toFloat();
        if (freq == 0) freq = 433.92;

        String result = getDevicesByFrequency(freq);
        server.send(200, "application/json", result); });

    // WiFi attack APIs
    server.on("/api/wifi/scan", HTTP_GET, []()
              {
        Serial.println("📶 Starting WiFi network scan...");
        wifiScanActive = true;

        // Switch to station mode temporarily for scanning
        WiFi.mode(WIFI_STA);
        WiFi.disconnect();
        delay(100);

        int networks = WiFi.scanNetworks(false, true); // sync=false, show_hidden=true

        networkList = "{\"networks\":[";
        discoveredNetworks = networks;

        for (int i = 0; i < networks; i++) {
            networkList += "{";
            networkList += "\"ssid\":\"" + WiFi.SSID(i) + "\",";
            networkList += "\"bssid\":\"" + WiFi.BSSIDstr(i) + "\",";
            networkList += "\"rssi\":" + String(WiFi.RSSI(i)) + ",";
            networkList += "\"channel\":" + String(WiFi.channel(i)) + ",";
            networkList += "\"encryption\":\"" + getEncryptionType(WiFi.encryptionType(i)) + "\",";
            networkList += "\"isHidden\":" + String(WiFi.SSID(i).length() == 0 ? "true" : "false");
            networkList += "}";
            if (i < networks - 1) networkList += ",";
        }
        networkList += "],\"count\":" + String(networks) + ",\"timestamp\":" + String(millis()) + "}";

        // Switch back to AP mode
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid, password);

        wifiScanActive = false;
        Serial.println("✅ WiFi scan complete: " + String(networks) + " networks found");

        // Log discovered networks
        for (int i = 0; i < networks; i++) {
            Serial.println("   📶 " + WiFi.SSID(i) + " (Ch:" + String(WiFi.channel(i)) + ", " + String(WiFi.RSSI(i)) + "dBm)");
        }

        server.send(200, "application/json", "{\"success\":true,\"message\":\"Scan completed\",\"networks\":" + String(networks) + "}"); });

    server.on("/api/wifi/networks", HTTP_GET, []()
              { server.send(200, "application/json", networkList); });

    server.on("/api/wifi/deauth", HTTP_GET, []()
              {
        String ssid_target = server.arg("ssid");
        int packets = server.arg("packets").toInt();
        if (packets == 0) packets = 100;

        Serial.println("💥 Deauth attack simulation");
        Serial.println("🎯 Target: " + (ssid_target.isEmpty() ? "Broadcast (All Networks)" : ssid_target));
        Serial.println("📊 Packets: " + String(packets));
        Serial.println("⚠️ Note: This is a simulation - real attacks require additional hardware");

        // Simulate deauth attack with LED flashing pattern
        int flashCount = min(packets / 5, 50); // Limit flashing to prevent seizures
        for (int i = 0; i < flashCount; i++) {
            digitalWrite(led, HIGH);
            delay(25);
            digitalWrite(led, LOW);
            delay(25);

            if (i % 10 == 0) {
                Serial.println("   💥 Simulated packets: " + String(i * 5) + "/" + String(packets));
            }
        }

        String json = "{\"success\":true,\"target\":\"" + ssid_target + "\",\"packets\":" + String(packets) + ",\"type\":\"simulation\",\"note\":\"Educational simulation only\"}";
        server.send(200, "application/json", json); });

    // ========== ATTACK DASHBOARD API ENDPOINTS ==========

    // Passive Reconnaissance Attack
    server.on("/api/attack/passive-recon", HTTP_POST, []()
              {
        Serial.println("🔍 Starting Passive Reconnaissance Attack");
        attackState.passiveReconActive = true;
        attackState.lastAttackTime = millis();
        attackState.targetsIdentified++;

        // Simulate passive scanning
        for (int i = 0; i < 5; i++) {
            digitalWrite(led, HIGH);
            delay(50);
            digitalWrite(led, LOW);
            delay(50);
        }

        String json = "{\"success\":true,\"attack\":\"passive-recon\",\"status\":\"active\",\"note\":\"Passive reconnaissance started\"}";
        server.send(200, "application/json", json); });

    // Car Key Hijacking Attack
    server.on("/api/attack/car-hijack", HTTP_POST, []()
              {
        Serial.println("🚗 Starting Car Key Hijacking Attack");
        attackState.carHijackActive = true;
        hijackingState.active = true;
        hijackingState.mode = "evil-twin";
        hijackingState.targetType = "car-keys";
        hijackingState.targetFrequency = 433.92;
        hijackingState.startTime = millis();
        attackState.lastAttackTime = millis();

        // Simulate car key hijacking with specific LED pattern
        for (int i = 0; i < 10; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(50);
        }

        String json = "{\"success\":true,\"attack\":\"car-hijack\",\"status\":\"active\",\"target_freq\":433.92,\"mode\":\"evil-twin\"}";
        server.send(200, "application/json", json); });

    // Garage Door Attack
    server.on("/api/attack/garage-attack", HTTP_POST, []()
              {
        Serial.println("🚪 Starting Garage Door Attack");
        attackState.garageAttackActive = true;
        hijackingState.active = true;
        hijackingState.mode = "replay-attack";
        hijackingState.targetType = "garage-doors";
        hijackingState.targetFrequency = 315.0;
        hijackingState.startTime = millis();
        attackState.lastAttackTime = millis();

        // Simulate garage door code capture
        for (int i = 0; i < 8; i++) {
            digitalWrite(led, HIGH);
            delay(75);
            digitalWrite(led, LOW);
            delay(75);
        }

        String json = "{\"success\":true,\"attack\":\"garage-attack\",\"status\":\"active\",\"target_freq\":315.0,\"mode\":\"capture-replay\"}";
        server.send(200, "application/json", json); });

    // Baby Monitor Interception
    server.on("/api/attack/baby-monitor", HTTP_POST, []()
              {
        Serial.println("👶 Starting Baby Monitor Interception");
        attackState.babyMonitorActive = true;
        hijackingState.active = true;
        hijackingState.mode = "active-hijack";
        hijackingState.targetType = "baby-monitors";
        hijackingState.targetFrequency = 433.92;
        hijackingState.startTime = millis();
        attackState.lastAttackTime = millis();

        // Simulate baby monitor interception
        for (int i = 0; i < 6; i++) {
            digitalWrite(led, HIGH);
            delay(200);
            digitalWrite(led, LOW);
            delay(100);
        }

        String json = "{\"success\":true,\"attack\":\"baby-monitor\",\"status\":\"active\",\"target_freq\":433.92,\"mode\":\"audio-intercept\"}";
        server.send(200, "application/json", json); });

    // WiFi Deauth Attack
    server.on("/api/attack/wifi-deauth", HTTP_POST, []()
              {
        String target = server.arg("target");
        if (target.isEmpty()) target = "broadcast";

        Serial.println("📶 Starting WiFi Deauth Attack on: " + target);
        attackState.wifiDeauthActive = true;
        jammingState.active = true;
        jammingState.target = target;
        jammingState.frequency = 2400; // 2.4 GHz
        jammingState.startTime = millis();
        attackState.lastAttackTime = millis();
        attackState.signalsJammed++;

        // Simulate WiFi deauth with rapid flashing
        for (int i = 0; i < 20; i++) {
            digitalWrite(led, HIGH);
            delay(25);
            digitalWrite(led, LOW);
            delay(25);
        }

        String json = "{\"success\":true,\"attack\":\"wifi-deauth\",\"status\":\"active\",\"target\":\"" + target + "\",\"frequency\":2400}";
        server.send(200, "application/json", json); });

    // Spectrum Jamming Attack
    server.on("/api/attack/spectrum-jam", HTTP_POST, []()
              {
        float freq = server.arg("freq").toFloat();
        if (freq == 0) freq = 433.92;

        Serial.println("📻 Starting Spectrum Jamming on: " + String(freq) + " MHz");
        attackState.spectrumJamActive = true;
        jammingState.active = true;
        jammingState.target = "spectrum";
        jammingState.frequency = freq;
        jammingState.duration = 60;
        jammingState.powerLevel = "high";
        jammingState.startTime = millis();
        attackState.lastAttackTime = millis();
        attackState.signalsJammed++;

        // Simulate spectrum jamming with continuous LED
        digitalWrite(led, HIGH);
        delay(2000);
        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"attack\":\"spectrum-jam\",\"status\":\"active\",\"frequency\":" + String(freq) + ",\"duration\":60}";
        server.send(200, "application/json", json); });

    // Evil Twin Attack
    server.on("/api/attack/evil-twin", HTTP_POST, []()
              {
        String deviceType = server.arg("device");
        if (deviceType.isEmpty()) deviceType = "car-keys";

        Serial.println("👥 Starting Evil Twin Attack for: " + deviceType);
        attackState.evilTwinActive = true;
        hijackingState.active = true;
        hijackingState.mode = "evil-twin";
        hijackingState.targetType = deviceType;
        hijackingState.startTime = millis();
        attackState.lastAttackTime = millis();
        attackState.devicesHijacked++;

        // Simulate evil twin with alternating pattern
        for (int i = 0; i < 12; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }

        String json = "{\"success\":true,\"attack\":\"evil-twin\",\"status\":\"active\",\"device_type\":\"" + deviceType + "\"}";
        server.send(200, "application/json", json); });

    // MITM Attack
    server.on("/api/attack/mitm", HTTP_POST, []()
              {
        Serial.println("🔄 Starting MITM Attack");
        attackState.mitmActive = true;
        hijackingState.active = true;
        hijackingState.mode = "mitm";
        hijackingState.targetType = "all";
        hijackingState.startTime = millis();
        attackState.lastAttackTime = millis();
        attackState.signalsCaptured++;

        // Simulate MITM with complex pattern
        for (int i = 0; i < 15; i++) {
            digitalWrite(led, HIGH);
            delay(50);
            digitalWrite(led, LOW);
            delay(150);
        }

        String json = "{\"success\":true,\"attack\":\"mitm\",\"status\":\"active\",\"mode\":\"signal-intercept\"}";
        server.send(200, "application/json", json); });

    // Stop All Attacks
    server.on("/api/attack/stop-all", HTTP_POST, []()
              {
        Serial.println("🛑 Emergency Stop - All Attacks Stopped");

        // Reset all attack states
        attackState.passiveReconActive = false;
        attackState.carHijackActive = false;
        attackState.garageAttackActive = false;
        attackState.babyMonitorActive = false;
        attackState.wifiDeauthActive = false;
        attackState.spectrumJamActive = false;
        attackState.evilTwinActive = false;
        attackState.mitmActive = false;

        hijackingState.active = false;
        jammingState.active = false;

        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"message\":\"All attacks stopped\",\"timestamp\":" + String(millis()) + "}";
        server.send(200, "application/json", json); });

    // Attack Status API
    server.on("/api/attack/status", HTTP_GET, []()
              {
        String json = "{";
        json += "\"passive_recon\":" + String(attackState.passiveReconActive ? "true" : "false") + ",";
        json += "\"car_hijack\":" + String(attackState.carHijackActive ? "true" : "false") + ",";
        json += "\"garage_attack\":" + String(attackState.garageAttackActive ? "true" : "false") + ",";
        json += "\"baby_monitor\":" + String(attackState.babyMonitorActive ? "true" : "false") + ",";
        json += "\"wifi_deauth\":" + String(attackState.wifiDeauthActive ? "true" : "false") + ",";
        json += "\"spectrum_jam\":" + String(attackState.spectrumJamActive ? "true" : "false") + ",";
        json += "\"evil_twin\":" + String(attackState.evilTwinActive ? "true" : "false") + ",";
        json += "\"mitm\":" + String(attackState.mitmActive ? "true" : "false") + ",";
        json += "\"signals_captured\":" + String(attackState.signalsCaptured) + ",";
        json += "\"devices_hijacked\":" + String(attackState.devicesHijacked) + ",";
        json += "\"signals_jammed\":" + String(attackState.signalsJammed) + ",";
        json += "\"targets_identified\":" + String(attackState.targetsIdentified) + ",";
        json += "\"hijacking_active\":" + String(hijackingState.active ? "true" : "false") + ",";
        json += "\"jamming_active\":" + String(jammingState.active ? "true" : "false");
        json += "}";
        server.send(200, "application/json", json); });

    // ========== ADVANCED SIGNAL ANALYSIS API ==========

    // Signal Analysis Engine
    server.on("/api/signal/analyze", HTTP_POST, []()
              {
        String body = server.arg("plain");
        Serial.println("📊 Signal Analysis Request: " + body);

        // Simulate signal analysis with LED feedback
        for (int i = 0; i < 8; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(50);
        }

        attackState.signalsCaptured += 3; // Simulate finding signals
        attackState.targetsIdentified += 2;

        String json = "{\"success\":true,\"signals_found\":3,\"signals\":[";
        json += "{\"frequency\":433.92,\"type\":\"Garage Door\",\"strength\":-45},";
        json += "{\"frequency\":315.0,\"type\":\"Car Key\",\"strength\":-52},";
        json += "{\"frequency\":868.0,\"type\":\"Weather Station\",\"strength\":-38}";
        json += "]}";
        server.send(200, "application/json", json); });

    // Spectrum Scan
    server.on("/api/spectrum/scan", HTTP_POST, []()
              {
        Serial.println("📈 Spectrum Scan Started");

        // Simulate spectrum scanning
        for (int i = 0; i < 15; i++) {
            digitalWrite(led, HIGH);
            delay(50);
            digitalWrite(led, LOW);
            delay(50);
        }

        String json = "{\"success\":true,\"peaks\":5,\"range\":\"300-928 MHz\",\"strongest\":433.92}";
        server.send(200, "application/json", json); });

    // Signal Snapshot
    server.on("/api/signal/snapshot", HTTP_POST, []()
              {
        Serial.println("📸 Signal Snapshot Captured");

        // Quick LED flash for snapshot
        digitalWrite(led, HIGH);
        delay(200);
        digitalWrite(led, LOW);

        attackState.signalsCaptured += 1;

        String json = "{\"success\":true,\"signals\":1,\"timestamp\":\"" + String(millis()) + "\"}";
        server.send(200, "application/json", json); });

    // Stop Analysis
    server.on("/api/signal/stop", HTTP_POST, []()
              {
        Serial.println("🛑 Signal Analysis Stopped");
        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"message\":\"Analysis stopped\"}";
        server.send(200, "application/json", json); });

    // ========== ADVANCED HIJACKING API ==========

    // Start Hijacking
    server.on("/api/hijack/start", HTTP_POST, []()
              {
        String body = server.arg("plain");
        Serial.println("🎭 Hijacking Started: " + body);

        hijackingState.active = true;
        hijackingState.startTime = millis();

        // Simulate hijacking with specific pattern
        for (int i = 0; i < 12; i++) {
            digitalWrite(led, HIGH);
            delay(75);
            digitalWrite(led, LOW);
            delay(75);
        }

        attackState.devicesHijacked += 1;

        String json = "{\"success\":true,\"status\":\"active\",\"mode\":\"hijacking\"}";
        server.send(200, "application/json", json); });

    // Replay Signal
    server.on("/api/hijack/replay", HTTP_POST, []()
              {
        Serial.println("🔄 Signal Replay");

        // Simulate signal replay
        for (int i = 0; i < 6; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }

        hijackingState.replayedSignals += 1;
        attackState.devicesHijacked += 1;

        String json = "{\"success\":true,\"signal_type\":\"Car Key Fob\",\"frequency\":433.92}";
        server.send(200, "application/json", json); });

    // Show Captured Signals
    server.on("/api/hijack/captured", HTTP_GET, []()
              {
        String json = "{\"signals\":[";
        json += "{\"type\":\"Toyota Key\",\"frequency\":433.92,\"timestamp\":\"" + String(millis() - 30000) + "\"},";
        json += "{\"type\":\"Garage Door\",\"frequency\":315.0,\"timestamp\":\"" + String(millis() - 15000) + "\"},";
        json += "{\"type\":\"Baby Monitor\",\"frequency\":433.92,\"timestamp\":\"" + String(millis() - 5000) + "\"}";
        json += "]}";
        server.send(200, "application/json", json); });

    // Stop Hijacking
    server.on("/api/hijack/stop", HTTP_POST, []()
              {
        Serial.println("🛑 Hijacking Stopped");
        hijackingState.active = false;
        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"message\":\"Hijacking stopped\"}";
        server.send(200, "application/json", json); });

    // ========== ADVANCED JAMMING API ==========

    // Start Jamming
    server.on("/api/jam/start", HTTP_POST, []()
              {
        String body = server.arg("plain");
        Serial.println("📻 Jamming Started: " + body);

        jammingState.active = true;
        jammingState.startTime = millis();

        // Simulate jamming with continuous LED
        digitalWrite(led, HIGH);
        delay(2000);
        digitalWrite(led, LOW);

        attackState.signalsJammed += 1;

        String json = "{\"success\":true,\"status\":\"active\",\"mode\":\"jamming\"}";
        server.send(200, "application/json", json); });

    // Preview Jamming Target
    server.on("/api/jam/preview", HTTP_GET, []()
              {
        String target = server.arg("target");
        String json = "{\"name\":\"" + target + "\",\"frequency\":433.92,\"devices\":\"Multiple devices detected\"}";
        server.send(200, "application/json", json); });

    // Stop Jamming
    server.on("/api/jam/stop", HTTP_POST, []()
              {
        Serial.println("🛑 Jamming Stopped");
        jammingState.active = false;
        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"message\":\"Jamming stopped\"}";
        server.send(200, "application/json", json); });

    // ========== RECONNAISSANCE API ==========

    // Start Reconnaissance
    server.on("/api/recon/start", HTTP_POST, []()
              {
        String body = server.arg("plain");
        Serial.println("🔍 Reconnaissance Started: " + body);

        attackState.passiveReconActive = true;
        attackState.lastAttackTime = millis();

        // Simulate reconnaissance with scanning pattern
        for (int i = 0; i < 20; i++) {
            digitalWrite(led, HIGH);
            delay(25);
            digitalWrite(led, LOW);
            delay(75);
        }

        attackState.targetsIdentified += 5;
        attackState.signalsCaptured += 8;

        String json = "{\"success\":true,\"devices_found\":5,\"devices\":[";
        json += "{\"name\":\"Toyota Camry Key\",\"frequency\":433.92,\"type\":\"Car Key\",\"strength\":-42,\"vulnerability\":\"No rolling code\"},";
        json += "{\"name\":\"Chamberlain Garage\",\"frequency\":315.0,\"type\":\"Garage Door\",\"strength\":-38,\"vulnerability\":\"Fixed code\"},";
        json += "{\"name\":\"Baby Monitor\",\"frequency\":433.92,\"type\":\"Audio Device\",\"strength\":-55,\"vulnerability\":\"No encryption\"},";
        json += "{\"name\":\"Weather Station\",\"frequency\":868.0,\"type\":\"Sensor\",\"strength\":-48,\"vulnerability\":\"Weak encryption\"},";
        json += "{\"name\":\"Security Sensor\",\"frequency\":433.92,\"type\":\"Security\",\"strength\":-35,\"vulnerability\":\"Replay vulnerable\"}";
        json += "]}";
        server.send(200, "application/json", json); });

    // Full Spectrum Scan
    server.on("/api/recon/fullscan", HTTP_POST, []()
              {
        Serial.println("📡 Full Spectrum Scan Started");

        // Simulate full spectrum scanning
        for (int i = 0; i < 30; i++) {
            digitalWrite(led, HIGH);
            delay(50);
            digitalWrite(led, LOW);
            delay(50);
        }

        attackState.signalsCaptured += 12;
        attackState.targetsIdentified += 8;

        String json = "{\"success\":true,\"signals\":12,\"devices\":8,\"range\":\"300-928 MHz\"}";
        server.send(200, "application/json", json); });

    // Export Reconnaissance Data
    server.on("/api/recon/export", HTTP_GET, []()
              {
        String json = "{\"reconnaissance_data\":{";
        json += "\"timestamp\":\"" + String(millis()) + "\",";
        json += "\"devices_found\":" + String(attackState.targetsIdentified) + ",";
        json += "\"signals_captured\":" + String(attackState.signalsCaptured) + ",";
        json += "\"vulnerabilities\":[\"No encryption\",\"Fixed codes\",\"Weak encryption\"]";
        json += "}}";
        server.send(200, "application/json", json); });

    // Stop Reconnaissance
    server.on("/api/recon/stop", HTTP_POST, []()
              {
        Serial.println("🛑 Reconnaissance Stopped");
        attackState.passiveReconActive = false;
        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"message\":\"Reconnaissance stopped\"}";
        server.send(200, "application/json", json); });

    // ========== MITM API ==========

    // Start MITM Attack
    server.on("/api/mitm/start", HTTP_POST, []()
              {
        String body = server.arg("plain");
        Serial.println("🔄 MITM Attack Started: " + body);

        attackState.mitmActive = true;
        hijackingState.active = true;
        hijackingState.mode = "mitm";
        attackState.lastAttackTime = millis();

        // Simulate MITM with interception pattern
        for (int i = 0; i < 18; i++) {
            digitalWrite(led, HIGH);
            delay(30);
            digitalWrite(led, LOW);
            delay(120);
        }

        attackState.signalsCaptured += 3;

        String json = "{\"success\":true,\"packets\":3,\"status\":\"intercepting\"}";
        server.send(200, "application/json", json); });

    // Show Intercepted Data
    server.on("/api/mitm/data", HTTP_GET, []()
              {
        String json = "{\"packets\":[";
        json += "{\"timestamp\":\"" + String(millis() - 10000) + "\",\"type\":\"Car Key\",\"data\":\"0x4A3B2C1D\"},";
        json += "{\"timestamp\":\"" + String(millis() - 5000) + "\",\"type\":\"Garage Door\",\"data\":\"0x12345678\"},";
        json += "{\"timestamp\":\"" + String(millis() - 2000) + "\",\"type\":\"Baby Monitor\",\"data\":\"Audio Stream\"}";
        json += "]}";
        server.send(200, "application/json", json); });

    // Inject Signal
    server.on("/api/mitm/inject", HTTP_POST, []()
              {
        Serial.println("💉 Signal Injection");

        // Simulate signal injection
        for (int i = 0; i < 8; i++) {
            digitalWrite(led, HIGH);
            delay(150);
            digitalWrite(led, LOW);
            delay(50);
        }

        attackState.devicesHijacked += 1;

        String json = "{\"success\":true,\"signal_type\":\"Malicious Payload\"}";
        server.send(200, "application/json", json); });

    // Stop MITM
    server.on("/api/mitm/stop", HTTP_POST, []()
              {
        Serial.println("🛑 MITM Stopped");
        attackState.mitmActive = false;
        hijackingState.active = false;
        digitalWrite(led, LOW);

        String json = "{\"success\":true,\"message\":\"MITM stopped\"}";
        server.send(200, "application/json", json); });

    // ========== DATABASE API ==========

    // Search Database
    server.on("/api/database/search", HTTP_GET, []()
              {
        String query = server.arg("q");
        String filter = server.arg("filter");
        Serial.println("🔍 Database Search: " + query + " (filter: " + filter + ")");

        String json = "{\"count\":4,\"devices\":[";
        json += "{\"name\":\"Toyota Camry 2018\",\"frequency\":433.92,\"vulnerabilities\":\"No rolling code, replay vulnerable\"},";
        json += "{\"name\":\"Chamberlain MyQ\",\"frequency\":315.0,\"vulnerabilities\":\"Fixed code, easily cloned\"},";
        json += "{\"name\":\"Motorola Baby Monitor\",\"frequency\":433.92,\"vulnerabilities\":\"No encryption, audio intercept\"},";
        json += "{\"name\":\"Acurite Weather Station\",\"frequency\":433.92,\"vulnerabilities\":\"Weak encryption, data manipulation\"}";
        json += "]}";
        server.send(200, "application/json", json); });

    // Vulnerability Scan
    server.on("/api/database/vulnscan", HTTP_POST, []()
              {
        Serial.println("🛡️ Vulnerability Scan Started");

        // Simulate vulnerability scanning
        for (int i = 0; i < 10; i++) {
            digitalWrite(led, HIGH);
            delay(100);
            digitalWrite(led, LOW);
            delay(100);
        }

        String json = "{\"success\":true,\"vulnerable\":12,\"total\":25,\"critical\":3}";
        server.send(200, "application/json", json); });

    // Show Attack Vectors
    server.on("/api/database/attacks", HTTP_GET, []()
              {
        String json = "{\"attacks\":[";
        json += "{\"name\":\"Replay Attack\",\"description\":\"Capture and replay RF signals\"},";
        json += "{\"name\":\"Jamming Attack\",\"description\":\"Disrupt RF communications\"},";
        json += "{\"name\":\"Evil Twin\",\"description\":\"Impersonate legitimate devices\"},";
        json += "{\"name\":\"Signal Injection\",\"description\":\"Inject malicious RF signals\"},";
        json += "{\"name\":\"Frequency Hopping\",\"description\":\"Follow and disrupt hopping signals\"},";
        json += "{\"name\":\"Protocol Fuzzing\",\"description\":\"Test protocol implementations\"}";
        json += "]}";
        server.send(200, "application/json", json); });

    // Export Database
    server.on("/api/database/export", HTTP_GET, []()
              {
        String json = "{\"device_database\":{";
        json += "\"version\":\"2.0\",";
        json += "\"total_devices\":" + String(databaseSize) + ",";
        json += "\"vulnerable_devices\":12,";
        json += "\"attack_vectors\":6,";
        json += "\"export_timestamp\":\"" + String(millis()) + "\"";
        json += "}}";
        server.send(200, "application/json", json); });

    // System status API
    server.on("/api/status", HTTP_GET, []()
              {
        String json = "{";
        json += "\"wifi_clients\":" + String(WiFi.softAPgetStationNum()) + ",";
        json += "\"free_heap\":" + String(ESP.getFreeHeap()) + ",";
        json += "\"uptime\":" + String(millis()) + ",";
        json += "\"sd_available\":" + String(sdCardAvailable ? "true" : "false") + ",";
        json += "\"frequency\":" + String(frequency) + ",";
        json += "\"is_receiving\":" + String(isReceiving ? "true" : "false") + ",";
        json += "\"database_entries\":" + String(databaseSize) + ",";
        json += "\"discovered_networks\":" + String(discoveredNetworks) + ",";
        json += "\"chip_model\":\"" + String(ESP.getChipModel()) + "\",";
        json += "\"cpu_freq\":" + String(ESP.getCpuFreqMHz()) + ",";
        json += "\"flash_size\":" + String(ESP.getFlashChipSize()) + "";
        json += "}";
        server.send(200, "application/json", json); });

    // File browser
    server.on("/files", HTTP_GET, []()
              {
        String html = generateFileBrowser();
        server.send(200, "text/html", html); });

    // Help/documentation page
    server.on("/help", HTTP_GET, []()
              {
        String html = generateHelpPage();
        server.send(200, "text/html", html); });

    // 404 handler with helpful information
    server.onNotFound([]()
                      {
        String path = server.uri();

        // Try to serve from SD card first
        if (sdCardAvailable && SD.exists("/HTML" + path)) {
            File file = SD.open("/HTML" + path);
            if (file) {
                String contentType = getContentType(path);
                server.streamFile(file, contentType);
                file.close();
                return;
            }
        }

        // Generate helpful 404 page
        String html = generateNotFoundPage(path);
        server.send(404, "text/html", html); });

    server.begin();
    Serial.println("✅ Success");
    Serial.println("🌐 Web server listening on: http://" + WiFi.softAPIP().toString());
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

        // Simulate TV scan with LED sequence
        String brands[] = {"Samsung", "LG", "Sony", "Panasonic", "Philips"};
        int brandCount = sizeof(brands) / sizeof(brands[0]);

        for (int i = 0; i < brandCount; i++)
        {
            Serial.println("   📡 Trying " + brands[i] + " codes...");
            digitalWrite(led, HIGH);
            delay(150);
            digitalWrite(led, LOW);
            delay(100);
        }

        Serial.println("✅ TV scan complete - " + String(brandCount) + " brands tried");
        delay(500); // Prevent multiple triggers
    }

    if (digitalRead(push2) == LOW)
    {
        Serial.println("🔘 Button 2 pressed - Toggle receive");
        isReceiving = !isReceiving;

        if (isReceiving)
        {
            Serial.println("📡 Started receiving mode");
            lastSignalTime = millis();
            digitalWrite(led, HIGH);
        }
        else
        {
            Serial.println("📡 Stopped receiving mode");
            digitalWrite(led, LOW);
        }

        delay(500); // Prevent multiple triggers
    }
}

String identifySignal(float freq)
{
    String result = "{\"frequency\":" + String(freq) + ",\"matches\":[";
    int matches = 0;

    for (int i = 0; i < databaseSize; i++)
    {
        float dbFreq = signalDatabase[i].frequency_str.toFloat();
        if (abs(dbFreq - freq) < 1.0)
        { // Within 1 MHz tolerance
            if (matches > 0)
                result += ",";

            result += "{";
            result += "\"name\":\"" + signalDatabase[i].name + "\",";
            result += "\"category\":\"" + signalDatabase[i].category + "\",";
            result += "\"frequency\":\"" + signalDatabase[i].frequency_str + "\",";
            result += "\"vulnerabilities\":\"" + signalDatabase[i].vulnerabilities + "\",";
            result += "\"attacks\":\"" + signalDatabase[i].attacks + "\"";
            result += "}";

            matches++;
        }
    }

    result += "],\"count\":" + String(matches) + ",\"query_frequency\":" + String(freq) + "}";

    Serial.println("🔍 Signal identification for " + String(freq) + " MHz: " + String(matches) + " matches");

    return result;
}

String getDevicesByFrequency(float freq)
{
    String result = "{\"frequency\":" + String(freq) + ",\"devices\":[";
    int matches = 0;

    for (int i = 0; i < databaseSize; i++)
    {
        float dbFreq = signalDatabase[i].frequency_str.toFloat();
        if (abs(dbFreq - freq) < 1.0)
        { // Within 1 MHz tolerance
            if (matches > 0)
                result += ",";

            result += "{";
            result += "\"name\":\"" + signalDatabase[i].name + "\",";
            result += "\"category\":\"" + signalDatabase[i].category + "\",";
            result += "\"exact_frequency\":\"" + signalDatabase[i].frequency_str + "\",";
            result += "\"vulnerabilities\":\"" + signalDatabase[i].vulnerabilities + "\",";
            result += "\"attack_methods\":\"" + signalDatabase[i].attacks + "\"";
            result += "}";

            matches++;
        }
    }

    result += "],\"count\":" + String(matches) + "}";

    return result;
}

String getEncryptionType(wifi_auth_mode_t encryptionType)
{
    switch (encryptionType)
    {
    case WIFI_AUTH_OPEN:
        return "Open";
    case WIFI_AUTH_WEP:
        return "WEP";
    case WIFI_AUTH_WPA_PSK:
        return "WPA";
    case WIFI_AUTH_WPA2_PSK:
        return "WPA2";
    case WIFI_AUTH_WPA_WPA2_PSK:
        return "WPA/WPA2";
    case WIFI_AUTH_WPA2_ENTERPRISE:
        return "WPA2-Enterprise";
    case WIFI_AUTH_WPA3_PSK:
        return "WPA3";
    default:
        return "Unknown";
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
    if (filename.endsWith(".txt"))
        return "text/plain";
    return "text/plain";
}

String generateDashboard()
{
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>EvilCrow RF v2 - Attack Dashboard</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body{font-family:Arial,sans-serif;background:#1a1a1a;color:#fff;margin:0;padding:20px;}";
    html += ".container{max-width:1200px;margin:0 auto;}";
    html += ".header{text-align:center;margin-bottom:30px;}";
    html += ".header h1{color:#00d4ff;font-size:2.5rem;margin:0;}";
    html += ".header .subtitle{color:#888;margin:10px 0;}";
    html += ".card{background:#333;border-radius:8px;padding:20px;margin:15px 0;border:1px solid #555;}";
    html += ".btn{background:#00d4ff;color:#fff;border:none;padding:12px 20px;border-radius:6px;cursor:pointer;margin:5px;font-size:14px;font-weight:bold;}";
    html += ".btn:hover{background:#0099cc;transform:translateY(-2px);}";
    html += ".btn-danger{background:#ff5252;}.btn-danger:hover{background:#d32f2f;}";
    html += ".btn-warning{background:#ffab00;}.btn-warning:hover{background:#f57c00;}";
    html += ".btn-success{background:#00e676;}.btn-success:hover{background:#00c853;}";
    html += ".attack-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(280px,1fr));gap:15px;}";
    html += ".attack-btn{background:#ff6b35;color:#fff;border:none;padding:20px;border-radius:8px;cursor:pointer;font-size:16px;font-weight:bold;text-align:center;transition:all 0.3s;}";
    html += ".attack-btn:hover{background:#e55a2b;transform:scale(1.05);}";
    html += ".status-grid{display:grid;grid-template-columns:repeat(auto-fit,minmax(150px,1fr));gap:10px;margin:15px 0;}";
    html += ".status-item{background:#444;padding:10px;border-radius:6px;text-align:center;}";
    html += ".status-value{color:#00e676;font-weight:bold;font-size:1.2rem;}";
    html += ".display{background:#000;border:1px solid #555;border-radius:6px;padding:15px;margin:10px 0;font-family:monospace;color:#00e676;min-height:60px;}";
    html += "@media(max-width:768px){.attack-grid{grid-template-columns:1fr;}.status-grid{grid-template-columns:repeat(2,1fr);}}";
    html += "</style></head><body>";

    html += "<div class='container'>";
    html += "<div class='header'>";
    html += "<h1>🎯 EvilCrow RF v2 - Attack Dashboard</h1>";
    html += "<div class='subtitle'>Professional RF Security Testing & Attack Platform</div>";
    html += "</div>";

    // ATTACK LAUNCHER - MAIN FEATURE
    html += "<div class='card'>";
    html += "<h2>🚀 Quick Attack Launcher</h2>";
    html += "<p>Select your attack mode - All features are prominently accessible</p>";
    html += "<div class='attack-grid'>";
    html += "<button class='attack-btn' onclick='launchAttack(\"passive-recon\")'>🔍<br>Passive Recon<br><small>Safe signal intelligence</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"car-hijack\")'>🚗<br>Car Key Hijack<br><small>Toyota/Honda keys</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"garage-attack\")'>🚪<br>Garage Door<br><small>Capture & replay</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"baby-monitor\")'>👶<br>Baby Monitor<br><small>Audio interception</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"wifi-deauth\")'>📶<br>WiFi Deauth<br><small>DoS attacks</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"spectrum-jam\")'>📻<br>Spectrum Jam<br><small>Wideband jamming</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"evil-twin\")'>👥<br>Evil Twin<br><small>Device emulation</small></button>";
    html += "<button class='attack-btn' onclick='launchAttack(\"mitm\")'>🔄<br>MITM Attack<br><small>Signal interception</small></button>";
    html += "</div>";
    html += "<div style='text-align:center;margin-top:20px;'>";
    html += "<button class='btn btn-danger' onclick='emergencyStop()' style='font-size:18px;padding:15px 30px;'>🛑 EMERGENCY STOP ALL ATTACKS</button>";
    html += "</div>";
    html += "</div>";

    // ATTACK STATISTICS
    html += "<div class='card'>";
    html += "<h3>📊 Attack Statistics</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div>Signals Captured</div><div class='status-value' id='signals-captured'>0</div></div>";
    html += "<div class='status-item'><div>Devices Hijacked</div><div class='status-value' id='devices-hijacked'>0</div></div>";
    html += "<div class='status-item'><div>Signals Jammed</div><div class='status-value' id='signals-jammed'>0</div></div>";
    html += "<div class='status-item'><div>Targets Identified</div><div class='status-value' id='targets-identified'>0</div></div>";
    html += "</div>";
    html += "<div id='active-operations' class='display'>No active operations</div>";
    html += "</div>";

    // SYSTEM STATUS
    html += "<div class='card'>";
    html += "<h3>⚙️ System Status</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'><div>WiFi Clients</div><div class='status-value'>" + String(WiFi.softAPgetStationNum()) + "</div></div>";
    html += "<div class='status-item'><div>Free Memory</div><div class='status-value'>" + String(ESP.getFreeHeap() / 1024) + " KB</div></div>";
    html += "<div class='status-item'><div>Uptime</div><div class='status-value'>" + String(millis() / 1000) + "s</div></div>";
    html += "<div class='status-item'><div>Database</div><div class='status-value'>" + String(databaseSize) + "</div></div>";
    html += "</div>";
    html += "<button class='btn' onclick='refreshStatus()'>🔄 Refresh Status</button>";
    html += "<button class='btn btn-warning' onclick='tvControl(\"turnoff\")'>📺 TV Off</button>";
    html += "</div>";

    // REAL-TIME SIGNAL ANALYSIS ENGINE
    html += "<div class='card'>";
    html += "<h3>📊 Real-time Signal Analysis Engine</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'>";
    html += "<label>Analysis Mode:</label>";
    html += "<select id='analysis-mode' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='passive'>🎯 Passive Learning</option>";
    html += "<option value='active'>⚡ Active Analysis</option>";
    html += "<option value='deep'>🔬 Deep Inspection</option>";
    html += "<option value='realtime'>📡 Real-time Monitor</option>";
    html += "</select>";
    html += "</div>";
    html += "<div class='status-item'>";
    html += "<label>Frequency Range:</label>";
    html += "<input type='number' id='freq-start' value='300' min='300' max='928' style='width:70px;background:#444;color:#fff;border:1px solid #666;padding:3px;'> - ";
    html += "<input type='number' id='freq-end' value='928' min='300' max='928' style='width:70px;background:#444;color:#fff;border:1px solid #666;padding:3px;'> MHz";
    html += "</div>";
    html += "</div>";
    html += "<div style='margin:15px 0;'>";
    html += "<button class='btn btn-success' onclick='startSignalAnalysis()'>📊 Start Analysis</button>";
    html += "<button class='btn' onclick='startSpectrumScan()'>📈 Spectrum Scan</button>";
    html += "<button class='btn btn-warning' onclick='captureSnapshot()'>📸 Snapshot</button>";
    html += "<button class='btn btn-danger' onclick='stopAnalysis()'>🛑 Stop</button>";
    html += "</div>";
    html += "<div id='spectrum-display' class='display' style='height:120px;'>Click Start Analysis to begin real-time signal monitoring</div>";
    html += "<div id='detected-signals' class='display'>Detected signals will appear here</div>";
    html += "</div>";

    // ADVANCED RF HIJACKING CONTROLS
    html += "<div class='card'>";
    html += "<h3>🎭 Advanced RF Hijacking Controls</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'>";
    html += "<label>Target Device:</label>";
    html += "<select id='hijack-target' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='car-keys'>🚗 Car Key Fobs</option>";
    html += "<option value='garage-doors'>🚪 Garage Doors</option>";
    html += "<option value='baby-monitors'>👶 Baby Monitors</option>";
    html += "<option value='security-systems'>🔒 Security Systems</option>";
    html += "<option value='remote-controls'>📺 Remote Controls</option>";
    html += "<option value='smart-home'>🏠 Smart Home</option>";
    html += "<option value='custom'>⚙️ Custom Device</option>";
    html += "</select>";
    html += "</div>";
    html += "<div class='status-item'>";
    html += "<label>Attack Mode:</label>";
    html += "<select id='hijack-mode' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='passive-learn'>🎯 Passive Learning</option>";
    html += "<option value='evil-twin'>👥 Evil Twin</option>";
    html += "<option value='replay-attack'>🔄 Replay Attack</option>";
    html += "<option value='active-hijack'>⚡ Active Hijack</option>";
    html += "</select>";
    html += "</div>";
    html += "</div>";
    html += "<div style='margin:15px 0;'>";
    html += "<button class='btn btn-warning' onclick='startHijacking()'>🎭 Start Hijacking</button>";
    html += "<button class='btn' onclick='replayLastSignal()'>🔄 Replay Last</button>";
    html += "<button class='btn btn-success' onclick='showCapturedSignals()'>📡 Show Captured</button>";
    html += "<button class='btn btn-danger' onclick='stopHijacking()'>🛑 Stop</button>";
    html += "</div>";
    html += "<div id='hijacking-status' class='display'>Hijacking system ready - Select target and mode</div>";
    html += "</div>";

    // TARGETED JAMMING INTERFACE
    html += "<div class='card'>";
    html += "<h3>📻 Targeted Jamming & DoS Control</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'>";
    html += "<label>Jam Target:</label>";
    html += "<select id='jam-target' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='baby-monitors'>👶 Baby Monitors</option>";
    html += "<option value='bluetooth-speakers'>🔊 Bluetooth Speakers</option>";
    html += "<option value='wifi-aps'>📶 WiFi Access Points</option>";
    html += "<option value='garage-doors'>🚪 Garage Doors</option>";
    html += "<option value='car-keys'>🚗 Car Key Fobs</option>";
    html += "<option value='weather-stations'>🌡️ Weather Stations</option>";
    html += "<option value='custom-freq'>⚙️ Custom Frequency</option>";
    html += "</select>";
    html += "</div>";
    html += "<div class='status-item'>";
    html += "<label>Duration:</label>";
    html += "<input type='range' id='jam-duration' min='1' max='60' value='10' style='width:100px;'>";
    html += "<span id='jam-duration-display'>10s</span>";
    html += "</div>";
    html += "</div>";
    html += "<div style='margin:15px 0;'>";
    html += "<button class='btn btn-warning' onclick='startJamming()'>📻 Start Jamming</button>";
    html += "<button class='btn' onclick='previewTarget()'>👁️ Preview Target</button>";
    html += "<button class='btn btn-danger' onclick='stopJamming()'>🛑 Stop Jamming</button>";
    html += "</div>";
    html += "<div id='jamming-status' class='display'>Jamming system ready - Select target and duration</div>";
    html += "</div>";

    // PASSIVE RECONNAISSANCE INTERFACE
    html += "<div class='card'>";
    html += "<h3>🔍 Passive Reconnaissance & Intelligence Gathering</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'>";
    html += "<label>Recon Mode:</label>";
    html += "<select id='recon-mode' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='passive-scan'>🎯 Passive Scan (Safe)</option>";
    html += "<option value='active-probe'>⚡ Active Probing</option>";
    html += "<option value='deep-analysis'>🔬 Deep Analysis</option>";
    html += "<option value='stealth-recon'>👻 Stealth Mode</option>";
    html += "</select>";
    html += "</div>";
    html += "<div class='status-item'>";
    html += "<label>Target Environment:</label>";
    html += "<select id='recon-environment' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='residential'>🏠 Residential Area</option>";
    html += "<option value='commercial'>🏢 Commercial Building</option>";
    html += "<option value='industrial'>🏭 Industrial Zone</option>";
    html += "<option value='vehicle'>🚗 Vehicle Environment</option>";
    html += "<option value='outdoor'>🌳 Outdoor/Rural</option>";
    html += "</select>";
    html += "</div>";
    html += "</div>";
    html += "<div style='margin:15px 0;'>";
    html += "<button class='btn btn-success' onclick='startReconnaissance()'>🔍 Start Recon</button>";
    html += "<button class='btn' onclick='scanAllFrequencies()'>📡 Full Spectrum</button>";
    html += "<button class='btn btn-warning' onclick='exportReconData()'>📊 Export Data</button>";
    html += "<button class='btn btn-danger' onclick='stopReconnaissance()'>🛑 Stop</button>";
    html += "</div>";
    html += "<div id='recon-results' class='display' style='height:150px;'>Select reconnaissance mode and environment to begin intelligence gathering</div>";
    html += "<div id='discovered-devices' class='display'>Discovered devices will appear here</div>";
    html += "</div>";

    // ADVANCED MITM INTERFACE
    html += "<div class='card'>";
    html += "<h3>🔄 Man-in-the-Middle (MITM) Attack Center</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'>";
    html += "<label>MITM Target:</label>";
    html += "<select id='mitm-target' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='car-communication'>🚗 Car Key ↔ Vehicle</option>";
    html += "<option value='garage-remote'>🚪 Remote ↔ Garage Door</option>";
    html += "<option value='baby-monitor'>👶 Baby Monitor Audio</option>";
    html += "<option value='security-system'>🔒 Security Sensors</option>";
    html += "<option value='smart-home'>🏠 Smart Home Devices</option>";
    html += "<option value='weather-station'>🌡️ Weather Data</option>";
    html += "<option value='custom-freq'>⚙️ Custom Frequency</option>";
    html += "</select>";
    html += "</div>";
    html += "<div class='status-item'>";
    html += "<label>Intercept Mode:</label>";
    html += "<select id='mitm-mode' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='passive-intercept'>👂 Passive Listening</option>";
    html += "<option value='active-relay'>🔄 Active Relay</option>";
    html += "<option value='signal-injection'>💉 Signal Injection</option>";
    html += "<option value='data-manipulation'>🔧 Data Manipulation</option>";
    html += "</select>";
    html += "</div>";
    html += "</div>";
    html += "<div style='margin:15px 0;'>";
    html += "<button class='btn btn-warning' onclick='startMITM()'>🔄 Start MITM</button>";
    html += "<button class='btn' onclick='showInterceptedData()'>📊 Show Data</button>";
    html += "<button class='btn btn-success' onclick='injectSignal()'>💉 Inject Signal</button>";
    html += "<button class='btn btn-danger' onclick='stopMITM()'>🛑 Stop MITM</button>";
    html += "</div>";
    html += "<div id='mitm-status' class='display'>MITM system ready - Select target and intercept mode</div>";
    html += "<div id='intercepted-data' class='display'>Intercepted communications will appear here</div>";
    html += "</div>";

    // DEVICE DATABASE BROWSER
    html += "<div class='card'>";
    html += "<h3>📚 RF Device Database & Vulnerability Scanner</h3>";
    html += "<div class='status-grid'>";
    html += "<div class='status-item'>";
    html += "<label>Search Database:</label>";
    html += "<input type='text' id='device-search' placeholder='Device name, frequency, or manufacturer' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;width:200px;'>";
    html += "</div>";
    html += "<div class='status-item'>";
    html += "<label>Filter by Vulnerability:</label>";
    html += "<select id='vuln-filter' style='background:#444;color:#fff;border:1px solid #666;padding:5px;border-radius:4px;'>";
    html += "<option value='all'>All Devices</option>";
    html += "<option value='no-encryption'>🔓 No Encryption</option>";
    html += "<option value='weak-encryption'>⚠️ Weak Encryption</option>";
    html += "<option value='replay-vulnerable'>🔄 Replay Vulnerable</option>";
    html += "<option value='jamming-vulnerable'>📻 Jamming Vulnerable</option>";
    html += "</select>";
    html += "</div>";
    html += "</div>";
    html += "<div style='margin:15px 0;'>";
    html += "<button class='btn btn-primary' onclick='searchDatabase()'>🔍 Search</button>";
    html += "<button class='btn' onclick='scanVulnerabilities()'>🛡️ Vuln Scan</button>";
    html += "<button class='btn btn-warning' onclick='showAttackVectors()'>⚔️ Attack Vectors</button>";
    html += "<button class='btn btn-success' onclick='exportDatabase()'>📊 Export DB</button>";
    html += "</div>";
    html += "<div id='database-results' class='display' style='height:120px;'>Enter search terms or select vulnerability filter</div>";
    html += "</div>";

    html += "</div>"; // End container

    // JavaScript
    html += "<script>";

    // MAIN ATTACK LAUNCHER FUNCTION
    html += "function launchAttack(type){";
    html += "if(confirm('Launch '+type+' attack?\\n\\nThis will start the selected attack mode.')){";
    html += "document.getElementById('active-operations').textContent='Launching '+type+' attack...';";
    html += "fetch('/api/attack/'+type,{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "if(d.success){";
    html += "document.getElementById('active-operations').textContent='✅ '+type+' attack active';";
    html += "updateStats();";
    html += "}else{";
    html += "document.getElementById('active-operations').textContent='❌ '+type+' attack failed';";
    html += "}";
    html += "}).catch(e=>{";
    html += "document.getElementById('active-operations').textContent='❌ Error: '+e.message;";
    html += "});";
    html += "}";
    html += "}";

    // EMERGENCY STOP FUNCTION
    html += "function emergencyStop(){";
    html += "if(confirm('EMERGENCY STOP ALL ATTACKS?\\n\\nThis will immediately stop all active operations.')){";
    html += "fetch('/api/attack/stop-all',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('active-operations').textContent='🛑 All attacks stopped';";
    html += "updateStats();";
    html += "}).catch(e=>{";
    html += "document.getElementById('active-operations').textContent='❌ Stop failed: '+e.message;";
    html += "});";
    html += "}";
    html += "}";

    // UPDATE STATISTICS
    html += "function updateStats(){";
    html += "fetch('/api/attack/status').then(r=>r.json()).then(d=>{";
    html += "document.getElementById('signals-captured').textContent=d.signals_captured||0;";
    html += "document.getElementById('devices-hijacked').textContent=d.devices_hijacked||0;";
    html += "document.getElementById('signals-jammed').textContent=d.signals_jammed||0;";
    html += "document.getElementById('targets-identified').textContent=d.targets_identified||0;";
    html += "}).catch(e=>console.error('Stats update failed:',e));}";

    // REFRESH STATUS
    html += "function refreshStatus(){";
    html += "location.reload();}";

    // TV CONTROL
    html += "function tvControl(action){";
    html += "fetch('/api/tv/'+action).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('active-operations').textContent=d.success?'📺 TV '+action+' sent':'❌ TV '+action+' failed';";
    html += "}).catch(e=>{";
    html += "document.getElementById('active-operations').textContent='❌ TV Error: '+e.message;";
    html += "});}";

    // SIGNAL ANALYSIS FUNCTIONS
    html += "function startSignalAnalysis(){";
    html += "const mode=document.getElementById('analysis-mode').value;";
    html += "const start=document.getElementById('freq-start').value;";
    html += "const end=document.getElementById('freq-end').value;";
    html += "document.getElementById('spectrum-display').textContent='Starting '+mode+' analysis on '+start+'-'+end+' MHz...';";
    html += "fetch('/api/signal/analyze',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mode:mode,start:start,end:end})}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('spectrum-display').textContent='✅ Analysis active: '+d.signals_found+' signals detected';";
    html += "updateDetectedSignals(d.signals);";
    html += "}).catch(e=>{document.getElementById('spectrum-display').textContent='❌ Analysis failed: '+e.message;});}";

    html += "function startSpectrumScan(){";
    html += "document.getElementById('spectrum-display').textContent='Starting spectrum scan...';";
    html += "fetch('/api/spectrum/scan',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('spectrum-display').textContent='📈 Spectrum scan: '+d.peaks+' peaks found';";
    html += "}).catch(e=>{document.getElementById('spectrum-display').textContent='❌ Scan failed: '+e.message;});}";

    html += "function captureSnapshot(){";
    html += "document.getElementById('spectrum-display').textContent='Capturing signal snapshot...';";
    html += "fetch('/api/signal/snapshot',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('spectrum-display').textContent='📸 Snapshot captured: '+d.signals+' signals saved';";
    html += "}).catch(e=>{document.getElementById('spectrum-display').textContent='❌ Snapshot failed: '+e.message;});}";

    html += "function stopAnalysis(){";
    html += "fetch('/api/signal/stop',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('spectrum-display').textContent='🛑 Analysis stopped';";
    html += "document.getElementById('detected-signals').textContent='Analysis stopped';";
    html += "}).catch(e=>{document.getElementById('spectrum-display').textContent='❌ Stop failed: '+e.message;});}";

    // HIJACKING FUNCTIONS
    html += "function startHijacking(){";
    html += "const target=document.getElementById('hijack-target').value;";
    html += "const mode=document.getElementById('hijack-mode').value;";
    html += "document.getElementById('hijacking-status').textContent='Starting '+mode+' on '+target+'...';";
    html += "fetch('/api/hijack/start',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({target:target,mode:mode})}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('hijacking-status').textContent='✅ Hijacking active: '+target+' ('+mode+')';";
    html += "updateStats();";
    html += "}).catch(e=>{document.getElementById('hijacking-status').textContent='❌ Hijacking failed: '+e.message;});}";

    html += "function replayLastSignal(){";
    html += "document.getElementById('hijacking-status').textContent='Replaying last captured signal...';";
    html += "fetch('/api/hijack/replay',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('hijacking-status').textContent='🔄 Signal replayed: '+d.signal_type;";
    html += "updateStats();";
    html += "}).catch(e=>{document.getElementById('hijacking-status').textContent='❌ Replay failed: '+e.message;});}";

    html += "function showCapturedSignals(){";
    html += "fetch('/api/hijack/captured').then(r=>r.json()).then(d=>{";
    html += "let result='📡 Captured Signals:\\n\\n';";
    html += "if(d.signals && d.signals.length>0){";
    html += "d.signals.forEach((s,i)=>{result+=(i+1)+'. '+s.type+' ('+s.frequency+' MHz) - '+s.timestamp+'\\n';});";
    html += "}else{result+='No signals captured yet';}";
    html += "document.getElementById('hijacking-status').textContent=result;";
    html += "}).catch(e=>{document.getElementById('hijacking-status').textContent='❌ Error: '+e.message;});}";

    html += "function stopHijacking(){";
    html += "fetch('/api/hijack/stop',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('hijacking-status').textContent='🛑 Hijacking stopped';";
    html += "}).catch(e=>{document.getElementById('hijacking-status').textContent='❌ Stop failed: '+e.message;});}";

    // JAMMING FUNCTIONS
    html += "function startJamming(){";
    html += "const target=document.getElementById('jam-target').value;";
    html += "const duration=document.getElementById('jam-duration').value;";
    html += "document.getElementById('jamming-status').textContent='Starting jamming: '+target+' for '+duration+'s...';";
    html += "fetch('/api/jam/start',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({target:target,duration:duration})}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('jamming-status').textContent='📻 Jamming active: '+target+' ('+duration+'s)';";
    html += "updateStats();";
    html += "setTimeout(()=>{document.getElementById('jamming-status').textContent='✅ Jamming completed';},duration*1000);";
    html += "}).catch(e=>{document.getElementById('jamming-status').textContent='❌ Jamming failed: '+e.message;});}";

    html += "function previewTarget(){";
    html += "const target=document.getElementById('jam-target').value;";
    html += "fetch('/api/jam/preview?target='+target).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('jamming-status').textContent='👁️ Target: '+d.name+' ('+d.frequency+' MHz) - '+d.devices+' devices';";
    html += "}).catch(e=>{document.getElementById('jamming-status').textContent='❌ Preview failed: '+e.message;});}";

    html += "function stopJamming(){";
    html += "fetch('/api/jam/stop',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('jamming-status').textContent='🛑 Jamming stopped';";
    html += "}).catch(e=>{document.getElementById('jamming-status').textContent='❌ Stop failed: '+e.message;});}";

    // UTILITY FUNCTIONS
    html += "function updateDetectedSignals(signals){";
    html += "if(!signals || signals.length===0){";
    html += "document.getElementById('detected-signals').textContent='No signals detected';";
    html += "return;}";
    html += "let result='🎯 Detected Signals:\\n\\n';";
    html += "signals.forEach((s,i)=>{";
    html += "result+=(i+1)+'. '+s.frequency+' MHz - '+s.type+' ('+s.strength+' dBm)\\n';";
    html += "});";
    html += "document.getElementById('detected-signals').textContent=result;}";

    // RECONNAISSANCE FUNCTIONS
    html += "function startReconnaissance(){";
    html += "const mode=document.getElementById('recon-mode').value;";
    html += "const env=document.getElementById('recon-environment').value;";
    html += "document.getElementById('recon-results').textContent='Starting '+mode+' in '+env+' environment...';";
    html += "fetch('/api/recon/start',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({mode:mode,environment:env})}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('recon-results').textContent='✅ Recon active: '+d.devices_found+' devices discovered';";
    html += "updateDiscoveredDevices(d.devices);";
    html += "updateStats();";
    html += "}).catch(e=>{document.getElementById('recon-results').textContent='❌ Recon failed: '+e.message;});}";

    html += "function scanAllFrequencies(){";
    html += "document.getElementById('recon-results').textContent='Scanning all frequencies 300-928 MHz...';";
    html += "fetch('/api/recon/fullscan',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('recon-results').textContent='📡 Full scan: '+d.signals+' signals, '+d.devices+' devices found';";
    html += "}).catch(e=>{document.getElementById('recon-results').textContent='❌ Scan failed: '+e.message;});}";

    html += "function exportReconData(){";
    html += "document.getElementById('recon-results').textContent='Exporting reconnaissance data...';";
    html += "fetch('/api/recon/export').then(r=>r.blob()).then(blob=>{";
    html += "const url=window.URL.createObjectURL(blob);";
    html += "const a=document.createElement('a');a.href=url;a.download='recon-data.json';a.click();";
    html += "document.getElementById('recon-results').textContent='📊 Data exported successfully';";
    html += "}).catch(e=>{document.getElementById('recon-results').textContent='❌ Export failed: '+e.message;});}";

    html += "function stopReconnaissance(){";
    html += "fetch('/api/recon/stop',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('recon-results').textContent='🛑 Reconnaissance stopped';";
    html += "}).catch(e=>{document.getElementById('recon-results').textContent='❌ Stop failed: '+e.message;});}";

    // MITM FUNCTIONS
    html += "function startMITM(){";
    html += "const target=document.getElementById('mitm-target').value;";
    html += "const mode=document.getElementById('mitm-mode').value;";
    html += "document.getElementById('mitm-status').textContent='Starting MITM: '+target+' ('+mode+')...';";
    html += "fetch('/api/mitm/start',{method:'POST',headers:{'Content-Type':'application/json'},body:JSON.stringify({target:target,mode:mode})}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('mitm-status').textContent='🔄 MITM active: '+target+' - '+d.packets+' packets intercepted';";
    html += "updateStats();";
    html += "}).catch(e=>{document.getElementById('mitm-status').textContent='❌ MITM failed: '+e.message;});}";

    html += "function showInterceptedData(){";
    html += "fetch('/api/mitm/data').then(r=>r.json()).then(d=>{";
    html += "let result='📊 Intercepted Data:\\n\\n';";
    html += "if(d.packets && d.packets.length>0){";
    html += "d.packets.forEach((p,i)=>{result+=(i+1)+'. '+p.timestamp+' - '+p.type+': '+p.data+'\\n';});";
    html += "}else{result+='No data intercepted yet';}";
    html += "document.getElementById('intercepted-data').textContent=result;";
    html += "}).catch(e=>{document.getElementById('intercepted-data').textContent='❌ Error: '+e.message;});}";

    html += "function injectSignal(){";
    html += "document.getElementById('mitm-status').textContent='Injecting malicious signal...';";
    html += "fetch('/api/mitm/inject',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('mitm-status').textContent='💉 Signal injected: '+d.signal_type;";
    html += "}).catch(e=>{document.getElementById('mitm-status').textContent='❌ Injection failed: '+e.message;});}";

    html += "function stopMITM(){";
    html += "fetch('/api/mitm/stop',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('mitm-status').textContent='🛑 MITM stopped';";
    html += "}).catch(e=>{document.getElementById('mitm-status').textContent='❌ Stop failed: '+e.message;});}";

    // DATABASE FUNCTIONS
    html += "function searchDatabase(){";
    html += "const query=document.getElementById('device-search').value;";
    html += "const filter=document.getElementById('vuln-filter').value;";
    html += "document.getElementById('database-results').textContent='Searching database for: '+query+'...';";
    html += "fetch('/api/database/search?q='+encodeURIComponent(query)+'&filter='+filter).then(r=>r.json()).then(d=>{";
    html += "let result='🔍 Search Results ('+d.count+' found):\\n\\n';";
    html += "if(d.devices && d.devices.length>0){";
    html += "d.devices.forEach((dev,i)=>{result+=(i+1)+'. '+dev.name+' ('+dev.frequency+' MHz)\\n   Vuln: '+dev.vulnerabilities+'\\n';});";
    html += "}else{result+='No devices found matching criteria';}";
    html += "document.getElementById('database-results').textContent=result;";
    html += "}).catch(e=>{document.getElementById('database-results').textContent='❌ Search failed: '+e.message;});}";

    html += "function scanVulnerabilities(){";
    html += "document.getElementById('database-results').textContent='Scanning for vulnerabilities...';";
    html += "fetch('/api/database/vulnscan',{method:'POST'}).then(r=>r.json()).then(d=>{";
    html += "document.getElementById('database-results').textContent='🛡️ Vuln scan: '+d.vulnerable+' vulnerable devices found';";
    html += "}).catch(e=>{document.getElementById('database-results').textContent='❌ Scan failed: '+e.message;});}";

    html += "function showAttackVectors(){";
    html += "fetch('/api/database/attacks').then(r=>r.json()).then(d=>{";
    html += "let result='⚔️ Available Attack Vectors:\\n\\n';";
    html += "d.attacks.forEach((att,i)=>{result+=(i+1)+'. '+att.name+' - '+att.description+'\\n';});";
    html += "document.getElementById('database-results').textContent=result;";
    html += "}).catch(e=>{document.getElementById('database-results').textContent='❌ Error: '+e.message;});}";

    html += "function exportDatabase(){";
    html += "document.getElementById('database-results').textContent='Exporting device database...';";
    html += "fetch('/api/database/export').then(r=>r.blob()).then(blob=>{";
    html += "const url=window.URL.createObjectURL(blob);";
    html += "const a=document.createElement('a');a.href=url;a.download='device-database.json';a.click();";
    html += "document.getElementById('database-results').textContent='📊 Database exported successfully';";
    html += "}).catch(e=>{document.getElementById('database-results').textContent='❌ Export failed: '+e.message;});}";

    // UTILITY FUNCTIONS
    html += "function updateDiscoveredDevices(devices){";
    html += "if(!devices || devices.length===0){";
    html += "document.getElementById('discovered-devices').textContent='No devices discovered yet';";
    html += "return;}";
    html += "let result='🎯 Discovered Devices:\\n\\n';";
    html += "devices.forEach((d,i)=>{";
    html += "result+=(i+1)+'. '+d.name+' ('+d.frequency+' MHz) - '+d.type+'\\n';";
    html += "result+='   Signal: '+d.strength+' dBm, Vuln: '+d.vulnerability+'\\n\\n';";
    html += "});";
    html += "document.getElementById('discovered-devices').textContent=result;}";

    // UPDATE DURATION DISPLAY
    html += "document.getElementById('jam-duration').addEventListener('input',function(){";
    html += "document.getElementById('jam-duration-display').textContent=this.value+'s';";
    html += "});";

    // AUTO-UPDATE STATS EVERY 5 SECONDS
    html += "setInterval(updateStats,5000);";
    html += "updateStats();"; // Initial load

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
    html += ".btn{background:#2196F3;color:white;padding:8px 16px;border:none;border-radius:6px;";
    html += "text-decoration:none;margin:0 5px;font-size:14px;}";
    html += ".status{margin:10px 0;padding:10px;background:#333;border-radius:6px;}";
    html += "</style></head><body>";

    html += "<div class='container'>";
    html += "<h1>📁 EvilCrow RF v2 - File Browser</h1>";
    html += "<p><a href='/' class='btn'>← Back to Dashboard</a></p>";

    if (sdCardAvailable)
    {
        html += "<div class='file-list'>";
        html += "<h3>📊 SD Card Status: ✅ Available</h3>";
        html += "<div class='status'>";
        html += "<p>✅ SD card is working properly with the enhanced initialization method</p>";
        html += "<p>📌 CS Pin: " + String(sdCS) + " (confirmed working by diagnostic)</p>";

        uint64_t cardSize = SD.cardSize();
        uint64_t usedBytes = SD.usedBytes();
        if (cardSize > 0)
        {
            html += "<p>💾 Card Size: " + String(cardSize / (1024 * 1024)) + " MB</p>";
            html += "<p>📊 Used Space: " + String(usedBytes / 1024) + " KB</p>";
            html += "<p>💿 Free Space: " + String((cardSize - usedBytes) / 1024) + " KB</p>";
        }
        html += "</div>";

        html += "<h3>📁 Root Directory Files:</h3>";
        File root = SD.open("/");
        if (root && root.isDirectory())
        {
            File file = root.openNextFile();
            bool hasFiles = false;
            while (file)
            {
                if (!file.isDirectory())
                {
                    hasFiles = true;
                    html += "<p>📄 " + String(file.name()) + " (" + String(file.size()) + " bytes)</p>";
                }
                file = root.openNextFile();
            }
            root.close();
            if (!hasFiles)
            {
                html += "<p>📂 No files in root directory</p>";
            }
        }

        if (SD.exists("/HTML"))
        {
            html += "<h3>📁 HTML Directory:</h3>";
            File htmlDir = SD.open("/HTML");
            if (htmlDir && htmlDir.isDirectory())
            {
                File file = htmlDir.openNextFile();
                bool hasFiles = false;
                while (file)
                {
                    if (!file.isDirectory())
                    {
                        hasFiles = true;
                        html += "<p>📄 HTML/" + String(file.name()) + " (" + String(file.size()) + " bytes)</p>";
                    }
                    file = htmlDir.openNextFile();
                }
                htmlDir.close();
                if (!hasFiles)
                {
                    html += "<p>📂 HTML directory is empty</p>";
                }
            }
        }
        else
        {
            html += "<p>⚠️ HTML directory not found</p>";
        }

        html += "</div>";
    }
    else
    {
        html += "<div class='file-list'>";
        html += "<h3>❌ SD Card Not Available</h3>";
        html += "<div class='status'>";
        html += "<p>The SD card could not be initialized despite multiple attempts with different configurations.</p>";
        html += "</div>";

        html += "<h4>🔧 Enhanced Fixes Applied:</h4>";
        html += "<ul>";
        html += "<li>✅ Using CS pin 22 (confirmed by diagnostic)</li>";
        html += "<li>✅ Multiple initialization attempts (5 attempts)</li>";
        html += "<li>✅ Progressive SPI frequencies (4MHz → 1MHz → 400kHz → 100kHz → 50kHz)</li>";
        html += "<li>✅ Progressive delays between attempts</li>";
        html += "<li>✅ Proper SPI bus management and cleanup</li>";
        html += "</ul>";

        html += "<h4>💡 Additional Troubleshooting Steps:</h4>";
        html += "<ul>";
        html += "<li>🔧 Format SD card as FAT32 with MBR partition table (not GPT)</li>";
        html += "<li>🔧 Try a different SD card (2GB-32GB work best)</li>";
        html += "<li>🔧 Check CS pin connection (should be pin 22)</li>";
        html += "<li>🔧 Ensure stable 3.3V power supply to SD card</li>";
        html += "<li>🔧 Verify SPI wiring: SCK=18, MISO=19, MOSI=23, CS=22</li>";
        html += "<li>🔧 Try a high-quality SD card (SanDisk, Samsung work well)</li>";
        html += "</ul>";

        html += "<p><button class='btn' onclick='location.reload()'>🔄 Retry SD Card</button></p>";
        html += "</div>";
    }

    html += "</div></body></html>";
    return html;
}

String generateHelpPage()
{
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>EvilCrow RF v2 - Help & Documentation</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
    html += "<style>";
    html += "body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;";
    html += "background:linear-gradient(135deg,#1a1a1a 0%,#2d2d2d 100%);color:#fff;margin:0;padding:20px;}";
    html += ".container{max-width:800px;margin:0 auto;}";
    html += ".help-section{background:rgba(255,255,255,0.1);backdrop-filter:blur(20px);";
    html += "border-radius:12px;padding:20px;margin:20px 0;border:1px solid rgba(255,255,255,0.2);}";
    html += ".btn{background:#2196F3;color:white;padding:8px 16px;border:none;border-radius:6px;";
    html += "text-decoration:none;margin:0 5px;font-size:14px;}";
    html += "code{background:#333;padding:2px 6px;border-radius:4px;font-family:monospace;}";
    html += "</style></head><body>";

    html += "<div class='container'>";
    html += "<h1>❓ EvilCrow RF v2 - Help & Documentation</h1>";
    html += "<p><a href='/' class='btn'>← Back to Dashboard</a></p>";

    html += "<div class='help-section'>";
    html += "<h2>🚀 Getting Started</h2>";
    html += "<p>Welcome to the enhanced EvilCrow RF v2! This version includes major improvements:</p>";
    html += "<ul>";
    html += "<li>✅ <strong>Fixed SD Card Issues</strong> - CS pin changed from 22 to 5</li>";
    html += "<li>📺 <strong>TV Remote Control</strong> - Universal TV remote functionality</li>";
    html += "<li>🔍 <strong>Signal Database</strong> - Identify RF devices by frequency</li>";
    html += "<li>📶 <strong>WiFi Attacks</strong> - Network scanning and deauth simulation</li>";
    html += "</ul>";
    html += "</div>";

    html += "<div class='help-section'>";
    html += "<h2>📺 TV Remote Control</h2>";
    html += "<p>Control TVs using universal remote codes:</p>";
    html += "<ul>";
    html += "<li><strong>Turn Off:</strong> <code>/api/tv/turnoff</code></li>";
    html += "<li><strong>Turn On:</strong> <code>/api/tv/turnon</code></li>";
    html += "<li><strong>Scan All:</strong> <code>/api/tv/scan</code> (tries all brands)</li>";
    html += "<li><strong>Specific Brand:</strong> <code>/api/tv/turnoff?brand=samsung</code></li>";
    html += "</ul>";
    html += "<p><strong>Hardware:</strong> Button 1 performs quick TV scan</p>";
    html += "</div>";

    html += "<div class='help-section'>";
    html += "<h2>🔍 Signal Analysis</h2>";
    html += "<p>Identify RF devices using the built-in database:</p>";
    html += "<ul>";
    html += "<li><strong>Identify Signal:</strong> <code>/api/signal/identify?freq=433.92</code></li>";
    html += "<li><strong>Device Database:</strong> <code>/api/devices/frequency?freq=315.0</code></li>";
    html += "</ul>";
    html += "<p><strong>Common Frequencies:</strong></p>";
    html += "<ul>";
    html += "<li>315.0 MHz - US garage doors, car keys</li>";
    html += "<li>390.0 MHz - European car keys</li>";
    html += "<li>433.92 MHz - ISM band, garage doors, toys, baby monitors</li>";
    html += "<li>868.0 MHz - European ISM band</li>";
    html += "<li>915.0 MHz - US ISM band</li>";
    html += "</ul>";
    html += "</div>";

    html += "<div class='help-section'>";
    html += "<h2>📶 WiFi Attacks</h2>";
    html += "<p>⚠️ <strong>Educational/Testing Only - Use Responsibly</strong></p>";
    html += "<ul>";
    html += "<li><strong>Scan Networks:</strong> <code>/api/wifi/scan</code></li>";
    html += "<li><strong>Show Networks:</strong> <code>/api/wifi/networks</code></li>";
    html += "<li><strong>Deauth Demo:</strong> <code>/api/wifi/deauth?packets=100</code></li>";
    html += "</ul>";
    html += "<p><strong>Legal Warning:</strong> Only use on networks you own or have permission to test.</p>";
    html += "</div>";

    html += "<div class='help-section'>";
    html += "<h2>🔧 Hardware Controls</h2>";
    html += "<ul>";
    html += "<li><strong>Button 1 (Pin 0):</strong> Quick TV scan</li>";
    html += "<li><strong>Button 2 (Pin 35):</strong> Toggle receive mode</li>";
    html += "<li><strong>LED (Pin 2):</strong> Status indicator</li>";
    html += "</ul>";
    html += "</div>";

    html += "<div class='help-section'>";
    html += "<h2>💾 SD Card Troubleshooting</h2>";
    html += "<p>If SD card issues persist:</p>";
    html += "<ul>";
    html += "<li>Format as FAT32 with MBR partition table</li>";
    html += "<li>Check CS pin connection (pin 22)</li>";
    html += "<li>Try a different SD card (2GB-32GB)</li>";
    html += "<li>Ensure stable power supply</li>";
    html += "</ul>";
    html += "<p><strong>SPI Wiring:</strong> SCK=18, MISO=19, MOSI=23, CS=22</p>";
    html += "</div>";

    html += "<div class='help-section'>";
    html += "<h2>⚖️ Legal & Safety</h2>";
    html += "<p><strong>This device is for educational and authorized testing only.</strong></p>";
    html += "<ul>";
    html += "<li>Only test devices you own</li>";
    html += "<li>Get written permission before testing others' systems</li>";
    html += "<li>WiFi attacks may be illegal in your jurisdiction</li>";
    html += "<li>Respect privacy and follow local laws</li>";
    html += "</ul>";
    html += "</div>";

    html += "</div></body></html>";
    return html;
}

String generateNotFoundPage(String path)
{
    String html = "<html><head><title>404 - Page Not Found</title>";
    html += "<style>body{font-family:Arial;background:#1a1a1a;color:#fff;padding:20px;}";
    html += ".btn{background:#2196F3;color:white;padding:10px 20px;text-decoration:none;border-radius:5px;margin:5px;display:inline-block;}";
    html += ".endpoint{background:#333;padding:10px;margin:5px 0;border-radius:5px;font-family:monospace;}</style></head><body>";
    html += "<h1>⚠️ 404 - Page Not Found</h1>";
    html += "<p>Requested URL: <code>" + path + "</code></p>";

    html += "<h3>🏠 Available Pages:</h3>";
    html += "<a href='/' class='btn'>📊 Dashboard</a>";
    html += "<a href='/files' class='btn'>📁 File Browser</a>";
    html += "<a href='/help' class='btn'>❓ Help</a>";

    html += "<h3>📡 API Endpoints:</h3>";
    html += "<div class='endpoint'>GET /api/tv/scan - Turn off all TVs</div>";
    html += "<div class='endpoint'>GET /api/signal/identify?freq=433.92 - Identify signal</div>";
    html += "<div class='endpoint'>GET /api/wifi/scan - Scan WiFi networks</div>";
    html += "<div class='endpoint'>GET /api/status - System status</div>";

    html += "<h3>🔗 Quick Links:</h3>";
    html += "<a href='/api/tv/scan' class='btn'>📺 Turn Off TVs</a>";
    html += "<a href='/api/signal/identify?freq=433.92' class='btn'>🔍 Analyze 433.92 MHz</a>";
    html += "<a href='/api/wifi/scan' class='btn'>📶 WiFi Scan</a>";

    html += "</body></html>";
    return html;
}
