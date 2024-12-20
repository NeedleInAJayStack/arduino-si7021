#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "Adafruit_Si7021.h"
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
String deviceId = DEVICE_ID;
char wifiSsid[] = WIFI_SSID;
char wifiPass[] = WIFI_PASS;
char mqttUser[] = MQTT_USER;
char mqttPass[] = MQTT_PASS;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

// const char broker[] = "JaysDesktop.local";
const IPAddress* broker = new IPAddress(192, 168, 4, 100);
const int port = 1883;
const String topicTemperature = "arduino/" + deviceId + "/temperature";
const String topicHumidity = "arduino/" + deviceId + "/humidity";

const long interval = 1000; // milliseconds
unsigned long previousMillis = 0;

const long reconnectInterval = 60*1000; // milliseconds

Adafruit_Si7021 si7021 = Adafruit_Si7021();
double temperature;
double humidity;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

  Serial.print("I am device: ");
  Serial.println(deviceId);

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(wifiSsid);
  while (WiFi.begin(wifiSsid, wifiPass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(*broker);
  mqttClient.setId(deviceId);
  mqttClient.setUsernamePassword(mqttUser, mqttPass);
  if (!mqttClient.connect(*broker, port)) {
    Serial.print("MQTT connection failed! Error code = ");
    Serial.println(mqttClient.connectError());
    while (1);
  }
  Serial.println("You're connected to the MQTT broker!");
  Serial.println();

  si7021.begin();
}

void loop() {
  // call poll() regularly to allow the library to send MQTT keep alive which
  // avoids being disconnected by the broker
  mqttClient.poll();
  if (!mqttClient.connected()) {
    Serial.println("MQTT disconnected!");
    while (!mqttClient.connect(*broker, port)) {
      Serial.print("MQTT connection failed! Error code = ");
      Serial.println(mqttClient.connectError());
      delay(reconnectInterval);
    }
    Serial.println("MQTT reconnected!");
  }

  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;

    // Read SI7021
    temperature = si7021.readTemperature();
    humidity = si7021.readHumidity();

    Serial.print("Sending message to topic: ");
    Serial.println(topicTemperature);
    Serial.println(temperature);

    mqttClient.beginMessage(topicTemperature);
    mqttClient.print(temperature);
    mqttClient.endMessage();

    Serial.print("Sending message to topic: ");
    Serial.println(topicHumidity);
    Serial.println(humidity);

    mqttClient.beginMessage(topicHumidity);
    mqttClient.print(humidity);
    mqttClient.endMessage();

    Serial.println();
  }
}