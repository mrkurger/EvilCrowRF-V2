#include "AttackManager.h"
#include <algorithm>

// Standard HID keyboard report structure
struct KeyboardReport
{
    uint8_t modifiers;
    uint8_t reserved;
    uint8_t keys[6];
};

AttackManager::AttackManager(TaskScheduler &scheduler, RFSignalProcessor &rfProcessor)
    : taskScheduler(scheduler), rfProcessor(rfProcessor), attackRunning(false), attackTask(nullptr), monitorTask(nullptr)
{

    // Initialize attack progress
    progress = {0};
    progress.lastStatus = "Ready";
    progress.currentAction = "Idle";

    // Create status update queue
    statusQueue = xQueueCreate(10, sizeof(String));
}

bool AttackManager::startAttack(const AttackConfig &config)
{
    if (attackRunning)
    {
        stopAttack();
    }

    currentConfig = config;
    attackRunning = true;

    // Create attack task with high priority for real-time performance
    xTaskCreatePinnedToCore(
        attackTaskFunction,
        "attack_task",
        8192,     // Stack size
        this,     // Pass AttackManager instance as parameter
        configMAX_PRIORITIES - 1, // High priority
        &attackTask,
        1 // Run on core 1
    );

    return true;
}

void AttackManager::stopAttack()
{
    if (attackRunning)
    {
        attackRunning = false;
        if (attackTask != nullptr)
        {
            vTaskDelete(attackTask);
            attackTask = nullptr;
        }
        // Stop RF processor if it has a stop method
        // rfProcessor.stopTransmission();
    }
}

bool AttackManager::isAttackRunning() const
{
    return attackRunning;
}

String AttackManager::getAttackStatus() const
{
    // Return current attack status in JSON format
    String status = "{\"running\":" + String(attackRunning);
    if (attackRunning)
    {
        status += ",\"type\":\"" + String(static_cast<int>(currentConfig.type)) + "\"";
        status += ",\"frequency\":" + String(currentConfig.frequency);
        status += ",\"protocol\":\"" + currentConfig.targetProtocol + "\"";
        status += ",\"duration\":" + String(currentConfig.attackDuration);
    }
    status += "}";
    return status;
}

void AttackManager::attackTaskFunction(void *parameter)
{
    AttackManager *manager = static_cast<AttackManager *>(parameter);

    // Configure RF parameters
    // manager->rfProcessor.setFrequency(manager->currentConfig.frequency);
    // manager->rfProcessor.setModulation(manager->currentConfig.modulation);
    // manager->rfProcessor.setDeviation(manager->currentConfig.deviation);

    // Run the appropriate attack
    switch (manager->currentConfig.type)
    {
    case AttackType::MOUSEJACKING:
        manager->runMousejackingAttack();
        break;
    case AttackType::ROLLJAM:
        manager->runRolljamAttack();
        break;
    case AttackType::BRUTEFORCE:
        manager->runBruteforceAttack();
        break;
    case AttackType::DIP_SWITCH:
        manager->runDipSwitchAttack();
        break;
    case AttackType::JAMMING:
        manager->runJammingAttack();
        break;
    }

    // Clean up after attack finishes
    manager->attackRunning = false;
    manager->progress.currentAction = "Attack completed";
    manager->progress.lastStatus = "Done";
    vTaskDelete(NULL);
}

uint8_t *AttackManager::generateMousejackPayload(uint8_t type, uint8_t *length)
{
    static uint8_t payload[32];
    *length = 0;

    switch (type)
    {
    case 0:
    { // Keyboard
        KeyboardReport *report = (KeyboardReport *)payload;
        report->modifiers = 0x08; // Windows key
        report->reserved = 0;
        report->keys[0] = 0x15; // 'r' key
        *length = sizeof(KeyboardReport);
        break;
    }
    case 1:
    {                      // Mouse
        payload[0] = 0x01; // Button press
        payload[1] = 0x00; // X movement
        payload[2] = 0x00; // Y movement
        *length = 3;
        break;
    }
    case 2:
    {                      // Multimedia
        payload[0] = 0x02; // Volume up
        payload[1] = 0x00;
        *length = 2;
        break;
    }
    }
    return payload;
}

uint32_t AttackManager::generateDIPCode(uint8_t switchCount, uint8_t state)
{
    // Generate valid DIP switch combinations
    uint32_t code = 0;
    for (uint8_t i = 0; i < switchCount; i++)
    {
        if (state & (1 << i))
        {
            code |= (1 << (2 * i)); // ON position
        }
        else
        {
            code |= (1 << (2 * i + 1)); // OFF position
        }
    }
    return code;
}

bool AttackManager::validateBruteforceCode(uint32_t code, uint8_t length)
{
    // Validate code against known protocol patterns
    uint8_t oneCount = 0;
    uint8_t zeroCount = 0;

    for (uint8_t i = 0; i < length; i++)
    {
        if (code & (1 << i))
        {
            oneCount++;
        }
        else
        {
            zeroCount++;
        }
    }

    // Most valid RF codes have a somewhat balanced number of 1s and 0s
    float ratio = (float)oneCount / length;
    return (ratio >= 0.3 && ratio <= 0.7);
}

void AttackManager::runMousejackingAttack()
{
    progress.startTime = millis();
    progress.currentAction = "Starting mousejacking attack";

    uint8_t payloadLength;
    uint8_t *payload = generateMousejackPayload(
        currentConfig.params.mousejack.payloadType,
        &payloadLength);

    while (attackRunning)
    {
        progress.totalAttempts++;
        // rfProcessor.transmitPacket(payload, payloadLength);

        // Simulate transmission for now
        bool success = (random(100) > 10); // 90% success rate simulation
        
        if (success)
        {
            progress.successCount++;
        }
        else
        {
            progress.failedAttempts++;
        }

        // progress.signalStrength = rfProcessor.getSignalStrength();
        progress.lastUpdateTime = millis();

        delay(100); // Delay between transmissions
    }
}

void AttackManager::runRolljamAttack()
{
    progress.startTime = millis();
    progress.currentAction = "Starting rolljam attack";

    // Record phase - simulated for now
    delay(currentConfig.params.rolljam.recordTime);
    
    // Simulate captured data
    bool capturedData = true;
    
    if (capturedData)
    {
        progress.currentAction = "Signal captured, starting jam";

        // Jamming phase
        delay(currentConfig.params.rolljam.jamTime);

        // Replay phase
        progress.currentAction = "Replaying captured signal";
        for (uint8_t i = 0; i < currentConfig.params.rolljam.replayCount; i++)
        {
            if (!attackRunning)
                break;

            progress.totalAttempts++;
            
            // Simulate transmission
            bool success = (random(100) > 20); // 80% success rate
            
            if (success)
            {
                progress.successCount++;
            }
            else
            {
                progress.failedAttempts++;
            }
            delay(100);
        }
    }
}

void AttackManager::runBruteforceAttack()
{
    progress.startTime = millis();
    progress.currentAction = "Starting bruteforce attack";

    uint32_t currentCode = currentConfig.params.bruteforce.startCode;
    uint32_t endCode = currentConfig.params.bruteforce.endCode;
    uint8_t codeLength = currentConfig.params.bruteforce.codeLength;

    while (attackRunning && currentCode <= endCode)
    {
        if (validateBruteforceCode(currentCode, codeLength))
        {
            progress.totalAttempts++;

            // Simulate transmission
            bool success = (random(100) > 30); // 70% success rate
            
            if (success)
            {
                progress.successCount++;
            }
            else
            {
                progress.failedAttempts++;
            }

            progress.lastUpdateTime = millis();
            progress.currentAction = "Testing code: " + String(currentCode, HEX);
        }
        currentCode++;
        delay(10); // Small delay to prevent overheating
    }
}

void AttackManager::runDipSwitchAttack()
{
    progress.startTime = millis();
    progress.currentAction = "Starting DIP switch attack";

    uint8_t switchCount = currentConfig.params.dipswitch.switchCount;
    uint8_t currentState = currentConfig.params.dipswitch.startState;
    uint8_t endState = currentConfig.params.dipswitch.endState;

    while (attackRunning && currentState <= endState)
    {
        uint32_t code = generateDIPCode(switchCount, currentState);
        progress.totalAttempts++;

        // Simulate transmission
        bool success = (random(100) > 25); // 75% success rate
        
        if (success)
        {
            progress.successCount++;
        }
        else
        {
            progress.failedAttempts++;
        }

        progress.lastUpdateTime = millis();
        progress.currentAction = "Testing DIP state: " + String(currentState, BIN);

        currentState++;
        delay(50); // Delay between combinations
    }
}

void AttackManager::runJammingAttack()
{
    progress.startTime = millis();
    progress.currentAction = "Starting jamming attack";

    switch (currentConfig.params.jamming.jamType)
    {
    case 0: // Constant jamming
        while (attackRunning)
        {
            progress.lastUpdateTime = millis();
            delay(100);
        }
        break;

    case 1: // Sweep jamming
        while (attackRunning)
        {
            float freq = currentConfig.params.jamming.sweepStart;
            float step = (currentConfig.params.jamming.sweepEnd - currentConfig.params.jamming.sweepStart) /
                         currentConfig.params.jamming.sweepSteps;

            while (freq <= currentConfig.params.jamming.sweepEnd && attackRunning)
            {
                delay(50);
                progress.lastUpdateTime = millis();
                freq += step;
            }
        }
        break;

    case 2: // Random jamming
        while (attackRunning)
        {
            delay(random(50, 200));
            progress.lastUpdateTime = millis();
        }
        break;
    }
}

AttackProgress &AttackManager::getProgress()
{
    return progress;
}
