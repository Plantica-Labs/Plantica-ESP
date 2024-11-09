#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Ticker.h>
#include <AsyncMqttClient.h>


const int dryVal = 860;
const int wetVal = 500;

#define WIFI_SSID "test"
#define WIFI_PASSWORD "test"

#define MQTT_HOST IPAddress(192, 168, 31, 3)
#define MQTT_PORT 1883

// 29% dry
// 58% wet
AsyncMqttClient mqttClient;
Ticker mqttReconnectTimer;

WiFiEventHandler wifiConnectHandler;
WiFiEventHandler wifiDisconnectHandler;
Ticker wifiReconnectTimer;

Ticker humidityTicker;



void connectToWifi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
}

void connectToMqtt() {
  Serial.println("Connecting to MQTT...");
  mqttClient.connect();
}

void onWifiConnect(const WiFiEventStationModeGotIP& event) {
  Serial.println("Connected to Wi-Fi.");
  connectToMqtt();
}

void onWifiDisconnect(const WiFiEventStationModeDisconnected& event) {
  Serial.println("Disconnected from Wi-Fi.");
  mqttReconnectTimer.detach(); // ensure we don't reconnect to MQTT while reconnecting to Wi-Fi
  wifiReconnectTimer.once(2, connectToWifi);
}

int prvHumid;
void updateSensorVal(){
  const int sensorVal = analogRead(A0);
  const int humidity = map(sensorVal, wetVal, dryVal, 100, 0);
  Serial.println(humidity);
  // don't do anything if we have the same humidity
  if(prvHumid == humidity || abs(prvHumid - humidity) == 1){
    return;
  }
  prvHumid = humidity;
  char payload[4];
  itoa(humidity, payload, 10);
  mqttClient.publish("plantica/plant1", 0, 0, payload);
}

void onMqttConnect(bool sessionPresent) {
  Serial.println("Connected to MQTT.");
  // run the ticker for every 3 when we connect to MQTT
  humidityTicker.attach(3, updateSensorVal);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  wifiConnectHandler = WiFi.onStationModeGotIP(onWifiConnect);
  wifiDisconnectHandler = WiFi.onStationModeDisconnected(onWifiDisconnect);

  mqttClient.onConnect(onMqttConnect);
  mqttClient.setServer(MQTT_HOST, MQTT_PORT);
  connectToWifi();
  
}

void loop() {

  // Serial.printf("%d%%\n",humidity);
  // delay(1000);
}

