#include <ESP8266WiFi.h>
#include <WiFiClient.h>
//#include <ESP8266WebServer.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <WiFiManager.h> 

//for LED status
#include <Ticker.h>

Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(BUILTIN_LED);  // get the current state of GPIO1 pin
  digitalWrite(BUILTIN_LED, !state);     // set pin to the opposite state
}

#define verbose
#ifdef verbose
 #define DEBUG_PRINT(x)         Serial.print (x)
 #define DEBUG_PRINTDEC(x)      Serial.print (x, DEC)
 #define DEBUG_PRINTLN(x)       Serial.println (x)
 #define DEBUG_PRINTF(x, y)     Serial.printf (x, y)
 #define PORTSPEED 115200
#else
 #define DEBUG_PRINT(x)
 #define DEBUG_PRINTDEC(x)
 #define DEBUG_PRINTLN(x)
 #define DEBUG_PRINTF(x, y)
#endif 


#define AIO_SERVER      "192.168.1.56"
//#define AIO_SERVER      "178.77.238.20"
#define AIO_SERVERPORT  1883
#define AIO_USERNAME    "datel"
#define AIO_KEY         "hanka12"

WiFiClient client;

Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

// /****************************** Feeds ***************************************/
// #define MQTTBASE "/home/RGB/1/"
// Adafruit_MQTT_Publish _temperature             = Adafruit_MQTT_Publish(&mqtt, MQTTBASE "RED");
// Adafruit_MQTT_Publish _humidity                = Adafruit_MQTT_Publish(&mqtt, MQTTBASE "GREEN");
// Adafruit_MQTT_Publish _bootTime                = Adafruit_MQTT_Publish(&mqtt, MQTTBASE "bootTime");
// Adafruit_MQTT_Publish _voltage                 = Adafruit_MQTT_Publish(&mqtt, MQTTBASE "Voltage");
// Adafruit_MQTT_Publish _versionSW               = Adafruit_MQTT_Publish(&mqtt, MQTTBASE "VersionSW");

IPAddress _ip           = IPAddress(192, 168, 1, 113);
IPAddress _gw           = IPAddress(192, 168, 1, 1);
IPAddress _sn           = IPAddress(255, 255, 255, 0);

void MQTT_connect(void);

extern "C" {
  #include "user_interface.h"
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  Serial.println("Entered config mode");
  Serial.println(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  Serial.println(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}

//ADC_MODE(ADC_VCC);

#define BLUE_PIN    0
#define RED_PIN     2
#define GREEN_PIN   3

byte redVal, grnVal, bluVal;

float versionSW                   = 0.1;
String versionSWString            = "LED RGB#1 v";
uint32_t heartBeat                = 0;

void setup() {
  #ifdef verbose
  Serial.begin(PORTSPEED);
#endif
  DEBUG_PRINTLN();
  DEBUG_PRINT(versionSWString);
  DEBUG_PRINTLN(versionSW);
  //set led pin as output
  pinMode(BUILTIN_LED, OUTPUT);
  // start ticker with 0.5 because we start in AP mode and try to connect
  ticker.attach(0.6, tick);
  
  DEBUG_PRINTLN(ESP.getResetReason());
  if (ESP.getResetReason()=="Software/System restart") {
    heartBeat=1;
  } else if (ESP.getResetReason()=="Power on") {
    heartBeat=2;
  } else if (ESP.getResetReason()=="External System") {
    heartBeat=3;
  } else if (ESP.getResetReason()=="Hardware Watchdog") {
    heartBeat=4;
  } else if (ESP.getResetReason()=="Exception") {
    heartBeat=5;
  } else if (ESP.getResetReason()=="Software Watchdog") {
    heartBeat=6;
  } else if (ESP.getResetReason()=="Deep-Sleep Wake") {
    heartBeat=7;
  }

  //WiFiManager
  WiFiManager wifiManager;
  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setSTAStaticIPConfig(_ip, _gw, _sn);
  wifiManager.setAPCallback(configModeCallback);
  
  if (!wifiManager.autoConnect("RGB LED", "password")) {
    DEBUG_PRINTLN("failed to connect, we should reset as see if it connects");
    delay(3000);
    ESP.reset();
    delay(5000);
  }

	DEBUG_PRINTLN("");
	DEBUG_PRINT("Connected to ");
	DEBUG_PRINT("IP address: ");
	DEBUG_PRINTLN(WiFi.localIP());

  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  
 
  ticker.detach();
  //LED off
  digitalWrite(BUILTIN_LED, HIGH);

}

void loop() {
  analogWrite(RED_PIN, 1023);
  delay(1000);
  analogWrite(RED_PIN, 0);
  analogWrite(BLUE_PIN, 1023);
  delay(1000);
  analogWrite(BLUE_PIN, 0);
  analogWrite(GREEN_PIN, 1023);
  delay(1000);
  analogWrite(GREEN_PIN, 0);
  analogWrite(RED_PIN, 1023);
  analogWrite(BLUE_PIN, 1023);
  analogWrite(GREEN_PIN, 1023);
  delay(1000);
  analogWrite(RED_PIN, 0);
  analogWrite(BLUE_PIN, 0);
  analogWrite(GREEN_PIN, 0);
	DEBUG_PRINTLN("RED");
  for (long i=0;i<1023; i++) {
    analogWrite(RED_PIN, i);
    delay(1);
  }
  analogWrite(RED_PIN, 0);
	DEBUG_PRINTLN("GREEN");
  for (long i=0;i<1023; i++) {
    analogWrite(GREEN_PIN, i);
    delay(1);
  }
  analogWrite(GREEN_PIN, 0);
	DEBUG_PRINTLN("BLUE");
  for (long i=0;i<1023; i++) {
    analogWrite(BLUE_PIN, i);
    delay(1);
  }
  analogWrite(BLUE_PIN, 0);
}