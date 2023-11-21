#include "Configuration.h"

//ESP8266-01  1M FS:none OTA 502kB

// Real values to write to the LEDs (ex. including brightness and state)
byte          realRed           = 0;
byte          realGreen         = 0;
byte          realBlue          = 0;
byte          smer              = 0;
byte          effect            = 0;

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


//MQTT callback
void callback(char* topic, byte* payload, unsigned int length) {
  String val =  String();
  DEBUG_PRINT("\nMessage arrived [");
  DEBUG_PRINT(topic);
  DEBUG_PRINT("] ");
  for (unsigned int i=0;i<length;i++) {
    DEBUG_PRINT((char)payload[i]);
    val += (char)payload[i];
  }
  DEBUG_PRINTLN();
  
  if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_restart)).c_str())==0) {
    DEBUG_PRINT("RESTART");
    ESP.restart();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_netinfo)).c_str())==0) {
    DEBUG_PRINT("NET INFO");
    sendNetInfoMQTT();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_config_portal)).c_str())==0) {
    startConfigPortal();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_config_portal_stop)).c_str())==0) {
    stopConfigPortal();
  } else if (strcmp(topic, (String(mqtt_base) + "/" + String(mqtt_topic_load)).c_str())==0) {
    processJson(val);
  }  
}

ADC_MODE(ADC_VCC); //vcc read

//----------------------------------------------------- S E T U P -----------------------------------------------------------
void setup() {
  preSetup();
  client.setCallback(callback);

  pinMode(redPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(bluePin, OUTPUT);

  digitalWrite(redPin, 0);
  digitalWrite(greenPin, 0);
  digitalWrite(bluePin, 0);

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


#ifdef timers
//setup timers
  timer.every(SENDSTAT_DELAY, sendStatisticMQTT);
#endif
  
  void * a=0;
  reconnect(a);
  sendStatisticMQTT(a);
  sendNetInfoMQTT();
  
  ticker.detach();
  //keep LED on
  digitalWrite(LED_BUILTIN, HIGH);

  drd.stop();

  DEBUG_PRINTLN(F("SETUP END......................."));
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

  client.loop();
  wifiManager.process();
  
  if (effect==1) {
    if (millis()%10==0) { 
      if (smer==0) {
        analogWrite(redPin, realRed--);
      } else {
        analogWrite(redPin, realRed++);
      }
    }

    if (smer==0 && realRed==0) {
      smer=1;
    }

    if (smer==1 && realRed==255) {
      smer=0;
    }
  } else if (effect==2) {
    if (millis()%1000==0) {
      analogWrite(redPin,   random(0,255));
      analogWrite(bluePin,  random(0,255));
      analogWrite(greenPin, random(0,255));
    }
  }
  
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


bool reconnect(void *) {
  if (!client.connected()) {
    DEBUG_PRINT("Attempting MQTT connection...");
    if (client.connect(mqtt_base, mqtt_username, mqtt_key, (String(mqtt_base) + "/LWT").c_str(), 2, true, "offline", true)) {
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_topic_restart)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_topic_netinfo)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_config_portal)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_config_portal_stop)).c_str());
      client.subscribe((String(mqtt_base) + "/" + String(mqtt_topic_load)).c_str());
      client.publish((String(mqtt_base) + "/LWT").c_str(), "online", true);
      DEBUG_PRINTLN("connected");
    } else {
      DEBUG_PRINT("disconected.");
      DEBUG_PRINT(" Wifi status:");
      DEBUG_PRINT(WiFi.status());
      DEBUG_PRINT(" Client status:");
      DEBUG_PRINTLN(client.state());
    }
  }
  return true;
}

bool processJson(String message) {
  char json[500];
  message.toCharArray(json, 500);
  DEBUG_PRINTLN(json);

  DynamicJsonDocument doc(1024);
  deserializeJson(doc, json);
  String e = doc["effect"];
  DEBUG_PRINTLN(e);
  if (e=="null") {
    //{effect:"",r:100,g:0,b:0}
    realRed = doc["r"];
    DEBUG_PRINTLN(realRed);
    realBlue = doc["b"];
    DEBUG_PRINTLN(realBlue);
    realGreen = doc["g"];
    DEBUG_PRINTLN(realGreen);
    analogWrite(redPin, realRed);
    analogWrite(bluePin, realBlue);
    analogWrite(greenPin, realGreen);
  } else if (e=="heartbeat") {
    //{effect:"heartbeat"}
    effect = 1;
  } else if (e=="random") {
    //{effect:"random"}
    effect = 2;
  } else if (e=="stop") {
    //{effect:"stop"}
    effect = 0;
    analogWrite(redPin, 0);
    analogWrite(bluePin, 0);
    analogWrite(greenPin, 0);
  }

 

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


void breath(int t, int p, int d) {
  if (t==0) { //up and down
    for (uint8_t i=0; i<255; i++) {
      analogWrite(p, i);
      delay(d);
    }
    for (uint8_t i=255; i>0; i--) {
      analogWrite(p, i);
      delay(d);
    }
  }
}