#include "AdvancedRFScanner.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"

AdvancedRFScanner rfScanner;

AdvancedRFScanner::AdvancedRFScanner() {
    currentFreq = 433.92;
    scanStep = 0.1;
    dwellTime = 10; // ms
    isScanning = false;
    isAnalyzing = false;
    rssiThreshold = -80.0;
    averagingCount = 5;
    lastScanTime = 0;
    scanStartTime = 0;
    noiseFloor = -100.0;
    signalThreshold = -70.0;
}

AdvancedRFScanner::~AdvancedRFScanner() {
    stopScan();
}

void AdvancedRFScanner::begin() {
    Serial.println("🔍 Initializing Advanced RF Scanner...");
    initializeFrequencyBands();
    calculateNoiseFloor();
    Serial.println("✅ RF Scanner initialized");
}

void AdvancedRFScanner::initializeFrequencyBands() {
    // Clear existing bands
    frequencyBands.clear();
    
    // Add common frequency bands
    addFrequencyBand(300.0, 348.0, "300MHz", "Low UHF band");
    addFrequencyBand(387.0, 464.0, "433MHz", "ISM band (433.92 MHz)");
    addFrequencyBand(779.0, 928.0, "868MHz", "European ISM band");
    addFrequencyBand(902.0, 928.0, "915MHz", "North American ISM band");
    addFrequencyBand(2400.0, 2500.0, "2.4GHz", "WiFi/Bluetooth band");
    
    // Add specific protocol bands
    addFrequencyBand(433.05, 434.79, "LPD433", "Low Power Devices");
    addFrequencyBand(868.0, 868.6, "SRD868", "Short Range Devices");
    addFrequencyBand(315.0, 315.0, "315MHz", "North American garage doors");
    addFrequencyBand(390.0, 390.0, "390MHz", "European car keys");
    
    Serial.println("📡 Loaded " + String(frequencyBands.size()) + " frequency bands");
}

bool AdvancedRFScanner::startScan(float startFreq, float endFreq, float step) {
    if (isScanning) {
        Serial.println("⚠️ Scan already in progress");
        return false;
    }
    
    Serial.println("🔍 Starting frequency scan: " + String(startFreq) + " - " + String(endFreq) + " MHz");
    
    currentFreq = startFreq;
    scanStep = step;
    isScanning = true;
    scanStartTime = millis();
    
    // Clear previous results
    detectedSignals.clear();
    
    return true;
}

bool AdvancedRFScanner::startBandScan(const String& bandName) {
    FrequencyBand* band = findBand(bandName);
    if (!band) {
        Serial.println("❌ Band not found: " + bandName);
        return false;
    }
    
    Serial.println("🔍 Starting band scan: " + band->name + " (" + band->description + ")");
    return startScan(band->startFreq, band->endFreq, 0.05);
}

bool AdvancedRFScanner::startFullScan() {
    Serial.println("🔍 Starting full spectrum scan (300-928 MHz)");
    return startScan(300.0, 928.0, 0.1);
}

void AdvancedRFScanner::stopScan() {
    if (isScanning) {
        isScanning = false;
        Serial.println("⏹️ Scan stopped. Found " + String(detectedSignals.size()) + " signals");
    }
}

bool AdvancedRFScanner::isScanActive() {
    return isScanning;
}

void AdvancedRFScanner::updateScan() {
    if (!isScanning) return;
    
    unsigned long now = millis();
    if (now - lastScanTime < dwellTime) return;
    
    lastScanTime = now;
    
    // Scan current frequency
    float rssi = scanFrequency(currentFreq);
    
    // Check for signal detection
    if (detectSignal(currentFreq, rssi)) {
        SignalDetection signal;
        signal.frequency = currentFreq;
        signal.rssi = rssi;
        signal.timestamp = now;
        signal.modulation = detectModulation(currentFreq);
        signal.protocol = identifyProtocol(currentFreq, rssi);
        signal.isActive = true;
        
        detectedSignals.push_back(signal);
        
        Serial.println("📡 Signal detected: " + String(currentFreq) + " MHz, RSSI: " + String(rssi) + " dBm");
    }
    
    // Move to next frequency
    currentFreq += scanStep;
    
    // Check if scan is complete
    bool scanComplete = false;
    for (const auto& band : frequencyBands) {
        if (currentFreq > band.endFreq) {
            scanComplete = true;
            break;
        }
    }
    
    if (scanComplete) {
        stopScan();
    }
}

float AdvancedRFScanner::scanFrequency(float freq) {
    // Set CC1101 to the frequency
    ELECHOUSE_cc1101.setMHZ(freq);
    delay(5); // Allow settling time
    
    // Take multiple RSSI measurements and average
    float totalRSSI = 0;
    for (int i = 0; i < averagingCount; i++) {
        float rssi = ELECHOUSE_cc1101.getRssi();
        totalRSSI += rssi;
        delay(1);
    }
    
    return totalRSSI / averagingCount;
}

bool AdvancedRFScanner::detectSignal(float freq, float rssi) {
    // Simple threshold detection
    if (rssi > rssiThreshold) {
        // Additional validation - check if signal is above noise floor
        float snr = calculateSNR(rssi, noiseFloor);
        return snr > 10.0; // 10 dB SNR threshold
    }
    return false;
}

bool AdvancedRFScanner::startSpectrumAnalysis(float centerFreq, float span) {
    Serial.println("📊 Starting spectrum analysis: " + String(centerFreq) + " MHz ± " + String(span/2) + " MHz");
    
    isAnalyzing = true;
    spectrumData.clear();
    
    float startFreq = centerFreq - (span / 2);
    float endFreq = centerFreq + (span / 2);
    float step = span / 100; // 100 points across the span
    
    for (float freq = startFreq; freq <= endFreq; freq += step) {
        SpectrumPoint point;
        point.frequency = freq;
        point.power = scanFrequency(freq);
        point.noise = noiseFloor;
        point.snr = calculateSNR(point.power, point.noise);
        
        spectrumData.push_back(point);
    }
    
    isAnalyzing = false;
    Serial.println("✅ Spectrum analysis complete: " + String(spectrumData.size()) + " points");
    return true;
}

std::vector<SignalDetection> AdvancedRFScanner::getDetectedSignals() {
    return detectedSignals;
}

String AdvancedRFScanner::identifyProtocol(float freq, float rssi) {
    // Simple protocol identification based on frequency
    if (freq >= 433.05 && freq <= 434.79) {
        if (freq == 433.92) return "Generic 433MHz";
        if (freq >= 433.05 && freq <= 433.15) return "Car Remote";
        if (freq >= 433.85 && freq <= 433.95) return "Garage Door";
        return "LPD433";
    }
    
    if (freq >= 868.0 && freq <= 868.6) {
        return "SRD868";
    }
    
    if (freq == 315.0) {
        return "Garage Door (US)";
    }
    
    if (freq >= 390.0 && freq <= 390.1) {
        return "Car Key (EU)";
    }
    
    return "Unknown";
}

String AdvancedRFScanner::detectModulation(float freq) {
    // Simplified modulation detection
    // In a real implementation, this would analyze the signal characteristics
    return "ASK/OOK"; // Most common for these frequencies
}

void AdvancedRFScanner::addFrequencyBand(float start, float end, const String& name, const String& desc) {
    FrequencyBand band;
    band.startFreq = start;
    band.endFreq = end;
    band.name = name;
    band.description = desc;
    frequencyBands.push_back(band);
}

std::vector<FrequencyBand> AdvancedRFScanner::getFrequencyBands() {
    return frequencyBands;
}

FrequencyBand* AdvancedRFScanner::findBand(const String& name) {
    for (auto& band : frequencyBands) {
        if (band.name == name) {
            return &band;
        }
    }
    return nullptr;
}

void AdvancedRFScanner::setRSSIThreshold(float threshold) {
    rssiThreshold = threshold;
    Serial.println("🔧 RSSI threshold set to: " + String(threshold) + " dBm");
}

String AdvancedRFScanner::getScanReport() {
    String report = "📊 RF Scan Report\n";
    report += "================\n";
    report += "Scan Duration: " + String(getScanDuration()) + " ms\n";
    report += "Signals Found: " + String(detectedSignals.size()) + "\n\n";
    
    for (const auto& signal : detectedSignals) {
        report += "📡 " + String(signal.frequency) + " MHz\n";
        report += "   RSSI: " + String(signal.rssi) + " dBm\n";
        report += "   Protocol: " + signal.protocol + "\n";
        report += "   Modulation: " + signal.modulation + "\n\n";
    }
    
    return report;
}

String AdvancedRFScanner::getDetectedSignalsJSON() {
    String json = "{\"signals\":[";
    
    for (size_t i = 0; i < detectedSignals.size(); i++) {
        const auto& signal = detectedSignals[i];
        json += "{";
        json += "\"frequency\":" + String(signal.frequency) + ",";
        json += "\"rssi\":" + String(signal.rssi) + ",";
        json += "\"protocol\":\"" + signal.protocol + "\",";
        json += "\"modulation\":\"" + signal.modulation + "\",";
        json += "\"timestamp\":" + String(signal.timestamp);
        json += "}";
        
        if (i < detectedSignals.size() - 1) {
            json += ",";
        }
    }
    
    json += "],\"count\":" + String(detectedSignals.size()) + "}";
    return json;
}

float AdvancedRFScanner::calculateNoiseFloor() {
    // Measure noise floor by scanning a quiet frequency
    float quietFreq = 434.5; // Usually quiet
    ELECHOUSE_cc1101.setMHZ(quietFreq);
    delay(10);
    
    float totalNoise = 0;
    int samples = 20;
    
    for (int i = 0; i < samples; i++) {
        totalNoise += ELECHOUSE_cc1101.getRssi();
        delay(5);
    }
    
    noiseFloor = totalNoise / samples;
    Serial.println("📊 Noise floor: " + String(noiseFloor) + " dBm");
    return noiseFloor;
}

float AdvancedRFScanner::calculateSNR(float signal, float noise) {
    return signal - noise;
}

void AdvancedRFScanner::clearDetectedSignals() {
    detectedSignals.clear();
    Serial.println("🗑️ Cleared detected signals");
}

float AdvancedRFScanner::getScanProgress() {
    if (!isScanning) return 100.0;
    
    // Calculate progress based on current frequency and scan range
    // This is a simplified calculation
    return 50.0; // Placeholder
}

String AdvancedRFScanner::getScanStatus() {
    if (isScanning) {
        return "Scanning " + String(currentFreq) + " MHz";
    } else if (isAnalyzing) {
        return "Analyzing spectrum";
    } else {
        return "Idle";
    }
}

unsigned long AdvancedRFScanner::getScanDuration() {
    if (isScanning) {
        return millis() - scanStartTime;
    }
    return 0;
}
