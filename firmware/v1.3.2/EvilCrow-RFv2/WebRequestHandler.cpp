#include "WebRequestHandler.h"
#include "MemoryManager.h"
#include <Arduino.h>

void WebRequestHandler::init()
{
    currentRequest = nullptr;
}

void WebRequestHandler::cleanup()
{
    if (currentRequest)
    {
        delete currentRequest;
        currentRequest = nullptr;
    }
}

bool WebRequestHandler::getParam(AsyncWebServerRequest *request, const char *name, char *buffer, size_t maxLen)
{
    if (!request->hasParam(name))
        return false;

    String param = request->getParam(name)->value();
    if (param.length() >= maxLen)
    {
        log_e("Parameter %s too long: %u >= %u", name, param.length(), maxLen);
        return false;
    }

    strlcpy(buffer, param.c_str(), maxLen);
    return true;
}

bool WebRequestHandler::parseRFConfig(const RequestParameters &params, RFConfig &config)
{
    // Parse frequency
    config.frequency = params.frequency;
    if (config.frequency < 300000000 || config.frequency > 928000000)
    {
        log_e("Invalid frequency: %u", config.frequency);
        return false;
    }

    // Parse modulation
    if (params.modulation < 0 || params.modulation > 4)
    {
        log_e("Invalid modulation: %d", params.modulation);
        return false;
    }

    // Parse deviation if present
    config.deviation = params.deviation;

    return true;
}

void WebRequestHandler::handleTxRequest(AsyncWebServerRequest *request)
{
    // Clean up any previous request
    cleanup();

    // Allocate new request parameters
    currentRequest = new RequestParameters();
    if (!currentRequest)
    {
        log_e("Failed to allocate request parameters");
        request->send(500, "text/html", "Memory allocation failed");
        return;
    }

    // Get basic parameters
    if (!request->hasParam("frequency") || !request->hasParam("modulation"))
    {
        log_e("Missing required parameters");
        request->send(400, "text/html", "Missing required parameters");
        cleanup();
        return;
    }

    // Parse parameters
    currentRequest->frequency = request->getParam("frequency")->value().toInt();
    currentRequest->modulation = request->getParam("modulation")->value().toInt();

    if (request->hasParam("deviation"))
    {
        currentRequest->deviation = request->getParam("deviation")->value().toInt();
    }

    if (request->hasParam("repetitions"))
    {
        currentRequest->repetitions = request->getParam("repetitions")->value().toInt();
    }

    // Get raw data
    if (request->hasParam("rawData"))
    {
        currentRequest->rawData = request->getParam("rawData")->value();
    }

    // Parse RF configuration
    RFConfig config;
    if (!parseRFConfig(*currentRequest, config))
    {
        request->send(400, "text/html", "Invalid RF configuration");
        cleanup();
        return;
    }

    // Process the request...
    // This will be implemented in the main loop to handle the actual RF transmission

    request->send(200, "text/html", "Request accepted");
}

void WebRequestHandler::handleRxRequest(AsyncWebServerRequest *request)
{
    // Clean up any previous request
    cleanup();

    // Allocate new request parameters
    currentRequest = new RequestParameters();
    if (!currentRequest)
    {
        log_e("Failed to allocate request parameters");
        request->send(500, "text/html", "Memory allocation failed");
        return;
    }

    // Get basic parameters
    if (!request->hasParam("frequency") || !request->hasParam("modulation"))
    {
        log_e("Missing required parameters");
        request->send(400, "text/html", "Missing required parameters");
        cleanup();
        return;
    }

    // Parse parameters
    currentRequest->frequency = request->getParam("frequency")->value().toInt();
    currentRequest->modulation = request->getParam("modulation")->value().toInt();

    if (request->hasParam("bandwidth"))
    {
        currentRequest->bandwidth = request->getParam("bandwidth")->value().toInt();
    }

    // Parse RF configuration
    RFConfig config;
    if (!parseRFConfig(*currentRequest, config))
    {
        request->send(400, "text/html", "Invalid RF configuration");
        cleanup();
        return;
    }

    request->send(200, "text/html", "RX Request accepted");
}

void WebRequestHandler::handleBinaryRequest(AsyncWebServerRequest *request)
{
    // Clean up any previous request
    cleanup();

    // Allocate new request parameters
    currentRequest = new RequestParameters();
    if (!currentRequest)
    {
        log_e("Failed to allocate request parameters");
        request->send(500, "text/html", "Memory allocation failed");
        return;
    }

    // Get basic parameters
    if (!request->hasParam("frequency") || !request->hasParam("binaryData"))
    {
        log_e("Missing required parameters");
        request->send(400, "text/html", "Missing required parameters");
        cleanup();
        return;
    }

    // Parse parameters
    currentRequest->frequency = request->getParam("frequency")->value().toInt();
    currentRequest->binaryData = request->getParam("binaryData")->value();

    if (request->hasParam("pulseLength"))
    {
        currentRequest->pulseLength = request->getParam("pulseLength")->value().toInt();
    }

    request->send(200, "text/html", "Binary Request accepted");
}

void WebRequestHandler::handleTeslaRequest(AsyncWebServerRequest *request)
{
    // Tesla-specific implementation
    request->send(200, "text/html", "Tesla Request accepted");
}

void WebRequestHandler::handleJammerRequest(AsyncWebServerRequest *request)
{
    // Clean up any previous request
    cleanup();

    // Allocate new request parameters
    currentRequest = new RequestParameters();
    if (!currentRequest)
    {
        log_e("Failed to allocate request parameters");
        request->send(500, "text/html", "Memory allocation failed");
        return;
    }

    // Get basic parameters
    if (!request->hasParam("frequency") || !request->hasParam("jammerMode"))
    {
        log_e("Missing required parameters");
        request->send(400, "text/html", "Missing required parameters");
        cleanup();
        return;
    }

    // Parse parameters
    currentRequest->frequency = request->getParam("frequency")->value().toInt();
    currentRequest->jammerMode = request->getParam("jammerMode")->value();

    request->send(200, "text/html", "Jammer Request accepted");
}

void WebRequestHandler::handleAttackStart(AsyncWebServerRequest *request)
{
    if (!request->hasParam("type") || !request->hasParam("frequency"))
    {
        request->send(400, "application/json", "{\"error\":\"Missing required parameters\"}");
        return;
    }

    String attackType = request->getParam("type")->value();
    uint32_t frequency = request->getParam("frequency")->value().toInt();

    // Create attack configuration based on type
    AttackConfig config;
    config.frequency = frequency;
    config.enabled = true;
    config.timeout = 30000; // 30 seconds default

    if (attackType == "mousejacking")
    {
        config.type = AttackType::MOUSEJACKING;
        config.params.mousejack.payloadType = request->hasParam("payloadType") ? request->getParam("payloadType")->value().toInt() : 0;
    }
    else if (attackType == "rolljam")
    {
        config.type = AttackType::ROLLJAM;
        config.params.rolljam.recordTime = request->hasParam("recordTime") ? request->getParam("recordTime")->value().toInt() : 5000;
        config.params.rolljam.jamTime = request->hasParam("jamTime") ? request->getParam("jamTime")->value().toInt() : 2000;
        config.params.rolljam.replayCount = request->hasParam("replayCount") ? request->getParam("replayCount")->value().toInt() : 3;
    }
    else if (attackType == "bruteforce")
    {
        config.type = AttackType::BRUTEFORCE;
        config.params.bruteforce.startCode = request->hasParam("startCode") ? request->getParam("startCode")->value().toInt() : 0;
        config.params.bruteforce.endCode = request->hasParam("endCode") ? request->getParam("endCode")->value().toInt() : 0xFFFF;
        config.params.bruteforce.codeLength = request->hasParam("codeLength") ? request->getParam("codeLength")->value().toInt() : 16;
    }
    else if (attackType == "jamming")
    {
        config.type = AttackType::JAMMING;
        config.params.jamming.jamType = request->hasParam("jamType") ? request->getParam("jamType")->value().toInt() : 0;
        config.params.jamming.sweepStart = request->hasParam("sweepStart") ? request->getParam("sweepStart")->value().toFloat() : frequency - 1000000;
        config.params.jamming.sweepEnd = request->hasParam("sweepEnd") ? request->getParam("sweepEnd")->value().toFloat() : frequency + 1000000;
    }
    else
    {
        request->send(400, "application/json", "{\"error\":\"Unknown attack type\"}");
        return;
    }

    // Start the attack using the global attack manager
    extern AttackManager *attackManager;
    if (attackManager && attackManager->startAttack(config))
    {
        request->send(200, "application/json", "{\"status\":\"Attack started\"}");
    }
    else
    {
        request->send(500, "application/json", "{\"error\":\"Failed to start attack\"}");
    }
}

void WebRequestHandler::handleAttackStop(AsyncWebServerRequest *request)
{
    extern AttackManager *attackManager;
    if (attackManager)
    {
        attackManager->stopAttack();
        request->send(200, "application/json", "{\"status\":\"Attack stopped\"}");
    }
    else
    {
        request->send(500, "application/json", "{\"error\":\"Attack manager not available\"}");
    }
}

void WebRequestHandler::handleAttackStatus(AsyncWebServerRequest *request)
{
    extern AttackManager *attackManager;
    if (attackManager)
    {
        String status = attackManager->getAttackStatus();
        request->send(200, "application/json", status);
    }
    else
    {
        request->send(500, "application/json", "{\"error\":\"Attack manager not available\"}");
    }
}
