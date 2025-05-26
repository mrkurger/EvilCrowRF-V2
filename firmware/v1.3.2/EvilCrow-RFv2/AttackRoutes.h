#ifndef ATTACK_ROUTES_H
#define ATTACK_ROUTES_H

#include <ESPAsyncWebSrv.h>

// Attack configuration paths
extern const char ATTACK_CONFIG_PATH[];

// Attack type endpoints
extern const char SMART_HOME_PATH[];
extern const char WEATHER_STATION_PATH[];
extern const char VEHICLE_DIAG_PATH[];
extern const char KEY_FOB_PATH[];
extern const char RF_REMOTE_PATH[];
extern const char DOORBELL_PATH[];

// Function to add attack-related routes to the web server
void addAttackRoutes(AsyncWebServer &server);

#endif // ATTACK_ROUTES_H
