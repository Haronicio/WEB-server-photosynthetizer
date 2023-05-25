#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>

#define MQTT_PORT 1884

// WiFi
const char* ssid = "Freebox-7D792E";                
const char* wifi_password = "SebastienSexy7";
WiFiClient wifiClient;

//MQTT
const char* clientID = "esp_synth";
const char* sub_topic = "pi2esp";
const char* pub_topic = "esp2pi";
IPAddress server(192, 168, 1, 12);
PubSubClient client(wifiClient);

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i=0;i<length;i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect(clientID)) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish(pub_topic,"0");
      // ... and resubscribe
      client.subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void connect_MQTT()
{
  client.setServer(server, MQTT_PORT);
  client.setCallback(callback);
}

void connect_WiFi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);
  // Connect to the WiFi
  WiFi.begin(ssid, wifi_password);
  // Wait until the connection is confirmed
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Debugging – Output the IP Address of the ESP8266
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup()
{
  Serial.begin(9600);
  connect_MQTT();
  connect_WiFi();
}

void loop()
{
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
}