# 手動 API 測試指令
# Manual API Testing Commands

## 1. 啟動伺服器 (Start the Server)

在終端機中，進入專案根目錄 `/home/haman/mini-auto`，然後執行以下命令：
In your terminal, navigate to the project root directory `/home/haman/mini-auto` and run the following command:

```bash
python main.py
```

伺服器將會運行在 `http://localhost:8000`。
The server will be running at `http://localhost:8000`.

## 2. 測試指令 (Testing Commands)

在伺服器運行的狀態下，您可以使用 `curl` 或瀏覽器來測試以下端點。
While the server is running, you can use `curl` or a web browser to test the following endpoints.

---

### 根目錄 (Root)
- **說明 (Description):** 取得歡迎訊息 (Get the welcome message).
- **curl:**
  ```bash
  curl http://localhost:8000/
  ```
- **Browser URL:** `http://localhost:8000/`

---

### 啟動相機串流處理器 (Start Camera Stream Processor)
- **說明 (Description):** 啟動後端相機串流處理器，開始從 ESP32-CAM 獲取影像。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/camera/start
  ```

---

### 停止相機串流處理器 (Stop Camera Stream Processor)
- **說明 (Description):** 停止後端相機串流處理器。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/camera/stop

---

### 獲取相機串流處理器狀態 (Get Camera Stream Processor Status)
- **說明 (Description):** 獲取後端相機串流處理器的運行狀態。
- **curl:**
  ```bash
  curl http://localhost:8000/camera/status
  ```
- **Browser URL:** `http://localhost:8000/camera/status`

---

### 獲取最新影像分析結果 (Get Latest Image Analysis Results)
- **說明 (Description):** 獲取最新處理過的影像分析結果。
- **curl:**
  ```bash
  curl http://localhost:8000/camera/analysis
  ```
- **Browser URL:** `http://localhost:8000/camera/analysis`

---

### 獲取 MJPEG 串流 (Get MJPEG Stream)
- **說明 (Description):** 從後端串流 MJPEG 影像，該影像已由後端進行視覺分析處理。
- **Browser URL:** `http://localhost:8000/camera/stream`

---

### 同步車輛狀態 (Sync Vehicle State)
- **說明 (Description):** ESP32 向後端同步車輛狀態數據，並接收後端發送的控制指令。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/sync 
       -H "Content-Type: application/json" 
       -d '{"s": 1, "v": 785, "t": [[2500, 2510, 2520, 2530, 2540, 2550, 2560, 2570], [2600, 2610, 2620, 2630, 2640, 2650, 2660, 2670], [2700, 2710, 2720, 2730, 2740, 2750, 2760, 2770], [2800, 2810, 2820, 2830, 2840, 2850, 2860, 2870], [2900, 2910, 2920, 2930, 2940, 2950, 2960, 2970], [3000, 3010, 3020, 3030, 3040, 3050, 3060, 3070], [3100, 3110, 3120, 3130, 3140, 3150, 3160, 3170], [3200, 3210, 3220, 3230, 3240, 3250, 3260, 3270]], "i": "192.168.1.100", "u": 50}'
  ```

---

### 註冊攝影機 IP (Register Camera IP)
- **說明 (Description):** ESP32 向後端註冊自身的 IP 地址。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/register_camera 
       -H "Content-Type: application/json" 
       -d '{"i": "192.168.1.101"}'
  ```

---

### 設定手動控制指令 (Set Manual Control Commands)
- **說明 (Description):** 設定車輛的手動控制指令 (馬達速度、方向、舵機角度、指令字節)。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/manual_control 
       -H "Content-Type: application/json" 
       -d '{"m": 60, "d": 45, "a": 120, "c": 2}'
  ```

---

### 切換控制模式 (Switch Control Mode)
- **說明 (Description):** 切換車輛的控制模式 (手動、避障、自主)。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/set_control_mode 
       -H "Content-Type: application/json" 
       -d '{"mode": "avoidance"}'
  ```

---

### 獲取最新數據 (Get Latest Data)
- **說明 (Description):** 獲取後端儲存的最新車輛數據、最新發送的指令、ESP32-S3 IP 和當前控制模式。
- **curl:**
  ```bash
  curl http://localhost:8000/api/latest_data
  ```

---

### 獲取後端日誌 (Get Backend Logs)
- **說明 (Description):** 獲取後端服務的運行日誌。
- **curl:**
  ```bash
  curl http://localhost:8000/api/logs
  ```

---

### 簡單移動控制測試 (Simple Movement Control Test)
- **說明 (Description):** 提供簡單的移動控制測試端點，用於直接觸發車輛的移動動作。
- **curl:**
  ```bash
  curl http://localhost:8000/forward
  curl http://localhost:8000/backward
  curl http://localhost:8000/turn_left
  curl http://localhost:8000/turn_right
  ```
- **Browser URL:** `http://localhost:8000/forward` (and similar for others)

---

echo "
--- API Testing Complete ---"