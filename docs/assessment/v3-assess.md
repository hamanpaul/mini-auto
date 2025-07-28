# Miniauto 系統架構升級評估：Wi-Fi 任務從 UNO 轉移至 ESP32 (方案三)

## 1. 背景與問題陳述

Miniauto 專案初期，Arduino UNO 透過 ESP-01S 模組和 AT 命令處理 Wi-Fi 通訊，並與 Python 後端伺服器進行 HTTP 同步。這種架構存在以下問題：

*   **UNO 資源受限**：Arduino UNO 的 SRAM 和 Flash 記憶體非常有限，處理複雜的 Wi-Fi 堆疊、AT 命令解析和 JSON 序列化/反序列化對其造成了沉重負擔，影響了即時控制的穩定性。
*   **通訊效率低**：AT 命令通訊相對低效，且容易出錯。
*   **硬體冗餘**：ESP-01S 模組增加了硬體複雜度和潛在的故障點。

## 2. 解決方案方向：將 Wi-Fi 任務轉移至 ESP32

鑑於 ESP32-CAM 模組本身具備強大的 Wi-Fi 和處理能力，我們決定將所有網路通訊的職責從 Arduino UNO 轉移到 ESP32。UNO 將透過 I2C 與 ESP32 進行通訊，ESP32 則負責處理所有網路相關的任務（Wi-Fi 連接、服務發現、HTTP 通訊）。

## 3. 方案討論與選擇

我們討論了三種主要方案來實現 UNO 和 ESP32 之間的協同工作：

*   **方案一：ESP32 作為 HTTP JSON Payload 代理**
    *   UNO 負責構建和解析 JSON，透過 I2C 將 JSON 字串傳給 ESP32。
    *   ESP32 負責發送完整的 HTTP 請求並處理回應。
    *   **評估**：UNO 仍需處理 JSON，SRAM 負擔較大，I2C 傳輸效率不高。

*   **方案二：ESP32 作為結構化數據轉換器**
    *   UNO 僅傳輸原始的、緊湊的二進位數據給 ESP32。
    *   ESP32 負責將二進位數據轉換為 JSON 發送給後端，並將後端回應的 JSON 轉換為二進位數據傳回給 UNO。
    *   **評估**：UNO 負擔最小化，I2C 傳輸效率高。但 ESP32 的 HTTP 同步行為是被動的，由 UNO 的 I2C 請求觸發，可能導致 UNO 主循環被網路延遲阻塞。

*   **方案三：ESP32 作為獨立同步代理 (Autonomous Sync Agent)**
    *   **核心思想**：ESP32 獨立地、主動地管理與後端伺服器 `/api/sync` 的通訊。UNO 僅透過 I2C 將感測器數據「推送」給 ESP32，並從 ESP32 的 I2C 暫存器中「拉取」控制指令。
    *   **通訊模式**：UNO 和 ESP32 之間採用**非同步推送/拉取**的 I2C 通訊模式。
    *   **評估**：
        *   **優點**：UNO 程式碼最簡潔，完全不涉及網路通訊的複雜性，主循環不會被網路延遲阻塞，保持高即時性。
        *   **缺點**：ESP32 複雜度最高，需要管理自身的同步邏輯、數據聚合、命令緩衝、以及 I2C 暫存器的設計。可能引入輕微的數據陳舊性（但可透過頻率管理）。

**選擇**：經過討論，我們決定採用**方案三**。儘管它增加了 ESP32 的複雜度，但它能最大限度地簡化 Arduino UNO 的程式碼，使其專注於硬體控制，從而提高整個系統的穩定性和即時響應能力。

## 4. 實作計畫 (方案三)

### 4.1 I2C 暫存器定義 (Shared I2C Register Map)

ESP32 作為 I2C 從機 (Slave)，UNO 作為 I2C 主機 (Master)。

| 暫存器位址 (I2C Address) | 讀/寫 | 數據內容 (位元組) | 說明 |
| :----------------------- | :---- | :---------------- | :--- |
| `0x00` (Input Data Register) | 寫入 (UNO -> ESP32) | `SENSOR_DATA_SIZE` 位元組 | UNO 將其感測器數據打包成固定大小的位元組陣列寫入此暫存器。 |
| `0x01` (Output Command Register) | 讀取 (UNO <- ESP32) | `COMMAND_DATA_SIZE` 位元組 | ESP32 將從後端接收到的控制指令打包成固定大小的位元組陣列，UNO 從此暫存器讀取。 |

### 4.2 數據結構定義 (C/C++ for UNO & ESP32)

為了確保數據的緊湊性和一致性，我們將定義 C 結構體來表示感測器數據和控制指令，並使用 `__attribute__((packed))` 確保緊湊打包。

**1. UNO 感測器數據結構 (`SensorData_t`)**

```c++
typedef struct __attribute__((packed)) {
  uint8_t status_byte;      // 狀態位元組 (s)
  uint16_t voltage_mv;      // 電壓 (v)，單位毫伏
  int16_t ultrasonic_distance_cm; // 超音波距離 (u)，單位厘米，-1 表示無效
  int16_t thermal_matrix_flat[64]; // 熱像儀數據 (t) - 8x8 矩陣扁平化
} SensorData_t;

const size_t SENSOR_DATA_SIZE = sizeof(SensorData_t); // 總大小約 133 位元組
```

**2. 後端控制指令結構 (`CommandData_t`)**

```c++
typedef struct __attribute__((packed)) {
  uint8_t command_byte;     // 命令位元組 (c)
  int16_t motor_speed;      // 馬達速度 (m)
  int16_t direction_angle;  // 方向角度 (d)
  int16_t servo_angle;      // 舵機角度 (a)
} CommandData_t;

const size_t COMMAND_DATA_SIZE = sizeof(CommandData_t); // 總大小約 7 位元組
```

## 5. 目前進度

我們已經開始實作，並完成了以下步驟：

*   **ESP32 韌體 (`src/miniauto/esp32_cam/esp32_cam.ino`)**：
    *   已添加必要的函式庫引入 (`HTTPClient.h`, `ArduinoJson.h`, `AsyncUDP.h`)。
    *   已定義 `SensorData_t` 和 `CommandData_t` 結構體及相關全域變數。
    *   已實現 I2C Slave 的 `receiveEvent` 和 `requestEvent` 回調函數。
    *   已在 `setup()` 中初始化 I2C Slave 並註冊回調。
    *   已在 `setup()` 中初始化 UDP 監聽，用於發現後端伺服器 IP。
    *   已在 `setup()` 中初始化並啟動 `esp_timer`，用於獨立的 HTTP 同步循環。
    *   已實現 `http_sync_callback` 函數，負責構建 JSON Payload、發送 HTTP POST、解析回應 JSON 並更新 `currentCommandData`。
    *   已修改 `loop()` 函數，使其保持簡潔，不阻塞。
    *   **已優化 `http_sync_callback` 的日誌輸出，在數據未變化時減少詳細日誌。**
    *   **已修正 `http_sync_callback` 中解析後端 `SyncResponse` 的鍵名錯誤。**
    *   **已修正 `registerCamera` 中發送 JSON 鍵名錯誤。**

*   **Arduino UNO 韌體 (`src/miniauto/arduino_uno/arduino_uno.ino`)**：
    *   已移除所有 Wi-Fi 相關的引入、常數定義、全域變數和函數（如 `SoftwareSerial`、AT 命令、`httpPost`、`get_json_int_value` 等）。
    *   已定義 `SensorData_t` 和 `CommandData_t` 結構體及相關全域變數，與 ESP32 端保持一致。
    *   已修改 `syncWithServer()` 函數，使其透過 I2C 推送感測器數據給 ESP32，並從 ESP32 拉取控制指令。

我們將繼續完成 UNO 韌體的剩餘修改，並進行全面的測試。