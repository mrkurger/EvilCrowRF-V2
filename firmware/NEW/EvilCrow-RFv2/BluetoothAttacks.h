#ifndef BLUETOOTH_ATTACKS_H
#define BLUETOOTH_ATTACKS_H

#include <Arduino.h>
#include <BluetoothSerial.h>
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_bt_api.h>
#include <esp_spp_api.h>
#include <vector>

// Bluetooth device information
struct BluetoothDevice {
    String name;
    String address;
    int rssi;
    uint32_t cod;  // Class of Device
    bool isConnectable;
    bool isPaired;
    unsigned long lastSeen;
    String deviceType;
};

// Bluetooth attack configuration
struct BluetoothAttackConfig {
    String targetAddress;
    String targetName;
    String attackType;
    int duration;
    int intensity;
    bool randomizeSource;
};

// Attack types
enum BluetoothAttackType {
    BT_ATTACK_SPAM,
    BT_ATTACK_FLOOD,
    BT_ATTACK_CRASH,
    BT_ATTACK_PAIR_SPAM,
    BT_ATTACK_NAME_SPAM,
    BT_ATTACK_DISCOVERY_FLOOD
};

class BluetoothAttacks {
private:
    std::vector<BluetoothDevice> discoveredDevices;
    BluetoothSerial SerialBT;
    bool isScanning;
    bool isAttacking;
    BluetoothAttackConfig currentConfig;
    
    // Attack payloads
    std::vector<String> spamNames;
    std::vector<String> crashPayloads;
    
    // Statistics
    unsigned long packetsTransmitted;
    unsigned long attackStartTime;
    
    // Helper functions
    String formatBluetoothAddress(const uint8_t* address);
    String getDeviceTypeString(uint32_t cod);
    void initializeSpamNames();
    void initializeCrashPayloads();
    
public:
    BluetoothAttacks();
    ~BluetoothAttacks();
    
    // Initialization
    bool begin();
    void end();
    bool isInitialized();
    
    // Device discovery
    bool startDeviceScan(int duration = 30);
    void stopDeviceScan();
    bool isDeviceScanActive();
    std::vector<BluetoothDevice> getDiscoveredDevices();
    void clearDiscoveredDevices();
    
    // Target management
    bool addTarget(const String& address, const String& name = "");
    bool removeTarget(const String& address);
    BluetoothDevice* findDevice(const String& address);
    
    // Bluetooth spam attacks
    bool startBluetoothSpam(int duration = 60);
    bool startNameSpam(const String& targetAddress, int count = 100);
    bool startPairSpam(const String& targetAddress, int count = 50);
    bool startDiscoveryFlood(int duration = 30);
    void stopBluetoothAttacks();
    
    // Device-specific attacks
    bool attackiPhone(const String& address);
    bool attackAndroid(const String& address);
    bool attackWindows(const String& address);
    bool attackAirPods(const String& address);
    bool attackSmartTV(const String& address);
    
    // Bluetooth Low Energy (BLE) attacks
    bool startBLESpam(int duration = 60);
    bool startBLEBeaconFlood(int count = 200);
    bool startAppleDeviceSpam(); // Target Apple devices specifically
    bool startAndroidSpam();     // Target Android devices
    
    // Proximity attacks
    bool startProximitySpam();   // Spam nearby devices
    bool startCarAttack();       // Target car Bluetooth systems
    bool startHeadphoneAttack(); // Target Bluetooth headphones
    
    // Advanced attacks
    bool startBlueJacking(const String& message, const String& targetAddress = "");
    bool startBlueSnarfing(const String& targetAddress);
    bool startBlueSmacking(const String& targetAddress);
    
    // Denial of Service attacks
    bool startBluetoothJammer(int duration = 30);
    bool startConnectionFlood(const String& targetAddress, int connections = 100);
    bool startL2CAPFlood(const String& targetAddress);
    
    // Social engineering
    bool startFakeDeviceSpam();  // Create fake attractive device names
    bool startHIDAttack(const String& targetAddress); // Fake keyboard/mouse
    bool startAudioSpam(const String& targetAddress); // Audio device spam
    
    // Configuration
    void setAttackIntensity(int intensity); // 1-10 scale
    void setRandomizeSource(bool enable);
    void addCustomSpamName(const String& name);
    void loadSpamNamesFromFile(const String& filename);
    
    // Statistics and monitoring
    unsigned long getPacketsTransmitted();
    unsigned long getAttackDuration();
    String getAttackStats();
    String getAttackStatsJSON();
    
    // Device information
    String getDeviceInfo(const String& address);
    bool isDeviceVulnerable(const String& address);
    String getVulnerabilityReport(const String& address);
    
    // Utility functions
    bool isBluetoothEnabled();
    String getLocalBluetoothAddress();
    void setLocalDeviceName(const String& name);
    void makeDiscoverable(bool discoverable);
    
    // Legal and safety
    String getLegalWarning();
    void enableSafeMode(bool enable);
    bool isAttackLegal(const String& attackType);
    
    // Status functions
    String getStatus();
    void printDeviceList();
    bool isAttackActive();
};

extern BluetoothAttacks bluetoothAttacks;

#endif // BLUETOOTH_ATTACKS_H
