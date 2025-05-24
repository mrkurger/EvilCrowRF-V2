#ifndef SIGNAL_DATABASE_H
#define SIGNAL_DATABASE_H

#include <Arduino.h>
#include <vector>
#include <map>
#include "SDCardManager.h"

// Signal fingerprint structure
struct SignalFingerprint {
    String name;                    // Human-readable name
    String description;             // Detailed description
    String category;                // Device category (garage, car, remote, etc.)
    String manufacturer;            // Device manufacturer
    float frequency;                // Operating frequency
    String modulation;              // Modulation type
    String pattern;                 // Binary/hex pattern
    int pulseWidth;                 // Pulse width in microseconds
    int bitCount;                   // Number of bits
    String protocol;                // Protocol name (if known)
    float confidence;               // Match confidence threshold
    String region;                  // Geographic region
    bool isRollingCode;             // Uses rolling code security
    String notes;                   // Additional notes
};

// Signal match result
struct SignalMatch {
    SignalFingerprint fingerprint;
    float matchConfidence;
    String matchType;               // "exact", "partial", "pattern"
    String differences;             // What didn't match
};

// Device category information
struct DeviceCategory {
    String name;
    String description;
    String commonFrequencies;
    String securityNotes;
    String legalWarnings;
};

class SignalDatabase {
private:
    std::vector<SignalFingerprint> fingerprints;
    std::map<String, DeviceCategory> categories;
    String databasePath;
    bool isLoaded;
    unsigned long lastUpdate;
    
    // Pattern matching functions
    float comparePatterns(const String& pattern1, const String& pattern2);
    float compareFrequencies(float freq1, float freq2);
    bool loadFingerprintFromJSON(const String& jsonData, SignalFingerprint& fp);
    String fingerprintToJSON(const SignalFingerprint& fp);
    
public:
    SignalDatabase();
    ~SignalDatabase();
    
    // Database management
    bool loadDatabase();
    bool saveDatabase();
    bool addFingerprint(const SignalFingerprint& fp);
    bool updateFingerprint(const String& name, const SignalFingerprint& fp);
    bool removeFingerprint(const String& name);
    
    // Signal identification
    std::vector<SignalMatch> identifySignal(float frequency, const String& pattern, 
                                           const String& modulation = "");
    SignalMatch getBestMatch(float frequency, const String& pattern);
    std::vector<SignalFingerprint> searchByFrequency(float freq, float tolerance = 0.1);
    std::vector<SignalFingerprint> searchByCategory(const String& category);
    std::vector<SignalFingerprint> searchByManufacturer(const String& manufacturer);
    
    // Database queries
    int getFingerprintCount();
    std::vector<String> getCategories();
    std::vector<String> getManufacturers();
    std::vector<String> getProtocols();
    DeviceCategory getCategoryInfo(const String& category);
    
    // Common device databases
    void loadGarageDoorDatabase();
    void loadCarKeyDatabase();
    void loadRemoteControlDatabase();
    void loadSecuritySystemDatabase();
    void loadWeatherStationDatabase();
    void loadToyDatabase();
    void loadMedicalDeviceDatabase();
    void loadIndustrialDatabase();
    
    // Analysis functions
    String analyzeSignalPattern(const String& pattern);
    String detectProtocol(float frequency, const String& pattern, int pulseWidth);
    String suggestAttackVector(const SignalFingerprint& fp);
    String getSecurityAssessment(const SignalFingerprint& fp);
    
    // Export/Import
    String exportToJSON();
    bool importFromJSON(const String& jsonData);
    bool exportToCSV(const String& filename);
    
    // Statistics
    String getDatabaseStats();
    std::vector<SignalFingerprint> getMostCommonSignals();
    std::vector<SignalFingerprint> getRecentlyAdded();
    
    // Learning mode
    bool learnNewSignal(float frequency, const String& pattern, 
                       const String& userDescription = "");
    void enableLearningMode(bool enable);
    bool isLearningModeEnabled();
    
    // Utility functions
    void clearDatabase();
    bool validateFingerprint(const SignalFingerprint& fp);
    String generateSignatureHash(const SignalFingerprint& fp);
};

extern SignalDatabase signalDB;

#endif // SIGNAL_DATABASE_H
