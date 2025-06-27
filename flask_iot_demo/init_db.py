import sqlite3

# 创建/连接数据库（会自动创建 potato_sensor.db 文件）
conn = sqlite3.connect('potato_sensor.db')
cursor = conn.cursor()

# 创建表，避免重复创建所以加了 IF NOT EXISTS
cursor.execute('''
CREATE TABLE IF NOT EXISTS potato_sensor (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    temperature REAL,
    humidity REAL,
    timestamp TEXT
)
''')

conn.commit()
conn.close()

print("✅ 数据库和表创建成功！")
