# 後端工作流程分析：伺服器啟動後的協同運作

這份文件旨在詳細分析 Miniauto 專案後端服務 (Py Agent) 在啟動後，各個核心組件 (FastAPI, OpenCV, 影像串流, 紅外線影像處理, GUI, 資料交換) 之間是如何協同運作的。

## 1. 伺服器啟動與初始化

當後端服務 (通常透過 `uvicorn src.py_rear.main:app --host 0.0.0.0 --port 8000` 啟動) 啟動時，會執行以下初始化步驟：

1.  **FastAPI 應用程式初始化**：`main.py` 作為入口點，會初始化 FastAPI 應用程式。
2.  **API 路由載入**：`main.py` 會自動掃描 `apis/` 目錄下的所有 Python 檔案 (例如 `vehicle_api.py`, `camera.py`, `status.py`)，並將其中定義的 `APIRouter` 實例載入到主 FastAPI 應用程式中，使其所有 API 端點可供訪問。
3.  **`CameraStreamProcessor` 初始化**：`services/camera_stream_processor.py` 中的 `CameraStreamProcessor` 實例會被創建。這個實例負責管理與 ESP32-S3 攝影機的影像串流連接和處理。此時，串流處理器尚未啟動，等待攝影機 IP 註冊。

## 2. 核心組件間的協同運作

伺服器啟動後，各組件之間的資料流和控制流如下：

### 2.1 ESP32-S3 攝影機影像串流與 OpenCV 處理

*   **註冊攝影機 IP (`POST /api/register_camera`)**：當 ESP32-S3 攝影機啟動並獲取到 IP 地址後，它會向後端發送 `POST /api/register_camera` 請求，將其 IP 地址告知後端。後端接收到此請求後，會更新 `CameraStreamProcessor` 實例的串流來源 (`update_stream_source`)，並自動啟動影像串流處理器 (`start()`)。
*   **影像串流獲取**：`CameraStreamProcessor` 會在一個獨立的執行緒中，透過 HTTP 連接到 ESP32-S3 攝影機的 MJPEG 串流 (`http://<ESP32-S3_IP>/stream`)，持續獲取原始 JPEG 影像幀。
*   **OpenCV 影像處理**：每個獲取到的 JPEG 幀會被 `CameraStreamProcessor` 解碼成 OpenCV 圖像格式，並進行即時影像分析。目前實作的分析包括：
    *   **障礙物檢測**：透過亮度閾值、高斯模糊、形態學操作和輪廓檢測等 OpenCV 技術，識別影像中的潛在障礙物，並計算其中心位置和佔比。
*   **結果儲存**：處理後的影像幀 (可選) 和視覺分析結果會被儲存在 `CameraStreamProcessor` 內部，供其他組件查詢。

### 2.2 ESP32 與 FastAPI 的資料交換 (`POST /api/sync`)

*   **ESP32 主動同步**：ESP32 會定期 (例如每 100 毫秒) 向後端發送 `POST /api/sync` 請求。這個請求包含了 ESP32 當前的狀態數據，例如：
    *   `s` (status_byte): 車輛狀態字節。
    *   `v` (voltage_mv): 電池電壓。
    *   `t` (thermal_matrix): 紅外線熱成像數據 (8x8 矩陣)，如果熱成像模組可用。
    *   `u` (ultrasonic_distance_cm): 超音波感測器距離，如果超音波模組可用。
    *   `i` (esp32_ip): ESP32-S3 攝影機的 IP 地址 (用於動態註冊，如果尚未註冊)。
*   **後端處理與指令生成**：FastAPI 的 `/api/sync` 端點接收到 ESP32 的數據後，會執行以下操作：
    *   **儲存最新數據**：將接收到的 ESP32 數據儲存為 `latest_arduino_data`。
    *   **紅外線影像分析**：如果接收到熱成像數據 (`t`)，會呼叫 `_analyze_thermal_data` 函數進行分析 (例如計算最高溫、最低溫、平均溫、熱點檢測)。
    *   **指令生成**：根據當前系統的 `current_control_mode` (手動、避障、自主) 和最新的感測器數據 (包括 `CameraStreamProcessor` 提供的視覺分析結果和超音波數據)，生成新的控制指令 (馬達速度、方向、舵機角度、指令字節)。
        *   **手動模式**：直接使用 `POST /api/manual_control` 設定的預設值。
        *   **避障模式**：優先使用超音波數據進行避障判斷，若無超音波數據或距離安全，則使用視覺分析結果進行避障。
        *   **自主模式**：主要依賴視覺分析結果 (障礙物位置、大小) 進行路徑規劃和控制。
    *   **回傳指令**：將生成的控制指令作為 `POST /api/sync` 請求的回應發送回 ESP32。ESP32 在其下一個控制週期中執行這些指令。

### 2.3 GUI (前端) 與後端的互動

GUI (或其他外部監控工具) 主要透過以下 API 與後端互動：

*   **控制指令 (`POST /api/manual_control`, `POST /api/set_control_mode`)**：GUI 可以發送請求來設定手動控制指令或切換車輛的控制模式。
*   **獲取最新數據 (`GET /api/latest_data`)**：GUI 定期呼叫此 API 以獲取車輛的最新狀態 (包括 ESP32 數據、發送的指令、攝影機 IP、控制模式、熱成像分析和視覺分析結果)，用於顯示在儀表板上。
*   **獲取影像 (`GET /api/camera/analysis`, `GET /api/camera/stream`)**：
    *   `GET /api/camera/analysis`：用於獲取最新的視覺分析結果。
    *   `GET /api/camera/stream`：提供 MJPEG 串流，允許 GUI 直接顯示即時影像。
*   **啟動/停止攝影機串流 (`POST /api/camera/start`, `POST /api/camera/stop`)**：GUI 可以手動控制攝影機串流的啟動和停止。
*   **獲取日誌 (`GET /api/logs`)**：GUI 可以獲取後端運行日誌，用於調試和監控。

## 3. 後端工作流程圖

```mermaid
graph TD
    subgraph "外部環境 (External Environment)"
        GUI[使用者介面/外部控制]
        ESP32[ESP32 模組]
    end

    subgraph "Python FastAPI 後端 (Backend)"
        F(FastAPI Server)

        subgraph "API 端點 (Endpoints)"
            VC_SYNC["/api/sync (POST)"]
            VC_MANUAL["/api/manual_control (POST)"]
            VC_MODE["/api/set_control_mode (POST)"]
            VC_LATEST["/api/latest_data (GET)"]
            VC_LOGS["/api/logs (GET)"]

            CC_REG["/api/register_camera (POST)"]
            CC_START["/api/camera/start (POST)"]
            CC_STOP["/api/camera/stop (POST)"]
            CC_STATUS["/api/camera/status (GET)"]
            CC_ANALYSIS["/api/camera/analysis (GET)"]
            CC_STREAM["/api/camera/stream (GET)"]
        end

        subgraph "服務層 (Service Layer)"
            style ControlLogic fill:#cde4ff,stroke:#6699ff,stroke-width:2px
            style ThermalAnalyzer fill:#cde4ff,stroke:#6699ff,stroke-width:2px
            style CSP fill:#cde4ff,stroke:#6699ff,stroke-width:2px

            ControlLogic[控制邏輯模組]
            ThermalAnalyzer[熱成像分析模組]
            CSP[影像串流處理器<br/>CameraStreamProcessor]
        end
    end

    %% 外部環境 -> API 端點
    GUI -->|"設定控制模式/手動指令"| F
    GUI -->|"獲取最新數據/影像/日誌"| F
    ESP32 -->|"定期 POST /api/sync (狀態, 電壓, 熱成像, 超音波)"| VC_SYNC
    ESP32 -->|"POST /api/register_camera (IP)"| CC_REG
    ESP32 -- "MJPEG 串流" --> CSP

    %% API 端點 <--> 服務層 (數據流動)
    VC_SYNC -- "請求/轉發數據" --> ControlLogic
    VC_SYNC -- "轉發熱成像數據" --> ThermalAnalyzer
    ControlLogic -- "生成指令" --> VC_SYNC

    VC_MANUAL -- "更新手動指令" --> ControlLogic
    VC_MODE -- "更新控制模式" --> ControlLogic

    VC_LATEST -- "請求數據" --> ControlLogic
    VC_LATEST -- "請求數據" --> ThermalAnalyzer

    CC_REG -- "更新來源/啟動" --> CSP
    CC_START -- "啟動" --> CSP
    CC_STOP -- "停止" --> CSP
    CC_STATUS -- "查詢狀態" --> CSP
    CC_ANALYSIS -- "請求分析結果" --> CSP
    CC_STREAM -- "請求即時串流" --> CSP

    %% 服務層內部溝通
    CSP -- "視覺分析結果" --> ControlLogic