### UI 的追蹤流程 (Tracking Flow)

這個 GUI 是一個基於 Vue.js 的單頁應用程式 (SPA)，它透過 HTTP 請求與後端的 FastAPI 伺服器進行通訊。

**1. 前端 (templates/index.html - Vue.js)**

*   **初始化與數據獲取：**
    *   當 `index.html` 在瀏覽器中載入時，Vue.js 應用程式會啟動。
    *   `mounted()` 鉤子會立即呼叫 `fetchData()` 方法，並設定一個定時器，每秒鐘再次呼叫 `fetchData()`。
    *   `fetchData()` 方法會向後端的 `/api/latest_data` 端點發送一個 GET 請求。
    *   後端返回的數據（包括 `latest_data`、`latest_command`、`esp32_cam_ip` 和 `current_control_mode` 等）會更新前端 Vue.js 實例的數據屬性。
    *   這些數據屬性會自動反映在 UI 上，例如顯示最新的感測器數據、當前控制模式，以及更新攝影機串流的 URL。

*   **虛擬搖桿互動：**
    *   使用者在 `joystick-base` 區域按下鼠標 (`@mousedown`) 並拖曳 (`@mousemove`) 時，會觸發 `startDrag()` 和 `onDrag()` 方法。
    *   `onDrag()` 會根據鼠標在搖桿基座內的相對位置，計算出 `motorSpeed` (馬達速度，0-255) 和 `directionAngle` (方向角度，0-360 度)。
    *   計算出的 `motorSpeed` 和 `directionAngle` 會立即透過 `sendManualControl()` 方法發送到後端。
    *   當使用者釋放鼠標 (`@mouseup`) 或鼠標離開搖桿區域 (`@mouseleave`) 時，會觸發 `stopDrag()` 方法。
    *   `stopDrag()` 會將搖桿重置回中心位置，並將 `motorSpeed` 和 `directionAngle` 設定為 0，然後再次呼叫 `sendManualControl()`，以停止機器人移動。

*   **速度滑塊互動：**
    *   使用者拖動速度滑塊 (`<input type="range">`) 時，其 `@input` 事件會觸發 `sendManualControl()` 方法。
    *   滑塊的值 (`speedSliderValue`) 會被傳遞到 `sendManualControl()`，但目前在 `sendManualControl` 中，馬達速度主要由搖桿控制。如果需要滑塊作為速度上限，則需要調整 `sendManualControl` 的邏輯。

*   **功能按鈕互動：**
    *   **設定控制模式：** 當使用者點擊 "Set Manual Mode"、"Set Avoidance Mode" 或 "Set Autonomous Mode" 按鈕時，會觸發 `setControlMode(mode)` 方法。
    *   `setControlMode(mode)` 會向後端的 `/api/set_control_mode` 端點發送一個 POST 請求，並將選定的模式 (`manual`, `avoidance`, `autonomous`) 作為 JSON 數據傳遞。成功後，前端會更新 `currentControlMode` 的顯示。
    *   **註冊攝影機：** 當使用者點擊 "Register Camera" 按鈕時，會觸發 `registerCamera(ip)` 方法。
    *   `registerCamera(ip)` 會向後端的 `/api/register_camera` 端點發送一個 POST 請求，並將預設的 ESP32-S3-CAM IP (`192.168.1.100`) 作為 JSON 數據傳遞。成功後，前端會更新 `esp32CamIp`，進而更新即時影像的 `src` 屬性。

*   **即時影像顯示：**
    *   `<img>` 標籤的 `src` 屬性會動態綁定到 `http://` + `esp32CamIp` + `/stream`。
    *   一旦 `esp32CamIp` 從後端獲取並設定，瀏覽器就會嘗試從該 IP 位址載入影像串流。

**2. 後端 (src/py_rear/main.py, src/py_rear/apis/vehicle_api.py - FastAPI)**

*   **`/api/latest_data` (GET)：**
    *   這是前端定期獲取最新數據的端點。它會返回後端儲存的最新 Arduino 數據 (`latest_arduino_data`)、最新發送的指令 (`latest_command_sent`)、ESP32-S3-CAM 的 IP (`latest_esp32_cam_ip`)、當前控制模式 (`current_control_mode`)，以及熱成像和視覺分析結果。

*   **`/api/manual_control` (POST)：**
    *   接收來自前端的馬達速度 (`m`)、方向角度 (`d`)、舵機角度 (`a`) 和指令字節 (`c`)。
    *   更新後端全域變數中對應的手動控制指令。
    *   返回一個成功訊息給前端。

*   **`/api/set_control_mode` (POST)：**
    *   接收來自前端的控制模式 (`mode`)。
    *   更新後端全域變數 `current_control_mode`。
    *   返回一個成功訊息給前端。

*   **`/api/register_camera` (POST)：**
    *   接收來自前端的 ESP32-S3-CAM IP (`i`)。
    *   更新後端全域變數 `latest_esp32_cam_ip`。
    *   如果 `CameraStreamProcessor` 實例存在，它會更新串流來源並嘗試啟動影像處理。
    *   返回一個成功訊息給前端。

*   **`/api/sync` (POST)：**
    *   這個端點主要設計用於 Arduino 韌體向後端發送感測器數據（例如狀態字節、電壓、熱成像矩陣、超音波距離等）。
    *   後端接收這些數據後，會更新 `latest_arduino_data`，並根據當前的 `current_control_mode` 生成相應的控制指令（例如在避障模式下根據超音波數據生成後退指令）。
    *   生成的指令會儲存為 `latest_command_sent`，並作為響應返回給 Arduino。

### 使用者如何使用它？

1.  **啟動後端伺服器：**
    *   首先，您需要在一個終端機中啟動 Python FastAPI 後端伺服器。
    *   導航到專案根目錄 (`mini-auto`)。
    *   執行 `python3 src/py_rear/main.py` (這會讓伺服器在前台運行，佔用該終端機)。
    *   或者，為了不阻塞終端機，可以使用 `python3 src/py_rear/main.py &` 在背景運行。

2.  **打開前端 GUI：**
    *   在伺服器運行後，導航到專案中的 `templates` 目錄。
    *   雙擊 `index.html` 檔案，它會在您的預設網頁瀏覽器中打開。

3.  **互動操作：**
    *   **攝影機串流：** 如果您有實際的 ESP32-S3-CAM，請確保它已連接到網路並獲取到 IP。然後點擊 "Register Camera (192.168.1.100)" 按鈕 (如果 IP 不同，您可能需要修改前端程式碼中的預設 IP 或新增輸入框)。成功註冊後，您應該能看到即時影像。
    *   **控制模式：** 點擊 "Set Manual Mode"、"Set Avoidance Mode" 或 "Set Autonomous Mode" 按鈕來切換機器人的控制模式。當前模式會顯示在按鈕上方。
    *   **手動控制 (在 Manual Mode 下)：**
        *   **虛擬搖桿：** 點擊並拖曳搖桿中心的橙色圓點。將圓點拖離中心會增加馬達速度，拖曳的方向決定機器人的行進方向。釋放鼠標會讓搖桿歸位，機器人停止。
        *   **速度滑塊：** 拖動滑塊可以設定一個速度值。
    *   **數據監控：** 儀表板會自動更新顯示來自 Arduino 的最新數據（例如距離、熱成像分析、視覺分析）以及後端發送給 Arduino 的最新指令。

### 需要怎樣的環境才能正常執行？

要讓這個系統正常運行，您需要以下環境：

**1. 後端 (Python FastAPI Server)：**

*   **作業系統：** 任何支援 Python 3 的作業系統，例如 Linux (推薦，因為專案環境是 Linux)、Windows、macOS。
*   **Python 版本：** 建議使用 Python 3.8 或更高版本 (例如 Python 3.12)。
*   **Python 依賴：**
    *   `fastapi`
    *   `uvicorn` (ASGI 伺服器)
    *   `opencv-python` (用於影像處理)
    *   `numpy` (用於數值運算)
    *   `httpx` (用於後端測試)
    *   `pytest` (用於後端測試)
*   **安裝步驟：**
    1.  **克隆專案：** `git clone <您的專案URL>`
    2.  **進入專案根目錄：** `cd mini-auto`
    3.  **建立並啟用虛擬環境 (強烈建議)：**
        *   `python3 -m venv venv`
        *   `source venv/bin/activate` (Linux/macOS) 或 `.\venv\Scripts\activate` (Windows PowerShell)
    4.  **安裝依賴：** `pip install -e .` (這會根據 `pyproject.toml` 安裝所有必要的套件，並將 `miniauto-pyrear` 安裝為可編輯模式)。
    5.  **啟動伺服器：** `python3 src/py_rear/main.py`

**2. 前端 (Web GUI)：**

*   **作業系統：** 任何支援現代網頁瀏覽器的作業系統 (Windows, macOS, Linux)。
*   **軟體：** 任何現代網頁瀏覽器 (例如 Google Chrome, Mozilla Firefox, Microsoft Edge, Apple Safari)。
*   **執行方式：**
    1.  確保後端 FastAPI 伺服器正在運行。
    2.  導航到專案中的 `templates` 目錄 (`mini-auto/templates/`)。
    3.  雙擊 `index.html` 檔案，它將在您的預設網頁瀏覽器中打開。

**3. 可選 (用於完整系統功能)：**

*   **Arduino UNO 微控制器：** 運行機器人韌體，負責與感測器和馬達互動，並透過 Wi-Fi 模組與後端通訊。
*   **ESP01S Wi-Fi 模組：** 連接到 Arduino，提供 Wi-Fi 連線能力。
*   **ESP32-S3-CAM 模組：** 提供即時影像串流。您需要確保其 IP 位址在後端和前端中正確配置。
*   **實際機器人硬體：** 馬達、超音波感測器、熱成像攝影機等，連接到 Arduino。

總之，這個 UI 是一個基於 Web 的控制介面，它透過 HTTP 請求與 Python FastAPI 後端通訊，而後端則負責處理邏輯並與實際的機器人硬體（透過 Arduino）互動。