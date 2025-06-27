#include <WiFi.h>
#include <PubSubClient.h>
#include <ESP32Ping.h>
#include <Wire.h>
#include <ArtronShop_SHT3x.h>

// WiFi 配置
const char* ssid = "Xiaomi";
const char* password = "88888888";

// MQTT 配置
#define MQTT_Server "9b0766b4b1.st1.iotda-device.cn-north-4.myhuaweicloud.com"
#define MQTT_Port 1883
#define MQTT_ClientID "67e53e652902516e866b8487_potato-sensor-test_0_0_2025032913"
#define MQTT_UserName "67e53e652902516e866b8487_potato-sensor-test"
#define MQTT_PassWord "576635eda4c0fb668d0c120eb1bff5a9d3f3f1f27e41dffd5ffc3b551f860ed3"

// 订阅与发布的主题
#define SET_TOPIC "$oc/devices/67e53e652902516e866b8487_potato-sensor-test/sys/messages/down"  // 订阅
#define POST_TOPIC "$oc/devices/67e53e652902516e866b8487_potato-sensor-test/sys/properties/report"  // 发布

WiFiClient espClient;
PubSubClient client(espClient);

void setupWiFi() {
    Serial.print("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
    Serial.println("Message processed successfully.");
}

void publishTemperatureHumidity(double temperature, double humidity) {
    char payload[256];

    snprintf(payload, sizeof(payload),
        "{"
        "\"services\":[{\"service_id\":\"sensor\","
        "\"properties\":{\"temperature\":%.1f,\"humidity\":%.1f}"
        "}]}",
        temperature, humidity);

    Serial.print("Publishing message: ");
    Serial.println(payload);
    
    if (client.publish(POST_TOPIC, payload)) {
        Serial.println("Message published successfully!");
    } else {
        Serial.println("Message publish failed!");
    }
}

void setup() {
    Serial.begin(115200);
    Serial.println("Initializing setup...");
    setupWiFi();
    delay(5000);
    client.setServer(MQTT_Server, MQTT_Port);
    client.setKeepAlive(60);
    client.setCallback(callback);
    Serial.println("Setup complete.");
    // testPing();
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(MQTT_ClientID, MQTT_UserName, MQTT_PassWord)) {
            Serial.println("Connected to MQTT Broker!");
            client.subscribe(SET_TOPIC);  // 订阅消息
            Serial.println("Subscribed to topic: $oc/devices/67e53e652902516e866b8487_potato-sensor-test/sys/messages/down");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println(" retrying in 5 seconds");
            delay(5000);
        }
    }
}

void loop() {
    if (!client.connected()) {
        Serial.println("MQTT disconnected, attempting to reconnect...");
        reconnect();
    }
    client.loop();
    Serial.println("MQTT loop running...");
    Serial.print("MQTT client state: ");
    Serial.println(client.state());

    delay(1000);

    // 模拟温湿度数据
    float temperature = random(20, 30) + random(0, 100) / 100.0;
    float humidity = random(40, 60) + random(0, 100) / 100.0;

    // 发送数据
    publishTemperatureHumidity(temperature, humidity);

    delay(10000);  // 每 10 秒上报一次
}
