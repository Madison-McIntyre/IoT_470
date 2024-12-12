#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <PubSubClient.h>

// WiFi variables
const char* ssid = "";  // Enter your WiFi name
const char* password = "";  // Enter WiFi password

// MQTT variables
const char* mqtt_server = "broker.mqtt-dashboard.com";
const char* publishTopic = "testtopic/temp/outTopic/potvalue";   // outTopic where ESP publishes
const char* subscribeTopic = "testtopic/temp/inTopic/potvalue";  // inTopic where ESP has subscribed to
#define publishTimeInterval 20000 // in seconds 

// Definitions 
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE (50)
const int LED = 13;  // LED on pin D7
const int Potentiometer = A0; // Potentiometer on pin A0
const int Button = 4; // Button on pin D4
#define BUILTIN_LED 2 // built-in LED
char msg[MSG_BUFFER_SIZE];
int value = 0;
int ledStatus = 0;

const int debounceTime = 50; // milliseconds
int lastButtonState = HIGH; // Initial state assumed HIGH (not pressed)
int currentButtonState;
unsigned long buttonPressTime = 0; // Time when button was pressed

WiFiClient espClient;
PubSubClient client(espClient); // define MQTTClient 

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
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

//------------------------------------------
//Receives published topic subscribed
void callback(char* topic, byte* payload, int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");

  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
    if (payload[i] == '1') {
      digitalWrite(LED, HIGH);
      Serial.println("LED turned ON");
    } else if (payload[i] == '0') {
      digitalWrite(LED, LOW);
      Serial.println("LED turned OFF");
    }
  }
  Serial.println();
}

//------------------------------------------

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // ... and resubscribe
      client.subscribe(subscribeTopic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

//------------------------------------------

void setup() {
  pinMode(LED, OUTPUT);
  pinMode(Button, INPUT);
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

//------------------------------------------
void loop() {
  if (!client.connected()) {
    reconnect(); // Check for the latest value in inTopic
  }
  client.loop();

  // Publish to outTopic
  unsigned long now = millis();
  if (now - lastMsg > publishTimeInterval) {
    lastMsg = now;
    ++value;
    snprintf(msg, MSG_BUFFER_SIZE, "Number # %d", value); // prints Number # 1, Number # 2, .....
    Serial.print("Publish message: ");
    Serial.println(msg);
    client.publish(publishTopic, msg); // Variables published

    // Prepare message for potentiometer value
    int potValue = analogRead(Potentiometer);
    float voltage = potValue * (5.0 / 1023.0); // Convert to voltage
    char potMessage[MSG_BUFFER_SIZE];
    snprintf(potMessage, MSG_BUFFER_SIZE, "Pot Value: %f", voltage);
    Serial.print("Publish potentiometer value: ");
    Serial.println(potMessage);

    // Publish the potentiometer value
    client.publish(publishTopic, potMessage); // Send pot value as a string      
  }

  // Button handling
  currentButtonState = digitalRead(Button);

  if (currentButtonState == LOW && lastButtonState == HIGH) {
    // Button pressed (state changed from HIGH to LOW)
    snprintf(msg, MSG_BUFFER_SIZE, "%d", 1);  // Send "1" as a string
    client.publish(publishTopic, msg);
    Serial.println("Button pressed, sent 1");
    buttonPressTime = millis(); // Start timer
    ledStatus = 1; // Mark that the button press event has occurred
  }

  if (ledStatus == 1 && millis() - buttonPressTime >= 5000) {
    // 5 seconds have passed since button press
    snprintf(msg, MSG_BUFFER_SIZE, "%d", 0);  // Send "0" as a string
    client.publish(publishTopic, msg);
    Serial.println("5 seconds passed, sent 0");
    ledStatus = 0; // Reset status
  }

  // Update the lastButtonState to detect changes
  lastButtonState = currentButtonState;
}
