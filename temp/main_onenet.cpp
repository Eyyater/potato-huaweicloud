#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <ArduinoJson.h>
#include "sensors.h"

// WiFi 配置
const char* ssid = "Xiaomi";
const char* password = "88888888";

// MQTT 配置
#define MQTT_Server "9L83yUyQ3a.mqtts.acc.cmcconenet.cn"
#define MQTT_Port 1883
#define MQTT_ClientID "potato_sensor"
#define MQTT_UserName "9L83yUyQ3a"
#define MQTT_PassWord "version=2018-10-31&res=products%2F9L83yUyQ3a&et=1807704708&method=sha1&sign=2sjCCqksYOJGw6ZafrGya3bFuvk%3D"

// 订阅与发布的主题
#define RECEIVE_TOPIC "$sys/9L83yUyQ3a/potato_sensor/#"  // 平台下发信息
#define SEVICE_TOPIC "$sys/9L83yUyQ3a/potato_sensor/dp/post/json"  // 设备属性更新

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

void publishTRH(double temperature, double humidity, const char* topic) {
    char payload[256];
    snprintf(payload, sizeof(payload),
        "{"
        "\"id\":123,"
        "\"dp\":{\"temperature\":[{"
        "\"v\":%.1f,"
        "}],"
        "\"humidity\":[{"
        "\"v\":%.1f,"
        "}]}}",
        temperature, humidity);

    if (client.publish(topic, payload)) {
        Serial.println("TRH message published successfully!");
    } else {
        Serial.println("TRH message publish failed!");
    }
}

void publishVIS(float vis[], int sensor_index, const char* topic) {
    const char* base_labels[] = {"violet", "blue", "green", "yellow", "orange", "red"};
    char payload[512];
    char label_with_index[16];
    
    snprintf(payload, sizeof(payload), "{\"id\": 123,\"dp\":{");

    for (int i = 0; i < 6; i++) {
        char buffer[64];
        snprintf(label_with_index, sizeof(label_with_index), "%s_%d", base_labels[i], sensor_index);
        snprintf(buffer, sizeof(buffer), "\"%s\":[{\"v\":%.2f}]%s", label_with_index, vis[i], (i < 5) ? "," : "");
        strcat(payload, buffer);
    }

    strcat(payload, "}}");

    if (client.publish(topic, payload)) {
        Serial.println("VIS message published to OneNet successfully!");
    } else {
        Serial.println("VIS message publish to OneNet failed!");
    }
}

void publishNIR(float ir[], int sensor_index, const char* topic) {
    const char* base_labels[] = {"r", "s", "t", "u", "v", "w"};
    char payload[512];
    char label_with_index[16];

    snprintf(payload, sizeof(payload), "{\"id\": 123,\"dp\":{");

    for (int i = 0; i < 6; i++) {
        char buffer[64];
        snprintf(label_with_index, sizeof(label_with_index), "%s_%d", base_labels[i], sensor_index);
        snprintf(buffer, sizeof(buffer), "\"%s\":[{\"v\":%.2f}]%s", label_with_index, ir[i], (i < 5) ? "," : "");
        strcat(payload, buffer);
    }

    strcat(payload, "}}");

    if (client.publish(topic, payload)) {
        Serial.println("NIR message published to OneNet successfully!");
    } else {
        Serial.println("NIR message publish to OneNet failed!");
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message received [");
    Serial.print(topic);
    Serial.print("] ");
    for (int i = 0; i < length; i++) {
        Serial.print((char)payload[i]);
    }
    Serial.println();
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

    while (!Serial);  // 等待串口连接
}

void reconnect() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection...");
        if (client.connect(MQTT_ClientID, MQTT_UserName, MQTT_PassWord)) {
            Serial.println("Connected to MQTT Broker!");
            client.subscribe(RECEIVE_TOPIC);  // 订阅消息
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

    delay(1000);

    // 采集第一组数据
    initializeGroup1();
    readGroup1();
    delay(25); // 短延时确保总线释放
    I2C_Group2.end();
    delay(100); // 组间间隔
    // 采集第二组数据
    initializeGroup2();
    readGroup2();
    delay(25);
    I2C_Group2.end();

    // 输出所有数据
    printData();

    // 发送数据
    publishTRH(temperature, humidity, SEVICE_TOPIC);
    publishVIS(g1_vis, 1, SEVICE_TOPIC);
    publishNIR(g1_ir, 1, SEVICE_TOPIC);
    publishVIS(g2_vis, 2, SEVICE_TOPIC);
    publishNIR(g2_ir, 2, SEVICE_TOPIC);

    delay(5000);  // 每 5 秒上报一次
}
