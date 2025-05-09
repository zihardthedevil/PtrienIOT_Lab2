#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

// Cấu hình cảm biến DHT11
#define DHTPIN 14
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

// Thông tin WiFi  
const char* ssid = "Lam Dien";
const char* password = "lam123456789";

// Thông tin MQTT Core IoT
const char* mqtt_server = "app.coreiot.io";
const int mqtt_port = 1883; 

// Access Token của thiết bị
const char* access_token = "wdvw6flumdfl828ytajc"; 

// WiFi & MQTT Client
WiFiClient espClient;
PubSubClient client(espClient);

// Kết nối WiFi
void connectWiFi() {
  Serial.print(" Connecting to WiFi...");
  WiFi.begin(ssid, password);

  unsigned long startAttemptTime = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 15000) {
    delay(500);
    Serial.print(".");
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n WiFi Connected!");
  } else {
    Serial.println("\n WiFi Connection Failed! Retrying...");
  }
}

// Kết nối MQTT với Core IoT
void connectMQTT() {
  client.setServer(mqtt_server, mqtt_port);
  
  while (!client.connected()) {
    Serial.println(" Connecting to MQTT...");
    if (client.connect("ESP32_Client", access_token, "")) {
      Serial.println(" MQTT Connected!");
    } else {
      Serial.print(" MQTT Failed, rc=");
      Serial.print(client.state());
      Serial.println(" -> Retry in 5 seconds");
      delay(5000);
    }
  }
}

// Gửi dữ liệu cảm biến lên Core IoT
void sendSensorData() {
  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  if (isnan(temperature) || isnan(humidity)) {
    Serial.println(" Failed to read from DHT sensor!");
    return;
  }

  // Định dạng dữ liệu JSON
  char payload[50];
  snprintf(payload, sizeof(payload), "{\"temperature\":%.2f,\"humidity\":%.2f}", temperature, humidity);

  // Kiểm tra kết nối MQTT trước khi gửi dữ liệu
  if (!client.connected()) {
    connectMQTT();
  }

  // Gửi dữ liệu lên Core IoT
  if (client.publish("v1/devices/me/telemetry", payload)) {
    Serial.print(" Sent: ");
    Serial.println(payload);
  } else {
    Serial.println(" Failed to send data!");
  }

  Serial.println("=====================");
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  
  connectWiFi();
  connectMQTT();
}

void loop() {
  static unsigned long lastSendTime = 0;
  unsigned long currentTime = millis();

  if (currentTime - lastSendTime >= 5000) { // Gửi dữ liệu mỗi 5 giây
    sendSensorData();
    lastSendTime = currentTime;
  }

  client.loop();
}