#ifndef RF_DEVICE_DATABASE_H
#define RF_DEVICE_DATABASE_H

#include <Arduino.h>
#include <vector>

// RF Device categories and their common frequencies
struct RFDeviceCategory
{
    String name;
    String description;
    std::vector<float> commonFrequencies;
    String vulnerabilities;
    String attackMethods;
    String legalNotes;
};

// Specific RF device information
struct RFDevice
{
    String name;
    String category;
    String manufacturer;
    float frequency;
    String modulation;
    String protocol;
    String description;
    String vulnerabilities;
    String attackVectors;
    bool hasRollingCode;
    String region;
    String notes;
};

class RFDeviceDatabase
{
private:
    std::vector<RFDeviceCategory> categories;
    std::vector<RFDevice> devices;

public:
    RFDeviceDatabase();
    ~RFDeviceDatabase();

    // Database initialization
    void initializeDatabase();
    void loadGarageDoorsAndGates();
    void loadCarKeyFobs();
    void loadHomeAutomation();
    void loadSecuritySystems();
    void loadToys();
    void loadMedicalDevices();
    void loadIndustrialDevices();
    void loadWeatherStations();
    void loadBabyMonitors();
    void loadWirelessDoorbells();
    void loadRemoteControls();
    void loadDroneControls();
    void loadModelAircraft();
    void loadWirelessSensors();
    void loadSmartHomeDevices();

    // Query functions
    std::vector<RFDevice> getDevicesByFrequency(float frequency, float tolerance = 0.1);
    std::vector<RFDevice> getDevicesByCategory(const String &category);
    std::vector<RFDevice> getDevicesByManufacturer(const String &manufacturer);
    std::vector<RFDevice> getVulnerableDevices();
    std::vector<RFDevice> getDevicesWithoutRollingCode();

    // Analysis functions
    String identifyDevice(float frequency, const String &pattern = "");
    String getAttackSuggestions(float frequency);
    String getVulnerabilityReport(const String &deviceName);
    String getFrequencyReport(float frequency);

    // Categories
    std::vector<RFDeviceCategory> getCategories();
    RFDeviceCategory *getCategory(const String &name);

    // Statistics
    int getDeviceCount();
    int getCategoryCount();
    std::vector<float> getAllFrequencies();
    String getDatabaseStats();

    // Export functions
    String exportToJSON();
    String exportCategoryToJSON(const String &category);

    // Fun and educational functions
    String getRandomDevice();
    String getDeviceOfTheDay();
    std::vector<String> getInterestingTargets();
    String getHackingTips(const String &category);

    // Specific device attack information
    String getGarageDoorAttacks();
    String getCarKeyAttacks();
    String getToyAttacks();
    String getBabyMonitorAttacks();
    String getDoorbellAttacks();
    String getDroneAttacks();
    String getSmartHomeAttacks();
};

extern RFDeviceDatabase rfDeviceDB;

#endif // RF_DEVICE_DATABASE_H
