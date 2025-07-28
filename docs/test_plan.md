# 後端功能性驗證測試計畫 (重構後)

本測試計畫旨在驗證 Miniauto 專案後端 FastAPI 服務在架構重構後的各項功能是否符合預期。

## 測試環境設定
- 確保 FastAPI 服務已在 `http://127.0.0.1:8000` 運行。
- 使用 `curl` 命令進行 API 呼叫。

## 測試項目、流程與預期結果

### 1. POST /api/sync

- **測試項目**: 驗證 ESP32 發送的車輛狀態數據的同步功能。
- **測試流程**: 模擬 ESP32 發送一次同步請求。
  ```bash
  curl -X POST http://127.0.0.1:8000/api/sync \
       -H "Content-Type: application/json" \
       -d '{"s": 79, "v": 785, "t": [], "u": 50}'
  ```
- **預期結果**: 返回 JSON 格式的指令，例如 `{"c":0,"m":0,"d":0,"a":90}`。

### 2. POST /api/register_camera

- **測試項目**: 驗證 ESP32-CAM 的 IP 地址註冊功能。
- **測試流程**: 模擬 ESP32 註冊其 IP。
  ```bash
  curl -X POST http://127.0.0.1:8000/api/register_camera \
       -H "Content-Type: application/json" \
       -d '{"i": "192.168.1.100"}'
  ```
- **預期結果**: 返回 `{"message":"Camera IP registered successfully."}`。

### 3. POST /api/manual_control

- **測試項目**: 驗證 GUI 設定手動控制指令的功能。
- **測試流程**: 模擬 GUI 發送手動控制指令。
  ```bash
  curl -X POST http://127.0.0.1:8000/api/manual_control \
       -H "Content-Type: application/json" \
       -d '{"m": 50, "d": 90, "a": 45, "c": 1}'
  ```
- **預期結果**: 返回 `{"message":"Manual control command received"}`。

### 4. POST /api/set_control_mode

- **測試項目**: 驗證 GUI 切換車輛控制模式的功能。
- **測試流程**: 模擬 GUI 切換到避障模式。
  ```bash
  curl -X POST http://127.0.0.1:8000/api/set_control_mode \
       -H "Content-Type: application/json" \
       -d '{"mode": "avoidance"}'
  ```
- **預期結果**: 返回 `{"message":"Control mode set to avoidance"}`。

### 5. GET /api/latest_data

- **測試項目**: 驗證 GUI 獲取後端儲存的最新數據和指令的功能。
- **測試流程**: 在執行其他請求後，獲取最新數據。
  ```bash
  curl -X GET http://127.0.0.1:8000/api/latest_data
  ```
- **預期結果**: 返回包含最新數據的 JSON 物件。

```