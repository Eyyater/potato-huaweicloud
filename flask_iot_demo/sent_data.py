import requests

url = 'http://127.0.0.1:5000/upload-data'
data = {
    'temperature': 23.5,
    'humidity': 60.2
}
response = requests.post(url, json=data)
print(response.json())
