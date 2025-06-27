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
#define MQTT_Server "9b0766b4b1.st1.iotda-device.cn-north-4.myhuaweicloud.com"
#define MQTT_Port 1883
#define MQTT_ClientID "67e53e652902516e866b8487_potato-sensor-test_0_0_2025062702"
#define MQTT_UserName "67e53e652902516e866b8487_potato-sensor-test"
#define MQTT_PassWord "45f1d1f8afe05abcd2db75facf5a16d38ffc5be3abf35a755742b82404fcdcbc"

// 订阅与发布的主题
#define RECEIVE_TOPIC "$oc/devices/67e53e652902516e866b8487_potato-sensor-test/sys/messages/down"  // 平台下发信息
#define SEVICE_TOPIC "$oc/devices/67e53e652902516e866b8487_potato-sensor-test/sys/properties/report"  // 设备属性更新

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
        "\"services\":[{\"service_id\":\"TRH\","
        "\"properties\":{\"temperature\":%.1f,\"humidity\":%.1f}"
        "}]}",
        temperature, humidity);

    if (client.publish(topic, payload)) {
        Serial.println("TRH message published successfully!");
    } else {
        Serial.println("TRH message publish failed!");
    }
}

void publishVIS(float vis[], const char* topic) {
    const char* vis_labels[] = {"V", "B", "G", "Y", "O", "R"};
    char payload[512];

    snprintf(payload, sizeof(payload), "{\"services\":[{\"service_id\":\"VIS\",\"properties\":{");
    
    for (int i = 0; i < 6; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "\"%s\":%.2f%s", vis_labels[i], vis[i], (i < 5) ? "," : "");
        strcat(payload, buffer);
    }

    strcat(payload, "}}]}");

    // printf(payload);

    if (client.publish(topic, payload)) {
        Serial.println("VIS message published successfully!");
    } else {
        Serial.println("VIS message publish failed!");
    }
}

void publishNIR(float ir[], const char* topic) {
    const char* ir_labels[] = {"R", "S", "T", "U", "V", "W"};
    char payload[512];

    snprintf(payload, sizeof(payload), "{\"services\":[{\"service_id\":\"NIR\",\"properties\":{");
    
    for (int i = 0; i < 6; i++) {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "\"%s\":%.2f%s", ir_labels[i], ir[i], (i < 5) ? "," : "");
        strcat(payload, buffer);
    }

    strcat(payload, "}}]}");

    if (client.publish(topic, payload)) {
        Serial.println("NIR message published successfully!");
    } else {
        Serial.println("NIR message publish failed!");
    }
}

void publishDWSC(double DW, double SC, const char *topic)
{
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{"
             "\"services\":[{\"service_id\":\"DW&SC\","
             "\"properties\":{\"DW\":%.1f,\"SC\":%.1f}"
             "}]}",
             DW, SC);

    if (client.publish(topic, payload))
    {
        Serial.println("DW&HC message published successfully!");
    }
    else
    {
        Serial.println("DW&HC message publish failed!");
    }
}

void publishLab(double L, double a, double b, const char *topic)
{
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{"
             "\"services\":[{\"service_id\":\"Lab\","
             "\"properties\":{\"L\":%.1f,\"a\":%.1f,\"b\":%.1f}"
             "}]}",
             L, a, b);

    if (client.publish(topic, payload))
    {
        Serial.println("Lab message published successfully!");
    }
    else
    {
        Serial.println("Lab message publish failed!");
    }
}

void publishLBBI(double LB, double BI, const char *topic)
{
    char payload[256];
    snprintf(payload, sizeof(payload),
             "{"
             "\"services\":[{\"service_id\":\"LB&BI\","
             "\"properties\":{\"LB\":%.1f,\"BI\":%.1f}"
             "}]}",
             LB, BI);

    if (client.publish(topic, payload))
    {
        Serial.println("LB&BI message published successfully!");
    }
    else
    {
        Serial.println("LB&BI message publish failed!");
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
    // TODO:运算预测数据
    prediction_data();

    // 输出所有数据
    printData();

    // 发送数据
    // TODO:接入数据预测
    publishTRH(temperature, humidity, SEVICE_TOPIC);
    publishVIS(vis, SEVICE_TOPIC);
    publishNIR(ir, SEVICE_TOPIC);
    publishDWSC(DW, SC, SEVICE_TOPIC);
    publishLab(L, a, b, SEVICE_TOPIC);
    publishLBBI(LB, BI, SEVICE_TOPIC);

    delay(5000);  // 每 5 秒上报一次
}
