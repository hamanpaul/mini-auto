# Miniauto 專案功能列表 (重構後)

本文件詳細列出了 Miniauto 專案在重構後，各個組件已實現的功能。

### 1. 後端服務 (Python FastAPI)

**核心功能:**
*   作為系統的中央控制與數據匯集平台。
*   提供 RESTful API 接口供 GUI 及 ESP32 網路代理互動。
*   代理並處理來自 ESP32-CAM 的影像串流。
*   根據從 ESP32 收到的感測器數據和當前控制模式生成控制指令。

**API 端點:**
*   `/api/sync` (POST): 接收 ESP32 的車輛狀態數據，回傳控制指令。
*   `/api/register_camera` (POST): 註冊 ESP32-CAM 的 IP 地址，並啟動後端相機串流處理器。
*   `/api/manual_control` (POST): 設定手動控制指令。
*   `/api/set_control_mode` (POST): 切換控制模式 (`manual`, `avoidance`, `autonomous`)。
*   `/api/latest_data` (GET): 獲取最新的車輛數據、指令、IP 和模式。
*   `/api/logs` (GET): 獲取後端服務的運行日誌。
*   `/camera/stream` (GET): 從後端串流經視覺分析處理後的 MJPEG 影像。
*   `/camera/analysis` (GET): 獲取最新的影像分析結果。

**控制邏輯與數據分析:**
*   **多模式控制**: 支援手動、避障和自主三種控制模式。
*   **熱成像分析**: 分析 8x8 熱成像數據。
*   **視覺分析**: 從 ESP32-CAM 獲取影像，使用 OpenCV 進行即時障礙物檢測。

### 2. Arduino UNO 韌體 (`src/miniauto/arduino_uno/arduino_uno.ino`)

**核心功能:**
*   作為車輛的即時控制單元，專注於硬體接口、感測器數據採集和致動器控制。

**通訊:**
*   **I2C Master**: 作為 I2C 主機，與 ESP32-CAM 進行通訊。
    *   **推送**: 將感測器數據 (熱成像、電壓、超音波) 打包並透過 I2C 發送給 ESP32。
    *   **拉取**: 從 ESP32 請求並接收最新的控制指令。

**感測器數據採集:**
*   AMG8833 熱像儀、電池電壓感測器、超音波感測器。

**致動器控制:**
*   精確控制四個馬達、舵機、RGB LED 和蜂鳴器。

### 3. ESP32-CAM 韌體 (`src/miniauto/esp32_cam/esp32_cam.ino`)

**核心功能:**
*   作為獨立的視覺模組和網路代理。

**通訊:**
*   **Wi-Fi 連線**: 連接到指定的 Wi-Fi 網路。
*   **MJPEG 影像串流**: 啟動 HTTP 伺服器，在 `/stream` 端點提供即時 MJPEG 影像串流。
*   **I2C Slave**: 作為 I2C 從機，與 Arduino UNO 交換數據。
    *   接收來自 UNO 的感測器數據。
    *   向 UNO 提供從後端獲取的控制指令。
*   **後端通訊 (HTTP Client)**:
    *   **UDP 服務發現**: 監聽後端伺服器的 UDP 廣播以發現其 IP。
    *   **自動註冊**: 向後端 `/api/register_camera` 註冊自身 IP。
    *   **定期同步**: 定期向後端 `/api/sync` 發送從 UNO 收到的感測器數據，並獲取控制命令。

**相機功能:**
*   初始化相機並採集影像幀。
