#include "Configuration.h"

uint32_t heartBeat                          = 0;

DoubleResetDetector drd(DRD_TIMEOUT, DRD_ADDRESS);

#ifdef time
WiFiUDP EthernetUdp;
static const char     ntpServerName[]       = "tik.cesnet.cz";
//const int timeZone = 2;     // Central European Time
//Central European Time (Frankfurt, Paris)
TimeChangeRule        CEST                  = {"CEST", Last, Sun, Mar, 2, 120};     //Central European Summer Time
TimeChangeRule        CET                   = {"CET", Last, Sun, Oct, 3, 60};       //Central European Standard Time
Timezone CE(CEST, CET);
unsigned int          localPort             = 8888;  // local port to listen for UDP packets
time_t getNtpTime();
#endif

// Real values to write to the LEDs (ex. including brightness and state)
byte          realRed           = 0;
byte          realGreen         = 0;
byte          realBlue          = 0;

const int     redPin            = CONFIG_PIN_RED;
const int     greenPin          = CONFIG_PIN_GREEN;
const int     bluePin           = CONFIG_PIN_BLUE;

// // Maintained state for reporting to HA
// byte          red               = 255;
// byte          green             = 255;
// byte          blue              = 255;
// byte          brightness        = 255;

// bool          stateOn           = false;

// // Globals for fade/transitions
// bool          startFade         = false;
// unsigned long lastLoop          = 0;
// int           transitionTime    = 0;
// bool          inFade            = false;
// int           loopCount         = 0;
// int           stepR, stepG, stepB;
// int           redVal, grnVal, bluVal;


bool isDebugEnabled()
{
#ifdef verbose
  return true;
#endif // verbose
  return false;
}

//for LED status
#include <Ticker.h>
Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(bluePin);  // get the current state of GPIO1 pin
  digitalWrite(bluePin, !state);     // set pin to the opposite state
}

//gets called when WiFiManager enters configuration mode
void configModeCallback (WiFiManager *myWiFiManager) {
  DEBUG_PRINTLN("Entered config mode");
  DEBUG_PRINTLN(WiFi.softAPIP());
  //if you used auto generated SSID, print it
  DEBUG_PRINTLN(myWiFiManager->getConfigPortalSSID());
  //entered config mode, make led toggle faster
  ticker.attach(0.2, tick);
}


#include <timer.h>
auto timer = timer_create_default(); // create a timer with default settings
Timer<> default_timer; // save as above

//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  char * pEnd;
  String val =  String();
  DEBUG_PRINT("\nMessage arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  for (int i=0;i<length;i++) {
    DEBUG_PRINT((char)payload[i]);
    val += (char)payload[i];
  }
  DEBUG_PRINTLN();
  
  if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_restart)).c_str())==0) {
    //printMessageToLCD(topic, val);
    DEBUG_PRINT("RESTART");
    //saveConfig();
    ESP.restart();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_load)).c_str())==0) {
    processJson(val);
    // if (val.substring(0,1)=="r") {
      // analogWrite(redPin, val.substring(1).toInt());
      // analogWrite(greenPin, 0);
      // analogWrite(bluePin, 0);
    // } else if (val.substring(0,1)=="g") {
      // analogWrite(redPin, 0);
      // analogWrite(greenPin, val.substring(1).toInt());
      // analogWrite(bluePin, 0);
    // } else if (val.substring(0,1)=="b") {
      // analogWrite(redPin, 0);
      // analogWrite(greenPin, 0);
      // analogWrite(bluePin, val.substring(1).toInt());
    // }
  // } else if (mymode == 'f') {   // f;8;255;34
    // ExtractValues(2, 3);
    // SetColor(atoi(vals[0]), atoi(vals[1]), atoi(vals[2]));
  }  



  // if (stateOn) {
    // // Update lights
    // realRed = map(red, 0, 255, 0, brightness);
    // realGreen = map(green, 0, 255, 0, brightness);
    // realBlue = map(blue, 0, 255, 0, brightness);
  // }
  // else {
    // realRed = 0;
    // realGreen = 0;
    // realBlue = 0;
  // }

  // startFade = true;
  // inFade = false; // Kill the current fade

  //sendState();
}

WiFiClient espClient;
PubSubClient client(espClient);

WiFiManager wifiManager;

//----------------------------------------------------- S E T U P -----------------------------------------------------------
void setup() {
  // put your setup code here, to run once:
  SERIAL_BEGIN;
  DEBUG_PRINT(F(SW_NAME));
  DEBUG_PRINT(F(" "));
  DEBUG_PRINTLN(F(VERSION));

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  ticker.attach(1, tick);
  //pinMode(txPin, OUTPUT);
  //digitalWrite(txPin, HIGH); // Turn off the on-board LED

  WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP

  //set callback that gets called when connecting to previous WiFi fails, and enters Access Point mode
  wifiManager.setAPCallback(configModeCallback);
  wifiManager.setConfigPortalTimeout(CONFIG_PORTAL_TIMEOUT);
  wifiManager.setConnectTimeout(CONNECT_TIMEOUT);

  if (drd.detectDoubleReset()) {
    DEBUG_PRINTLN("Double reset detected, starting config portal...");
    ticker.attach(0.2, tick);
    if (!wifiManager.startConfigPortal(HOSTNAMEOTA)) {
      DEBUG_PRINTLN("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }
  }
  
  rst_info *_reset_info = ESP.getResetInfoPtr();
  uint8_t _reset_reason = _reset_info->reason;
  DEBUG_PRINT("Boot-Mode: ");
  DEBUG_PRINTLN(_reset_reason);
  heartBeat = _reset_reason;
  
 /*
 REASON_DEFAULT_RST             = 0      normal startup by power on 
 REASON_WDT_RST                 = 1      hardware watch dog reset 
 REASON_EXCEPTION_RST           = 2      exception reset, GPIO status won't change 
 REASON_SOFT_WDT_RST            = 3      software watch dog reset, GPIO status won't change 
 REASON_SOFT_RESTART            = 4      software restart ,system_restart , GPIO status won't change 
 REASON_DEEP_SLEEP_AWAKE        = 5      wake up from deep-sleep 
 REASON_EXT_SYS_RST             = 6      external system reset 
  */

  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);

  WiFi.printDiag(Serial);

  if (!wifiManager.autoConnect(AUTOCONNECTNAME, AUTOCONNECTPWD)) { 
    DEBUG_PRINTLN("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
  } 

  sendNetInfoMQTT();

#ifdef time
  DEBUG_PRINTLN("Setup TIME");
  EthernetUdp.begin(localPort);
  DEBUG_PRINT("Local port: ");
  DEBUG_PRINTLN(EthernetUdp.localPort());
  DEBUG_PRINTLN("waiting for sync");
  setSyncProvider(getNtpTime);
  setSyncInterval(300);
  
  printSystemTime();
#endif

#ifdef ota
  ArduinoOTA.setHostname(HOSTNAMEOTA);

  ArduinoOTA.onStart([]() {
    DEBUG_PRINTLN("Start updating ");
  });
  ArduinoOTA.onEnd([]() {
   DEBUG_PRINTLN("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    DEBUG_PRINTF("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    DEBUG_PRINTF("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) DEBUG_PRINTLN("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) DEBUG_PRINTLN("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) DEBUG_PRINTLN("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) DEBUG_PRINTLN("Receive Failed");
    else if (error == OTA_END_ERROR) DEBUG_PRINTLN("End Failed");
  });
  ArduinoOTA.begin();
  ticker.detach();
  
  //test
  for (uint8_t i=0; i<255; i++) {
    analogWrite(redPin, i);
    delay(5);
  }
  digitalWrite(redPin, 0);
  
  for (uint8_t i=0; i<255; i++) {
    analogWrite(greenPin, i);
    delay(5);
  }
  digitalWrite(greenPin, 0);

  for (uint8_t i=0; i<255; i++) {
    analogWrite(bluePin, i);
    delay(5);
  }
  analogWrite(bluePin, 0);
  #endif

//setup timers
  timer.every(SENDSTAT_DELAY, sendStatisticMQTT);

  void * a;
  sendStatisticMQTT(a);
  
  DEBUG_PRINTLN(" Ready");
 
  drd.stop();
  
  DEBUG_PRINTLN(F("Setup end."));
}

//----------------------------------------------------- L O O P -----------------------------------------------------------
void loop() {
  timer.tick(); // tick the timer
#ifdef serverHTTP
  server.handleClient();
#endif

#ifdef ota
  ArduinoOTA.handle();
#endif

  reconnect();
  client.loop();
  
} //loop


void sendState() {
  // StaticJsonBuffer<BUFFER_SIZE> jsonBuffer;

  // JsonObject& root = jsonBuffer.createObject();

  // root["state"] = (stateOn) ? on_cmd : off_cmd;
  // JsonObject& color = root.createNestedObject("color");
  // color["r"] = red;
  // color["g"] = green;
  // color["b"] = blue;

  // root["brightness"] = brightness;

  // if (colorfade) {
    // if (transitionTime == CONFIG_COLORFADE_TIME_SLOW) {
      // root["effect"] = "colorfade_slow";
    // }
    // else {
      // root["effect"] = "colorfade_fast";
    // }
  // }
  // else {
    // root["effect"] = "null";
  // }

  // char buffer[root.measureLength() + 1];
  // root.printTo(buffer, sizeof(buffer));

  // client.publish(light_state_topic, buffer, true);
}

bool sendStatisticMQTT(void *) {
  DEBUG_PRINTLN(F("Statistic"));

  SenderClass sender;
  sender.add("VersionSW", VERSION);
  sender.add("Napeti",  ESP.getVcc());
  sender.add("HeartBeat", heartBeat++);
  sender.add("RSSI", WiFi.RSSI());
  DEBUG_PRINTLN(F("Calling MQTT"));
  
  sender.sendMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_key, mqtt_base);
  return true;
}

void sendNetInfoMQTT() {
  DEBUG_PRINTLN(F("Net info"));

  SenderClass sender;
  sender.add("IP",              WiFi.localIP().toString().c_str());
  sender.add("MAC",             WiFi.macAddress());
  
  DEBUG_PRINTLN(F("Calling MQTT"));
  
  sender.sendMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_key, mqtt_base);
  return;
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    if (lastConnectAttempt == 0 || lastConnectAttempt + connectDelay < millis()) {
      DEBUG_PRINT("Attempting MQTT connection...");
      // Attempt to connect
      if (client.connect(mqtt_base, mqtt_username, mqtt_key)) {
        DEBUG_PRINTLN("connected");
        client.subscribe((String(mqtt_base) + "/#").c_str());
      } else {
        lastConnectAttempt = millis();
        DEBUG_PRINT("failed, rc=");
        DEBUG_PRINTLN(client.state());
      }
    }
  }
}

bool processJson(String message) {
  char json[500];
  message.toCharArray(json, 500);
  DEBUG_PRINTLN(json);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  realRed = doc["r"];
  DEBUG_PRINTLN(realRed);
  realBlue = doc["be"];
  DEBUG_PRINTLN(realBlue);
  realGreen = doc["g"];
  DEBUG_PRINTLN(realGreen);
  analogWrite(redPin, realRed);
  analogWrite(bluePin, realBlue);
  analogWrite(greenPin, realGreen);
 

  // if (root.containsKey("state")) {
    // if (strcmp(root["state"], on_cmd) == 0) {
      // stateOn = true;
    // }
    // else if (strcmp(root["state"], off_cmd) == 0) {
      // stateOn = false;
    // }
  // }

  // // If "flash" is included, treat RGB and brightness differently
  // if (root.containsKey("flash") ||
       // (root.containsKey("effect") && strcmp(root["effect"], "flash") == 0)) {

    // if (root.containsKey("flash")) {
      // flashLength = (int)root["flash"] * 1000;
    // }
    // else {
      // flashLength = CONFIG_DEFAULT_FLASH_LENGTH * 1000;
    // }

    // if (root.containsKey("brightness")) {
      // flashBrightness = root["brightness"];
    // }
    // else {
      // flashBrightness = brightness;
    // }

    // if (root.containsKey("color")) {
      // flashRed = root["color"]["r"];
      // flashGreen = root["color"]["g"];
      // flashBlue = root["color"]["b"];
    // }
    // else {
      // flashRed = red;
      // flashGreen = green;
      // flashBlue = blue;
    // }

    // flashRed = map(flashRed, 0, 255, 0, flashBrightness);
    // flashGreen = map(flashGreen, 0, 255, 0, flashBrightness);
    // flashBlue = map(flashBlue, 0, 255, 0, flashBrightness);

    // flash = true;
    // startFlash = true;
  // }
  // else if (root.containsKey("effect") &&
      // (strcmp(root["effect"], "colorfade_slow") == 0 || strcmp(root["effect"], "colorfade_fast") == 0)) {
    // flash = false;
    // colorfade = true;
    // currentColor = 0;
    // if (strcmp(root["effect"], "colorfade_slow") == 0) {
      // transitionTime = CONFIG_COLORFADE_TIME_SLOW;
    // }
    // else {
      // transitionTime = CONFIG_COLORFADE_TIME_FAST;
    // }
  // }
  // else if (colorfade && !root.containsKey("color") && root.containsKey("brightness")) {
    // // Adjust brightness during colorfade
    // // (will be applied when fading to the next color)
    // brightness = root["brightness"];
  // }
  // else { // No effect
    // flash = false;
    // colorfade = false;

    // if (root.containsKey("color")) {
      // red = root["color"]["r"];
      // green = root["color"]["g"];
      // blue = root["color"]["b"];
    // }

    // if (root.containsKey("brightness")) {
      // brightness = root["brightness"];
    // }

    // if (root.containsKey("transition")) {
      // transitionTime = root["transition"];
    // }
    // else {
      // transitionTime = 0;
    // }
  // }

  return true;
}