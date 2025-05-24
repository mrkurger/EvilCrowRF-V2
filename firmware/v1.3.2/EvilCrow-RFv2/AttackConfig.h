#pragma once
#include "AttackTypes.h"

struct AttackConfig
{
    AttackType type;
    uint32_t frequency;
    uint8_t modulation;
    uint16_t deviation;
    uint32_t attackDuration;
    String targetProtocol;
    bool enabled;
    uint32_t timeout;
    String description;

    union
    {
        struct
        {
            uint8_t payloadType; // 0=keyboard, 1=mouse, 2=multimedia
            uint32_t targetVID;  // Vendor ID
            uint32_t targetPID;  // Product ID
        } mousejack;

        struct
        {
            uint32_t recordTime;  // Time to record signal in ms
            uint32_t replayDelay; // Delay before replay in ms
        } rolljam;

        struct
        {
            uint32_t startCode; // Starting code for bruteforce
            uint32_t endCode;   // Ending code for bruteforce
            uint8_t codeLength; // Length of code in bits
        } bruteforce;

        struct
        {
            uint8_t switchCount; // Number of DIP switches
        } dipswitch;

        struct
        {
            uint8_t jamType;    // 0=constant, 1=sweep, 2=targeted
            uint32_t sweepStep; // Frequency step for sweep jamming
        } jamming;
    } params;

    AttackConfig() : type(AttackType::MOUSEJACKING),
                     frequency(433920000),
                     modulation(0),
                     deviation(0),
                     attackDuration(0),
                     enabled(false),
                     timeout(0)
    {
        params.mousejack = {0, 0, 0};
    }
};
