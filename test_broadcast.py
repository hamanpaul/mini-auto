import requests
import json

# 您的 ngrok 通道 URL，請確保這是您目前正在使用的正確 URL
NGROK_URL = "https://1a7e1ee33fc7.ngrok-free.app"  # <<< 請替換成您自己的 ngrok URL

# 要廣播的訊息
message_to_broadcast = "這是一則來自測試腳本的廣播訊息！"

# API 端點的完整 URL
broadcast_url = f"{NGROK_URL}/broadcast"

# 請求的標頭
headers = {
    "Content-Type": "application/json"
}

# 請求的內容
payload = {
    "message": message_to_broadcast
}

try:
    # 發送 POST 請求
    response = requests.post(broadcast_url, headers=headers, data=json.dumps(payload))

    # 檢查回應
    if response.status_code == 200:
        print("廣播請求成功！")
        print("回應內容:", response.json())
    else:
        print(f"廣播請求失敗，狀態碼: {response.status_code}")
        print("錯誤訊息:", response.text)

except requests.exceptions.RequestException as e:
    print(f"請求發生錯誤: {e}")

