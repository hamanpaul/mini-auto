#!/bin/bash

# 測試 POST /api/register_camera 端點
# 模擬 Arduino 發送 ESP32-S3 的 IP 位址

# 伺服器地址
SERVER_URL="http://127.0.0.1:8000"

# 模擬的 ESP32-S3 IP 位址
ESP32_IP="192.168.1.105"

# 構建 JSON 酬載
JSON_PAYLOAD='{
  "i": "'$ESP32_IP'"
}'

echo "發送 POST 請求到 $SERVER_URL/api/register_camera"
echo "酬載: $JSON_PAYLOAD"

curl -X POST \
     -H "Content-Type: application/json" \
     -d "$JSON_PAYLOAD" \
     $SERVER_URL/api/register_camera

echo "\n"

