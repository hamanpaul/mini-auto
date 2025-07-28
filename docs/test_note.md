# 後端 API 測試檔案分析

本文件旨在分析 `test/unit/api/` 目錄下的主要 Python 測試檔案：`test_api.py` 和 `test_main.py`，並闡明它們各自的用途和區別。

## 檔案用途分析

### 1. `test_api.py`

*   **核心焦點**: 此檔案是針對 `src/py_rear/apis/vehicle_api.py` 中定義的核心 API 端點的**完整單元測試套件**。
*   **測試範圍**: 涵蓋了所有與車輛核心功能直接相關的 API，確保其業務邏輯的正確性。
    *   `/api/sync`：測試與 ESP32 的數據同步功能。
    *   `/api/register_camera`：驗證 ESP32-CAM 的 IP 註冊功能。
    *   `/api/manual_control`：測試手動控制指令的設定與更新。
    *   `/api/set_control_mode`：測試不同控制模式的切換。
    *   `/api/latest_data`：驗證獲取最新車輛狀態和指令的端點。
*   **測試方法**: 使用 `fastapi.testclient.TestClient` 模擬 HTTP 請求，並對每個端點的回應狀態碼和 JSON 內容進行詳細斷言。

### 2. `test_main.py`

*   **核心焦點**: 此檔案專注於測試 FastAPI **應用程式的核心機制**，也就是 `src/py_rear/main.py` 的功能。
*   **測試範圍**:
    *   **根路徑 (`/`)**: 驗證應用程式的根 URL 是否能成功返回 `index.html`。
    *   **動態 API 載入機制**: 這是此檔案最關鍵的測試。它透過呼叫定義在不同 API 模組中的端點，來驗證 `main.py` 的動態路由載入和整合機制是否正常運作。
*   **測試方法**: 測試的目標並非單一 API 的業務邏輯，而是應用程式整體的**啟動、路由配置和模組化載入**等框架層面的功能。

## 主要區別總結

| 檔案 | 測試目標 | 測試範圍 | 
| :--- | :--- | :--- | 
| **`test_api.py`** | **業務邏輯** | 針對 `vehicle_api.py` 的所有核心功能 | 
| **`test_main.py`** | **框架功能** | 應用程式啟動、動態路由載入 | 

簡單來說：
- `test_api.py` 確保**車輛控制功能**正確無誤。
- `test_main.py` 確保**應用程式的基礎架構**（特別是路由）穩固可靠。