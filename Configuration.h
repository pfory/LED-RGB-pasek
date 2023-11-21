#ifndef CONFIGURATION_H
#define CONFIGURATION_H

/*RGB pasek
--------------------------------------------------------------------------------------------------------------------------
Petr Fory pfory@seznam.cz
GIT - https://github.com/pfory/LED_RGB_pasek
//ESP8266-01 1MB no FS LED on 2
*/

/*TODO
*/

//SW name & version
#define     VERSION                      "0.3"
#define     SW_NAME                      "RGBLEDSTRIP1 "

#define timers
#define ota
#define verbose
#include <DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector
// #include "cred.h"
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson

#define AUTOCONNECTNAME   HOSTNAMEOTA
#define AUTOCONNECTPWD    "password"

#define ota
#ifdef ota
#include <ArduinoOTA.h>
#define HOSTNAMEOTA         SW_NAME VERSION
#endif

#ifdef time
#include <TimeLib.h>
#include <Timezone.h>
#endif

static const char* const      mqtt_server                    = "192.168.1.56";
static const uint16_t         mqtt_port                      = 1883;
static const char* const      mqtt_username                  = "datel";
static const char* const      mqtt_key                       = "hanka12";
static const char* const      mqtt_base                      = "/home/rgb1";
static const char* const      mqtt_topic_restart             = "restart";
static const char* const      mqtt_topic_netinfo             = "netinfo";
static const char* const      mqtt_config_portal             = "config";
static const char* const      mqtt_config_portal_stop        = "disconfig";
static const char* const      mqtt_topic_load                = "load";


#define CONFIG_MQTT_PAYLOAD_ON  "ON"
#define CONFIG_MQTT_PAYLOAD_OFF "OFF"

uint32_t              connectDelay                = 30000; //30s
uint32_t              lastConnectAttempt          = 0;  

#define SENDSTAT_DELAY                       60000  //poslani statistiky kazdou minutu

// Pins
#define CONFIG_PIN_RED          0
#define CONFIG_PIN_GREEN        2
#define CONFIG_PIN_BLUE         3

#include <fce.h>

#endif