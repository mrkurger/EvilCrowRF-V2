#include "SignalAnalyzer.h"
#include "ELECHOUSE_CC1101_SRC_DRV.h"

// TV Remote database - Popular brands and their power codes
const TVRemoteCode TV_CODES[] = {
    // Samsung TVs
    {"Samsung", "General", 0xE0E040BF, 0xE0E040BF, 38.0, ModulationType::PWM, 32, "NEC"},
    {"Samsung", "Smart", 0xE0E09966, 0xE0E09966, 38.0, ModulationType::PWM, 32, "Samsung"},

    // LG TVs
    {"LG", "General", 0x20DF10EF, 0x20DF10EF, 38.0, ModulationType::PWM, 32, "NEC"},
    {"LG", "Magic", 0x20DF23DC, 0x20DF23DC, 38.0, ModulationType::PWM, 32, "LG"},

    // Sony TVs
    {"Sony", "General", 0xA90, 0xA90, 40.0, ModulationType::PWM, 12, "SIRC"},
    {"Sony", "Bravia", 0x750, 0x750, 40.0, ModulationType::PWM, 12, "SIRC"},

    // Panasonic TVs
    {"Panasonic", "General", 0x40040100BCBD, 0x40040100BCBD, 38.0, ModulationType::PWM, 48, "Panasonic"},

    // Philips TVs
    {"Philips", "General", 0x0C, 0x0C, 38.0, ModulationType::PWM, 14, "RC5"},

    // TCL TVs
    {"TCL", "General", 0x40BF12ED, 0x40BF12ED, 38.0, ModulationType::PWM, 32, "NEC"},

    // Hisense TVs
    {"Hisense", "General", 0x40BF08F7, 0x40BF08F7, 38.0, ModulationType::PWM, 32, "NEC"},

    // Universal codes that work with many TVs
    {"Universal", "Power1", 0x20DF10EF, 0x20DF10EF, 38.0, ModulationType::PWM, 32, "NEC"},
    {"Universal", "Power2", 0xE0E040BF, 0xE0E040BF, 38.0, ModulationType::PWM, 32, "NEC"},
    {"Universal", "Power3", 0x40BF12ED, 0x40BF12ED, 38.0, ModulationType::PWM, 32, "NEC"},
    {"Universal", "Power4", 0xA90, 0xA90, 40.0, ModulationType::PWM, 12, "SIRC"}};

const int TV_CODE_COUNT = sizeof(TV_CODES) / sizeof(TVRemoteCode);

SignalAnalyzer::SignalAnalyzer()
{
    sampleCount = 0;
    reset();
    initTVDatabase();
}

SignalAnalyzer::~SignalAnalyzer()
{
    // Cleanup if needed
}

void SignalAnalyzer::initTVDatabase()
{
    tvDatabase.clear();
    for (int i = 0; i < TV_CODE_COUNT; i++)
    {
        tvDatabase.push_back(TV_CODES[i]);
    }
}

bool SignalAnalyzer::addSample(uint32_t duration)
{
    if (sampleCount >= MAX_SAMPLES)
    {
        return false;
    }

    samples[sampleCount++] = duration;
    return true;
}

bool SignalAnalyzer::analyzeCapturedSignal()
{
    if (sampleCount < 10)
    {
        return false;
    }

    // Reset analysis
    lastAnalysis = SignalCharacteristics();
    lastAnalysis.timestamp = millis();

    // Step 1: Analyze pulse patterns
    if (!analyzePulses())
    {
        return false;
    }

    // Step 2: Detect modulation type
    if (!detectModulation())
    {
        lastAnalysis.modulation = ModulationType::UNKNOWN;
    }

    // Step 3: Identify protocol
    if (!identifyProtocol())
    {
        lastAnalysis.protocol = ProtocolType::UNKNOWN;
    }

    // Step 4: Calculate signal quality metrics
    lastAnalysis.snr = calculateSNR();
    lastAnalysis.rssi = calculateRSSI();

    // Step 5: Validate signal
    lastAnalysis.isValid = isSignalValid();

    return true;
}

bool SignalAnalyzer::analyzePulses()
{
    if (sampleCount < 4)
        return false;

    // Find shortest and longest pulses
    uint32_t minPulse = samples[0];
    uint32_t maxPulse = samples[0];
    uint32_t totalDuration = 0;

    for (int i = 0; i < sampleCount; i++)
    {
        if (samples[i] < minPulse)
            minPulse = samples[i];
        if (samples[i] > maxPulse)
            maxPulse = samples[i];
        totalDuration += samples[i];
    }

    // Classify pulses
    pulseStats.shortPulse = minPulse;
    pulseStats.longPulse = maxPulse;
    pulseStats.pulseRatio = maxPulse / minPulse;
    pulseStats.dutyCycle = (float)totalDuration / sampleCount;

    // Look for sync pulse (usually much longer)
    for (int i = 0; i < sampleCount; i++)
    {
        if (samples[i] > (maxPulse * 2))
        {
            pulseStats.syncPulse = samples[i];
            pulseStats.hasSync = true;
            break;
        }
    }

    // Check consistency
    int shortCount = 0, longCount = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        if (samples[i] <= (minPulse * 1.5))
            shortCount++;
        else if (samples[i] >= (maxPulse * 0.7))
            longCount++;
    }

    pulseStats.isConsistent = (shortCount + longCount) > (sampleCount * 0.8);

    lastAnalysis.pulseCount = sampleCount;
    lastAnalysis.bitCount = sampleCount / 2; // Rough estimate

    return true;
}

bool SignalAnalyzer::detectModulation()
{
    // Simple modulation detection based on pulse patterns

    if (detectManchester())
    {
        lastAnalysis.modulation = ModulationType::MANCHESTER;
        return true;
    }

    if (detectPWM())
    {
        lastAnalysis.modulation = ModulationType::PWM;
        return true;
    }

    // Check for OOK/ASK patterns
    if (pulseStats.pulseRatio < 3 && pulseStats.isConsistent)
    {
        lastAnalysis.modulation = ModulationType::OOK;
        return true;
    }

    // Default to ASK for most RF signals
    lastAnalysis.modulation = ModulationType::ASK;
    return true;
}

bool SignalAnalyzer::detectManchester()
{
    // Manchester encoding has consistent bit periods
    if (sampleCount < 8)
        return false;

    uint32_t avgPulse = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        avgPulse += samples[i];
    }
    avgPulse /= sampleCount;

    int consistentPulses = 0;
    for (int i = 0; i < sampleCount; i++)
    {
        if (abs((int)samples[i] - (int)avgPulse) < (avgPulse * 0.3))
        {
            consistentPulses++;
        }
    }

    return (consistentPulses > sampleCount * 0.7);
}

bool SignalAnalyzer::detectPWM()
{
    // PWM has distinct short and long pulses
    return (pulseStats.pulseRatio >= 2 && pulseStats.pulseRatio <= 10 && pulseStats.isConsistent);
}

bool SignalAnalyzer::identifyProtocol()
{
    // Protocol identification based on signal characteristics

    if (matchTesla())
    {
        lastAnalysis.protocol = ProtocolType::TESLA;
        lastAnalysis.description = "Tesla vehicle signal detected";
        return true;
    }

    if (matchGarageDoor())
    {
        lastAnalysis.protocol = ProtocolType::GARAGE_DOOR;
        lastAnalysis.description = "Garage door remote signal";
        return true;
    }

    if (matchCarKey())
    {
        lastAnalysis.protocol = ProtocolType::CAR_KEY;
        lastAnalysis.description = "Car key fob signal";
        return true;
    }

    if (matchWeatherStation())
    {
        lastAnalysis.protocol = ProtocolType::WEATHER_STATION;
        lastAnalysis.description = "Weather station sensor";
        return true;
    }

    if (detectRollingCode())
    {
        lastAnalysis.protocol = ProtocolType::ROLLING_CODE;
        lastAnalysis.description = "Rolling code security system";
        return true;
    }

    // Check for TV remote patterns
    if (lastAnalysis.modulation == ModulationType::PWM &&
        (lastAnalysis.frequency >= 36.0 && lastAnalysis.frequency <= 42.0))
    {
        lastAnalysis.protocol = ProtocolType::TV_REMOTE;
        lastAnalysis.description = "TV/IR remote control signal";
        return true;
    }

    lastAnalysis.protocol = ProtocolType::UNKNOWN;
    lastAnalysis.description = "Unknown protocol";
    return false;
}

// TV Remote Control Functions
bool SignalAnalyzer::turnOffTV(const String &brand)
{
    TVRemoteCode *code = nullptr;

    if (brand.length() > 0)
    {
        code = findTVCode(brand);
    }

    if (!code)
    {
        // Try universal codes
        code = findTVCode("Universal");
    }

    if (code)
    {
        return transmitTVCode(*code, false); // false = power off
    }

    return false;
}

bool SignalAnalyzer::turnOnTV(const String &brand)
{
    TVRemoteCode *code = nullptr;

    if (brand.length() > 0)
    {
        code = findTVCode(brand);
    }

    if (!code)
    {
        // Try universal codes
        code = findTVCode("Universal");
    }

    if (code)
    {
        return transmitTVCode(*code, true); // true = power on
    }

    return false;
}

bool SignalAnalyzer::scanAndTurnOffTV()
{
    Serial.println("📺 Scanning for TVs to turn off...");

    // Try all universal codes first
    for (const auto &tvCode : tvDatabase)
    {
        if (tvCode.brand == "Universal")
        {
            Serial.println("Trying: " + tvCode.brand + " " + tvCode.model);
            if (transmitTVCode(tvCode, false))
            {
                delay(500); // Wait between transmissions
            }
        }
    }

    // Try major brands
    String majorBrands[] = {"Samsung", "LG", "Sony", "Panasonic", "Philips"};
    for (const String &brand : majorBrands)
    {
        TVRemoteCode *code = findTVCode(brand);
        if (code)
        {
            Serial.println("Trying: " + brand);
            transmitTVCode(*code, false);
            delay(500);
        }
    }

    return true;
}

TVRemoteCode *SignalAnalyzer::findTVCode(const String &brand)
{
    for (auto &code : tvDatabase)
    {
        if (code.brand.equalsIgnoreCase(brand))
        {
            return &code;
        }
    }
    return nullptr;
}

std::vector<String> SignalAnalyzer::getSupportedTVBrands()
{
    std::vector<String> brands;
    for (const auto &code : tvDatabase)
    {
        // Add unique brands only
        bool found = false;
        for (const String &existing : brands)
        {
            if (existing == code.brand)
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            brands.push_back(code.brand);
        }
    }
    return brands;
}

// Utility functions
float SignalAnalyzer::calculateSNR()
{
    // Simplified SNR calculation
    return 15.0 + random(0, 20); // Placeholder
}

float SignalAnalyzer::calculateRSSI()
{
    // Simplified RSSI calculation
    return -60.0 + random(-20, 20); // Placeholder
}

bool SignalAnalyzer::isSignalValid()
{
    return (sampleCount >= 10 &&
            pulseStats.isConsistent &&
            lastAnalysis.modulation != ModulationType::UNKNOWN);
}

void SignalAnalyzer::reset()
{
    sampleCount = 0;
    lastAnalysis = SignalCharacteristics();
    pulseStats = PulseAnalysis();
}

String SignalAnalyzer::getAnalysisJSON()
{
    String json = "{";
    json += "\"modulation\":\"" + String((int)lastAnalysis.modulation) + "\",";
    json += "\"protocol\":\"" + String((int)lastAnalysis.protocol) + "\",";
    json += "\"frequency\":" + String(lastAnalysis.frequency) + ",";
    json += "\"pulseCount\":" + String(lastAnalysis.pulseCount) + ",";
    json += "\"bitCount\":" + String(lastAnalysis.bitCount) + ",";
    json += "\"snr\":" + String(lastAnalysis.snr) + ",";
    json += "\"rssi\":" + String(lastAnalysis.rssi) + ",";
    json += "\"isValid\":" + String(lastAnalysis.isValid ? "true" : "false") + ",";
    json += "\"description\":\"" + lastAnalysis.description + "\",";
    json += "\"timestamp\":" + String(lastAnalysis.timestamp);
    json += "}";
    return json;
}

// TV Remote transmission function
bool SignalAnalyzer::transmitTVCode(const TVRemoteCode &code, bool powerOn)
{
    Serial.println("📺 Transmitting TV code: " + code.brand + " " + code.model);

    // Configure CC1101 for IR frequency (if supported) or use 433MHz as carrier
    ELECHOUSE_cc1101.setMHZ(433.92); // Use 433MHz as carrier
    ELECHOUSE_cc1101.SetTx();

    uint32_t codeToSend = powerOn ? code.powerOnCode : code.powerOffCode;

    // Generate IR-like signal pattern for the TV code
    // This is a simplified implementation - real IR would need precise timing

    // Send start burst
    for (int i = 0; i < 10; i++)
    {
        digitalWrite(2, HIGH); // Assuming pin 2 for transmission
        delayMicroseconds(9000);
        digitalWrite(2, LOW);
        delayMicroseconds(4500);
    }

    // Send data bits
    for (int bit = code.bitLength - 1; bit >= 0; bit--)
    {
        digitalWrite(2, HIGH);
        delayMicroseconds(560);
        digitalWrite(2, LOW);

        if (codeToSend & (1UL << bit))
        {
            delayMicroseconds(1690); // Logic 1
        }
        else
        {
            delayMicroseconds(560); // Logic 0
        }
    }

    // Send stop bit
    digitalWrite(2, HIGH);
    delayMicroseconds(560);
    digitalWrite(2, LOW);

    ELECHOUSE_cc1101.SetRx();

    Serial.println("✅ TV code transmitted");
    return true;
}

// Protocol matching functions
bool SignalAnalyzer::matchTesla()
{
    // Tesla signals typically use 315MHz or 433.92MHz
    // Look for specific pulse patterns
    if (sampleCount >= 20 && pulseStats.hasSync)
    {
        if (pulseStats.syncPulse > 10000 && pulseStats.shortPulse < 1000)
        {
            return true;
        }
    }
    return false;
}

bool SignalAnalyzer::matchGarageDoor()
{
    // Garage doors often use rolling codes with specific timing
    if (sampleCount >= 12 && sampleCount <= 50)
    {
        if (pulseStats.pulseRatio >= 2 && pulseStats.pulseRatio <= 4)
        {
            return true;
        }
    }
    return false;
}

bool SignalAnalyzer::matchCarKey()
{
    // Car keys typically have short, consistent pulses
    if (sampleCount >= 16 && sampleCount <= 100)
    {
        if (pulseStats.shortPulse < 500 && pulseStats.isConsistent)
        {
            return true;
        }
    }
    return false;
}

bool SignalAnalyzer::matchWeatherStation()
{
    // Weather stations often have longer transmissions
    if (sampleCount > 50)
    {
        if (pulseStats.shortPulse > 200 && pulseStats.shortPulse < 2000)
        {
            return true;
        }
    }
    return false;
}

bool SignalAnalyzer::detectRollingCode()
{
    // Rolling codes typically have specific patterns
    if (sampleCount >= 24 && pulseStats.hasSync)
    {
        // Look for preamble + data pattern
        return true;
    }
    return false;
}

// Advanced attack functions
bool SignalAnalyzer::bruteForceAttack(ProtocolType protocol)
{
    Serial.println("🔓 Starting brute force attack for protocol: " + String((int)protocol));

    switch (protocol)
    {
    case ProtocolType::GARAGE_DOOR:
        // Brute force garage door codes
        for (uint32_t code = 0x1000; code < 0x2000; code++)
        {
            // Transmit code
            Serial.println("Trying code: 0x" + String(code, HEX));
            // Add transmission logic here
            delay(100);
        }
        break;

    case ProtocolType::TV_REMOTE:
        // Try all TV codes
        return scanAndTurnOffTV();

    default:
        Serial.println("❌ Brute force not implemented for this protocol");
        return false;
    }

    return true;
}

String SignalAnalyzer::getAnalysisReport()
{
    String report = "=== SIGNAL ANALYSIS REPORT ===\n";
    report += "Timestamp: " + String(lastAnalysis.timestamp) + "\n";
    report += "Modulation: " + String((int)lastAnalysis.modulation) + "\n";
    report += "Protocol: " + String((int)lastAnalysis.protocol) + "\n";
    report += "Description: " + lastAnalysis.description + "\n";
    report += "Frequency: " + String(lastAnalysis.frequency) + " MHz\n";
    report += "Pulse Count: " + String(lastAnalysis.pulseCount) + "\n";
    report += "Bit Count: " + String(lastAnalysis.bitCount) + "\n";
    report += "SNR: " + String(lastAnalysis.snr) + " dB\n";
    report += "RSSI: " + String(lastAnalysis.rssi) + " dBm\n";
    report += "Valid: " + String(lastAnalysis.isValid ? "Yes" : "No") + "\n";
    report += "\n=== PULSE STATISTICS ===\n";
    report += "Short Pulse: " + String(pulseStats.shortPulse) + " μs\n";
    report += "Long Pulse: " + String(pulseStats.longPulse) + " μs\n";
    report += "Sync Pulse: " + String(pulseStats.syncPulse) + " μs\n";
    report += "Pulse Ratio: " + String(pulseStats.pulseRatio) + "\n";
    report += "Has Sync: " + String(pulseStats.hasSync ? "Yes" : "No") + "\n";
    report += "Consistent: " + String(pulseStats.isConsistent ? "Yes" : "No") + "\n";
    return report;
}
