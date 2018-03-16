#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <ESP8266HTTPClient.h>

#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>
#include <Ticker.h>

#define OK_DELAY 1000
#define FAIL_DELAY 100
#define ERROR_DELAY 10
#define LED 5
// blink for at least 5 seconds

//const char* ssid = "open25";
//const char* password = "makeanicecupoftea";

// how much time before dsleep
signed long gracetime = 10000;
enum States { BOOTUP, SUCCESS, FAILURE, SLEEP, FAILED_CONNECT };
int state = BOOTUP;
const char* query_url = "http://192.168.8.2/cgi-bin/toggle-adblock";
Ticker ticker;

void tick()
{
  //toggle state
  int state = digitalRead(LED);  // get the current state of GPIO1 pin
  digitalWrite(LED, !state);     // set pin to the opposite state
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


void setup() {
  WiFiManager wifiManager;
  // initialize NodeMCU LED as an output.
  pinMode(5, OUTPUT);

  Serial.begin(115200);
  Serial.println("Booting");
  ticker.attach(0.6, tick);

  wifiManager.autoConnect("AutoConnectAP");
  wifiManager.setAPCallback(configModeCallback);

  // Port defaults to 8266
  // ArduinoOTA.setPort(8266);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname("unblockr");

  // No authentication by default
  // ArduinoOTA.setPassword("admin");

  // Password can be set with it's md5 value as well
  // MD5(admin) = 21232f297a57a5a743894a0e4a801fc3
  // ArduinoOTA.setPasswordHash("21232f297a57a5a743894a0e4a801fc3");

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });
  
  Serial.print("Starting ArduinoOTA");
  ArduinoOTA.begin();

  ticker.detach();
  // configure static ip for faster bootup
}

void loop() {
  ArduinoOTA.handle();
  digitalWrite(5, HIGH);
  switch (state) {
    case BOOTUP:
      {
      HTTPClient http;
      int httpCode = http.GET();
      Serial.println("Sending HTTP Query");
      http.begin(query_url);
      http.end();
      if (httpCode < 0) {
        Serial.println("SUCCESS");
        state = FAILURE;
      } else {
        Serial.println("FAILURE");
        state = SUCCESS;
      };
      }break;
    case SUCCESS:
      {
        delay(FAIL_DELAY);
        digitalWrite(5, LOW);
        delay(FAIL_DELAY);
        gracetime -= FAIL_DELAY *2;
      }
      break;
    case FAILURE:
      delay(OK_DELAY);
      digitalWrite(5, LOW);
      delay(OK_DELAY);
      gracetime -= OK_DELAY *2;
      break;
    case SLEEP:
      Serial.println("Deep Sleep");
      ESP.deepSleep(0);
      // TODO: maybe we can simply wake up with the last wifi state
      // ESP.deepSleep(0, WAKE_RF_DISABLED)
      delay(100);
  };
  if (gracetime < 0) {
    state = SLEEP;
  }
}
