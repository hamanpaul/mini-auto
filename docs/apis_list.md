# API 文件

本文件描述了 Miniauto 專案後端 FastAPI 服務提供的 API 端點及其功能。

---

## 車輛通訊 API (`/api`)

### `POST /api/sync`

*   **描述**: ESP32 定期呼叫此端點，上傳從 Arduino 收集的感測器數據，並獲取後端計算出的最新控制指令。
*   **請求 Body**:
    ```json
    {
      "s": 1,
      "v": 7850,
      "u": 50,
      "t_max": 3150,
      "t_min": 2800,
      "t_hx": 4,
      "t_hy": 5
    }
    ```
*   **參數說明 (Request)**:
    | 參數 | 類型 | 說明 |
    |---|---|---|
    | `s` | Integer | **狀態位元組 (status_byte)**：一個 8 位元整數，表示車輛的各種狀態。 |
    | `v` | Integer | 電池電壓 (voltage_mv)，單位毫伏。 |
    | `u` | Integer | (可選) 超音波距離 (ultrasonic_distance_cm)。 |
    | `t_max` | Integer | (可選) 熱像儀偵測到的最高溫度，乘以 100。 |
    | `t_min` | Integer | (可選) 熱像儀偵測到的最低溫度，乘以 100。 |
    | `t_hx` | Integer | (可選) 最熱點的 X 座標 (0-7)。 |
    | `t_hy` | Integer | (可選) 最熱點的 Y 座標 (0-7)。 |
*   **成功回應 (Response)**:
    ```json
    {
      "c": 1,
      "m": 50,
      "d": 90,
      "a": 90,
      "r": 0,
      "is_avoidance_enabled": 1
    }
    ```
*   **參數說明 (Response)**:
    | 參數 | 類型 | 說明 |
    |---|---|---|
    | `c` | Integer | **指令位元組 (command_byte)**：控制蜂鳴器和 LED。 |
    | `m` | Integer | 馬達速度 (motor_speed)。 |
    | `d` | Integer | 方向角度 (direction_angle)。 |
    | `a` | Integer | 舵機角度 (servo_angle)。 |
    | `r` | Integer | 旋轉速度 (rotation_speed)。 |
    | `is_avoidance_enabled` | Integer | 是否啟用 Arduino 上的硬體避障 (0 或 1)。 |

---

### `POST /api/register_camera`

*   **描述**: ESP32-CAM 在啟動並連接到 Wi-Fi 後，呼叫此端點向後端註冊其 IP 位址。
*   **請求 Body**:
    ```json
    {
      "i": "192.168.1.101"
    }
    ```
*   **成功回應**:
    ```json
    {
      "message": "ESP32-S3 IP 註冊成功"
    }
    ```

---

### `POST /api/manual_control`

*   **描述**: 從 GUI 接收使用者的手動控制指令，並將其儲存在後端狀態變數中。
*   **請求 Body**:
    ```json
    {
      "m": 60,
      "d": 45,
      "a": 120,
      "c": 2,
      "r": 15
    }
    ```
*   **成功回應**:
    ```json
    {
      "message": "手動控制命令已接收"
    }
    ```

---

### `POST /api/set_control_mode`

*   **描述**: 從 GUI 切換車輛的控制模式。
*   **請求 Body**:
    ```json
    {
      "mode": "avoidance"
    }
    ```
*   **參數說明 (Request)**:
    | 參數 | 類型 | 說明 |
    |---|---|---|
    | `mode` | String | 控制模式，可選值為 `"manual"`, `"avoidance"`, `"autonomous"`。 |
*   **成功回應**:
    ```json
    {
      "message": "控制模式已設定為 avoidance"
    }
    ```

---

### `GET /api/latest_data`

*   **描述**: 從 GUI 獲取後端儲存的所有最新數據，用於更新儀表板。
*   **成功回應**:
    ```json
    {
      "latest_data": { ... },
      "latest_command": { ... },
      "esp32_cam_ip": "192.168.1.101",
      "current_control_mode": "manual",
      "thermal_analysis": { ... }, 
      "visual_analysis": { ... }
    }
    ```

---

### `GET /api/logs`

*   **描述**: 獲取後端的日誌緩衝區內容。
*   **成功回應**:
    ```json
    {
      "logs": [
        "[2023-10-27 10:00:00] [INFO] [Backend] Server started",
        "[2023-10-27 10:00:05] [INFO] [Backend] ESP32-S3 IP registered: 192.168.1.101"
      ]
    }
    ```

---

## 攝影機 API (`/camera`)

### `POST /camera/start`
*   **描述**: 啟動後端的影像串流處理器，使其開始從 ESP32-CAM 拉取影像。
*   **成功回應**: `{"message": "Camera stream processor started.", "status": "starting"}`

### `POST /camera/stop`
*   **描述**: 停止後端的影像串流處理器。
*   **成功回應**: `{"message": "Camera stream processor stopped.", "status": "stopping"}`

### `GET /camera/status`
*   **描述**: 獲取影像串流處理器的目前狀態（`running` 或 `stopped`）和 FPS。
*   **成功回應**: `{"status": "running", "fps": 25.5}`

### `GET /camera/analysis`
*   **描述**: 獲取最新的視覺分析結果。
*   **成功回應**:
    ```json
    {
        "obstacle_detected": true,
        "obstacle_center_x": 240,
        "obstacle_area_ratio": 0.15
    }
    ```

### `GET /camera/stream`
*   **描述**: 提供給前端的 MJPEG 影像串流端點。瀏覽器中的 `<img>` 標籤會直接指向此路徑。
*   **回應**: 一個 `multipart/x-mixed-replace` 格式的 HTTP 串流響應。
