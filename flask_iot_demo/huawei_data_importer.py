import sqlite3
from get_shadow import get_shadow

def extract_temp_humidity(shadow_data):
    for s in shadow_data["shadow"]:
        if s["service_id"] == "TRH":
            reported = s["reported"]
            temp = reported["properties"]["temperature"]
            hum = reported["properties"]["humidity"]
            time = reported["event_time"]
            return temp, hum, time
    return None, None, None

def save_to_db(temp, hum, time):
    # 转成 float 类型
    temp = float(temp)
    hum = float(hum)
    
    conn = sqlite3.connect("potato_sensor.db")
    cursor = conn.cursor()
    cursor.execute(
        "INSERT INTO potato_sensor (temperature, humidity, timestamp) VALUES (?, ?, ?)",
        (temp, hum, time)
    )
    conn.commit()
    conn.close()

if __name__ == "__main__":
    data = get_shadow()
    if data:
        temp, hum, time = extract_temp_humidity(data)
        if temp is not None:
            save_to_db(temp, hum, time)
            print(f"✅ 存入成功：温度 {temp}°C，湿度 {hum}% 时间 {time}")
        else:
            print("❌ 未找到温湿度数据")
    else:
        print("❌ 获取数据失败")
