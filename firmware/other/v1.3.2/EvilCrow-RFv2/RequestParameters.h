#pragma once

#include <Arduino.h>

struct RequestParameters
{
    String command;
    uint32_t frequency;
    uint8_t modulation;
    uint16_t deviation;
    uint32_t samplingRate;
    uint32_t bandwidth;
    bool useAGC;
    String rawData;
    String binaryData;
    uint32_t pulseLength;
    uint16_t repetitions;
    String jammerMode;
};
