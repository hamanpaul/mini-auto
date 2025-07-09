# 後端功能性驗證測試計畫 (Backend Functional Validation Test Plan)

本測試計畫旨在驗證 Miniauto 專案後端 FastAPI 服務的各項功能是否符合預期。

## 測試環境設定
- 確保 FastAPI 服務已在 `http://127.0.0.1:8000` 運行。
- 使用 `curl` 命令進行 API 呼叫。

## 測試項目、流程與預期結果

### 1. POST /api/sync

#### 測試項目
- 驗證車輛狀態數據的同步功能。
- 驗證後端根據控制模式返回正確的指令。

#### 測試流程
1. **同步帶有熱成像數據的狀態**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/sync \
          -H "Content-Type: application/json" \
          -d '{"s": 79, "v": 785, "t": [[2550, 2600], [2575, 2625]], "i": "192.168.1.200"}'
     ```
   - **預期結果**: 返回 JSON 格式的指令，例如 `{"c":0,"m":0,"d":0,"a":0}` (具體值取決於當前控制模式的預設行為)。

2. **同步不帶熱成像數據的狀態**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/sync \
          -H "Content-Type: application/json" \
          -d '{"s": 78, "v": 750}'
     ```
   - **預期結果**: 返回 JSON 格式的指令，例如 `{"c":0,"m":0,"d":0,"a":0}`。

### 2. POST /api/register_camera

#### 測試項目
- 驗證 ESP32-S3 視覺模組 IP 地址的註冊功能。

#### 測試流程
1. **註冊攝影機 IP**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/register_camera \
          -H "Content-Type: application/json" \
          -d '{"i": "192.168.1.100"}'
     ```
   - **預期結果**: 返回 `{"message":"ESP32-S3 IP registered successfully"}`。

### 3. POST /api/manual_control

#### 測試項目
- 驗證手動控制指令的設定功能。

#### 測試流程
1. **設定手動控制指令**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/manual_control \
          -H "Content-Type: application/json" \
          -d '{"m": 50, "d": 90, "a": 45, "c": 1}'
     ```
   - **預期結果**: 返回 `{"message":"Manual control command updated"}`。

### 4. POST /api/set_control_mode

#### 測試項目
- 驗證車輛控制模式的切換功能。

#### 測試流程
1. **切換至避障模式**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/set_control_mode \
          -H "Content-Type: application/json" \
          -d '{"mode": "avoidance"}'
     ```
   - **預期結果**: 返回 `{"message":"Control mode set to avoidance"}`。

2. **切換至手動模式**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/set_control_mode \
          -H "Content-Type: application/json" \
          -d '{"mode": "manual"}'
     ```
   - **預期結果**: 返回 `{"message":"Control mode set to manual"}`。

### 5. GET /api/latest_data

#### 測試項目
- 驗證後端儲存的最新數據和指令的獲取功能。

#### 測試流程
1. **準備數據 (設定手動控制和模式，並同步一次)**
   - **命令**:
     ```bash
     curl -X POST http://127.0.0.1:8000/api/manual_control \
          -H "Content-Type: application/json" \
          -d '{"m": 60, "d": 180, "a": 70, "c": 2}'
     curl -X POST http://127.0.0.1:8000/api/set_control_mode \
          -H "Content-Type: application/json" \
          -d '{"mode": "autonomous"}'
     curl -X POST http://127.0.0.1:8000/api/register_camera \
          -H "Content-Type: application/json" \
          -d '{"i": "192.168.1.200"}'
     curl -X POST http://127.0.0.1:8000/api/sync \
          -H "Content-Type: application/json" \
          -d '{"s": 100, "v": 800, "t": [[100, 200], [300, 400]], "i": "192.168.1.200"}'
     ```
   - **預期結果**: 各個 API 呼叫返回成功訊息。

2. **獲取最新數據**
   - **命令**:
     ```bash
     curl -X GET http://127.0.0.1:8000/api/latest_data
     ```
   - **預期結果**: 返回包含最新數據的 JSON 物件，例如：
     ```json
     {
       "latest_data": {"s": 100, "v": 800, "t": [[100, 200], [300, 400]], "i": "192.168.1.200"},
       "latest_command": {"c": 2, "m": 60, "d": 180, "a": 70},
       "esp32_cam_ip": "192.168.1.200",
       "current_control_mode": "autonomous",
       "thermal_analysis": {"max_temp": 4.0, "min_temp": 1.0, "avg_temp": 2.5, "hotspot_detected": false},
       "visual_analysis": null
     }
     ```

## 驗證方法
- 觀察 `curl` 命令的輸出，與「預期結果」進行比對。
- 檢查 FastAPI 服務的控制台輸出 (如果運行在前景或查看 `server.log`)，確認是否有錯誤或異常訊息。

```