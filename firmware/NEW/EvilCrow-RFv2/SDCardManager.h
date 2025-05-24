#ifndef SDCARD_MANAGER_H
#define SDCARD_MANAGER_H

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

class SDCardManager {
private:
    SPIClass* sdSPI;
    bool isInitialized;
    int csPin;
    int attempts;
    unsigned long lastCheck;
    
    // SPI conflict resolution
    bool cc1101Active;
    unsigned long lastCC1101Use;
    
public:
    SDCardManager();
    ~SDCardManager();
    
    // Core functions
    bool begin(int cs = 5, SPIClass* spi = nullptr);
    bool isAvailable();
    bool reinitialize();
    
    // SPI conflict management
    void setCC1101Active(bool active);
    bool canUseSD();
    void waitForSafeAccess();
    
    // File operations with conflict handling
    bool exists(const String& path);
    File open(const String& path, const char* mode = FILE_READ);
    bool remove(const String& path);
    bool mkdir(const String& path);
    
    // Status functions
    uint64_t cardSize();
    uint64_t usedBytes();
    String getStatus();
    
    // Recovery functions
    bool recover();
    void forceReinit();
};

extern SDCardManager sdManager;

#endif // SDCARD_MANAGER_H
