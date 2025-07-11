#include <Arduino.h>
#include <Wire.h>
#include <AS726X.h>
#include <ArtronShop_SHT3x.h>
#include "data_process.h"

// ==== I2C总线定义（使用两个I2C控制器分时复用）====
TwoWire I2C_Group1 = TwoWire(0);  // I2C0 控制器
TwoWire I2C_Group2 = TwoWire(1);  // I2C1 控制器
// ==== 传感器对象初始化 ====
AS726X as7262_1, as7263_1; // 第一组传感器
AS726X as7262_2, as7263_2; // 第二组传感器
ArtronShop_SHT3x sht3x(0x44, &I2C_Group1);  // 温湿度传感器共享Group1总线
// ==== 传感器地址和引脚配置 ====
#define AS726X_ADDR 0x49  // 所有AS726X使用默认地址（需硬件区分总线）
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
// ==== 数据存储变量 ====
float g1_vis[6], g1_ir[6], g2_vis[6], g2_ir[6], vis[6], ir[6];
float temperature = 0, humidity = 0;
float DW, SC, L, a, b, LB, BI, x;
bool shtValid = false;
float vis_norm[6];
float ir_norm[6];

void normalize_array(const float *input, float *output, int n);

float DW_k[12] = {0.209086075, 0.731743515, 0.62925601, 0.552059948, 1.87850106, 0.118908055, 0.132879481, 0.222064957, 1.14516759, 2.40498614, 2.98307157, 2.99394321};
float SC_k[12] = {0.273429662, 0.956928253, 0.822901607, 0.721949399, 2.45658588, 0.155500501, 0.173771471, 0.290402651, 1.49757838, 3.14509034, 3.90107393, 3.91529131};
float L_k[12] = {27.2930412, 95.5180969, 82.1399078, 72.063118, 245.210037, 15.521657, 17.345417, 28.9872379, 149.48439, 313.934753, 389.395081, 390.81424};
float a_k[12] = {10.8823223, 38.0851173, 32.750946, 28.7331142, 97.7705078, 6.18881798, 6.91599083, 11.5578346, 59.6026382, 125.172531, 155.260193, 155.826035};
float b_k[12] = {27.8884449, 97.6018524, 83.9318085, 73.6351929, 250.559357, 15.8602648, 17.7238121, 29.6196022, 152.745422, 320.783325, 397.889832, 399.339935};
float DW_b = 0.174750224, SC_b = 0.228527382, L_b = 22.8110123, a_b = 9.09524059, b_b = 23.3086395;

void initializeGroup1()
{
  // 配置 Group1 传感器的 I2C 总线
  I2C_Group1.begin(GROUP1_SDA1, GROUP1_SCL1);
  I2C_Group2.begin(GROUP1_SDA2, GROUP1_SCL2);
  
  // 初始化 SHT3x（只初始化一次）
  if (!sht3x.begin()) {
    Serial.println("SHT3x 初始化失败!");
  }

  // 初始化第一组光谱传感器
  if (!as7262_1.begin(I2C_Group1, AS726X_ADDR)) {
    Serial.println("AS7262_1 初始化失败!");
    while(1);
  }
  if (!as7263_1.begin(I2C_Group2, AS726X_ADDR)) {
    Serial.println("AS7263_1 初始化失败!");
    while(1);
  }
}

void initializeGroup2() {
  // 配置Group2的总线引脚
  I2C_Group1.begin(GROUP2_SDA1, GROUP2_SCL1);
  I2C_Group2.begin(GROUP2_SDA2, GROUP2_SCL2);
  // 初始化第二组传感器
  if(!as7262_2.begin(I2C_Group1, AS726X_ADDR)) {
    Serial.println("AS7262_2初始化失败!");
    while(1);
  }
  if(!as7263_2.begin(I2C_Group2, AS726X_ADDR)) {
    Serial.println("AS7263_2初始化失败!");
    while(1);
  }
}
void readGroup1() {
  // 启用并读取第一组数据
  as7262_1.enableBulb();
  as7263_1.enableBulb();
  
  as7262_1.takeMeasurements();
  as7263_1.takeMeasurements();
  
  as7262_1.disableBulb();
  as7263_1.disableBulb();
  // 存储可见光数据
  g1_vis[0] = as7262_1.getCalibratedViolet();
  g1_vis[1] = as7262_1.getCalibratedBlue();
  g1_vis[2] = as7262_1.getCalibratedGreen();
  g1_vis[3] = as7262_1.getCalibratedYellow();
  g1_vis[4] = as7262_1.getCalibratedOrange();
  g1_vis[5] = as7262_1.getCalibratedRed();
  // 存储红外数据
  g1_ir[0] = as7263_1.getCalibratedR();
  g1_ir[1] = as7263_1.getCalibratedS();
  g1_ir[2] = as7263_1.getCalibratedT();
  g1_ir[3] = as7263_1.getCalibratedU();
  g1_ir[4] = as7263_1.getCalibratedV();
  g1_ir[5] = as7263_1.getCalibratedW();
  // 读取温湿度
  shtValid = sht3x.measure();
  if(shtValid) {
    temperature = sht3x.temperature();
    humidity = sht3x.humidity();
  }
}

void readGroup2() {
  // 启用并读取第二组数据
  as7262_2.enableBulb();
  as7263_2.enableBulb();
  
  as7262_2.takeMeasurements();
  as7263_2.takeMeasurements();
  
  as7262_2.disableBulb();
  as7263_2.disableBulb();
  // 存储可见光数据
  g2_vis[0] = as7262_2.getCalibratedViolet();
  g2_vis[1] = as7262_2.getCalibratedBlue();
  g2_vis[2] = as7262_2.getCalibratedGreen();
  g2_vis[3] = as7262_2.getCalibratedYellow();
  g2_vis[4] = as7262_2.getCalibratedOrange();
  g2_vis[5] = as7262_2.getCalibratedRed();
  // 存储红外数据
  g2_ir[0] = as7263_2.getCalibratedR();
  g2_ir[1] = as7263_2.getCalibratedS();
  g2_ir[2] = as7263_2.getCalibratedT();
  g2_ir[3] = as7263_2.getCalibratedU();
  g2_ir[4] = as7263_2.getCalibratedV();
  g2_ir[5] = as7263_2.getCalibratedW();

  for (int i = 0; i < 6; i++)
  {
    // 计算vis的平均值
    vis[i] = (g1_vis[i] + g2_vis[i]) / 2.0f;
    // 计算ir的平均值
    ir[i] = (g1_ir[i] + g2_ir[i]) / 2.0f;
  }

  // 调用归一化函数
  normalize_array(vis, vis_norm, 6);
  normalize_array(ir, ir_norm, 6);
}

void prediction_data() { // 预测数据
  DW = predict_data(DW_k, DW_b, vis_norm, ir_norm);
  SC = predict_data(SC_k, SC_b, vis_norm, ir_norm);
  L = predict_data(L_k, L_b, vis_norm, ir_norm);
  a = predict_data(a_k, a_b, vis_norm, ir_norm);
  b = predict_data(b_k, b_b, vis_norm, ir_norm);
  LB = L / b;
  x = (a + 1.75 * L) / (5.645 * L + a - 3.012 * b);
  BI = 100 * (x - 0.31) / 0.172;
}

void printData() {
  // 输出第一组数据
  // Serial.print("[AS7262_1] V:"); Serial.print(g1_vis[0]);
  // Serial.print(" B:"); Serial.print(g1_vis[1]);
  // Serial.print(" G:"); Serial.print(g1_vis[2]);
  // Serial.print(" Y:"); Serial.print(g1_vis[3]);
  // Serial.print(" O:"); Serial.print(g1_vis[4]);
  // Serial.print(" R:"); Serial.println(g1_vis[5]);
  // Serial.print("[AS7263_1] R:"); Serial.print(g1_ir[0]);
  // Serial.print(" S:"); Serial.print(g1_ir[1]);
  // Serial.print(" T:"); Serial.print(g1_ir[2]);
  // Serial.print(" U:"); Serial.print(g1_ir[3]);
  // Serial.print(" V:"); Serial.print(g1_ir[4]);
  // Serial.print(" W:"); Serial.println(g1_ir[5]);
  // 输出第二组数据
  // Serial.print("[AS7262_2] V:"); Serial.print(g2_vis[0]);
  // Serial.print(" B:"); Serial.print(g2_vis[1]);
  // Serial.print(" G:"); Serial.print(g2_vis[2]);
  // Serial.print(" Y:"); Serial.print(g2_vis[3]);
  // Serial.print(" O:"); Serial.print(g2_vis[4]);
  // Serial.print(" R:"); Serial.println(g2_vis[5]);
  // Serial.print("[AS7263_2] R:"); Serial.print(g2_ir[0]);
  // Serial.print(" S:"); Serial.print(g2_ir[1]);
  // Serial.print(" T:"); Serial.print(g2_ir[2]);
  // Serial.print(" U:"); Serial.print(g2_ir[3]);
  // Serial.print(" V:"); Serial.print(g2_ir[4]);
  // Serial.print(" W:"); Serial.println(g2_ir[5]);
  // 输出平均数据
  Serial.print("[AS7262_ave] V:"); Serial.print(vis[0]);
  Serial.print(" B:"); Serial.print(vis[1]);
  Serial.print(" G:"); Serial.print(vis[2]);
  Serial.print(" Y:"); Serial.print(vis[3]);
  Serial.print(" O:"); Serial.print(vis[4]);
  Serial.print(" R:"); Serial.println(vis[5]);
  Serial.print("[AS7263_ave] R:"); Serial.print(ir[0]);
  Serial.print(" S:"); Serial.print(ir[1]);
  Serial.print(" T:"); Serial.print(ir[2]);
  Serial.print(" U:"); Serial.print(ir[3]);
  Serial.print(" V:"); Serial.print(ir[4]);
  Serial.print(" W:"); Serial.println(ir[5]);
  // 输出归一化数据
  Serial.print("[AS7262_norm] V:"); Serial.print(vis_norm[0]);
  Serial.print(" B:"); Serial.print(vis_norm[1]);
  Serial.print(" G:"); Serial.print(vis_norm[2]);
  Serial.print(" Y:"); Serial.print(vis_norm[3]);
  Serial.print(" O:"); Serial.print(vis_norm[4]);
  Serial.print(" R:"); Serial.println(vis_norm[5]);
  Serial.print("[AS7263_norm] R:"); Serial.print(ir_norm[0]);
  Serial.print(" S:"); Serial.print(ir_norm[1]);
  Serial.print(" T:"); Serial.print(ir_norm[2]);
  Serial.print(" U:"); Serial.print(ir_norm[3]);
  Serial.print(" V:"); Serial.print(ir_norm[4]);
  Serial.print(" W:"); Serial.println(ir_norm[5]);
  // 输出温湿度
  if(shtValid) {
    Serial.print("[SHT3x] Temp:");
    Serial.print(temperature, 1);
    Serial.print("°C  Humi:");
    Serial.print(humidity, 1);
    Serial.println("%");
  } else {
    Serial.println("[SHT3x] 读取错误");
  }
  // Serial.print(SC);
  // Serial.println("==================================");
  // Serial.print(L);
  // Serial.println("==================================");
  // Serial.print(a);
  // Serial.println("==================================");
  // Serial.print(b);
  Serial.println("==================================");
}
