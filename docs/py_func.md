# 後端重要功能分析

這份文件分析了 Miniauto 專案後端 Python 服務中的重要功能，並以表格形式呈現其職責，最後附上主要的 API 呼叫流程圖。

## 1. 重要功能列表

| 模組 | 功能名稱 | 類型 | 描述 |
|---|---|---|---|
| `vehicle_api.py` | `sync_data` | API 端點 (POST) | 接收 Arduino UNO 的車輛狀態數據 (狀態字節、電壓、熱成像、超音波)，並根據當前控制模式 (手動、避障、自主) 計算並回傳控制指令 (馬達速度、方向、舵機角度、指令字節)。同時更新後端儲存的最新數據。 |
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
graph TD
    subgraph Arduino UNO
        A[啟動] --> B(定期發送 POST /api/sync)
        B --> C{接收控制指令}
        C --> D[執行指令]
        D --> B
    end

    subgraph Python FastAPI Server
        E[啟動] --> F(監聽 API 請求)

        subgraph Vehicle Control APIs
            F --> G{POST /api/sync}
            G -- 接收狀態數據 (s, v, t, u) --> H{判斷控制模式}
            H -- 手動模式 --> I[使用手動控制參數]
            H -- 避障模式 --> J[執行避障邏輯 (超音波/視覺)]
            H -- 自主模式 --> K[執行自主導航邏輯 (視覺)]
            I --> L(生成控制指令)
            J --> L
            K --> L
            L -- 回傳指令 (c, m, d, a) --> G
            G --> M[更新最新數據]
            M --> F

            F --> N{POST /api/manual_control}
            N -- 接收手動指令 --> O[更新手動控制參數]
            O --> F

            F --> P{POST /api/set_control_mode}
            P -- 接收模式 --> Q[更新控制模式]
            Q --> F

            F --> R{GET /api/latest_data}
            R -- 返回最新數據 --> F

            F --> S{GET /api/logs}
            S -- 返回日誌 --> F
        end

        subgraph Camera Stream APIs
            F --> T{POST /api/register_camera}
            T -- 接收 ESP32-S3 IP --> U[更新 CameraStreamProcessor 來源]
            U --> V[啟動 CameraStreamProcessor]
            V --> F

            F --> W{POST /api/camera/start}
            W --> V

            F --> X{POST /api/camera/stop}
            X --> Y[停止 CameraStreamProcessor]
            Y --> F

            F --> Z{GET /api/camera/status}
            Z -- 返回串流狀態 --> F

            F --> AA{GET /api/camera/latest_frame}
            AA -- 返回處理後影像 --> F

            F --> BB{GET /api/camera/stream_mjpeg}
            BB -- 返回 MJPEG 串流 --> F
        end

        subgraph CameraStreamProcessor (獨立執行緒)
            V --> CC[連接 ESP32-S3 MJPEG 串流]
            CC --> DD{獲取原始影像幀}
            DD --> EE[OpenCV 影像處理/分析]
            EE --> FF[儲存最新幀及分析結果]
            FF --> DD
            Y --> GG[停止串流執行緒]
        end
    end

    Arduino UNO -- 註冊 IP --> T
    CameraStreamProcessor -- 提供影像/分析結果 --> AA
    CameraStreamProcessor -- 提供 MJPEG 串流 --> BB
    CameraStreamProcessor -- 提供視覺分析結果 --> J
    CameraStreamProcessor -- 提供視覺分析結果 --> K
```
