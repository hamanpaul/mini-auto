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
- **說明 (Description):** 取得前端 GUI 頁面 (Get the frontend GUI page).
- **Browser URL:** `http://localhost:8000/`

---

### 啟動/停止相機串流處理器 (Start/Stop Camera Stream Processor)
- **說明 (Description):** 啟動或停止後端相機串流處理器。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/camera/start
  curl -X POST http://localhost:8000/camera/stop
  ```

---

### 獲取相機串流處理器狀態 (Get Camera Stream Processor Status)
- **說明 (Description):** 獲取後端相機串流處理器的運行狀態。
- **Browser URL:** `http://localhost:8000/camera/status`

---

### 獲取 MJPEG 串流 (Get MJPEG Stream)
- **說明 (Description):** 從後端串流經視覺分析處理後的 MJPEG 影像。
- **Browser URL:** `http://localhost:8000/camera/stream`

---

### 同步車輛狀態 (Sync Vehicle State)
- **說明 (Description):** **(主要由 ESP32 使用)** 向後端同步車輛狀態數據，並接收控制指令。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/sync \
       -H "Content-Type: application/json" \
       -d '{"s": 1, "v": 785, "t": [], "u": 50}'
  ```

---

### 註冊攝影機 IP (Register Camera IP)
- **說明 (Description):** **(主要由 ESP32 使用)** 向後端註冊自身的 IP 地址。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/register_camera \
       -H "Content-Type: application/json" \
       -d '{"i": "192.168.1.101"}'
  ```

---

### 設定手動控制指令 (Set Manual Control Commands)
- **說明 (Description):** **(主要由 GUI 使用)** 設定車輛的手動控制指令。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/manual_control \
       -H "Content-Type: application/json" \
       -d '{"m": 60, "d": 45, "a": 120, "c": 2}'
  ```

---

### 切換控制模式 (Switch Control Mode)
- **說明 (Description):** **(主要由 GUI 使用)** 切換車輛的控制模式。
- **curl:**
  ```bash
  curl -X POST http://localhost:8000/api/set_control_mode \
       -H "Content-Type: application/json" \
       -d '{"mode": "avoidance"}'
  ```

---

### 獲取最新數據 (Get Latest Data)
- **說明 (Description):** **(主要由 GUI 使用)** 獲取後端儲存的最新車輛數據、指令、IP 和模式。
- **Browser URL:** `http://localhost:8000/api/latest_data`

---

### 獲取後端日誌 (Get Backend Logs)
- **說明 (Description):** **(主要由 GUI 使用)** 獲取後端服務的運行日誌。
- **Browser URL:** `http://localhost:8000/api/logs`

