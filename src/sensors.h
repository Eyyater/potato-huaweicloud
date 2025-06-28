// SensorSystem.h
#ifndef SENSOR_SYSTEM_H
#define SENSOR_SYSTEM_H

#include <Arduino.h>
#include <Wire.h>
#include <AS726X.h>
#include <ArtronShop_SHT3x.h>

// ==== I2C总线控制器对象 ====
extern TwoWire I2C_Group1;
extern TwoWire I2C_Group2;

// ==== 传感器对象 ====
extern AS726X as7262_1, as7263_1;
extern AS726X as7262_2, as7263_2;
extern ArtronShop_SHT3x sht3x;

// ==== I2C地址定义 ====
#define AS726X_ADDR 0x49

// ==== 引脚定义 ====
// 第一组引脚（Group1）
#define GROUP1_SDA1 15
#define GROUP1_SCL1 16
#define GROUP1_SDA2 1
#define GROUP1_SCL2 2
// 第二组引脚（Group2）
#define GROUP2_SDA1 4
#define GROUP2_SCL1 5
#define GROUP2_SDA2 10
#define GROUP2_SCL2 11

// ==== 数据变量声明 ====
extern float vis[6], ir[6];
extern float temperature, humidity;
extern float DW, SC, L, a, b, LB, BI;
extern bool shtValid;

// ==== 函数声明 ====
void initializeGroup1();
void initializeGroup2();
void readGroup1();
void readGroup2();
void prediction_data();
void printData();

#endif // SENSOR_SYSTEM_H
