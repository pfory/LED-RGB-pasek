/*
 * This is a sample configuration file for the "mqtt_esp8266_rgb" light.
 *
 * Change the settings below and save the file as "config.h"
 * You can then upload the code using the Arduino IDE.
 */

// Pins
#define CONFIG_PIN_RED 0
#define CONFIG_PIN_GREEN 2
#define CONFIG_PIN_BLUE 3

// WiFi
#define CONFIG_WIFI_SSID "Datlovo"
#define CONFIG_WIFI_PASS "Nu6kMABmseYwbCoJ7LyG"

// MQTT
#define CONFIG_MQTT_HOST "192.168.1.56"
#define CONFIG_MQTT_USER "datel"
#define CONFIG_MQTT_PASS "hanka12"

#define CONFIG_MQTT_CLIENT_ID "ESPRGBLED" // Must be unique on the MQTT network

// MQTT Topics
#define CONFIG_MQTT_TOPIC_STATE "/home/rgb1"
#define CONFIG_MQTT_TOPIC_SET "/home/rgb1/set"

#define CONFIG_MQTT_PAYLOAD_ON "ON"
#define CONFIG_MQTT_PAYLOAD_OFF "OFF"

// Miscellaneous
// Default number of flashes if no value was given
#define CONFIG_DEFAULT_FLASH_LENGTH 2
// Number of seconds for one transition in colorfade mode
#define CONFIG_COLORFADE_TIME_SLOW 10
#define CONFIG_COLORFADE_TIME_FAST 3

// Reverse the LED logic
// false: 0 (off) - 255 (bright)
// true: 255 (off) - 0 (bright)
#define CONFIG_INVERT_LED_LOGIC false

// Enables Serial and print statements
#define CONFIG_DEBUG true
