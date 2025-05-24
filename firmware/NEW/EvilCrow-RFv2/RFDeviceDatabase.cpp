#include "RFDeviceDatabase.h"

RFDeviceDatabase rfDeviceDB;

RFDeviceDatabase::RFDeviceDatabase() {
    // Constructor
}

RFDeviceDatabase::~RFDeviceDatabase() {
    // Destructor
}

void RFDeviceDatabase::initializeDatabase() {
    Serial.println("📡 Initializing RF Device Database...");
    
    devices.clear();
    categories.clear();
    
    loadGarageDoorsAndGates();
    loadCarKeyFobs();
    loadHomeAutomation();
    loadSecuritySystems();
    loadToys();
    loadMedicalDevices();
    loadIndustrialDevices();
    loadWeatherStations();
    loadBabyMonitors();
    loadWirelessDoorbells();
    loadRemoteControls();
    loadDroneControls();
    loadModelAircraft();
    loadWirelessSensors();
    loadSmartHomeDevices();
    
    Serial.println("✅ RF Device Database loaded: " + String(devices.size()) + " devices, " + String(categories.size()) + " categories");
}

void RFDeviceDatabase::loadGarageDoorsAndGates() {
    // Category
    RFDeviceCategory category;
    category.name = "garage_doors";
    category.description = "Garage doors, gates, and barriers";
    category.commonFrequencies = {315.0, 390.0, 433.92, 868.0, 915.0};
    category.vulnerabilities = "Many use fixed codes, vulnerable to replay attacks";
    category.attackMethods = "Code grabbing, replay attacks, brute force";
    category.legalNotes = "Only test on your own property";
    categories.push_back(category);
    
    // Devices
    RFDevice device;
    
    // Chamberlain/LiftMaster
    device = {};
    device.name = "Chamberlain MyQ";
    device.category = "garage_doors";
    device.manufacturer = "Chamberlain";
    device.frequency = 315.0;
    device.modulation = "ASK/OOK";
    device.protocol = "Security+ 2.0";
    device.description = "Modern garage door opener with rolling code";
    device.vulnerabilities = "Older models use fixed codes";
    device.attackVectors = "Replay attacks on older models, jamming";
    device.hasRollingCode = true;
    device.region = "North America";
    device.notes = "Very common in US, newer models have better security";
    devices.push_back(device);
    
    // Genie
    device = {};
    device.name = "Genie Intellicode";
    device.category = "garage_doors";
    device.manufacturer = "Genie";
    device.frequency = 315.0;
    device.modulation = "ASK/OOK";
    device.protocol = "Intellicode";
    device.description = "Genie garage door with rolling code";
    device.vulnerabilities = "DIP switch models vulnerable";
    device.attackVectors = "Replay attacks on DIP models, code grabbing";
    device.hasRollingCode = true;
    device.region = "North America";
    device.notes = "Check for DIP switch vs rolling code models";
    devices.push_back(device);
    
    // Linear/Nortek
    device = {};
    device.name = "Linear MegaCode";
    device.category = "garage_doors";
    device.manufacturer = "Linear";
    device.frequency = 318.0;
    device.modulation = "ASK/OOK";
    device.protocol = "MegaCode";
    device.description = "Linear garage door and gate opener";
    device.vulnerabilities = "Some models use predictable codes";
    device.attackVectors = "Code prediction, replay attacks";
    device.hasRollingCode = false;
    device.region = "North America";
    device.notes = "Popular for commercial gates";
    devices.push_back(device);
}

void RFDeviceDatabase::loadCarKeyFobs() {
    // Category
    RFDeviceCategory category;
    category.name = "car_keys";
    category.description = "Car key fobs and remote controls";
    category.commonFrequencies = {315.0, 433.92, 868.0, 915.0};
    category.vulnerabilities = "Older models use fixed codes, relay attacks possible";
    category.attackMethods = "Replay attacks, relay attacks, jamming";
    category.legalNotes = "Illegal to attack vehicles you don't own";
    categories.push_back(category);
    
    RFDevice device;
    
    // Toyota
    device = {};
    device.name = "Toyota Key Fob";
    device.category = "car_keys";
    device.manufacturer = "Toyota";
    device.frequency = 433.92;
    device.modulation = "ASK/OOK";
    device.protocol = "Toyota Protocol";
    device.description = "Toyota car remote control";
    device.vulnerabilities = "Older models use fixed codes";
    device.attackVectors = "Replay attacks on pre-2010 models";
    device.hasRollingCode = false; // Older models
    device.region = "Global";
    device.notes = "Very common target, check model year";
    devices.push_back(device);
    
    // BMW
    device = {};
    device.name = "BMW Comfort Access";
    device.category = "car_keys";
    device.manufacturer = "BMW";
    device.frequency = 868.0;
    device.modulation = "FSK";
    device.protocol = "BMW CAS";
    device.description = "BMW keyless entry system";
    device.vulnerabilities = "Relay attacks possible";
    device.attackVectors = "Relay attacks, signal amplification";
    device.hasRollingCode = true;
    device.region = "Europe/Global";
    device.notes = "High-value target, advanced security";
    devices.push_back(device);
    
    // Ford
    device = {};
    device.name = "Ford PATS";
    device.category = "car_keys";
    device.manufacturer = "Ford";
    device.frequency = 315.0;
    device.modulation = "ASK/OOK";
    device.protocol = "PATS";
    device.description = "Ford Passive Anti-Theft System";
    device.vulnerabilities = "Some models vulnerable to replay";
    device.attackVectors = "Replay attacks, code grabbing";
    device.hasRollingCode = false;
    device.region = "North America";
    device.notes = "Common in older Ford vehicles";
    devices.push_back(device);
}

void RFDeviceDatabase::loadToys() {
    // Category
    RFDeviceCategory category;
    category.name = "toys";
    category.description = "RC toys, drones, and remote-controlled devices";
    category.commonFrequencies = {27.0, 40.0, 433.92, 868.0, 915.0, 2400.0};
    category.vulnerabilities = "Usually no security, easy to hijack";
    category.attackMethods = "Signal hijacking, control takeover, jamming";
    category.legalNotes = "Generally legal for educational purposes";
    categories.push_back(category);
    
    RFDevice device;
    
    // RC Cars
    device = {};
    device.name = "Generic RC Car";
    device.category = "toys";
    device.manufacturer = "Generic";
    device.frequency = 27.0;
    device.modulation = "AM";
    device.protocol = "RC Protocol";
    device.description = "Basic remote control car";
    device.vulnerabilities = "No security, easily hijacked";
    device.attackVectors = "Signal takeover, interference";
    device.hasRollingCode = false;
    device.region = "Global";
    device.notes = "Fun target for demonstrations";
    devices.push_back(device);
    
    // RC Helicopters
    device = {};
    device.name = "RC Helicopter";
    device.category = "toys";
    device.manufacturer = "Generic";
    device.frequency = 433.92;
    device.modulation = "ASK/OOK";
    device.protocol = "RC Protocol";
    device.description = "Remote control helicopter";
    device.vulnerabilities = "No authentication";
    device.attackVectors = "Control hijacking, crash attacks";
    device.hasRollingCode = false;
    device.region = "Global";
    device.notes = "Can cause physical damage if hijacked";
    devices.push_back(device);
    
    // Drones
    device = {};
    device.name = "Consumer Drone";
    device.category = "toys";
    device.manufacturer = "Generic";
    device.frequency = 2400.0;
    device.modulation = "FHSS";
    device.protocol = "2.4GHz RC";
    device.description = "Consumer quadcopter drone";
    device.vulnerabilities = "Weak authentication on some models";
    device.attackVectors = "Control takeover, GPS spoofing";
    device.hasRollingCode = false;
    device.region = "Global";
    device.notes = "Legal restrictions on drone interference";
    devices.push_back(device);
}

void RFDeviceDatabase::loadBabyMonitors() {
    // Category
    RFDeviceCategory category;
    category.name = "baby_monitors";
    category.description = "Baby monitors and surveillance cameras";
    category.commonFrequencies = {433.92, 868.0, 915.0, 2400.0};
    category.vulnerabilities = "Often unencrypted, privacy concerns";
    category.attackMethods = "Eavesdropping, signal interception";
    category.legalNotes = "Illegal to intercept others' communications";
    categories.push_back(category);
    
    RFDevice device;
    device = {};
    device.name = "Analog Baby Monitor";
    device.category = "baby_monitors";
    device.manufacturer = "Generic";
    device.frequency = 433.92;
    device.modulation = "FM";
    device.protocol = "Analog Audio";
    device.description = "Basic analog baby monitor";
    device.vulnerabilities = "Completely unencrypted";
    device.attackVectors = "Audio interception, privacy invasion";
    device.hasRollingCode = false;
    device.region = "Global";
    device.notes = "Major privacy risk, easily intercepted";
    devices.push_back(device);
}

void RFDeviceDatabase::loadWirelessDoorbells() {
    RFDevice device;
    device = {};
    device.name = "Wireless Doorbell";
    device.category = "home_automation";
    device.manufacturer = "Generic";
    device.frequency = 433.92;
    device.modulation = "ASK/OOK";
    device.protocol = "Simple Protocol";
    device.description = "Basic wireless doorbell";
    device.vulnerabilities = "No security, easily triggered";
    device.attackVectors = "False triggering, DoS attacks";
    device.hasRollingCode = false;
    device.region = "Global";
    device.notes = "Annoying but harmless target";
    devices.push_back(device);
}

void RFDeviceDatabase::loadWeatherStations() {
    RFDevice device;
    device = {};
    device.name = "Weather Station";
    device.category = "sensors";
    device.manufacturer = "Generic";
    device.frequency = 433.92;
    device.modulation = "ASK/OOK";
    device.protocol = "Weather Protocol";
    device.description = "Wireless weather station sensor";
    device.vulnerabilities = "Data can be intercepted";
    device.attackVectors = "Data interception, false data injection";
    device.hasRollingCode = false;
    device.region = "Global";
    device.notes = "Contains temperature, humidity data";
    devices.push_back(device);
}

std::vector<RFDevice> RFDeviceDatabase::getDevicesByFrequency(float frequency, float tolerance) {
    std::vector<RFDevice> matches;
    
    for (const auto& device : devices) {
        if (abs(device.frequency - frequency) <= tolerance) {
            matches.push_back(device);
        }
    }
    
    return matches;
}

std::vector<RFDevice> RFDeviceDatabase::getDevicesByCategory(const String& category) {
    std::vector<RFDevice> matches;
    
    for (const auto& device : devices) {
        if (device.category.equalsIgnoreCase(category)) {
            matches.push_back(device);
        }
    }
    
    return matches;
}

String RFDeviceDatabase::identifyDevice(float frequency, const String& pattern) {
    auto matches = getDevicesByFrequency(frequency, 0.1);
    
    if (matches.empty()) {
        return "Unknown device on " + String(frequency) + " MHz";
    }
    
    String result = "Possible devices on " + String(frequency) + " MHz:\n";
    for (const auto& device : matches) {
        result += "• " + device.name + " (" + device.manufacturer + ")\n";
        result += "  Category: " + device.category + "\n";
        result += "  Vulnerabilities: " + device.vulnerabilities + "\n\n";
    }
    
    return result;
}

String RFDeviceDatabase::getAttackSuggestions(float frequency) {
    auto devices = getDevicesByFrequency(frequency, 0.1);
    
    if (devices.empty()) {
        return "No known devices on this frequency";
    }
    
    String suggestions = "Attack suggestions for " + String(frequency) + " MHz:\n\n";
    
    for (const auto& device : devices) {
        suggestions += "🎯 " + device.name + ":\n";
        suggestions += "   " + device.attackVectors + "\n";
        if (!device.hasRollingCode) {
            suggestions += "   ⚠️ No rolling code - replay attacks possible\n";
        }
        suggestions += "\n";
    }
    
    return suggestions;
}

int RFDeviceDatabase::getDeviceCount() {
    return devices.size();
}

String RFDeviceDatabase::getRandomDevice() {
    if (devices.empty()) return "No devices in database";
    
    int index = random(0, devices.size());
    const auto& device = devices[index];
    
    String info = "🎲 Random Device: " + device.name + "\n";
    info += "📡 Frequency: " + String(device.frequency) + " MHz\n";
    info += "🏷️ Category: " + device.category + "\n";
    info += "🔧 Manufacturer: " + device.manufacturer + "\n";
    info += "⚠️ Vulnerabilities: " + device.vulnerabilities + "\n";
    info += "🎯 Attack vectors: " + device.attackVectors + "\n";
    
    return info;
}

String RFDeviceDatabase::getGarageDoorAttacks() {
    return "🏠 Garage Door Attack Methods:\n"
           "1. Code Grabbing - Capture remote signals\n"
           "2. Replay Attacks - Retransmit captured codes\n"
           "3. Brute Force - Try all possible codes\n"
           "4. Jamming - Block legitimate signals\n"
           "5. Rolling Code Analysis - Predict next code\n\n"
           "⚠️ Only test on your own garage door!";
}

String RFDeviceDatabase::getCarKeyAttacks() {
    return "🚗 Car Key Attack Methods:\n"
           "1. Replay Attacks - Retransmit unlock codes\n"
           "2. Relay Attacks - Amplify key signals\n"
           "3. Jamming - Block lock signals\n"
           "4. Code Grabbing - Capture key fob signals\n"
           "5. Rolling Code Prediction - Advanced analysis\n\n"
           "⚖️ ILLEGAL to attack vehicles you don't own!";
}

String RFDeviceDatabase::getToyAttacks() {
    return "🎮 RC Toy Attack Methods:\n"
           "1. Control Hijacking - Take over remote control\n"
           "2. Signal Jamming - Disable remote control\n"
           "3. Crash Attacks - Send malformed commands\n"
           "4. Frequency Scanning - Find active toys\n"
           "5. Protocol Analysis - Reverse engineer commands\n\n"
           "😄 Generally safe and fun for demonstrations!";
}
