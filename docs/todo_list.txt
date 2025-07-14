### A. 已完成 / 已備資產 (Completed / Available Assets)

這部分是我們已經擁有、可以立即投入使用的穩固基礎。

1.  **後端伺服器框架 (Backend Server Framework):**
    *   `main.py` 提供了一個基於 **Python FastAPI** 的可運行伺服器骨架。
    *   具備 **動態 API 路由載入** 功能。
    *   `requirements.txt` 提供了完整的 Python 環境依賴列表。
    *   `CameraStreamProcessor` 服務提供了一個處理影像串流的基礎類別。
2.  **硬體控制邏輯庫 (Hardware Control Logic Library):**
    *   `app_control.ino` 是一個非常有價值的 **程式碼庫**，提供了所有必要的硬體底層控制函式 (馬達、舵機、LED、電壓)。
3.  **感測器驅動程式 (Sensor Driver):**
    *   `reference/Melopero_AMG8833-1.1.0/` 提供了 **完整且經過驗證的 AMG8833 熱感應器函式庫**。
4.  **專案藍圖與規格 (Project Blueprint & Specifications):**
    *   `GEMINI.md` 提供了整個專案的 **核心指導文件**，包含精確的硬體針腳定義、I2C 設備位址、開發語言和風格偏好。
5.  **Arduino UNO 核心通訊層 (Arduino UNO Core Communication Layer):**
    *   `arduino_client.ino` 已重構，具備透過 **SoftwareSerial** 和 **AT 指令** 控制 **ESP-01S 模組** 進行 Wi-Fi 通訊的能力。
    *   **`httpPost()` 函式**：已修改為通用同步端點，能發送數據並回傳伺服器回應的 Body。
    *   **`sendAtCommand()` 函式**：底層 AT 指令發送與回應處理。
    *   **`setupEsp01s()` 函式**：ESP-01S 模組的 Wi-Fi 連線設定。
6.  **Arduino UNO 狀態收集與打包 (Arduino UNO Status Collection & Packing):**
    *   **`status_byte` 定義**：已在程式碼中詳細註解，包含電池電量、感測器狀態、錯誤代碼、LED 狀態。
    *   **`getBatteryLevelCode()` 函式**：讀取類比電壓，轉換為百分比和 2-bit 編碼，並儲存實際電壓值。
    *   **`getErrorCode()` 函式**：框架已建立，待實作實際錯誤檢測。
    *   **`getLedStatusCode()` 函式**：基於優先級邏輯 (未連線 > 錯誤 > 忙碌 > 正常) 判斷 LED 狀態碼。
    *   **`Rgb_Show()` 和 `setLedStatus()` 函式**：LED 顏色控制。
7.  **Arduino UNO 核心同步邏輯 (Arduino UNO Core Sync Logic):**
    *   **`syncWithServer()` 函式**：已實作核心框架，負責：
        *   收集狀態數據並打包到 `status_byte`。
        *   收集熱成像數據 (每秒一次，已優化)。
        *   將所有數據組合成優化的 JSON 酬載。
        *   呼叫 `httpPost()` 發送到後端的 `/api/sync` 端點。
        *   解析後端回應的 JSON，並為指令執行預留框架。
    *   **`loop()` 函式**：已簡化為定時呼叫 `syncWithServer()`。

---

### B. 待辦 / 新增功能 (To-Do / New Features)

這是我們將要執行的、從無到有的開發任務清單，依照實作順序和依賴關係排列：

**優先任務：建立版號控制機制 (Priority Task: Establish Version Control) - 已完成 ✅**

1.  **建立版號管理系統 (Implement Versioning System):**
    *   在專案中建立一個集中的地方來管理當前版本號 (例如，一個 `VERSION` 檔案或在 `GEMINI.md` 中明確標示)。 ✅
    *   確保所有未來的開發工作都遵循 `GEMINI.md` 中定義的版號規則。 ✅
    *   **當前起始版本**: `1.1.1` ✅

**第一階段：板間通訊與 IP 自動發現 (Inter-Board Communication & IP Discovery) - 已完成 ✅**

1.  **ESP32-S3 端 (視覺模組):**
    *   修改韌體，使其作為 **I2C 從機 (Slave)**，回應來自 UNO 的 IP 位址請求。
    *   確保其 Camera Stream Server 在 STA 模式下正常運作。
    *   **備註:** 這是 ESP32-S3 獨立的韌體開發任務。
2.  **Arduino UNO 端 (主控制器):**
    *   實作 **I2C 主機 (Master)** 邏輯，向 ESP32-S3 的 I2C 位址 (`0x53`) 請求 IP 位址數據。
    *   將收到的 ESP32-S3 IP 位址，透過 `syncWithServer()` 發送到後端。

**第二階段：後端 API 實作 (Backend API Implementation) - 部分完成 ✅**

3.  **Python 後端 (`main.py`):**
    *   **`POST /api/sync` 端點**：實作接收 Arduino 數據 (status_byte, voltage, thermal_matrix) 的邏輯。 ✅
    *   **`POST /api/register_camera` 端點**：實作接收 Arduino 回報的 ESP32-S3 IP 位址的邏9輯。 ✅
    *   **指令生成邏輯**：根據接收到的數據或使用者輸入，生成並回傳控制指令 (motor speed, direction, servo angle, command byte)。
        *   **框架化完成** (引入控制模式、模式切換 API、指令生成輔助函式框架)。 ✅
        *   **手動控制已實作** (`_generate_manual_commands()`)。 ✅
        *   **避障邏輯** (`_generate_avoidance_commands()`)：**已實作基本邏輯** (基於錯誤、熱點、視覺障礙物優先級判斷)。 ✅
        *   **自主導航邏輯** (`_generate_autonomous_commands()`)：待實作。

**第三階段：Arduino UNO 硬體控制與狀態更新 (Arduino UNO Hardware Control & Status Update) - 部分完成 ✅**

4.  **Arduino UNO 端 (主控制器):**
    *   **硬體初始化**：在 `setup()` 中加入馬達、舵機、蜂鳴器等硬體的初始化程式碼。 ✅
    *   **指令執行**：在 `syncWithServer()` 內部，根據後端回傳的指令 (motor speed, direction, servo angle, command byte)，呼叫 `app_control.ino` 中對應的硬體控制函式。 ✅
    *   **錯誤檢測實作**：
        *   完善 `getErrorCode()` 函式，加入實際的通訊錯誤、感測器錯誤、馬達/執行器錯誤檢測邏輯。 ✅
        *   在 `syncWithServer()` 中，根據錯誤碼和連線狀態，動態設定 `status_byte` 中的 LED 狀態位元，並呼叫 `setLedStatus()`。
    *   **蜂鳴器控制**：根據 `command_byte` 中的位元，實作蜂鳴器的鳴響行為。 ✅
    *   **LED 覆蓋控制**：根據 `command_byte` 中的位元，實作 LED 的特殊覆蓋行為 (例如閃爍)。 ✅
    *   **整合超音波感測器 (Integrate Ultrasonic Sensor):**
        *   連接 HC-SR04 感測器至數位 I/O 腳位。 ✅
        *   確認 HC-SR04 是使用 I2C 還是 GPIO 介面。 ✅
        *   在韌體中新增讀取距離的函式 (注意 `pulseIn` 超時設定)。 ✅
        *   將距離值加入傳送給後端的 JSON 酬載 (例如 `"u": 55`)。 ✅

**第四階段：Python 後端應用邏輯 (Python Backend Application Logic) - 部分完成 ✅**

5.  **Python 後端 (`main.py`):**
    *   **影像串流處理**：修改 `CameraStreamProcessor`，使其使用動態註冊的 ESP32-S3 IP 位址來開啟影像串流，並進行 OpenCV 處理。 ✅
    *   **控制決策邏輯 (感測器融合)**：
        *   修改 `SyncRequest` Pydantic 模型以接收超音波數據 (`u`)。 ✅
        *   **自主模式 (`AUTONOMOUS`)**:
            *   基於視覺分析的基礎障礙物閃避 (前進、轉彎、後退)。 ✅
            *   整合**超音波**與**熱感應**數據至決策邏輯中，實現感測器融合。 ✅
        *   **避障模式 (`AVOIDANCE`)**:
            *   實作完整的避障模式 (`_generate_avoidance_commands`)，融合視覺、超音波、熱感應等多重感測器數據。 ✅

**第五階段：建構與測試 (Build & Test)**

6.  **Arduino 程式碼建構**：建構 Arduino UNO 和 ESP32 程式碼。 待辦 ⏳
7.  **API 功能排查**：檢查後端 API 是否存在功能重疊或可合併的端點 (例如 `manual_control` 和 `sync` 的指令部分)。 ✅

---

### C. 第六階段：重構與優化 (Refactoring & Optimization)

1.  **UNO 端服務發現 (UNO Side Server Discovery):**
    *   在 Arduino UNO 上實作服務器自動發現機制，以取代目前硬編碼 (hard-coded) 的後端伺服器 IP 位址。
2.  **Python 路徑管理 (Python Path Management):**
    *   改用 `pyproject.toml` 和 `pip install -e .` 的方式來管理專案路徑與依賴，解決潛在的 `sys.path` 問題。
3.  **Arduino 程式碼清理 (Arduino Code Cleanup):**
    *   全面排查 `app_control.ino` 和 `arduino_client.ino`，移除未被使用的函式 (如 `httpGet`) 和變數，以節省記憶體和提高程式碼清晰度。
