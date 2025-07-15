## Miniauto 專案使用者手冊

本手冊旨在提供 Miniauto 專案的設定、運行和互動的詳細指南。本專案包含一個 Python FastAPI 後端服務、一個運行在 Arduino UNO 上的韌體，以及一個運行在 ESP32-CAM 上的韌體。

### 1. 系統架構概覽

*   **Arduino UNO**：車輛的控制核心，負責感測器數據採集（熱像儀、電壓）、致動器控制（馬達、舵機、LED、蜂鳴器），並透過 ESP-01S Wi-Fi 模組與後端通訊。
*   **ESP32-CAM**：獨立的視覺模組，負責相機影像採集，並透過 Wi-Fi 提供 MJPEG 影像串流。同時作為 I2C 從機向 Arduino UNO 報告自身 IP。
*   **Python FastAPI 後端**：系統的中央大腦，接收來自 Arduino 的車輛狀態，處理視覺和熱成像數據，根據控制模式生成控制指令，並提供 API 接口供使用者或外部系統互動。

### 2. 環境設定

#### 2.1. 硬體準備

*   Arduino UNO 開發板
*   ESP32-S3-WROOM 開發板（帶有 GC2145 相機模組）
*   ESP-01S Wi-Fi 模組（連接至 Arduino UNO）
*   AMG8833 熱像儀
*   馬達、舵機、RGB LED、蜂鳴器等車輛致動器
*   必要的連接線和電源

#### 2.2. 軟體準備

**A. Python 後端環境**

1.  **安裝 Python**：確保您的系統安裝了 Python 3.8 或更高版本。
2.  **建立虛擬環境 (推薦)**：
    ```bash
    python -m venv venv
    source venv/bin/activate  # Linux/macOS
    # venv\Scripts\activate   # Windows
    ```
3.  **安裝依賴**：
    ```bash
    pip install -r requirements.txt
    ```
    *   `requirements.txt` 應包含 `fastapi`, `uvicorn`, `opencv-python`, `requests` 等。

**B. Arduino 開發環境**

1.  **安裝 Arduino IDE 或 Arduino CLI**：
    *   **Arduino IDE**：從 [Arduino 官網](https://www.arduino.cc/en/software) 下載並安裝。
    *   **Arduino CLI**：從 [Arduino CLI GitHub](https://github.com/arduino/arduino-cli/releases) 下載並安裝。
2.  **安裝 Arduino UNO 板支援包**：
    *   在 Arduino IDE 中：`工具` -> `開發板` -> `開發板管理員`，搜尋並安裝 `Arduino AVR Boards`。
    *   使用 Arduino CLI：`arduino-cli core install arduino:avr`
3.  **安裝 ESP32 板支援包**：
    *   在 Arduino IDE 中：`檔案` -> `偏好設定` -> `額外的開發板管理員網址`，添加 `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`。然後在 `開發板管理員` 中搜尋並安裝 `esp32`。
    *   使用 Arduino CLI：`arduino-cli core install esp32:esp32`
4.  **安裝函式庫**：
    *   **Melopero AMG8833**：在 Arduino IDE 中：`草稿碼` -> `匯入函式庫` -> `管理函式庫`，搜尋並安裝 `Melopero AMG8833`。**注意：此函式庫的原始碼已在專案中修改過，請確保使用專案目錄下的版本。**
    *   **SoftwareSerial**：Arduino 內建函式庫，無需額外安裝。
    *   **FastLED**：在函式庫管理員中搜尋並安裝 `FastLED`。
    *   **Servo**：Arduino 內建函式庫，無需額外安裝。
    *   **WebServer** (ESP32)：ESP32 板支援包內建，無需額外安裝。
    *   **esp_camera** (ESP32)：ESP32 板支援包內建，無需額外安裝。
    *   **Wire**：Arduino 內建函式庫，無需額外安裝。

### 3. 程式碼部署

#### 3.1. 部署 Arduino UNO 韌體

1.  **開啟專案**：在 Arduino IDE 中開啟 `src/miniauto/arduino_uno/arduino_uno.ino`。
2.  **配置 Wi-Fi**：修改 `arduino_uno.ino` 中的 `ssid` 和 `password` 變數，使其與您的 Wi-Fi 網路匹配。
    ```cpp
    const char ssid[] PROGMEM = "您的Wi-Fi名稱";
    const char password[] PROGMEM = "您的Wi-Fi密碼";
    ```
3.  **選擇開發板**：`工具` -> `開發板` -> `Arduino Uno`。
4.  **選擇序列埠**：`工具` -> `序列埠`，選擇連接 Arduino UNO 的正確序列埠。
5.  **上傳**：點擊 `上傳` 按鈕將韌體燒錄到 Arduino UNO。

#### 3.2. 部署 ESP32-CAM 韌體

1.  **開啟專案**：在 Arduino IDE 中開啟 `src/miniauto/esp32_cam/esp32_cam.ino`。
2.  **配置 Wi-Fi**：修改 `esp32_cam.ino` 中的 `ssid` 和 `password` 變數，使其與您的 Wi-Fi 網路匹配。
    ```cpp
    const char* ssid = "您的Wi-Fi名稱";
    const char* password = "您的Wi-Fi密碼";
    ```
3.  **選擇開發板**：`工具` -> `開發板` -> `ESP32 Arduino` -> `ESP32S3 Dev Module` (或您實際使用的 ESP32-S3 開發板型號)。
4.  **選擇序列埠**：`工具` -> `序列埠`，選擇連接 ESP32-CAM 的正確序列埠。
5.  **上傳**：點擊 `上傳` 按鈕將韌體燒錄到 ESP32-CAM。

#### 3.3. 運行 Python 後端服務

1.  **啟動虛擬環境** (如果之前建立過)：
    ```bash
    source venv/bin/activate  # Linux/macOS
    # venv\Scripts\activate   # Windows
    ```
2.  **運行 FastAPI 應用**：
    ```bash
    python main.py
    ```
    *   服務將預設在 `http://0.0.0.0:8000` 運行。
    *   您可以在瀏覽器中訪問 `http://localhost:8000` 查看基本介面。
    *   API 文件將在 `http://localhost:8000/docs` 提供。

### 4. 系統互動

#### 4.1. 啟動順序

1.  **啟動 ESP32-CAM**：確保 ESP32-CAM 已上電並連接到 Wi-Fi。它將啟動 MJPEG 串流伺服器並作為 I2C 從機等待 IP 查詢。
2.  **啟動 Arduino UNO**：確保 Arduino UNO 已上電並連接到 Wi-Fi。它將嘗試透過 UDP 發現後端服務，並透過 I2C 獲取 ESP32-CAM 的 IP。
3.  **啟動 Python FastAPI 後端**：運行 `python main.py`。後端啟動後，Arduino UNO 將開始向其同步數據，ESP32-CAM 的 IP 也會被註冊。

#### 4.2. 透過 API 互動

您可以使用任何 HTTP 客戶端（如 `curl`、Postman 或瀏覽器）與後端 API 互動。

*   **獲取最新數據**：
    ```bash
    curl http://localhost:8000/api/latest_data
    ```
*   **設定手動控制**：
    ```bash
    curl -X POST -H "Content-Type: application/json" -d '{"m": 100, "d": 0, "a": 90, "c": 0}' http://localhost:8000/api/manual_control
    ```
    *   `m`: 馬達速度 (-255 到 255)
    *   `d`: 方向角度 (0 到 359)
    *   `a`: 舵機角度 (0 到 180)
    *   `c`: 指令字節 (0-3: 蜂鳴器, 4-7: LED 覆蓋)
*   **切換控制模式**：
    ```bash
    curl -X POST -H "Content-Type: application/json" -d '{"mode": "avoidance"}' http://localhost:8000/api/set_control_mode
    ```
    *   `mode` 可選值：`manual`, `avoidance`, `autonomous`
*   **啟動/停止相機串流處理** (後端從 ESP32-CAM 獲取影像)：
    ```bash
    curl -X POST http://localhost:8000/camera/start
    curl -X POST http://localhost:8000/camera/stop
    ```
*   **查看後端日誌**：
    ```bash
    curl http://localhost:8000/api/logs
    ```

#### 4.3. 視覺化介面 (如果提供)

如果專案包含前端網頁介面，您可以直接訪問 `http://localhost:8000` 來進行視覺化控制和監控。

### 5. 故障排除

*   **Wi-Fi 連線問題**：
    *   檢查 `ssid` 和 `password` 是否正確。
    *   確保開發板在 Wi-Fi 訊號範圍內。
    *   檢查路由器設定，確保沒有 MAC 位址過濾或防火牆阻擋。
*   **Arduino UNO 記憶體不足**：
    *   本專案已進行極致記憶體優化，如果仍遇到問題，請檢查是否有額外函式庫或程式碼引入。
*   **ESP32-CAM 影像串流問題**：
    *   確保 ESP32-CAM 的 IP 地址已正確註冊到後端。
    *   檢查相機模組是否正確連接。
    *   嘗試調整 `frame_size` 或 `jpeg_quality` 以降低串流負載。
*   **API 通訊問題**：
    *   檢查後端服務是否正在運行。
    *   檢查防火牆設定，確保埠 8000 未被阻擋。
    *   確認 Arduino UNO 發現的伺服器 IP 是否正確。

### 6. 進階配置

*   **ESP32-CAM IP 地址**：在 `main.py` 中，`ESP32_CAM_IP` 變數在某些情況下可能需要手動設定，但通常會透過 Arduino UNO 的 I2C 註冊自動更新。
*   **日誌限制**：`vehicle_api.py` 中的 `MAX_LOG_ENTRIES` 可以調整後端日誌緩衝區的大小。
*   **相機 Pin 腳**：如果使用不同型號的 ESP32-CAM 或相機模組，可能需要修改 `esp32_cam.ino` 中的 `CAM_PIN_` 定義。

