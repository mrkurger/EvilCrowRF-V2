#include "SDCardManager.h"

SDCardManager sdManager;

SDCardManager::SDCardManager() {
    isInitialized = false;
    csPin = 5;
    attempts = 0;
    lastCheck = 0;
    cc1101Active = false;
    lastCC1101Use = 0;
    sdSPI = nullptr;
}

SDCardManager::~SDCardManager() {
    if (sdSPI) {
        SD.end();
    }
}

bool SDCardManager::begin(int cs, SPIClass* spi) {
    csPin = cs;
    
    if (spi == nullptr) {
        // Create our own SPI instance for SD card
        static SPIClass sdSPIInstance(VSPI);
        sdSPI = &sdSPIInstance;
        sdSPI->begin(18, 19, 23, csPin); // SCK, MISO, MOSI, CS
    } else {
        sdSPI = spi;
    }
    
    Serial.println("💾 SDCardManager: Starting initialization...");
    Serial.println("📌 Using CS pin: " + String(csPin));
    
    // Multiple initialization attempts with progressive delays
    for (int attempt = 1; attempt <= 5; attempt++) {
        Serial.println("🔄 SD Init Attempt " + String(attempt) + "/5");
        
        // Progressive delay between attempts
        if (attempt > 1) {
            delay(attempt * 300);
        }
        
        // End any previous SD session
        SD.end();
        delay(100);
        
        // Try different frequencies
        uint32_t frequency = 4000000; // Start with 4MHz
        if (attempt > 2) frequency = 1000000; // Drop to 1MHz
        if (attempt > 3) frequency = 400000;  // Drop to 400kHz
        
        if (SD.begin(csPin, *sdSPI, frequency, "/sd", 10)) {
            isInitialized = true;
            attempts = attempt;
            Serial.println("✅ SD Card initialized on attempt " + String(attempt));
            
            // Test basic functionality
            if (testBasicFunctionality()) {
                Serial.println("✅ SD Card basic functionality test passed");
                return true;
            } else {
                Serial.println("⚠️  SD Card basic functionality test failed");
                isInitialized = false;
            }
        }
        
        Serial.println("❌ Attempt " + String(attempt) + " failed");
    }
    
    Serial.println("❌ SD Card initialization failed after 5 attempts");
    isInitialized = false;
    return false;
}

bool SDCardManager::testBasicFunctionality() {
    try {
        // Test card size
        uint64_t size = SD.cardSize();
        if (size == 0) {
            Serial.println("⚠️  Card size is 0");
            return false;
        }
        
        // Test file creation
        File testFile = SD.open("/test_sd.txt", FILE_WRITE);
        if (!testFile) {
            Serial.println("⚠️  Cannot create test file");
            return false;
        }
        
        testFile.println("SD test: " + String(millis()));
        testFile.close();
        
        // Test file reading
        testFile = SD.open("/test_sd.txt", FILE_READ);
        if (!testFile) {
            Serial.println("⚠️  Cannot read test file");
            return false;
        }
        testFile.close();
        
        // Clean up
        SD.remove("/test_sd.txt");
        
        return true;
    } catch (...) {
        Serial.println("⚠️  Exception during SD test");
        return false;
    }
}

bool SDCardManager::isAvailable() {
    if (!isInitialized) {
        return false;
    }
    
    // Check if we can still access the card
    unsigned long now = millis();
    if (now - lastCheck > 5000) { // Check every 5 seconds
        lastCheck = now;
        
        try {
            uint64_t size = SD.cardSize();
            if (size == 0) {
                Serial.println("⚠️  SD Card lost - attempting recovery");
                return reinitialize();
            }
        } catch (...) {
            Serial.println("⚠️  SD Card access error - attempting recovery");
            return reinitialize();
        }
    }
    
    return true;
}

bool SDCardManager::reinitialize() {
    Serial.println("🔄 Reinitializing SD Card...");
    isInitialized = false;
    delay(500);
    return begin(csPin, sdSPI);
}

void SDCardManager::setCC1101Active(bool active) {
    cc1101Active = active;
    if (active) {
        lastCC1101Use = millis();
    }
}

bool SDCardManager::canUseSD() {
    if (!isInitialized) {
        return false;
    }
    
    // If CC1101 is actively being used, wait
    if (cc1101Active) {
        return false;
    }
    
    // Wait a bit after CC1101 use to avoid conflicts
    unsigned long now = millis();
    if (now - lastCC1101Use < 100) { // 100ms safety margin
        return false;
    }
    
    return true;
}

void SDCardManager::waitForSafeAccess() {
    while (!canUseSD()) {
        delay(10);
    }
}

bool SDCardManager::exists(const String& path) {
    if (!canUseSD()) {
        waitForSafeAccess();
    }
    
    try {
        return SD.exists(path);
    } catch (...) {
        Serial.println("⚠️  SD exists() failed for: " + path);
        return false;
    }
}

File SDCardManager::open(const String& path, const char* mode) {
    if (!canUseSD()) {
        waitForSafeAccess();
    }
    
    try {
        return SD.open(path, mode);
    } catch (...) {
        Serial.println("⚠️  SD open() failed for: " + path);
        return File();
    }
}

bool SDCardManager::remove(const String& path) {
    if (!canUseSD()) {
        waitForSafeAccess();
    }
    
    try {
        return SD.remove(path);
    } catch (...) {
        Serial.println("⚠️  SD remove() failed for: " + path);
        return false;
    }
}

bool SDCardManager::mkdir(const String& path) {
    if (!canUseSD()) {
        waitForSafeAccess();
    }
    
    try {
        return SD.mkdir(path);
    } catch (...) {
        Serial.println("⚠️  SD mkdir() failed for: " + path);
        return false;
    }
}

uint64_t SDCardManager::cardSize() {
    if (!canUseSD()) {
        return 0;
    }
    
    try {
        return SD.cardSize();
    } catch (...) {
        Serial.println("⚠️  SD cardSize() failed");
        return 0;
    }
}

uint64_t SDCardManager::usedBytes() {
    if (!canUseSD()) {
        return 0;
    }
    
    try {
        return SD.usedBytes();
    } catch (...) {
        Serial.println("⚠️  SD usedBytes() failed");
        return 0;
    }
}

String SDCardManager::getStatus() {
    if (!isInitialized) {
        return "Not Initialized";
    }
    
    if (!canUseSD()) {
        return "Busy (CC1101 Active)";
    }
    
    uint64_t size = cardSize();
    if (size == 0) {
        return "Error/Disconnected";
    }
    
    uint64_t used = usedBytes();
    return "OK (" + String(size / (1024 * 1024)) + "MB, " + 
           String(used / 1024) + "KB used)";
}

bool SDCardManager::recover() {
    Serial.println("🔧 SD Card recovery initiated...");
    
    // Force end current session
    SD.end();
    delay(1000);
    
    // Reset SPI
    if (sdSPI) {
        sdSPI->end();
        delay(500);
        sdSPI->begin(18, 19, 23, csPin);
        delay(500);
    }
    
    // Try to reinitialize
    return begin(csPin, sdSPI);
}

void SDCardManager::forceReinit() {
    Serial.println("🔧 Forcing SD Card reinitialization...");
    isInitialized = false;
    recover();
}
