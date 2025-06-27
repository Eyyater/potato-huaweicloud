#include <ESP32Ping.h>  // 需要安装 ESP32Ping 库

void testPing() {
    IPAddress ip;
    if (WiFi.hostByName(MQTT_Server, ip)) {
        Serial.print("Resolved IP: ");
        Serial.println(ip);
        bool result = Ping.ping(ip, 3);  // Ping 服务器 3 次
        if (result) {
            Serial.println("Ping successful!");
        } else {
            Serial.println("Ping failed.");
        }
    } else {
        Serial.println("DNS resolution failed.");
    }
}
