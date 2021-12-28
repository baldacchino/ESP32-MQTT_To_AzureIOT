#include <WiFi.h>
#include <PubSubClient.h>
#include "Esp32MQTTClient.h"
 
const char* ssid = "****";
const char* password =  "****";
const char* mqttServer = "****";
const int mqttPort = 1883;
String MQTTTopic;
String MQTTPayload;

//Azure IOT Hub Setup
static const char* connectionString = "****";
static bool hasIoTHub = false;
 
WiFiClient espClient;
PubSubClient client(espClient);
 
void callback(char* topic, byte* payload, unsigned int length) {

  MQTTTopic = String(topic);
  MQTTPayload = ""; 
  for (int i = 0; i < length; i++) {
    // Serial.print((char)payload[i]); - Use for debugging
    MQTTPayload = String(MQTTPayload + (char)payload[i]);
  }    
}



void MQTTConnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("MQTT : Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32Client")) {
      Serial.println("MQTT : Connected");
      // Once connected, publish an announcement...
      client.publish("stat/ESP32/IP_Address","10.0.0.203");
      //Subscribe to topics, one topic per line.
      client.subscribe("stat/+/POWER");      
    } else {
      Serial.print("MQTT : Failed to connect to MQTT , rc=");
      Serial.print(client.state());
      Serial.println("MQTT : Trying again to connect to MQTT in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}



 
void setup() {
  //Set baud rate
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.println("ESP32 : Connecting to WiFi...");
  }
  Serial.println("ESP32 : WiFi connected");
  Serial.println("ESP32 : IP address: ");
  Serial.println(WiFi.localIP());
  //Set MQTT details
  client.setServer(mqttServer, mqttPort);
  client.setCallback(callback);
 
  //Connect to Azure IOT
  if (!Esp32MQTTClient_Init((const uint8_t*)connectionString))
  {
    hasIoTHub = false;
    Serial.println("Azure IoT Hub : Initializing IoT hub failed.");
    return;
  }
  hasIoTHub = true;
 
}

void loop() {
   //Connect to MQTT and reconnect if connection drops
   if (!client.connected()) {
     MQTTConnect();
   }
   //Respond to messages received
   if (MQTTTopic != "") { 
      Serial.println("MQTT : Topic is [" + MQTTTopic +"]");
      Serial.println("MQTT : Payload is [" + MQTTPayload + "]");
      AzureIoTHub(); 
  }
  client.loop();
}

void AzureIoTHub() {
  if (hasIoTHub)
      {
        String tempString;
        tempString = "{" + MQTTTopic + ":" + MQTTPayload + "}";
        if (Esp32MQTTClient_SendEvent(tempString.c_str()))
        {
          Serial.println("Azure IoT Hub : Sending data to Azure IoT Hub succeed");
        }
        else
        {
          Serial.println("Azure IoT Hub : Failure...");
        }
      MQTTPayload = "";
      MQTTTopic = "";

   }
}
