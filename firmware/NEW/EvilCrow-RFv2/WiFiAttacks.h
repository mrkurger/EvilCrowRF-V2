#ifndef WIFI_ATTACKS_H
#define WIFI_ATTACKS_H

#include <Arduino.h>
#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_err.h>
#include <vector>

// WiFi network information
struct WiFiTarget {
    String ssid;
    String bssid;
    int channel;
    int rssi;
    wifi_auth_mode_t authMode;
    bool isHidden;
    unsigned long lastSeen;
};

// Deauth attack configuration
struct DeauthConfig {
    String targetSSID;
    String targetBSSID;
    int channel;
    int packetCount;
    int delayMs;
    bool broadcastMode;
    bool randomizeSource;
};

// Attack statistics
struct AttackStats {
    unsigned long packetsTransmitted;
    unsigned long attackDuration;
    int targetsHit;
    float successRate;
    unsigned long startTime;
    bool isActive;
};

class WiFiAttacks {
private:
    std::vector<WiFiTarget> discoveredNetworks;
    AttackStats currentStats;
    bool isScanning;
    bool isAttacking;
    DeauthConfig currentConfig;
    
    // Raw packet structures
    struct DeauthFrame {
        uint8_t frame_ctrl[2];
        uint8_t duration[2];
        uint8_t destination[6];
        uint8_t source[6];
        uint8_t bssid[6];
        uint8_t seq_ctrl[2];
        uint8_t reason_code[2];
    };
    
    struct BeaconFrame {
        uint8_t frame_ctrl[2];
        uint8_t duration[2];
        uint8_t destination[6];
        uint8_t source[6];
        uint8_t bssid[6];
        uint8_t seq_ctrl[2];
        uint8_t timestamp[8];
        uint8_t beacon_interval[2];
        uint8_t capability_info[2];
    };
    
    // Helper functions
    void macStringToBytes(const String& mac, uint8_t* bytes);
    String macBytesToString(const uint8_t* bytes);
    void generateRandomMAC(uint8_t* mac);
    bool isValidChannel(int channel);
    
public:
    WiFiAttacks();
    ~WiFiAttacks();
    
    // Network discovery
    bool startNetworkScan();
    void stopNetworkScan();
    bool isNetworkScanActive();
    std::vector<WiFiTarget> getDiscoveredNetworks();
    void clearDiscoveredNetworks();
    
    // Target management
    bool addTarget(const String& ssid, const String& bssid = "", int channel = 0);
    bool removeTarget(const String& ssid);
    std::vector<WiFiTarget> getTargets();
    WiFiTarget* findTarget(const String& ssid);
    
    // Deauth attacks
    bool startDeauthAttack(const DeauthConfig& config);
    bool startDeauthAttack(const String& targetSSID, int packetCount = 100);
    bool startBroadcastDeauth(int channel = 0, int packetCount = 500);
    bool startTargetedDeauth(const String& ssid, const String& bssid, int packetCount = 100);
    void stopDeauthAttack();
    bool isDeauthActive();
    
    // Beacon spam attacks
    bool startBeaconSpam(const std::vector<String>& ssidList, int count = 100);
    bool startRandomBeaconSpam(int count = 50);
    void stopBeaconSpam();
    
    // Evil twin attacks
    bool startEvilTwin(const String& targetSSID, const String& password = "");
    void stopEvilTwin();
    bool isEvilTwinActive();
    
    // Probe request attacks
    bool startProbeFlood(const String& targetSSID, int count = 200);
    void stopProbeFlood();
    
    // WiFi jamming
    bool startWiFiJammer(int channel = 0, int duration = 30);
    void stopWiFiJammer();
    bool isJammerActive();
    
    // Packet injection
    bool injectDeauthPacket(const uint8_t* target, const uint8_t* ap, int channel);
    bool injectBeaconPacket(const String& ssid, int channel, const uint8_t* bssid = nullptr);
    bool injectProbeRequest(const String& ssid, const uint8_t* source = nullptr);
    
    // Configuration
    void setDeauthConfig(const DeauthConfig& config);
    DeauthConfig getDeauthConfig();
    void setAttackChannel(int channel);
    void setPacketDelay(int delayMs);
    
    // Statistics and monitoring
    AttackStats getAttackStats();
    void resetStats();
    String getStatsJSON();
    
    // Advanced features
    bool startKarmaAttack();  // Respond to all probe requests
    bool startPineappleMode(); // Captive portal for all connections
    bool startWPSAttack(const String& targetBSSID);
    
    // Utility functions
    String getWiFiSecurityString(wifi_auth_mode_t authMode);
    int getOptimalChannel();
    std::vector<int> getActiveChannels();
    bool isChannelClear(int channel);
    
    // Legal and safety
    String getLegalWarning();
    bool isAttackLegal(const String& attackType);
    void enableSafeMode(bool enable);
    
    // Status functions
    String getStatus();
    bool isInitialized();
    void printNetworkList();
};

extern WiFiAttacks wifiAttacks;

#endif // WIFI_ATTACKS_H
