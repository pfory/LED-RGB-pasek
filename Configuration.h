#ifndef CONFIGURATION_H
#define CONFIGURATION_H

//SW name & version
#define     VERSION                      "0.02"
#define     SW_NAME                      "RGBLEDSTRIP1"

#define timers
#define ota
#define verbose
#include <DoubleResetDetector.h>      //https://github.com/khoih-prog/ESP_DoubleResetDetector

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

/*
--------------------------------------------------------------------------------------------------------------------------

Version history:

--------------------------------------------------------------------------------------------------------------------------
HW
ESP8266-01
RGB LED strip
*/

#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include <Ticker.h>
#include <ArduinoJson.h> //https://github.com/bblanchon/ArduinoJson
#include "DoubleResetDetector.h" // https://github.com/datacute/DoubleResetDetector
#include "Sender.h"
#include <Wire.h>
#include <PubSubClient.h>

#define verbose
#ifdef verbose
  #define DEBUG_PRINT(x)         Serial.print (x)
  #define DEBUG_PRINTDEC(x)      Serial.print (x, DEC)
  #define DEBUG_PRINTLN(x)       Serial.println (x)
  #define DEBUG_PRINTF(x, y)     Serial.printf (x, y)
  #define DEBUG_PRINTHEX(x)      Serial.print (x, HEX)
  #define PORTSPEED 115200
  #define SERIAL_BEGIN           Serial.begin(PORTSPEED);
#else
  #define DEBUG_PRINT(x)
  #define DEBUG_PRINTDEC(x)
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINTF(x, y)
#endif 

// Number of seconds after reset during which a
// subseqent reset will be considered a double reset.
#define DRD_TIMEOUT 2
// RTC Memory Address for the DoubleResetDetector to use
#define DRD_ADDRESS 0

#define CONFIG_PORTAL_TIMEOUT 60 //jak dlouho zustane v rezimu AP nez se cip resetuje
#define CONNECT_TIMEOUT 120 //jak dlouho se ceka na spojeni nez se aktivuje config portal

static const char* const      mqtt_server                    = "192.168.1.56";
static const uint16_t         mqtt_port                      = 1883;
static const char* const      mqtt_username                  = "datel";
static const char* const      mqtt_key                       = "hanka12";
static const char* const      mqtt_base                      = "/home/rgb1";
// static const char* const      static_ip                      = "192.168.1.122";
// static const char* const      static_gw                      = "192.168.1.1";
// static const char* const      static_sn                      = "255.255.255.0";
static const char* const      mqtt_topic_state               = "";
static const char* const      mqtt_topic_set                 = "set";
static const char* const      mqtt_topic_restart             = "restart";
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

#endif