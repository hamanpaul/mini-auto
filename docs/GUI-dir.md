# GUI 控制與影像分析實現原理

本文件詳細解釋了 Miniauto 專案中，使用者如何透過網頁介面 (GUI) 控制車輛移動，以及如何處理和顯示來自攝影機的影像與分析結果。整個流程橫跨前端、後端和 Arduino 硬體，形成一個完整的閉環控制與感知系統。

## 總覽

Miniauto 的 GUI 系統遵循一個「**前端發送控制、後端中繼指令、硬體拉取執行**」的控制模式，並結合了「**前端直接串流、後端分析提供結果**」的影像處理模式：

1.  **前端 (Frontend)**：
    *   捕捉使用者的鍵盤輸入，並將其轉換為控制指令，透過 HTTP API 發送給後端。
    *   直接連接並顯示來自 ESP32-S3 攝影機的 MJPEG 串流。
    *   定期向後端請求最新的影像分析結果。
2.  **後端 (Backend)**：
    *   接收前端的控制指令，將其儲存在一個全域狀態變數中，等待硬體拉取。
    *   運行一個獨立的攝影機串流處理器，負責從 ESP32-S3 攝影機拉取影像串流，進行即時影像分析（例如障礙物檢測），並儲存分析結果。
    *   提供 API 端點供前端獲取最新的影像分析結果。
3.  **硬體 (Arduino)**：
    *   以固定的時間間隔，主動向後端發送請求，拉取最新的控制指令，並執行它。
    *   （ESP32-S3 攝影機）負責生成 MJPEG 影像串流，供前端和後端處理器直接連接。

---

### 1. 前端 (`index.html`)：事件捕捉、指令發送與影像顯示

前端的職責是提供使用者介面，並將使用者的操作即時轉換為 API 請求，同時負責顯示影像串流和分析結果。

-   **事件監聽**: 頁面載入後，Vue.js 應用程式會為整個視窗註冊 `keydown` (按鍵按下) 和 `keyup` (按鍵放開) 的事件監聽器。
-   **狀態追蹤**: 一個名為 `activeKeys` 的物件被用來追蹤當前有哪些按鍵正被按住。
-   **指令生成**: `updateMotorControl` 函式會檢查 `activeKeys` 的狀態，並根據被按下的方向鍵（`w`, `a`, `s`, `d`）來設定馬達速度 (`motorSpeed`) 和方向角度 (`directionAngle`)。
-   **API 呼叫 (控制)**: 只有當控制狀態（速度或方向）發生改變時，`sendManualControl` 函式才會被觸發。它使用 `fetch` API 向後端的 `/api/manual_control` 端點發送一個 `POST` 請求，請求的 body 中包含了 JSON 格式的控制指令。
-   **影像串流顯示**: 前端直接透過 `<img>` 標籤的 `src` 屬性連接到 ESP32-S3 攝影機的 MJPEG 串流 URL (例如 `http://<ESP32_CAM_IP>:81/stream`)。這意味著影像資料流不經過 Python 後端，減輕了後端的負擔。
-   **影像分析結果獲取**: 前端會定期向後端的 `/api/camera/analysis` 端點發送請求，獲取最新的影像分析結果（例如障礙物檢測狀態、位置等），並可以在介面上進行顯示或處理。

```javascript
// in index.html

// ...
methods: {
    handleKeyDown(event) {
        if (event.repeat) return; // Ignore key repeat
        const key = event.key.toLowerCase(); // 使用小寫鍵名
        this.activeKeys[key] = true;
        this.updateMotorControl();
    },
    handleKeyUp(event) {
        const key = event.key.toLowerCase();
        this.activeKeys[key] = false;
        this.updateMotorControl();
    },
    updateMotorControl() {
        let newMotorSpeed = 0;
        let newDirectionAngle = 0;

        if (this.activeKeys['w']) {
            newMotorSpeed = this.speedSliderValue;
            newDirectionAngle = 0; // Forward
        } else if (this.activeKeys['s']) {
            newMotorSpeed = this.speedSliderValue;
            newDirectionAngle = 180; // Backward
        } else if (this.activeKeys['a']) {
            newMotorSpeed = this.speedSliderValue;
            newDirectionAngle = 270; // Turn Left
        } else if (this.activeKeys['d']) {
            newMotorSpeed = this.speedSliderValue;
            newDirectionAngle = 90; // Turn Right
        }

        // Only send if control state changes
        if (newMotorSpeed !== this.motorSpeed || newDirectionAngle !== this.directionAngle) {
            this.motorSpeed = newMotorSpeed;
            this.directionAngle = newDirectionAngle;
            this.sendManualControl();
        }
    },
    async sendManualControl() {
        // ... 向 /api/manual_control 發送 POST 請求 ...
    },
    toggleStream() {
        // 控制影像串流的啟動/停止，直接修改 img 標籤的 src
        if (this.streamUrl) {
            this.streamUrl = ''; // 停止串流
        } else {
            this.streamUrl = `http://${this.esp32CamIp}:81/stream`; // 啟動串流
        }
    },
    async fetchAnalysisResults() {
        // 定期向 /api/camera/analysis 請求分析結果
        const response = await fetch('/api/camera/analysis');
        const data = await response.json();
        // ... 處理並顯示分析結果 ...
    }
}
// ...
```

---

### 2. 後端 (`main.py`, `vehicle_api.py`, `camera.py`, `camera_stream_processor.py`)：指令中繼與影像分析

後端作為一個中介層，負責維護車輛的狀態，包括當前的控制模式和手動控制指令，同時也負責處理來自攝影機的影像並提供分析結果。

#### 2.1 控制指令中繼 (`vehicle_api.py`)

-   **API 端點**: 使用 FastAPI 框架，`@router.post("/api/manual_control")` 裝飾器定義了一個端點來接收前端的請求。
-   **資料驗證**: `ManualControlRequest` Pydantic 模型確保了傳入的資料符合預期的格式。
-   **狀態更新**: `manual_control` 函式被執行時，它會從請求中解析出數據，並將這些值更新到**全域變數**中，例如 `current_manual_motor_speed` 和 `current_manual_direction_angle`。

**關鍵點**：這個端點只負責「儲存」最新的手動指令，它並不會主動將這個指令推送給 Arduino。

```python
# in src/py_rear/apis/vehicle_api.py

# Global variables for current manual control commands
current_manual_motor_speed: int = 0
current_manual_direction_angle: int = 0
# ...

class ManualControlRequest(BaseModel):
    m: int  # motor_speed
    d: int  # direction_angle
    a: int  # servo_angle
    c: int  # command_byte

@router.post("/api/manual_control")
async def manual_control(request: ManualControlRequest):
    global current_manual_motor_speed, current_manual_direction_angle, ...

    # 更新全域變數
    current_manual_motor_speed = request.m
    current_manual_direction_angle = request.d
    # ...
    return {"message": "Manual control command received"}
```

#### 2.2 攝影機串流處理與影像分析 (`camera.py`, `camera_stream_processor.py`)

這部分是後端處理影像的核心。

-   **`CameraStreamProcessor` 類別 (`camera_stream_processor.py`)**:
    *   **職責**: 負責連接到 ESP32-S3 攝影機的 MJPEG 串流 (`http://<IP>:81/stream`)，並在一個獨立的執行緒中持續讀取影像幀。
    *   **串流讀取**: 使用 `cv2.VideoCapture` 來處理 MJPEG 串流的連接、讀取和解碼，這比手動解析 MJPEG 邊界更為高效和穩健。
    *   **影像分析**: 在 `_process_frame` 方法中，對每個讀取到的影像幀執行 OpenCV 影像處理和分析（例如，透過亮度閾值和輪廓檢測進行障礙物檢測）。
    *   **結果儲存**: 將最新的原始影像幀（JPEG 格式）和影像分析結果儲存在內部變數中，並透過執行緒鎖 (`threading.Lock`) 確保多執行緒存取安全。
    *   **生命週期管理**: 提供 `start()` 和 `stop()` 方法來控制串流處理執行緒的啟動和停止。

-   **攝影機 API (`camera.py`)**:
    *   **職責**: 提供 FastAPI API 端點，用於控制 `CameraStreamProcessor` 的生命週期，並向前端暴露影像分析結果。
    *   **`camera_processor` 全域實例**: 在 `main.py` 啟動時，會創建 `CameraStreamProcessor` 的實例並賦值給 `camera.py` 中的 `camera_processor` 全域變數，確保整個應用程式共享同一個處理器。
    *   **API 端點**:
        *   `/camera/start`: 啟動攝影機串流處理器。
        *   `/camera/stop`: 停止攝影機串流處理器。
        *   `/camera/status`: 查詢攝影機串流處理器的運行狀態。
        *   `/camera/analysis`: **核心端點**，返回 `CameraStreamProcessor` 中最新的影像分析結果。這個端點取代了直接提供影像串流的舊有設計，將後端職責從影像代理轉變為影像分析結果提供者。

**架構轉變原因總結**:
*   **前端直接消費 ESP32-S3 串流**: 減輕了 Python 後端的影像代理負擔，提高了效率。
*   **後端專注於分析**: Python 後端現在主要專注於對影像進行深度處理和分析，提供高價值的結構化數據。
*   **簡化與穩健性**: 使用 `cv2.VideoCapture` 簡化了串流處理邏輯，提高了系統的穩健性。

---

### 3. Arduino (`arduino_uno.ino`)：指令的拉取與執行

Arduino 作為最終的執行者，它採用「**主動拉取 (Polling)**」的模式從後端獲取指令。

-   **定時輪詢**: 在主 `loop()` 函式中，一個非阻塞的計時器會每隔 200 毫秒呼叫一次 `syncWithServer()` 函式。
-   **發送同步請求**: `syncWithServer()` 會呼叫 `httpPost()`，向後端的 `/api/sync` 端點發送一個 `POST` 請求。這個請求的 body 中包含了 Arduino 感測器的最新狀態資料。
-   **後端回應**: 後端的 `sync_data` 函式在收到請求後，會檢查 `current_control_mode`。在手動模式下，它會讀取之前儲存的全域變數，並將這些指令作為對 `/api/sync` 請求的回應，發送回給 Arduino。
-   **解析與執行**: Arduino 接收到回應後，使用一個輕量級的手動 JSON 解析函式 `get_json_int_value()` 來提取指令值（`m`, `d`, `a`, `c`）。最後，它呼叫 `Velocity_Controller()` 等函式，將這些指令轉換為具體的硬體動作。

```c++
// in src/miniauto/arduino_uno/arduino_uno.ino

void loop() {
  unsigned long currentTime = millis();
  // 每 200ms 執行一次
  if (currentTime - lastCommandPollTime >= commandPollInterval) {
    syncWithServer();
    lastCommandPollTime = currentTime;
  }
}

void syncWithServer() {
  // 1. httpPost 會向 /api/sync 發送請求
  if (httpPost("/api/sync", http_response_buffer, sizeof(http_response_buffer))) {
    
    // 2. 手動解析 JSON 回應
    int motor_speed = get_json_int_value(http_response_buffer, ""m":");
    int direction_angle = get_json_int_value(http_response_buffer, ""d":");
    // ...

    // 3. 執行指令
    Velocity_Controller(direction_angle, motor_speed, 0);
    myservo.write(servo_angle);
    // ...
  }
}
```

---

### 4. Arduino 馬達控制 (`Velocity_Controller`)

這個函式是移動控制的核心，它將抽象的方向和速度指令轉換為四個馬達的具體動作。

-   **運動學模型**: 該函式內建了麥克納姆輪 (Mecanum wheel) 的運動學模型。
-   **三角函數分解**: 它使用 `sin()` 和 `cos()` 三角函數，將單一的速度向量分解到四個呈 45 度角安裝的輪子上。
-   **獨立控制**: 計算結果為四個獨立的馬達速度值（`velocity_0` 到 `velocity_3`），其中包含了正負號（代表正轉或反轉）。
-   **硬體輸出**: `Motors_Set` 函式接收這些值，並設定每個馬達的方向接腳和 PWM 責任週期，從而精確控制車輛的移動。

```c++
// in src/miniauto/arduino_uno/arduino_uno.ino

void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot) {
  int8_t velocity_0, velocity_1, velocity_2, velocity_3;
  angle += 90; // 校準座標系
  float rad = angle * PI / 180;
  
  // ... 速度正規化 ...

  // 核心運動學計算
  velocity_0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot * speed;
  velocity_1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot * speed;
  velocity_2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot * speed;
  velocity_3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot * speed;
  
  Motors_Set(velocity_0, velocity_1, velocity_2, velocity_3);
}
```