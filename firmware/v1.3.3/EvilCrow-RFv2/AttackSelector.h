#ifndef ATTACK_SELECTOR_H
#define ATTACK_SELECTOR_H

#include "SmartHomeAttack.h"
#include "WeatherStationAttack.h"
#include "VehicleDiagnosticsAttack.h"
#include "KeyFobExtensionAttack.h"
#include "RFRemoteOverrideAttack.h"
#include "WirelessDoorbellAttack.h"
#include "AttackManager.h"

#include "AttackTypes.h" // Use the unified AttackType enum

class AttackSelector
{
public:
    static AttackSelector &getInstance()
    {
        static AttackSelector instance;
        return instance;
    }

    // Delete copy constructor and assignment operator
    AttackSelector(const AttackSelector &) = delete;
    AttackSelector &operator=(const AttackSelector &) = delete;

    bool initializeAttack(AttackType type);
    bool startAttack(AttackType type);
    bool stopAttack(AttackType type);
    bool isAttackRunning(AttackType type) const;

    // Smart Home Attack methods
    SmartHomeAttack *getSmartHomeAttack() { return smartHomeAttack; }
    bool configureSmartHomeAttack(const SmartHomeAttack::Config &config);

    // Weather Station Attack methods
    WeatherStationAttack *getWeatherStationAttack() { return weatherStationAttack; }
    bool configureWeatherStationAttack(const WeatherStationAttack::Config &config);

    // Vehicle Diagnostics Attack methods
    VehicleDiagnosticsAttack *getVehicleDiagnosticsAttack() { return vehicleDiagnosticsAttack; }
    bool configureVehicleDiagnosticsAttack(const VehicleDiagnosticsAttack::Config &config);

    // Key Fob Extension Attack methods
    KeyFobExtensionAttack *getKeyFobExtensionAttack() { return keyFobExtensionAttack; }
    bool configureKeyFobExtensionAttack(const KeyFobExtensionAttack::Config &config);

    // RF Remote Override Attack methods
    RFRemoteOverrideAttack *getRFRemoteOverrideAttack() { return rfRemoteOverrideAttack; }
    bool configureRFRemoteOverrideAttack(const RFRemoteOverrideAttack::Config &config);

    // Wireless Doorbell Attack methods
    WirelessDoorbellAttack *getWirelessDoorbellAttack() { return wirelessDoorbellAttack; }
    bool configureWirelessDoorbellAttack(const WirelessDoorbellAttack::Config &config);

private:
    AttackSelector()
        : smartHomeAttack(nullptr), weatherStationAttack(nullptr), vehicleDiagnosticsAttack(nullptr), keyFobExtensionAttack(nullptr), rfRemoteOverrideAttack(nullptr), wirelessDoorbellAttack(nullptr), currentAttackType(AttackType::SMART_HOME), attackRunning(false)
    {
        // Initialize attacks in constructor
    }

    ~AttackSelector()
    {
        cleanupAttacks();
    }

    SmartHomeAttack *smartHomeAttack;
    WeatherStationAttack *weatherStationAttack;
    VehicleDiagnosticsAttack *vehicleDiagnosticsAttack;
    KeyFobExtensionAttack *keyFobExtensionAttack;
    RFRemoteOverrideAttack *rfRemoteOverrideAttack;
    WirelessDoorbellAttack *wirelessDoorbellAttack;

    AttackType currentAttackType;
    bool attackRunning;

    void cleanupAttacks();
    bool stopCurrentAttack();
    RFSignalProcessor *createSignalProcessor();
};

#endif // ATTACK_SELECTOR_H
