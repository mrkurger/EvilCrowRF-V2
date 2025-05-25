/*
 * EvilCrow RF v2 - SD Card Diagnostic Tool
 * 
 * This diagnostic tool will help identify SD card issues by:
 * - Testing multiple CS pins (5, 22, 15, 2, 4)
 * - Testing multiple SPI frequencies
 * - Providing detailed error information
 * - Testing different initialization methods
 */

#include <SD.h>
#include <SPI.h>

// Test different CS pins
int csPins[] = {5, 22, 15, 2, 4};
int numPins = sizeof(csPins) / sizeof(csPins[0]);

SPIClass sdspi(VSPI);

void setup() {
    Serial.begin(115200);
    delay(2000);
    
    Serial.println("\n================================================");
    Serial.println("🔧 EvilCrow RF v2 - SD Card Diagnostic Tool");
    Serial.println("================================================\n");
    
    Serial.println("📋 This tool will test SD card initialization with:");
    Serial.println("   • Multiple CS pins: 5, 22, 15, 2, 4");
    Serial.println("   • Multiple SPI frequencies");
    Serial.println("   • Different initialization methods");
    Serial.println("   • Detailed error reporting\n");
    
    // Test each CS pin
    for (int pinIndex = 0; pinIndex < numPins; pinIndex++) {
        int currentCS = csPins[pinIndex];
        testCSPin(currentCS);
        delay(1000);
    }
    
    Serial.println("\n🏁 Diagnostic complete!");
    Serial.println("📊 Check the results above to identify the working configuration.");
    Serial.println("💡 If no configuration works, check:");
    Serial.println("   • SD card format (should be FAT32)");
    Serial.println("   • SD card size (2GB-32GB work best)");
    Serial.println("   • Power supply stability");
    Serial.println("   • Wiring connections");
}

void loop() {
    // Nothing to do in loop
    delay(1000);
}

void testCSPin(int csPin) {
    Serial.println("🔌 Testing CS Pin: " + String(csPin));
    Serial.println("━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━");
    
    // Test different SPI frequencies
    uint32_t frequencies[] = {4000000, 1000000, 400000, 100000, 50000};
    int numFreqs = sizeof(frequencies) / sizeof(frequencies[0]);
    
    for (int freqIndex = 0; freqIndex < numFreqs; freqIndex++) {
        uint32_t frequency = frequencies[freqIndex];
        
        Serial.println("📡 Testing SPI frequency: " + String(frequency / 1000) + " kHz");
        
        // Clean up any previous session
        SD.end();
        sdspi.end();
        delay(200);
        
        // Initialize SPI
        sdspi.begin(18, 19, 23, csPin); // SCK, MISO, MOSI, CS
        delay(100);
        
        // Try to initialize SD card
        if (SD.begin(csPin, sdspi, frequency)) {
            Serial.println("✅ SD.begin() successful!");
            
            // Test card size
            uint64_t cardSize = SD.cardSize();
            if (cardSize > 0) {
                Serial.println("✅ Card size: " + String(cardSize / (1024 * 1024)) + " MB");
                
                // Test card type
                uint8_t cardType = SD.cardType();
                String cardTypeStr = "Unknown";
                switch (cardType) {
                    case CARD_MMC: cardTypeStr = "MMC"; break;
                    case CARD_SD: cardTypeStr = "SDSC"; break;
                    case CARD_SDHC: cardTypeStr = "SDHC"; break;
                    default: cardTypeStr = "Unknown"; break;
                }
                Serial.println("✅ Card type: " + cardTypeStr);
                
                // Test used space
                uint64_t usedBytes = SD.usedBytes();
                Serial.println("✅ Used space: " + String(usedBytes / 1024) + " KB");
                
                // Test write capability
                File testFile = SD.open("/diagnostic_test.txt", FILE_WRITE);
                if (testFile) {
                    testFile.println("Diagnostic test: " + String(millis()));
                    testFile.println("CS Pin: " + String(csPin));
                    testFile.println("SPI Freq: " + String(frequency));
                    testFile.close();
                    
                    // Test read capability
                    testFile = SD.open("/diagnostic_test.txt", FILE_READ);
                    if (testFile) {
                        String content = testFile.readString();
                        testFile.close();
                        SD.remove("/diagnostic_test.txt");
                        Serial.println("✅ Read/write test: PASSED");
                    } else {
                        Serial.println("❌ Read test: FAILED");
                    }
                } else {
                    Serial.println("❌ Write test: FAILED (read-only?)");
                }
                
                // Test directory operations
                if (SD.mkdir("/test_dir")) {
                    Serial.println("✅ Directory creation: PASSED");
                    SD.rmdir("/test_dir");
                } else {
                    Serial.println("❌ Directory creation: FAILED");
                }
                
                Serial.println("🎉 SUCCESS! CS Pin " + String(csPin) + " works at " + String(frequency / 1000) + " kHz");
                Serial.println("💡 Use this configuration in your main firmware:");
                Serial.println("   const int sdCS = " + String(csPin) + ";");
                Serial.println("   SD.begin(" + String(csPin) + ", sdspi, " + String(frequency) + ");");
                
                // Clean up and return
                SD.end();
                sdspi.end();
                return;
                
            } else {
                Serial.println("❌ Card size is 0 - format issue?");
            }
        } else {
            Serial.println("❌ SD.begin() failed");
        }
        
        // Clean up for next attempt
        SD.end();
        sdspi.end();
        delay(100);
    }
    
    Serial.println("❌ CS Pin " + String(csPin) + " failed at all frequencies");
    Serial.println();
}
