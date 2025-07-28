# GUI 控制與 API 互動原理

本文件詳細解釋了 Miniauto 專案中，使用者如何透過網頁介面 (GUI) 控制車輛移動，以及 GUI 如何與後端 API 進行互動以顯示影像和獲取分析結果。整個流程橫跨前端、後端和 Arduino 硬體，形成一個完整的閉環控制與感知系統。

## 總覽

Miniauto 的 GUI 系統遵循一個「**前端發送控制、後端中繼指令、硬體拉取執行**」的控制模式，並結合了「**前端透過後端 API 獲取串流與分析結果**」的影像顯示模式：

1.  **前端 (Frontend)**：
    *   捕捉使用者的鍵盤輸入，並將其轉換為控制指令，透過 HTTP API 發送給後端。
    *   透過後端提供的 API (`/api/camera/stream`) 連接並顯示來自攝影機的 MJPEG 串流。
    *   定期向後端請求最新的影像分析結果 (`/api/camera/analysis`)。
2.  **後端 (Backend)**：
    *   接收前端的控制指令，將其儲存在一個全域狀態變數中，等待硬體拉取。
    *   作為攝影機影像串流的代理，從 ESP32-S3 攝影機拉取影像，進行處理和分析，並將處理後的影像串流和分析結果透過 API 提供給前端。
    *   在啟動時，可選擇性地透過 UDP 廣播自己的 IP 位址，供 Arduino UNO 進行服務發現。
3.  **硬體 (Arduino)**：
    *   透過監聽 UDP 廣播來動態發現後端伺服器的 IP 位址。
    *   以固定的時間間隔，主動向後端發送請求，拉取最新的控制指令，並執行它。
    *   （ESP32-S3 攝影機）負責生成 MJPEG 影像串流，供後端處理器直接連接。

---

### 1. 前端 (`index.html`)：事件捕捉、指令發送與影像顯示

前端的職責是提供使用者介面，並將使用者的操作即時轉換為 API 請求，同時負責顯示影像串流和分析結果。

-   **事件監聽**: 頁面載入後，Vue.js 應用程式會為整個視窗註冊 `keydown` (按鍵按下) 和 `keyup` (按鍵放開) 的事件監聽器。
-   **狀態追蹤**: 一個名為 `activeKeys` 的物件被用來追蹤當前有哪些按鍵正被按住。
-   **指令生成**: `updateMotorControl` 函式會檢查 `activeKeys` 的狀態，並根據被按下的方向鍵（`w`, `a`, `s`, `d`）來設定馬達速度 (`motorSpeed`) 和方向角度 (`directionAngle`)。
-   **API 呼叫 (控制)**: 只有當控制狀態（速度或方向）發生改變時，`sendManualControl` 函式才會被觸發。它使用 `fetch` API 向後端的 `/api/manual_control` 端點發送一個 `POST` 請求，請求的 body 中包含了 JSON 格式的控制指令。
-   **影像串流顯示**: 前端透過 `<img>` 標籤的 `src` 屬性連接到後端的 `/api/camera/stream` 端點。這意味著影像資料流會經過 Python 後端進行處理和轉發。前端會自動在後端偵測到 ESP32 IP 後啟動串流。
-   **影像分析結果獲取**: 前端會定期向後端的 `/api/camera/analysis` 端點發送請求，獲取最新的影像分析結果（例如障礙物檢測狀態、位置等），並可以在介面上進行顯示或處理。

```javascript
// in index.html

// ...
methods: {
    handleKeyDown(event) {
        // ... (省略日誌) ...
        if (event.repeat) return; // Ignore key repeat
        const key = event.key.toLowerCase(); // 使用小寫鍵名
        this.activeKeys[key] = true;
        this.updateMotorControl();
    },
    handleKeyUp(event) {
        // ... (省略日誌) ...
        const key = event.key.toLowerCase();
        this.activeKeys[key] = false;
        this.updateMotorControl();
    },
    updateMotorControl() {
        // ... (省略日誌) ...
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
    async toggleStream() {
        // 控制影像串流的啟動/停止，直接修改 img 標籤的 src
        if (this.streamUrl) {
            // 如果串流正在運行，停止它
            await fetch('/camera/stop', { method: 'POST' });
            this.streamUrl = ''; // 停止串流
            this.streamButtonText = 'Start Stream';
        } else {
            // 如果串流已停止，啟動它
            const response = await fetch('/camera/start', { method: 'POST' });
            const data = await response.json();
            if (response.ok) {
                this.streamUrl = '/camera/stream'; // 啟動串流，指向後端代理
                this.streamButtonText = 'Stop Stream';
            } else {
                throw new Error(data.detail || 'Failed to start stream');
            }
        }
    },
    async fetchAnalysisResults() {
        // 定期向 /api/camera/analysis 請求分析結果
        const response = await fetch('/api/camera/analysis');
        const data = await response.json();
        // ... 處理並顯示分析結果 ...
    },
    async fetchData() {
        // ... (省略其他邏輯) ...
        if (data.esp32_cam_ip && !this.streamUrl) {
            // 如果從後端獲取到 ESP32 IP 且串流尚未啟動，則自動啟動串流
            await this.toggleStream();
        }
    }
}
// ...
```

---

### 2. 後端 API 互動 (`main.py`, `vehicle_api.py`, `camera.py`)

後端作為一個中介層，負責維護車輛的狀態，包括當前的控制模式和手動控制指令，並透過 API 提供影像串流和分析結果。

#### 2.1 伺服器 IP 廣播 (Service Discovery)

為了讓 Arduino UNO 能夠動態發現後端伺服器的 IP 位址，後端引入了 UDP 廣播機制：

-   **`broadcast_server_ip.py` 腳本**: 這是一個獨立的 Python 腳本，負責向區域網路廣播後端伺服器的 IP 位址和埠號。它會每秒發送一次 UDP 封包到埠 5005，訊息格式為 `MINIAUTO_SERVER_IP:<server_ip>:8000`。
-   **`main.py` 中的生命週期管理**: `main.py` 在啟動時，可以選擇性地（透過命令列參數 `b_ip`）啟動 `broadcast_server_ip.py` 作為子進程。這個子進程的生命週期由 FastAPI 應用程式管理，並在應用程式關閉時終止。
-   **`vehicle_api.py` 中的停止機制**: 當 Arduino UNO 透過 `/api/register_camera` 端點成功註冊 ESP32-CAM 的 IP 位址後，`vehicle_api.py` 會觸發停止 IP 廣播的指令，避免不必要的網路流量。

#### 2.2 控制指令中繼 (`vehicle_api.py`)

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

#### 2.3 攝影機 API (`camera.py`)

這部分是後端提供給前端的 API 介面，用於影像串流和分析結果的獲取。

-   **職責**: 提供 FastAPI API 端點，用於控制 `CameraStreamProcessor` 的生命週期，並向前端暴露影像分析結果，同時**代理影像串流**。
-   **`camera_processor` 全域實例**: 在 `main.py` 啟動時，會創建 `CameraStreamProcessor` 的實例並賦值給 `camera.py` 中的 `camera_processor` 全域變數，確保整個應用程式共享同一個處理器。
-   **API 端點**:
    *   `/camera/start`: 啟動攝影機串流處理器。
    *   `/camera/stop`: 停止攝影機串流處理器。
    *   `/camera/status`: 查詢攝影機串流處理器的運行狀態。
    *   `/camera/analysis`: **核心端點**，返回 `CameraStreamProcessor` 中最新的影像分析結果。
    *   `/api/camera/stream`: **影像串流代理端點**，它會從 `CameraStreamProcessor` 獲取處理後的影像幀，並以 `multipart/x-mixed-replace` 格式轉發給前端，實現 MJPEG 串流。

**架構轉變原因總結**:
*   **後端統一影像來源**: 所有影像現在都透過後端代理，方便集中處理、分析和轉發。
*   **後端專注於分析與代理**: Python 後端現在主要專注於對影像進行深度處理和分析，並作為影像串流的可靠代理。
*   **簡化與穩健性**: 使用 `cv2.VideoCapture` 簡化了串流處理邏輯，提高了系統的穩健性。

---

### 3. Arduino (`arduino_uno.ino`)：伺服器發現、指令的拉取與執行

Arduino 作為最終的執行者，它採用「**主動拉取 (Polling)**」的模式從後端獲取指令，並透過 UDP 廣播進行伺服器發現。

-   **伺服器 IP 發現**: Arduino 在啟動後，會監聽來自後端伺服器的 UDP 廣播訊息 (`MINIAUTO_SERVER_IP:<ip>:<port>`)，從中解析出後端伺服器的 IP 位址和埠號，以便建立後續的 HTTP 連線。
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
    // 確保伺服器 IP 已被發現後才進行同步
    if (g_server_ip[0] != '\0') { 
      syncWithServer();
    } else {
      // 這裡可以加入一些等待或重試邏輯，或只是等待廣播
      Serial.println(F("伺服器 IP 尚未發現。正在等待廣播..."));
    }
    lastCommandPollTime = currentTime;
  }
  // 處理來自 ESP-01S 的 UDP 廣播訊息，以發現伺服器 IP
  // ... (相關的 UDP 監聽和解析邏輯)
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

``` mermaid 
flowchart TD
    %% 前端 GUI
    subgraph GUI ["前端 GUI - index.html"]
        GUI_Keys["鍵盤事件偵測"]
        GUI_State["控制狀態追蹤"]
        GUI_Cmd["控制指令產生"]
        GUI_Send["傳送控制指令"]
        GUI_Stream["影像串流顯示"]
        GUI_Analysis["獲取分析結果"]
    end

    %% 後端 FastAPI
    subgraph Backend ["後端 FastAPI"]
        API_Manual["API 接收控制指令"]
        API_State["儲存全域控制狀態"]
        API_Stream["MJPEG 串流代理"]
        API_Analysis["影像分析 API"]
        API_Sync["指令同步 API"]
        API_Broadcast["UDP 廣播 IP"]
    end

    %% 硬體區塊
    subgraph Hardware ["硬體設備"]
        ESP32["ESP32 CAM - MJPEG 串流"]
        Arduino["Arduino UNO"]
        UDP_Discover["接收廣播 IP"]
        Sync_Loop["定時請求指令"]
        Execute_Ctrl["執行 Velocity 控制"]
    end

    %% 前端流程
    GUI_Keys --> GUI_State --> GUI_Cmd --> GUI_Send --> API_Manual --> API_State
    GUI_Stream --> API_Stream --> ESP32
    GUI_Analysis --> API_Analysis

    %% 後端流程
    API_Broadcast --> UDP_Discover
    API_Sync --> Sync_Loop --> Execute_Ctrl

    %% 補連線
    Sync_Loop --> API_Sync
    Arduino --> UDP_Discover
    Arduino --> Sync_Loop
```
