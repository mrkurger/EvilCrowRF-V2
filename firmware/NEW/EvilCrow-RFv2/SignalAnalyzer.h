#ifndef SIGNAL_ANALYZER_H
#define SIGNAL_ANALYZER_H

#include <Arduino.h>
#include <vector>
#include <complex>

// Signal analysis constants
#define MAX_SAMPLES 4096
#define MIN_PULSE_WIDTH 50
#define MAX_PULSE_WIDTH 50000
#define NOISE_THRESHOLD 100

// Modulation types
enum class ModulationType {
    UNKNOWN = 0,
    OOK,        // On-Off Keying
    ASK,        // Amplitude Shift Keying
    FSK,        // Frequency Shift Keying
    PSK,        // Phase Shift Keying
    GFSK,       // Gaussian FSK
    MSK,        // Minimum Shift Keying
    MANCHESTER, // Manchester encoding
    PWM         // Pulse Width Modulation
};

// Protocol types
enum class ProtocolType {
    UNKNOWN = 0,
    ROLLING_CODE,
    FIXED_CODE,
    WEATHER_STATION,
    GARAGE_DOOR,
    CAR_KEY,
    SMART_HOME,
    TESLA,
    TV_REMOTE,
    DOORBELL,
    SENSOR
};

// Signal characteristics structure
struct SignalCharacteristics {
    ModulationType modulation;
    ProtocolType protocol;
    float frequency;
    float bandwidth;
    float symbolRate;
    float deviation;
    int pulseCount;
    int bitCount;
    float snr;
    float rssi;
    bool isValid;
    String description;
    uint32_t timestamp;
};

// Pulse analysis structure
struct PulseAnalysis {
    uint32_t shortPulse;
    uint32_t longPulse;
    uint32_t syncPulse;
    uint32_t guardTime;
    float dutyCycle;
    int pulseRatio;
    bool hasSync;
    bool isConsistent;
};

// Spectral analysis structure
struct SpectralInfo {
    float centerFreq;
    float bandwidth;
    float peakPower;
    float avgPower;
    std::vector<float> spectrum;
    ModulationType detectedMod;
};

// TV Remote database entry
struct TVRemoteCode {
    String brand;
    String model;
    uint32_t powerOnCode;
    uint32_t powerOffCode;
    float frequency;
    ModulationType modulation;
    int bitLength;
    String protocol;
};

class SignalAnalyzer {
private:
    uint32_t samples[MAX_SAMPLES];
    int sampleCount;
    SignalCharacteristics lastAnalysis;
    PulseAnalysis pulseStats;
    SpectralInfo spectralData;
    
    // TV Remote database
    std::vector<TVRemoteCode> tvDatabase;
    
    // Analysis methods
    bool analyzePulses();
    bool detectModulation();
    bool identifyProtocol();
    bool performFFT();
    float calculateSNR();
    float calculateRSSI();
    bool detectManchester();
    bool detectPWM();
    bool detectRollingCode();
    
    // TV Remote methods
    void initTVDatabase();
    TVRemoteCode* findTVCode(const String& brand);
    bool transmitTVCode(const TVRemoteCode& code, bool powerOn);
    
    // Pattern matching
    bool matchWeatherStation();
    bool matchGarageDoor();
    bool matchCarKey();
    bool matchSmartHome();
    bool matchTesla();
    bool matchDoorbell();
    
public:
    SignalAnalyzer();
    ~SignalAnalyzer();
    
    // Main analysis functions
    bool addSample(uint32_t duration);
    bool analyzeCapturedSignal();
    SignalCharacteristics getLastAnalysis();
    String getAnalysisReport();
    String getAnalysisJSON();
    
    // Signal generation
    bool generateSignal(const SignalCharacteristics& characteristics);
    bool replayLastSignal();
    bool transmitRawPulses(uint32_t* pulses, int count);
    
    // TV Remote control functions
    bool turnOffTV(const String& brand = "");
    bool turnOnTV(const String& brand = "");
    bool scanAndTurnOffTV();
    std::vector<String> getSupportedTVBrands();
    
    // Advanced features
    bool bruteForceAttack(ProtocolType protocol);
    bool rollingCodePredict();
    bool jamSignal(float frequency, uint32_t duration);
    
    // Utility functions
    void reset();
    void clearSamples();
    bool isSignalValid();
    float getConfidenceLevel();
    
    // Debug functions
    void printPulseStats();
    void printSpectralInfo();
    String dumpRawData();
};

// Global TV Remote database - Popular brands and codes
extern const TVRemoteCode TV_CODES[];
extern const int TV_CODE_COUNT;

#endif // SIGNAL_ANALYZER_H
