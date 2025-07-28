## Miniauto 專案使用者手冊 (重構後)

本手冊旨在提供 Miniauto 專案在架構重構後的設定、運行和互動的詳細指南。

### 1. 系統架構概覽

*   **Arduino UNO**: 車輛的控制核心，負責感測器數據採集和致動器控制。它 **只** 透過 I2C 與 ESP32-CAM 通訊。
*   **ESP32-CAM**: 獨立的視覺模組和 **網路代理**。它負責處理所有 Wi-Fi 通訊（服務發現、數據同步、相機註冊），並作為 I2C 從機與 Arduino UNO 交換數據。
*   **Python FastAPI 後端**: 系統的中央大腦，接收來自 ESP32 的數據，處理視覺和熱成像數據，生成控制指令，並提供 API 接口。

### 2. 環境設定

#### 2.1. 軟體準備

**A. Python 後端環境**
1.  安裝 Python 3.8 或更高版本。
2.  建立虛擬環境並透過 `pip install -r requirements.txt` 安裝依賴。

**B. Arduino 開發環境**
1.  安裝 Arduino IDE 或 Arduino CLI。
2.  安裝 `Arduino AVR Boards` 和 `esp32` 的板支援包。
3.  安裝函式庫: `Melopero AMG8833`, `FastLED`, `Servo`, `Ultrasound`。**注意：`SoftwareSerial` 不再需要。**

### 3. 程式碼部署

#### 3.1. 部署 Arduino UNO 韌體
1.  在 Arduino IDE 中開啟 `src/miniauto/arduino_uno/arduino_uno.ino`。
2.  **無需配置 Wi-Fi**。
3.  選擇 `Arduino Uno` 開發板和正確的序列埠，然後上傳。

#### 3.2. 部署 ESP32-CAM 韌體
1.  在 Arduino IDE 中開啟 `src/miniauto/esp32_cam/esp32_cam.ino`。
2.  修改 `ssid` 和 `password` 變數以匹配您的 Wi-Fi 網路。
3.  選擇 `ESP32S3 Dev Module` 開發板和正確的序列埠，然後上傳。

#### 3.3. 運行 Python 後端服務
1.  運行 `python main.py`。
2.  服務將預設在 `http://0.0.0.0:8000` 運行。

### 4. 系統互動

#### 4.1. 啟動順序
1.  **啟動 Python FastAPI 後端**：運行 `python main.py`。後端啟動後，會開始 UDP 廣播自己的 IP。
2.  **啟動 ESP32-CAM**：ESP32 上電後會連接 Wi-Fi，監聽後端廣播，發現後端後自動註冊並開始定期同步數據。
3.  **啟動 Arduino UNO**：UNO 上電後會透過 I2C 向 ESP32-CAM 發送感測器數據並獲取控制命令。

#### 4.2. 透過 API 互動

您可以使用任何 HTTP 客戶端（如 `curl`）與後端 API 互動。

*   **獲取最新數據**: `curl http://localhost:8000/api/latest_data`
*   **設定手動控制**: `curl -X POST -H "Content-Type: application/json" -d '{"m": 100, "d": 0, "a": 90, "c": 0}' http://localhost:8000/api/manual_control`
*   **切換控制模式**: `curl -X POST -H "Content-Type: application/json" -d '{"mode": "avoidance"}' http://localhost:8000/api/set_control_mode`

### 5. 故障排除
*   **通訊問題**: 
    *   檢查後端服務是否正在運行。
    *   檢查防火牆設定，確保埠 8000 未被阻擋。
    *   透過序列埠監控器檢查 ESP32 和 UNO 的日誌輸出，確認 I2C 通訊是否正常，以及 ESP32 是否成功發現並註冊到後端。
*   **ESP32-CAM 影像串流問題**: 
    *   確保 ESP32-CAM 的 IP 地址已正確註冊到後端 (可透過 `/api/latest_data` 確認)。
