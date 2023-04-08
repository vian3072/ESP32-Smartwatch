#include <WiFi.h>
#include <PubSubClient.h>
#include <Servo.h>

// Update these with values suitable for your network.
const char* ssid = "Testing";
const char* password = "test1234";
const char* mqtt_server = "soldier.cloudmqtt.com";

WiFiClient espClient;
PubSubClient client(espClient);

Servo myServo;  // create a servo object
int angle;   // variable to hold the angle for the servo motor

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
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

  long long_payload = atol((char *) payload);
  
  Serial.print(long_payload);

  angle = map(long_payload, 0, 9, 0, 179);
  myServo.write(angle);
  Serial.println();
  if (long_payload > 5 && 9 > long_payload){
    digitalWrite(13, HIGH);
  }else{
    digitalWrite(13, LOW);
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "Client2-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(), "muuntdrr", "LwjsnkExCFwA")) {
      Serial.println("connected");
      client.subscribe("outTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(115200); // open a serial connection to your computer
  
  myServo.attach(23); // attaches the servo on pin 9 to the servo object
   
  setup_wifi();
  client.setServer(mqtt_server, 11487);
  client.setCallback(callback);
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(15);
}
