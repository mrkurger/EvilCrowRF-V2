#ifndef ADVANCED_RF_SCANNER_H
#define ADVANCED_RF_SCANNER_H

#include <Arduino.h>
#include <vector>

// Frequency bands for scanning
struct FrequencyBand {
    float startFreq;
    float endFreq;
    String name;
    String description;
};

// Signal detection result
struct SignalDetection {
    float frequency;
    float rssi;
    float bandwidth;
    String modulation;
    String protocol;
    unsigned long timestamp;
    bool isActive;
};

// Spectrum analysis point
struct SpectrumPoint {
    float frequency;
    float power;
    float noise;
    float snr;
};

class AdvancedRFScanner {
private:
    std::vector<FrequencyBand> frequencyBands;
    std::vector<SignalDetection> detectedSignals;
    std::vector<SpectrumPoint> spectrumData;
    
    float currentFreq;
    float scanStep;
    float dwellTime;
    bool isScanning;
    bool isAnalyzing;
    
    // Scanning parameters
    float rssiThreshold;
    int averagingCount;
    unsigned long lastScanTime;
    unsigned long scanStartTime;
    
    // Analysis parameters
    float noiseFloor;
    float signalThreshold;
    
public:
    AdvancedRFScanner();
    ~AdvancedRFScanner();
    
    // Initialization
    void begin();
    void initializeFrequencyBands();
    
    // Scanning functions
    bool startScan(float startFreq, float endFreq, float step = 0.1);
    bool startBandScan(const String& bandName);
    bool startFullScan();
    void stopScan();
    bool isScanActive();
    
    // Real-time scanning
    void updateScan();
    float scanFrequency(float freq);
    bool detectSignal(float freq, float rssi);
    
    // Spectrum analysis
    bool startSpectrumAnalysis(float centerFreq, float span);
    void updateSpectrumAnalysis();
    std::vector<SpectrumPoint> getSpectrumData();
    
    // Signal detection and analysis
    std::vector<SignalDetection> getDetectedSignals();
    SignalDetection analyzeSignal(float freq);
    String identifyProtocol(float freq, float rssi);
    String detectModulation(float freq);
    
    // Frequency band management
    void addFrequencyBand(float start, float end, const String& name, const String& desc);
    std::vector<FrequencyBand> getFrequencyBands();
    FrequencyBand* findBand(const String& name);
    
    // Configuration
    void setRSSIThreshold(float threshold);
    void setScanStep(float step);
    void setDwellTime(float time);
    void setAveragingCount(int count);
    
    // Results and reporting
    String getScanReport();
    String getSpectrumJSON();
    String getDetectedSignalsJSON();
    
    // Advanced features
    bool jamDetectedSignals();
    bool recordSignal(float freq, unsigned long duration);
    bool replaySignal(const String& signalData);
    
    // Utility functions
    void clearDetectedSignals();
    void clearSpectrumData();
    float calculateNoiseFloor();
    float calculateSNR(float signal, float noise);
    
    // Status functions
    float getScanProgress();
    String getScanStatus();
    unsigned long getScanDuration();
};

extern AdvancedRFScanner rfScanner;

#endif // ADVANCED_RF_SCANNER_H
