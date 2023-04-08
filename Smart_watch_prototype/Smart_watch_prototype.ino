#include <ArduinoJson.h>
#include "Adafruit_VL53L0X.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <PubSubClient.h>

Adafruit_VL53L0X lox = Adafruit_VL53L0X();
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

const char* ssid = "Testing";
const char* password =  "test1234";
const char* mqtt_server = "soldier.cloudmqtt.com";

const String endpoint = "http://api.openweathermap.org/data/2.5/weather?q=Jakarta,IDN&units=metric&APPID=";
const String key = "655ed144f00964157765efc7c5cfe58f";

WiFiClient espClient;
PubSubClient client(espClient);

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Client1-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "muuntdrr", "LwjsnkExCFwA")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void sendPayload() {
  // Once connected, publish an announcement...
  VL53L0X_RangingMeasurementData_t measure;

  Serial.print("Reading a measurement... ");
  lox.rangingTest(&measure, false); // pass in 'true' to get debug data printout!

  if (measure.RangeStatus != 5)
  { // phase failures have incorrect data
    Serial.print("Distance (cm): ");
    Serial.println(measure.RangeMilliMeter / 10);
  }
  else
  {
    Serial.println(" out of range ");
  }

  char payload[50];
  float measurement1 = measure.RangeMilliMeter / 10;
  sprintf(payload, "%f", measurement1);

  client.publish("outTopic", payload);
}

void setup() {
  Serial.println("Adafruit VL53L0X test");

  if (!lox.begin())
  {
    Serial.println(F("Failed to boot VL53L0X"));
  }

  Serial.println(F("VL53L0X API Simple Ranging example\n\n"));
  Serial.begin(115200);

  setup_wifi();
  client.setServer(mqtt_server, 11487);
  client.setCallback(callback);

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
  }

  delay(2000);
  display.clearDisplay();

  display.setTextSize(1.5);
  display.setTextColor(WHITE);
  display.setCursor(0, 10);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }

  client.loop();

  if (WiFi.status() == WL_CONNECTED) { // Check the current connection status
    HTTPClient http;

    http.begin(endpoint + key); // Specify the URL
    int httpCode = http.GET();  // Make the request

    if (httpCode > 0) { // Check for the returning code
      const size_t bufferSize = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(1) + \
                                2 * JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(4) + \
                                JSON_OBJECT_SIZE(5) + JSON_OBJECT_SIZE(6) + JSON_OBJECT_SIZE(13);
      
      DynamicJsonDocument doc(bufferSize);
      DeserializationError error = deserializeJson(doc, http.getString()); // Parse JSON object
      
      if (error) {
        Serial.print(F("deserializeJson() failed: "));
        Serial.println(error.c_str());
      }
      
      Serial.println(httpCode);

      display.setCursor(0, 20);
      display.printf("Condition =%s\n", doc["weather"][0]["description"].as<char*>());
      display.printf("Temperature = %d\n", doc["main"]["temp"].as<int>());
      display.printf("Pressure    = %d\n", doc["main"]["pressure"].as<int>());
      display.printf("Humidity    = %d\n", doc["main"]["humidity"].as<int>());
      display.display();
    }
    else {
      Serial.println("Error on HTTP request");
    }
    
    http.end(); // Free the resources
  }

  sendPayload();

  delay(100);
  display.clearDisplay();
  display.setCursor(0, 10);
}
