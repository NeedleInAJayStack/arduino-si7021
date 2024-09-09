#include <ArduinoMqttClient.h>
#include <WiFiNINA.h>
#include "Adafruit_Si7021.h"
#include "arduino_secrets.h"

///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;
char pass[] = SECRET_PASS;

WiFiClient wifiClient;
MqttClient mqttClient(wifiClient);

const char broker[] = "JaysDesktop.local";
int        port     = 1883;
const char topicTemperature[]  = "test_device.temperature";
const char topicHumidity[]  = "test_device.humidity";

const long interval = 1000; // milliseconds
unsigned long previousMillis = 0;

Adafruit_Si7021 si7021 = Adafruit_Si7021();
double temperature;
double humidity;

int count = 0;

void setup() {
  Serial.begin(9600);
  while (!Serial) {
    delay(10);
  }

  Serial.print("Attempting to connect to WPA SSID: ");
  Serial.println(ssid);
  while (WiFi.begin(ssid, pass) != WL_CONNECTED) {
    // failed, retry
    Serial.print(".");
    delay(5000);
  }
  Serial.println("You're connected to the network");
  Serial.println();

  Serial.print("Attempting to connect to the MQTT broker: ");
  Serial.println(broker);
  if (!mqttClient.connect(broker, port)) {
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