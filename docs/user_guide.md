# Miniauto 使用者指南 (重構後)

本指南說明如何設定、運行並與重構後的 Miniauto 專案互動。

### 系統架構

系統現在由三個核心部分組成：

1.  **Arduino UNO**: 硬體即時控制器。
2.  **ESP32-CAM**: 視覺模組與網路代理。
3.  **Python FastAPI 後端**: 中央伺服器與控制大腦。

通訊流程為：`GUI <-> Backend <-> ESP32 <-> UNO`。

### 環境需求

**1. 後端 (Python FastAPI Server):**
*   Python 3.8+.
*   必要的 Python 套件: `fastapi`, `uvicorn`, `opencv-python`, `numpy`。透過 `pip install -r requirements.txt` 安裝。

**2. 前端 (Web GUI):**
*   任何現代網頁瀏覽器 (Chrome, Firefox, etc.)。

**3. 硬體:**
*   已燒錄最新韌體的 Arduino UNO 和 ESP32-CAM。
*   正確連接的感測器和致動器。

### 如何運行系統

1.  **啟動後端伺服器:**
    *   在您的終端機中，導航到專案根目錄 (`mini-auto`)。
    *   (可選，但建議) 啟動您的 Python 虛擬環境。
    *   執行 `python main.py`。伺服器將在前台運行。

2.  **啟動硬體:**
    *   為 ESP32-CAM 和 Arduino UNO 接通電源。
    *   ESP32 會自動連接到 Wi-Fi，尋找後端伺服器，並註冊自己。

3.  **打開前端 GUI:**
    *   在瀏覽器中打開 `http://<後端伺服器IP>:8000` (例如 `http://127.0.0.1:8000`)。
    *   `index.html` 頁面將會載入。

### 如何使用 GUI

*   **影像串流**: 當 ESP32 成功註冊後，影像串流會自動開始。如果沒有，可以手動點擊 "Start Stream"。
*   **控制模式**: 點擊 "Set Manual Mode", "Set Avoidance Mode", 或 "Set Autonomous Mode" 按鈕來切換機器人的控制模式。
*   **手動控制 (在 Manual Mode 下)**:
    *   **鍵盤**: 使用 `W`, `A`, `S`, `D` 鍵控制車輛移動。
    *   **速度滑塊**: 拖動滑塊調整速度。
*   **數據監控**: 儀表板會自動更新顯示來自車輛的最新數據以及後端發送的指令。