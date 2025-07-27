# 後端重要功能分析

這份文件分析了 Miniauto 專案後端 Python 服務中的重要功能，並以表格形式呈現其職責，最後附上主要的 API 呼叫流程圖。

## 1. 重要功能列表

| 模組 | 功能名稱 | 類型 | 描述 |
|---|---|---|---|
| `vehicle_api.py` | `sync_data` | API 端點 (POST) | 接收 ESP32 的車輛狀態數據 (狀態字節、電壓、熱成像、超音波)，並根據當前控制模式 (手動、避障、自主) 計算並回傳控制指令 (馬達速度、方向、舵機角度、指令字節)。同時更新後端儲存的最新數據。 |
| `vehicle_api.py` | `manual_control` | API 端點 (POST) | 接收來自外部介面的手動控制指令 (馬達速度、方向、舵機角度、指令字節)，並更新後端儲存的手動控制參數。這些參數將在 `sync_data` 呼叫時被使用。 |
| `vehicle_api.py` | `set_control_mode` | API 端點 (POST) | 接收來自外部介面的控制模式切換請求 (手動、避障、自主)，並更新系統的當前控制模式。 |
| `vehicle_api.py` | `register_camera` | API 端點 (POST) | 接收 ESP32-S3 視覺模組的 IP 地址註冊請求。註冊成功後，會更新 `CameraStreamProcessor` 的串流來源並嘗試啟動串流。 |
| `vehicle_api.py` | `get_latest_data` | API 端點 (GET) | 提供後端儲存的最新車輛數據、最新發送的指令、ESP32-S3 IP、當前控制模式、熱成像分析結果以及視覺分析結果。 |
| `vehicle_api.py` | `get_logs` | API 端點 (GET) | 返回後端系統的運行日誌緩衝區內容。 |
| `vehicle_api.py` | `_generate_manual_commands` | 內部輔助函數 | 根據當前儲存的手動控制參數生成 `SyncResponse` 物件。 |
| `vehicle_api.py` | `_generate_avoidance_commands` | 內部輔助函數 | 根據超音波感測器數據和視覺分析結果生成避障控制指令。優先處理超音波數據，若無障礙則執行視覺避障邏輯。 |
| `vehicle_api.py` | `_generate_autonomous_commands` | 內部輔助函數 | 根據視覺分析結果 (障礙物檢測、位置、大小) 生成自主導航控制指令。包含超音波數據的優先判斷。 |
| `vehicle_api.py` | `_analyze_thermal_data` | 內部輔助函數 | 分析熱成像矩陣數據，計算最高溫、最低溫、平均溫，並判斷是否存在熱點。 |
| `camera.py` | `start_camera_stream` | API 端點 (POST) | 啟動 `CameraStreamProcessor`，開始從 ESP32-S3 獲取並處理影像串流。 |
| `camera.py` | `stop_camera_stream` | API 端點 (POST) | 停止 `CameraStreamProcessor`，停止影像串流的獲取與處理。 |
| `camera.py` | `get_camera_status` | API 端點 (GET) | 返回 `CameraStreamProcessor` 的運行狀態 (是否正在運行)。 |
| `camera.py` | `get_latest_processed_frame` | API 端點 (GET) | 返回最新處理過的影像幀 (JPEG 格式的 Base64 編碼字串)，用於前端顯示。 |
| `camera.py` | `stream_mjpeg_from_processor` | API 端點 (GET) | 提供 MJPEG 串流，直接從 `CameraStreamProcessor` 獲取原始影像幀並以 multipart/x-mixed-replace 格式傳輸。 |
| `status.py` | `update_vehicle_status` | API 端點 (POST) | 接收來自車輛 (Arduino) 的通用狀態更新，例如電池電量和當前狀態字串。 |
| `camera_stream_processor.py` | `CameraStreamProcessor` | 類別 | 負責管理與 ESP32-S3 攝影機的 MJPEG 串流連接，獲取影像幀，並進行 OpenCV 影像處理 (例如障礙物檢測)。 |
| `camera_stream_processor.py` | `update_stream_source` | 類別方法 | 更新攝影機串流的 IP 地址，並在必要時重啟串流。 |
| `camera_stream_processor.py` | `_get_mjpeg_stream` | 內部方法 | 在獨立執行緒中運行，負責連接 MJPEG 串流並解析 JPEG 幀。 |
| `camera_stream_processor.py` | `_process_frame` | 內部方法 | 對獲取的 JPEG 幀進行 OpenCV 解碼和影像分析 (例如亮度閾值、輪廓檢測以判斷障礙物)。 |
| `camera_stream_processor.py` | `start` | 類別方法 | 啟動影像串流處理執行緒。 |
| `camera_stream_processor.py` | `stop` | 類別方法 | 停止影像串流處理執行緒。 |
| `camera_stream_processor.py` | `get_latest_frame` | 類別方法 | 返回最新的原始影像幀、處理後的影像幀和視覺分析結果。 |
| `camera_stream_processor.py` | `is_running` | 類別方法 | 返回攝影機串流處理器是否正在運行。 |

## 2. 主要 API 呼叫流程圖

```mermaid
flowchart TD

%% 區塊 A：ESP32-S3-CAM
subgraph A[ESP32-S3-CAM]
    A1[初始化]
    A2[發送 /api/sync]
    A3[接收控制指令]
    A4[執行動作]
    A1 --> A2 --> A3 --> A4 --> A2
end

%% 區塊 B：FastAPI 控制流程
subgraph B[FastAPI 控制流程]
    B1[接收 /api/sync]
    B2[判斷控制模式]
    B3a[手動模式]
    B3b[避障模式]
    B3c[自主模式]
    B4[生成控制指令]
    B5[回傳指令]
    B1 --> B2
    B2 --> B3a --> B4
    B2 --> B3b --> B4
    B2 --> B3c --> B4
    B4 --> B5
end

%% 區塊 C：控制參數設定
subgraph C[控制參數與模式設定]
    C1[手動控制 /manual_control]
    C2[設定模式 /set_control_mode]
    C3[更新參數與模式]
    C1 --> C3
    C2 --> C3
end

%% 區塊 D：資料查詢
subgraph D[資料查詢]
    D1[查詢 /latest_data]
    D2[查詢 /logs]
    D3[傳回資料或日誌]
    D1 --> D3
    D2 --> D3
end

%% 區塊 E：Camera 控制 API
subgraph E[Camera API 控制]
    E1[註冊 /register_camera]
    E2[啟動串流 /camera/start]
    E3[停止串流 /camera/stop]
    E4[查詢狀態 /camera/status]
    E5[取得分析 /camera/analysis]
    E6[取得影像 /camera/stream]
    E1 --> F1
    E2 --> F1
    E3 --> F2
end

%% 區塊 F：Camera Stream Processor
subgraph F[Camera Stream 處理器]
    F1[啟動並連線 MJPEG]
    F2[停止處理器]
    F3[影像分析]
    F4[儲存結果與幀]
    F1 --> F3 --> F4 --> F3
end

%% 邏輯連線
A2 --> B1
B5 --> A3
F4 --> E5
F4 --> E6
F3 --> B3b
F3 --> B3c

```
