from flask import Flask, request, jsonify
import sqlite3
import datetime

app = Flask(__name__)

@app.route('/upload-data', methods=['POST'])
def upload_data():
    try:
        data = request.get_json() # 获取 POST 的 JSON 数据
        temperature = data.get('temperature') # 从 JSON 中提取字段
        humidity = data.get('humidity')
        timestamp = datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S') 
        # 自动添加时间戳

        # 写入数据库
        conn = sqlite3.connect('potato_sensor.db') # 连接本地数据库文件
        cursor = conn.cursor()
        cursor.execute( # 执行 SQL 写入操作
            'INSERT INTO potato_sensor (temperature, humidity, timestamp) VALUES (?, ?, ?)',
            (temperature, humidity, timestamp)
        )
        conn.commit() # 提交事务
        conn.close() # 关闭连接

        return jsonify({'message': '数据已成功上传并存储！'}), 200

    except Exception as e:
        return jsonify({'error': str(e)}), 500

@app.route('/get-data', methods=['GET'])
def get_data():
    try:
        conn = sqlite3.connect('potato_sensor.db')
        cursor = conn.cursor()

        cursor.execute('SELECT id, temperature, humidity, timestamp FROM potato_sensor ORDER BY id DESC')
        # 从数据库读取所有数据，最新数据排在前面
        rows = cursor.fetchall() # 每行转成字典，加入列表
        conn.close()

        data = []
        for row in rows:
            data.append({
                'id': row[0],
                'temperature': row[1],
                'humidity': row[2],
                'timestamp': row[3]
            })

        return jsonify(data), 200 # 返回 JSON 响应

    except Exception as e:
        return jsonify({'error': str(e)}), 500

if __name__ == '__main__':
    app.run(debug=True)
