#include <Arduino.h>
#include <Wire.h>
#include <ArtronShop_SHT3x.h>

// ==== I2C 总线引脚定义 ====
#define SDA_PIN 21  // SDA 连接至 GPIO 21
#define SCL_PIN 22  // SCL 连接至 GPIO 22

// ==== 传感器对象初始化 ====
TwoWire I2C_bus = TwoWire(0);  // 创建 I2C 控制器
ArtronShop_SHT3x sht3x(0x44, &I2C_bus);  // 传感器 I2C 地址 0x44

// ==== 数据存储变量 ====
float temperature = 0, humidity = 0;
bool shtValid = false;

void setup() {
    Serial.begin(115200);
    while (!Serial);  // 等待串口连接

    // ==== 初始化 I2C 总线 ====
    I2C_bus.begin(SDA_PIN, SCL_PIN, 100000);  // 100kHz I2C 速率

    // ==== 初始化 SHT3x 传感器 ====
    if (!sht3x.begin()) {
        Serial.println("[SHT3x] 传感器初始化失败，请检查连接！");
        while (1);
    }
}

void readSHT3x() {
    shtValid = sht3x.measure();  // 读取数据
    if (shtValid) {
        temperature = sht3x.temperature();
        humidity = sht3x.humidity();
    }
}

void printSHT3xData() {
    if (shtValid) {
        Serial.print("[SHT3x] Temp: ");
        Serial.print(temperature, 1);
        Serial.print("°C  Humi: ");
        Serial.print(humidity, 1);
        Serial.println("%");
    } else {
        Serial.println("[SHT3x] 读取错误");
    }
}

void loop() {
    readSHT3x();
    printSHT3xData();
    delay(2000);  // 每 2 秒采集一次数据
}
