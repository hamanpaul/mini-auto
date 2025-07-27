# 影像系統呼叫流程與分析

本文件詳細解釋了 Miniauto 專案中，影像從 ESP32-CAM 產生、經由 Python 後端處理與分析，最終呈現在網頁介面 (GUI) 上的完整呼叫流程與數據傳輸。

這個系統的影像傳輸流程是單向的，並且經過後端處理，可以拆解成三個主要部分：

1.  **ESP32-CAM (影像來源)**
2.  **Python Backend (影像處理、分析與轉發中心)**
3.  **Web GUI (影像顯示端)**

---

### 完整流程說明

這是一個典型的「後端代理串流」架構。這樣設計的好處是，所有複雜的影像處理、數據整合都在後端完成，前端 GUI 只需負責顯示，保持輕量化。

#### 1. ESP32 Camera 端 (影像串流的起點)

*   **程式碼**:`src/miniauto/esp32_cam/esp32_cam.ino`
*   **角色**：一個獨立的微型 Web Server，負責生成原始 MJPEG 影像串流。
*   **流程**：
    1.  ESP32 啟動後，會初始化相機模組。
    2.  它會連上指定的 Wi-Fi (例如 `Hcedu01`)，並獲得一個 IP 位址 (例如 `192.168.1.100`)。
    3.  ESP32 會啟動一個 Web Server，並建立一個特定的串流路徑，例如 `http://192.168.1.100/stream` (預設埠號 80)。
    4.  任何時候當有客戶端 (Client) 連接到這個 `/stream` 路徑時，ESP32 就會開始從相機讀取畫面，並將每一幀 (frame) 影像以 `MJPEG` (Motion JPEG) 格式不斷地發送出去。
    *   **關鍵點**：ESP32 只負責提供 **原始、未經處理** 的影像串流。

#### 2. Python 後端 (FastAPI Server) - 影像處理、分析與代理

*   **程式碼**：
    *   `main.py` (啟動伺服器)
    *   `src/py_rear/apis/camera.py` (提供給前端的 API)
    *   `src/py_rear/services/camera_stream_processor.py` (影像處理與分析核心)
*   **角色**：系統的中樞，負責從 ESP32 獲取影像、進行分析，並將處理後的影像和分析結果轉發給前端。
*   **流程**：
    1.  **獲取影像**：`camera_stream_processor.py` 會作為一個 **客戶端**，主動去連接 ESP32 的影像串流 URL (`http://<ESP32_IP>/stream`)。它在一個獨立的執行緒中持續讀取影像幀。
    2.  **影像處理與分析**：`camera_stream_processor.py` 會接收從 ESP32 傳來的原始影像幀。接著，它使用 `OpenCV` 之類的庫對每一幀影像進行加工和分析。例如：
        *   **障礙物偵測**：透過亮度閾值化、高斯模糊、形態學操作和輪廓偵測等技術，識別影像中的潛在障礙物。
        *   **結果提取**：計算障礙物的中心點、面積比例等資訊。
        *   **數據整合**：疊加熱感應儀 (AMG8833) 的數據（如果可用）。
        *   顯示車輛狀態資訊 (速度、模式) 等。
        處理後的影像幀和分析結果會儲存在 `camera_stream_processor` 的內部變數中，並透過執行緒鎖確保多執行緒存取安全。
    3.  **影像串流代理 (`/api/camera/stream`)**：`camera.py` 中定義了一個 API 端點 `/api/camera/stream`。這個端點會返回一個 `StreamingResponse`。`StreamingResponse` 的內容來源，就是 `camera_stream_processor.py` **處理過後** 的影像幀。後端將處理完的影像一幀一幀地發送給連接到此端點的客戶端 (也就是前端 GUI)。
    4.  **影像分析結果提供 (`/api/camera/analysis`)**：`camera.py` 也提供 `/api/camera/analysis` 端點，供前端定期請求最新的視覺分析結果（例如障礙物偵測狀態、位置等）。

#### 3. 前端 GUI (Vue.js) - 影像顯示與分析結果呈現

*   **程式碼**：
    *   `templates/index.html`
    *   `templates/vue.min.js`
*   **角色**：使用者操作與影像顯示介面，負責從後端獲取並呈現影像串流和分析結果。
*   **流程**：
    1.  使用者在瀏覽器中打開應用程式頁面 (`index.html`)。
    2.  頁面中的 JavaScript (Vue.js) 會向 Python 後端發起請求。
    3.  **影像串流顯示**：在 HTML 中，會有一個 `<img>` 標籤，它的 `src` 屬性會被設定為 Python 後端的串流 API 路徑，像這樣：
        ```html
        <img src="/api/camera/stream">
        ```
        瀏覽器會向 `http://<your_pc_ip>:<port>/api/camera/stream` 發送一個 GET 請求。由於後端返回的是 `MJPEG` 串流，瀏覽器會將這個 `<img>` 標籤的內容持續更新，看起來就像是正在播放的影片。
    4.  **影像分析結果呈現**：前端會定期向後端的 `/api/camera/analysis` 端點發送請求，獲取最新的影像分析結果，並在介面上進行顯示或處理（例如，在影像上疊加障礙物框，或顯示文字提示）。

---

### 總結 Call Flow

**數據流向:**

`ESP32 Camera (原始串流)` -> `Python Backend (Processor)` -> `Python Backend (API)` -> `Web Browser (GUI)`

**呼叫順序:**

1.  **[Backend -> ESP32]**：後端服務 (`camera_stream_processor`) 主動請求 ESP32 的 `/stream`。
2.  **[ESP32 -> Backend]**：ESP32 開始向後端發送原始影像幀。
3.  **[Backend (Processor) -> Backend (API)]**：後端處理器對影像進行分析，並將處理後的幀和分析結果提供給後端 API。
4.  **[GUI -> Backend]**：前端 `<img>` 標籤請求後端的 `/api/camera/stream`，同時前端 JavaScript 定期請求 `/api/camera/analysis`。
5.  **[Backend (API) -> GUI]**：後端將處理後的影像幀以串流形式轉發給前端 GUI，並將分析結果以 JSON 形式返回給前端。

這個流程會不斷循環，從而實現了在使用者介面中看到經過即時處理和分析的遠端影像。

```mermaid
flowchart TD

    %% 第一階段：ESP32 CAM 影像來源
    subgraph ESP32_CAM ["ESP32 CAM 裝置"]
        E1["啟動與相機初始化"]
        E2["連接 Wi-Fi 並獲得 IP"]
        E3["啟動 Web Server 提供 MJPEG 串流"]
        E4["提供原始影像於 stream 路徑"]
    end

    %% 第二階段：Python FastAPI 後端
    subgraph Backend ["Python FastAPI 後端"]
        B1["Camera Processor 發起請求至 ESP32"]
        B2["讀取影像幀於背景執行緒"]
        B3["使用 OpenCV 等處理與分析"]
        B4["儲存處理後影像與分析結果"]
        B5["/api/camera/stream 提供 MJPEG 串流"]
        B6["/api/camera/analysis 提供分析結果 JSON"]
    end

    %% 第三階段：前端 Vue.js GUI
    subgraph Frontend ["前端 Web GUI"]
        F1["瀏覽器載入 index.html"]
        F2["img 請求串流路徑"]
        F3["畫面中顯示串流影像"]
        F4["JavaScript 定期發送分析請求"]
        F5["顯示障礙物資訊與車輛狀態"]
    end

    %% 流向關係
    E4 --> B1
    B1 --> B2 --> B3 --> B4
    B4 --> B5 --> F2 --> F3
    B4 --> B6 --> F4 --> F5
    F1 --> F2
```