/*
 * CC1101 Library Test - Standalone version
 * 
 * This is a minimal test to check if CC1101 can be detected properly
 * Use this SEPARATELY from the main SDR firmware
 * 
 * INSTRUCTIONS:
 * 1. Close the main EvilCrow-SDR-Working.ino file
 * 2. Open ONLY this file in Arduino IDE
 * 3. Make sure ELECHOUSE_CC1101_SRC_DRV.h and .cpp are in the same folder
 * 4. Upload and check serial output
 */

#include "ELECHOUSE_CC1101_SRC_DRV.h"

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("🧪 CC1101 Library Test Starting...");
    
    // Initialize CC1101
    Serial.print("📡 Initializing CC1101... ");
    ELECHOUSE_cc1101.Init();
    Serial.println("Done");
    
    // Set frequency
    Serial.print("📻 Setting frequency to 433.92 MHz... ");
    ELECHOUSE_cc1101.setMHZ(433.92);
    Serial.println("Done");
    
    // Check CC1101 detection
    Serial.print("🔍 Reading CC1101 registers... ");
    byte partnum = ELECHOUSE_cc1101.getCC1101_PARTNUM();
    byte version = ELECHOUSE_cc1101.getCC1101_VERSION();
    
    Serial.println("Done");
    Serial.println("📊 Results:");
    Serial.println("   Part Number: 0x" + String(partnum, HEX));
    Serial.println("   Version: 0x" + String(version, HEX));
    
    if (partnum == 0x00 && version == 0x14) {
        Serial.println("✅ CC1101 DETECTED SUCCESSFULLY!");
        Serial.println("🎉 Hardware is working properly");
    } else {
        Serial.println("❌ CC1101 NOT DETECTED");
        Serial.println("🔧 Check hardware connections:");
        Serial.println("   - Power (3.3V)");
        Serial.println("   - SPI connections");
        Serial.println("   - Remove SD card if present");
    }
    
    // Test FIFO functions
    Serial.print("📦 Testing FIFO functions... ");
    byte rxBytes = ELECHOUSE_cc1101.Bytes_In_RXFIFO();
    byte txBytes = ELECHOUSE_cc1101.Bytes_In_TXFIFO();
    Serial.println("Done");
    Serial.println("   RX FIFO bytes: " + String(rxBytes));
    Serial.println("   TX FIFO bytes: " + String(txBytes));
    
    Serial.println("\n🏁 Test completed!");
}

void loop() {
    // Test continuous operation
    static unsigned long lastTest = 0;
    
    if (millis() - lastTest > 5000) {
        Serial.println("💓 Heartbeat - CC1101 status check");
        
        byte marcstate = ELECHOUSE_cc1101.getCC1101_MARCSTATE();
        byte rxbytes = ELECHOUSE_cc1101.getCC1101_RXBYTES();
        
        Serial.println("   MARC State: 0x" + String(marcstate, HEX));
        Serial.println("   RX Bytes: " + String(rxbytes));
        
        lastTest = millis();
    }
    
    delay(100);
}
