#include <ArduinoJson.h>
#include <PubSubClient.h>
#define TINY_GSM_MODEM_SIM800
#include <TinyGsmClient.h>
#include <SoftwareSerial.h>
#include "xCredentials.h"

#define DEVICE_TYPE "SIM808"
#define JSON_BUFFER_LENGTH 100

// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "internet";
const char user[] = "";
const char pass[] = "";
const char publishTopic[] = "iot-2/evt/status/fmt/json";
const char server[] = ORG ".messaging.internetofthings.ibmcloud.com";
const char authMethod[] = "use-token-auth";
const char token[] = TOKEN;
const char clientId[] = "d:" ORG ":" DEVICE_TYPE ":" DEVICE_ID;

SoftwareSerial SerialAT(8, 9); // RX, TX
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
PubSubClient mqtt(server, 1883, client);

void setup() {
  // Set console baud rate
  Serial.begin(115200);
  delay(10);

  // Set GSM module baud rate
  SerialAT.begin(19200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  Serial.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  Serial.print("Modem: ");
  Serial.println(modemInfo);

  Serial.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");

  Serial.print("Connecting to ");
  Serial.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    Serial.println(" fail");
    while (true);
  }
  Serial.println(" OK");
}

unsigned long lastSent = 0;
void loop() {
  unsigned long now = millis();
  if (now - lastSent > 60000) {
    lastSent = 0;
    if (!mqtt.connected()) {
      mqttConnect();
    }
    publishData();
  }
}

boolean mqttConnect() {
  Serial.print("Connecting MQTT");
  int tryCount = 10;
  while (tryCount-- > 0 && !mqtt.connect(clientId, authMethod, token)) {
    delay(500);
  }

  return tryCount > 0;
}

void mqttDisconnect() {
  if (mqtt.connected()) {
    mqtt.disconnect();
  }
}

boolean publishData() {
  StaticJsonBuffer<JSON_BUFFER_LENGTH> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  JsonObject& d = root.createNestedObject("d");

  d["TA"] = 21.1;
  d["RH"] = 32.2;

  char buff[JSON_BUFFER_LENGTH];
  root.printTo(buff, JSON_BUFFER_LENGTH);

  if (mqtt.publish(publishTopic, buff)) {
    Serial.println("Publish OK");
    return true;
  } else {
    Serial.println("Publish FAILED");
    return false;
  }
}

