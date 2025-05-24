#include "SignalDatabase.h"
#include <ArduinoJson.h>

SignalDatabase signalDB;

SignalDatabase::SignalDatabase() {
    databasePath = "/signals_db.json";
    isLoaded = false;
    lastUpdate = 0;
}

SignalDatabase::~SignalDatabase() {
    if (isLoaded) {
        saveDatabase();
    }
}

bool SignalDatabase::loadDatabase() {
    Serial.println("📚 Loading signal fingerprint database...");
    
    if (!sdManager.exists(databasePath)) {
        Serial.println("📚 Database not found, creating default database...");
        loadGarageDoorDatabase();
        loadCarKeyDatabase();
        loadRemoteControlDatabase();
        loadSecuritySystemDatabase();
        loadWeatherStationDatabase();
        saveDatabase();
        isLoaded = true;
        return true;
    }
    
    File dbFile = sdManager.open(databasePath, FILE_READ);
    if (!dbFile) {
        Serial.println("❌ Failed to open database file");
        return false;
    }
    
    String jsonData = dbFile.readString();
    dbFile.close();
    
    DynamicJsonDocument doc(32768); // 32KB for large database
    DeserializationError error = deserializeJson(doc, jsonData);
    
    if (error) {
        Serial.println("❌ Failed to parse database JSON: " + String(error.c_str()));
        return false;
    }
    
    fingerprints.clear();
    JsonArray fpArray = doc["fingerprints"];
    
    for (JsonObject fpObj : fpArray) {
        SignalFingerprint fp;
        fp.name = fpObj["name"].as<String>();
        fp.description = fpObj["description"].as<String>();
        fp.category = fpObj["category"].as<String>();
        fp.manufacturer = fpObj["manufacturer"].as<String>();
        fp.frequency = fpObj["frequency"];
        fp.modulation = fpObj["modulation"].as<String>();
        fp.pattern = fpObj["pattern"].as<String>();
        fp.pulseWidth = fpObj["pulseWidth"];
        fp.bitCount = fpObj["bitCount"];
        fp.protocol = fpObj["protocol"].as<String>();
        fp.confidence = fpObj["confidence"];
        fp.region = fpObj["region"].as<String>();
        fp.isRollingCode = fpObj["isRollingCode"];
        fp.notes = fpObj["notes"].as<String>();
        
        fingerprints.push_back(fp);
    }
    
    isLoaded = true;
    Serial.println("✅ Loaded " + String(fingerprints.size()) + " signal fingerprints");
    return true;
}

bool SignalDatabase::saveDatabase() {
    if (!isLoaded) return false;
    
    Serial.println("💾 Saving signal database...");
    
    DynamicJsonDocument doc(32768);
    JsonArray fpArray = doc.createNestedArray("fingerprints");
    
    for (const auto& fp : fingerprints) {
        JsonObject fpObj = fpArray.createNestedObject();
        fpObj["name"] = fp.name;
        fpObj["description"] = fp.description;
        fpObj["category"] = fp.category;
        fpObj["manufacturer"] = fp.manufacturer;
        fpObj["frequency"] = fp.frequency;
        fpObj["modulation"] = fp.modulation;
        fpObj["pattern"] = fp.pattern;
        fpObj["pulseWidth"] = fp.pulseWidth;
        fpObj["bitCount"] = fp.bitCount;
        fpObj["protocol"] = fp.protocol;
        fpObj["confidence"] = fp.confidence;
        fpObj["region"] = fp.region;
        fpObj["isRollingCode"] = fp.isRollingCode;
        fpObj["notes"] = fp.notes;
    }
    
    doc["version"] = "1.0";
    doc["lastUpdate"] = millis();
    doc["count"] = fingerprints.size();
    
    File dbFile = sdManager.open(databasePath, FILE_WRITE);
    if (!dbFile) {
        Serial.println("❌ Failed to open database file for writing");
        return false;
    }
    
    serializeJson(doc, dbFile);
    dbFile.close();
    
    Serial.println("✅ Database saved successfully");
    return true;
}

std::vector<SignalMatch> SignalDatabase::identifySignal(float frequency, const String& pattern, const String& modulation) {
    std::vector<SignalMatch> matches;
    
    for (const auto& fp : fingerprints) {
        SignalMatch match;
        match.fingerprint = fp;
        match.matchConfidence = 0.0;
        match.matchType = "none";
        
        // Check frequency match
        float freqScore = compareFrequencies(frequency, fp.frequency);
        
        // Check pattern match
        float patternScore = comparePatterns(pattern, fp.pattern);
        
        // Check modulation match
        float modScore = 1.0;
        if (!modulation.isEmpty() && !fp.modulation.isEmpty()) {
            modScore = (modulation.equalsIgnoreCase(fp.modulation)) ? 1.0 : 0.5;
        }
        
        // Calculate overall confidence
        match.matchConfidence = (freqScore * 0.4 + patternScore * 0.5 + modScore * 0.1);
        
        if (match.matchConfidence > 0.7) {
            match.matchType = "exact";
        } else if (match.matchConfidence > 0.5) {
            match.matchType = "partial";
        } else if (match.matchConfidence > 0.3) {
            match.matchType = "pattern";
        }
        
        if (match.matchConfidence > 0.3) {
            matches.push_back(match);
        }
    }
    
    // Sort by confidence
    std::sort(matches.begin(), matches.end(), 
              [](const SignalMatch& a, const SignalMatch& b) {
                  return a.matchConfidence > b.matchConfidence;
              });
    
    return matches;
}

void SignalDatabase::loadGarageDoorDatabase() {
    Serial.println("🏠 Loading garage door database...");
    
    // Chamberlain/LiftMaster
    SignalFingerprint fp;
    fp.name = "Chamberlain 390MHz";
    fp.description = "Chamberlain/LiftMaster garage door opener";
    fp.category = "garage_door";
    fp.manufacturer = "Chamberlain";
    fp.frequency = 390.0;
    fp.modulation = "ASK/OOK";
    fp.pattern = "101010101100110011001100";
    fp.pulseWidth = 500;
    fp.bitCount = 24;
    fp.protocol = "Chamberlain DIP";
    fp.confidence = 0.8;
    fp.region = "North America";
    fp.isRollingCode = false;
    fp.notes = "Fixed code, vulnerable to replay attacks";
    fingerprints.push_back(fp);
    
    // Genie
    fp = {};
    fp.name = "Genie 315MHz";
    fp.description = "Genie garage door opener remote";
    fp.category = "garage_door";
    fp.manufacturer = "Genie";
    fp.frequency = 315.0;
    fp.modulation = "ASK/OOK";
    fp.pattern = "110011001010101011001100";
    fp.pulseWidth = 400;
    fp.bitCount = 24;
    fp.protocol = "Genie DIP";
    fp.confidence = 0.8;
    fp.region = "North America";
    fp.isRollingCode = false;
    fp.notes = "DIP switch based, easily cloned";
    fingerprints.push_back(fp);
    
    // Modern rolling code
    fp = {};
    fp.name = "Chamberlain Security+";
    fp.description = "Modern Chamberlain with rolling code";
    fp.category = "garage_door";
    fp.manufacturer = "Chamberlain";
    fp.frequency = 315.0;
    fp.modulation = "ASK/OOK";
    fp.pattern = "ROLLING_CODE";
    fp.pulseWidth = 300;
    fp.bitCount = 64;
    fp.protocol = "Security+ 2.0";
    fp.confidence = 0.9;
    fp.region = "Global";
    fp.isRollingCode = true;
    fp.notes = "Rolling code - replay attacks ineffective";
    fingerprints.push_back(fp);
}

void SignalDatabase::loadCarKeyDatabase() {
    Serial.println("🚗 Loading car key database...");
    
    // Toyota
    SignalFingerprint fp;
    fp.name = "Toyota Key Fob";
    fp.description = "Toyota car key remote";
    fp.category = "car_key";
    fp.manufacturer = "Toyota";
    fp.frequency = 433.92;
    fp.modulation = "ASK/OOK";
    fp.pattern = "1100110010101010";
    fp.pulseWidth = 400;
    fp.bitCount = 16;
    fp.protocol = "Toyota Fixed";
    fp.confidence = 0.7;
    fp.region = "Global";
    fp.isRollingCode = false;
    fp.notes = "Older Toyota models, fixed code";
    fingerprints.push_back(fp);
    
    // BMW
    fp = {};
    fp.name = "BMW Key Fob";
    fp.description = "BMW car key remote";
    fp.category = "car_key";
    fp.manufacturer = "BMW";
    fp.frequency = 868.0;
    fp.modulation = "FSK";
    fp.pattern = "ROLLING_CODE";
    fp.pulseWidth = 200;
    fp.bitCount = 128;
    fp.protocol = "BMW Rolling";
    fp.confidence = 0.9;
    fp.region = "Europe";
    fp.isRollingCode = true;
    fp.notes = "Modern BMW with encryption";
    fingerprints.push_back(fp);
    
    // Ford
    fp = {};
    fp.name = "Ford Key Fob";
    fp.description = "Ford car key remote";
    fp.category = "car_key";
    fp.manufacturer = "Ford";
    fp.frequency = 315.0;
    fp.modulation = "ASK/OOK";
    fp.pattern = "10101100110011001010";
    fp.pulseWidth = 500;
    fp.bitCount = 20;
    fp.protocol = "Ford Fixed";
    fp.confidence = 0.8;
    fp.region = "North America";
    fp.isRollingCode = false;
    fp.notes = "Older Ford models, vulnerable";
    fingerprints.push_back(fp);
}

void SignalDatabase::loadRemoteControlDatabase() {
    Serial.println("📺 Loading remote control database...");
    
    // TV Remote
    SignalFingerprint fp;
    fp.name = "Generic TV Remote";
    fp.description = "Generic TV power button";
    fp.category = "tv_remote";
    fp.manufacturer = "Generic";
    fp.frequency = 433.92;
    fp.modulation = "ASK/OOK";
    fp.pattern = "1010101011001100";
    fp.pulseWidth = 600;
    fp.bitCount = 16;
    fp.protocol = "RC5/NEC";
    fp.confidence = 0.6;
    fp.region = "Global";
    fp.isRollingCode = false;
    fp.notes = "Standard TV remote protocol";
    fingerprints.push_back(fp);
    
    // Ceiling Fan
    fp = {};
    fp.name = "Ceiling Fan Remote";
    fp.description = "Ceiling fan speed control";
    fp.category = "home_automation";
    fp.manufacturer = "Generic";
    fp.frequency = 433.92;
    fp.modulation = "ASK/OOK";
    fp.pattern = "110011001100110010101010";
    fp.pulseWidth = 400;
    fp.bitCount = 24;
    fp.protocol = "Fan Control";
    fp.confidence = 0.7;
    fp.region = "Global";
    fp.isRollingCode = false;
    fp.notes = "DIP switch based fan control";
    fingerprints.push_back(fp);
}

void SignalDatabase::loadSecuritySystemDatabase() {
    Serial.println("🔒 Loading security system database...");
    
    // Door/Window Sensor
    SignalFingerprint fp;
    fp.name = "Door Sensor";
    fp.description = "Wireless door/window sensor";
    fp.category = "security";
    fp.manufacturer = "Generic";
    fp.frequency = 433.92;
    fp.modulation = "ASK/OOK";
    fp.pattern = "101010101100110011001100101010";
    fp.pulseWidth = 300;
    fp.bitCount = 30;
    fp.protocol = "Security Sensor";
    fp.confidence = 0.8;
    fp.region = "Global";
    fp.isRollingCode = false;
    fp.notes = "Basic door sensor, often unencrypted";
    fingerprints.push_back(fp);
    
    // Motion Detector
    fp = {};
    fp.name = "PIR Motion Sensor";
    fp.description = "Wireless PIR motion detector";
    fp.category = "security";
    fp.manufacturer = "Generic";
    fp.frequency = 868.0;
    fp.modulation = "FSK";
    fp.pattern = "11001100101010101100110010101010";
    fp.pulseWidth = 250;
    fp.bitCount = 32;
    fp.protocol = "PIR Protocol";
    fp.confidence = 0.8;
    fp.region = "Europe";
    fp.isRollingCode = false;
    fp.notes = "Motion sensor with device ID";
    fingerprints.push_back(fp);
}

void SignalDatabase::loadWeatherStationDatabase() {
    Serial.println("🌡️ Loading weather station database...");
    
    SignalFingerprint fp;
    fp.name = "Weather Station";
    fp.description = "Wireless weather station sensor";
    fp.category = "weather";
    fp.manufacturer = "Generic";
    fp.frequency = 433.92;
    fp.modulation = "ASK/OOK";
    fp.pattern = "1100110010101010110011001010101011001100";
    fp.pulseWidth = 500;
    fp.bitCount = 40;
    fp.protocol = "Weather Protocol";
    fp.confidence = 0.7;
    fp.region = "Global";
    fp.isRollingCode = false;
    fp.notes = "Temperature/humidity data transmission";
    fingerprints.push_back(fp);
}

float SignalDatabase::comparePatterns(const String& pattern1, const String& pattern2) {
    if (pattern1.isEmpty() || pattern2.isEmpty()) return 0.0;
    if (pattern1 == "ROLLING_CODE" || pattern2 == "ROLLING_CODE") return 0.5;
    
    int matches = 0;
    int total = min(pattern1.length(), pattern2.length());
    
    for (int i = 0; i < total; i++) {
        if (pattern1.charAt(i) == pattern2.charAt(i)) {
            matches++;
        }
    }
    
    return (float)matches / total;
}

float SignalDatabase::compareFrequencies(float freq1, float freq2) {
    float diff = abs(freq1 - freq2);
    if (diff < 0.01) return 1.0;      // Exact match
    if (diff < 0.1) return 0.9;       // Very close
    if (diff < 0.5) return 0.7;       // Close
    if (diff < 1.0) return 0.5;       // Somewhat close
    return 0.0;                       // Too far
}

SignalMatch SignalDatabase::getBestMatch(float frequency, const String& pattern) {
    auto matches = identifySignal(frequency, pattern);
    if (matches.empty()) {
        SignalMatch noMatch;
        noMatch.matchConfidence = 0.0;
        noMatch.matchType = "none";
        return noMatch;
    }
    return matches[0];
}

String SignalDatabase::analyzeSignalPattern(const String& pattern) {
    String analysis = "Pattern Analysis:\n";
    analysis += "Length: " + String(pattern.length()) + " bits\n";
    
    int ones = 0, zeros = 0;
    for (char c : pattern) {
        if (c == '1') ones++;
        else if (c == '0') zeros++;
    }
    
    analysis += "Ones: " + String(ones) + ", Zeros: " + String(zeros) + "\n";
    analysis += "Balance: " + String((float)ones / pattern.length() * 100) + "% ones\n";
    
    // Look for repeating patterns
    if (pattern.indexOf("1010") >= 0) analysis += "Contains alternating pattern\n";
    if (pattern.indexOf("1111") >= 0) analysis += "Contains long high sequence\n";
    if (pattern.indexOf("0000") >= 0) analysis += "Contains long low sequence\n";
    
    return analysis;
}

int SignalDatabase::getFingerprintCount() {
    return fingerprints.size();
}

std::vector<String> SignalDatabase::getCategories() {
    std::vector<String> cats;
    for (const auto& fp : fingerprints) {
        if (std::find(cats.begin(), cats.end(), fp.category) == cats.end()) {
            cats.push_back(fp.category);
        }
    }
    return cats;
}
