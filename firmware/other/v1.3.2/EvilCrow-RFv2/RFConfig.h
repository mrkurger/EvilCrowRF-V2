#pragma once

#include <Arduino.h>
#include "RFSignalProcessor.h"

struct RFConfig
{
    uint32_t frequency;
    RFSignalProcessor::ModulationType modulation;
    uint16_t deviation;
    uint32_t bandwidth;
    uint32_t samplingRate;
    bool useAGC;
    uint32_t pulseLength;
    uint16_t repetitions;
    String mode;

    RFConfig() : frequency(433920000),
                 modulation(RFSignalProcessor::ModulationType::OOK),
                 deviation(0),
                 bandwidth(100000),
                 samplingRate(250000),
                 useAGC(true),
                 pulseLength(350),
                 repetitions(1),
                 mode("normal")
    {
    }
};
