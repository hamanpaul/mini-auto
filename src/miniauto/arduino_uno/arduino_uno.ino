/*
  Arduino API Client for Vehicle Control (Memory Optimized)

  這個程式碼（sketch）運行在 Arduino UNO 上，透過 AT 命令控制 ESP-01S 模組，
  與 Python FastAPI 伺服器進行互動。它負責發送車輛狀態更新並接收移動命令。

  此版本經過重構，旨在最小化 SRAM 使用量，方法包括：
  1. 使用 F() 巨集將常數字串儲存在 Flash 記憶體中，以節省 SRAM。
  2. 將動態的 'String' 物件替換為靜態的 'char' 陣列，避免記憶體碎片化。
  3. 串流 HTTP 請求，而不是在記憶體中構建整個請求，進一步減少記憶體佔用。
  4. 集中定義 AT 命令，以便於維護和管理。
  5. 手動構建和解析 JSON，完全移除 Arduino_JSON 函式庫的開銷，進一步優化記憶體。
*/

// --- 引入函式庫 ---
#include <Wire.h> // 引入 Wire 函式庫，用於 I2C 通訊，與熱像儀和 ESP32-CAM 模組通訊。
#include <Melopero_AMG8833.h> // 引入 Melopero_AMG8833 函式庫，用於控制 AMG8833 熱像儀。
#include <SoftwareSerial.h> // 引入 SoftwareSerial 函式庫，用於軟體模擬序列埠，與 ESP-01S 模組通訊。
#include <FastLED.h> // 引入 FastLED 函式庫，用於控制 WS2812B RGB LED。
#include <Servo.h> // 引入 Servo 函式庫，用於控制舵機。
#include <math.h> // 引入 math 函式庫，提供數學函數，例如三角函數。
#include <Ultrasound.h> // 引入 Ultrasound 函式庫，用於超音波感測器。

// --- 常數定義 ---

// --- I2C 從機位址 ---
#define ESP32_I2C_SLAVE_ADDRESS 0x53 // 定義 ESP32 模組的 I2C 從機位址。

// --- 引腳定義 ---
const static uint8_t ledPin = 2; // 定義 RGB LED 的資料引腳。
const static uint8_t buzzerPin = 3; // 定義蜂鳴器的引腳。
const static uint8_t servoPin = 5; // 定義舵機的控制引腳。
const static uint8_t motorpwmPin[4] = {10, 9, 6, 11}; // 定義四個馬達的 PWM 控制引腳。
const static uint8_t motordirectionPin[4] = {12, 8, 7, 13}; // 定義四個馬達的方向控制引腳。

// --- 網路配置 ---
const char ssid[] PROGMEM = "Hcedu01"; // WiFi 網路名稱 (SSID)，儲存在 Flash 記憶體中。
const char password[] PROGMEM = "035260089"; // WiFi 密碼，儲存在 Flash 記憶體中。
char g_server_ip[16] = ""; // 伺服器 IP 位址的緩衝區，例如 "192.168.1.100"。
int g_server_port = 8000; // 伺服器埠號。

// --- AT 命令定義 (用於 ESP-01S) ---
// 儲存在 PROGMEM (Flash) 中以節省 SRAM。
const char AT_CMD[] PROGMEM = "AT"; // 基本 AT 命令，用於檢查模組是否回應。
const char AT_RST[] PROGMEM = "AT+RST"; // 重置 ESP-01S 模組。
const char AT_CWMODE[] PROGMEM = "AT+CWMODE=1"; // 設定 WiFi 模式為站點模式 (Station Mode)。
const char AT_CIPMUX[] PROGMEM = "AT+CIPMUX=1"; // 設定多連接模式。
const char AT_CIPSTART_UDP[] PROGMEM = "AT+CIPSTART=0,\"UDP\",\"0.0.0.0\",0,5005,0"; // 啟動 UDP 連接，用於伺服器發現。
const char AT_CWJAP_PART1[] PROGMEM = "AT+CWJAP=\""; // 連接 WiFi 命令的第一部分。
const char AT_CWJAP_PART2[] PROGMEM = "\",\""; // 連接 WiFi 命令的第二部分。
const char AT_CWJAP_PART3[] PROGMEM = "\""; // 連接 WiFi 命令的第三部分。
const char AT_CIFSR[] PROGMEM = "AT+CIFSR"; // 獲取本地 IP 位址。
const char AT_CIPSTART_TCP[] PROGMEM = "AT+CIPSTART=\"TCP\",\""; // 啟動 TCP 連接命令的部分。
const char AT_CIPSEND[] PROGMEM = "AT+CIPSEND="; // 發送資料命令。
const char AT_CIPCLOSE[] PROGMEM = "AT+CIPCLOSE"; // 關閉 TCP 連接命令。

// --- 硬體與感測器物件 ---
Melopero_AMG8833 sensor; // 創建 AMG8833 熱像儀感測器物件。
Servo myservo; // 創建舵機物件。
Ultrasound ultrasound; // 創建超音波感測器物件。
static CRGB rgbs[1]; // 創建一個 CRGB 陣列，用於 FastLED 庫控制 RGB LED。

// --- 時序控制 ---
const static int pwmFrequency = 500;                /* PWM 頻率，單位是赫茲 */
const static int period = 1000000 / pwmFrequency;  /* PWM 週期，單位是微秒 */
static uint32_t previousTime_us = 0;          /* 上一次的微秒計數時間間隔，用於非阻塞延時 */
unsigned long lastCommandPollTime = 0; // 上次輪詢命令的時間。
const long commandPollInterval = 200; // 命令輪詢間隔，單位毫秒。

// --- ESP-01S 的軟體序列埠 ---
// Arduino UNO RX = Pin 2, UNO TX = Pin 3
SoftwareSerial espSerial(2, 3); // 創建 SoftwareSerial 物件，用於與 ESP-01S 模組通訊。

// --- 全域狀態變數 ---
int g_current_voltage_mv = 0; // 當前電壓，單位毫伏。
bool g_is_wifi_connected = false; // WiFi 連接狀態。
bool g_thermal_sensor_error = false; // 熱像儀感測器錯誤狀態。
bool g_vision_module_error = false; // 視覺模組錯誤狀態。
bool g_motor_error = false; // 馬達錯誤狀態。
bool g_communication_error = false; // 通訊錯誤狀態。

// --- 緩衝區 ---
// 全域緩衝區，用於避免在迴圈中頻繁分配堆疊記憶體，減少記憶體碎片化。
char esp_response_buffer[96]; // 用於儲存 AT 命令回應的緩衝區。
char http_response_buffer[96]; // 用於解析伺服器 HTTP 回應的緩衝區。

// --- 函數宣告 ---
void Motor_Init(void); // 馬達初始化函數。
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot); // 速度控制器函數。
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3); // 設定馬達速度和方向函數。
void PWM_Out(uint8_t PWM_Pin, int8_t DutyCycle); // PWM 輸出函數。
void setupEsp01s(); // ESP-01S 模組設定函數。
bool httpPost(const char* path, char* response_buffer, int buffer_len); // HTTP POST 請求函數。
bool sendAtCommand(const char* command, const int timeout, char* response_buffer, int buffer_len, bool is_progmem = true); // 發送 AT 命令函數。
bool getEsp32IpAddress(char* ip_buffer, int buffer_len); // 獲取 ESP32 IP 位址函數。
void setLedStatus(uint8_t led_code); // 設定 LED 狀態函數。
void controlBuzzer(uint8_t buzzer_code); // 控制蜂鳴器函數。
uint8_t getBatteryLevelCode(); // 獲取電池電量代碼函數。
uint8_t getErrorCode(); // 獲取錯誤代碼函數。
uint8_t getLedStatusCode(bool is_wifi_connected, uint8_t current_error_code, bool is_busy); // 獲取 LED 狀態代碼函數。
void syncWithServer(); // 與伺服器同步函數。
void Rgb_Show(uint8_t rValue, uint8_t gValue, uint8_t bValue); // 顯示 RGB 顏色函數。
int get_json_int_value(const char* json, const char* key); // 從 JSON 字串中獲取整數值函數。

// --- 設定 (Setup) ---
void setup() {
  Serial.begin(9600); // 初始化硬體序列埠，波特率為 9600，用於與電腦通訊。
  while (!Serial) {} // 等待序列埠連接。
  Serial.println(F("Arduino UNO 已準備就緒。")); // 列印準備就緒訊息。

  espSerial.begin(9600); // 初始化軟體序列埠，波特率為 9600，用於與 ESP-01S 通訊。
  Serial.println(F("ESP-01S 序列埠已啟動，波特率 9600。")); // 列印 ESP-01S 序列埠啟動訊息。

  Motor_Init(); // 初始化馬達。
  myservo.attach(servoPin); // 將舵機連接到指定的引腳。
  myservo.write(90); // 將舵機設置到 90 度（通常是中間位置）。

  tone(buzzerPin, 1200, 100); // 蜂鳴器發出 1200 Hz 的聲音，持續 100 毫秒。

  FastLED.addLeds<WS2812, ledPin, RGB>(rgbs, 1); // 將 WS2812B LED 添加到 FastLED 庫中，指定引腳和 LED 數量。
  FastLED.setBrightness(50); // 設定 LED 的亮度為 50（最大 255）。

  Serial.println(F("正在初始化 I2C 和 AMG8833 感測器...")); // 列印初始化訊息。
  Wire.begin(); // 啟動 I2C 通訊。
  int sensorStatus = sensor.initI2C(AMG8833_I2C_ADDRESS_B, Wire); // 初始化 AMG8833 感測器，指定 I2C 位址和 Wire 物件。
  if (sensorStatus != 0) { // 如果感測器初始化失敗。
    Serial.print(F("感測器初始化失敗，錯誤碼: ")); // 列印錯誤訊息。
    Serial.println(sensor.getErrorDescription(sensorStatus)); // 列印錯誤描述。
  } else { // 如果感測器初始化成功。
    Serial.println(F("感測器初始化成功。")); // 列印成功訊息。
    sensor.setFPSMode(FPS_MODE::FPS_10); // 設定感測器幀率模式為 10 FPS。
  }

  setupEsp01s(); // 設定 ESP-01S 模組。
}

// --- 主迴圈 (Main Loop) ---
void loop() {
  unsigned long currentTime = millis(); // 獲取當前時間（毫秒）。

  if (currentTime - lastCommandPollTime >= commandPollInterval) { // 如果距離上次命令輪詢時間超過設定間隔。
    if (g_server_ip[0] != '\0') { // 如果伺服器 IP 已被發現。
      syncWithServer(); // 與伺服器同步資料。
    } else { // 如果伺服器 IP 尚未被發現。
      Serial.println(F("伺服器 IP 尚未發現。正在等待...")); // 列印等待訊息。
    }
    lastCommandPollTime = currentTime; // 更新上次命令輪詢時間。
  }

  // 檢查是否有傳入的 UDP 廣播，用於伺服器發現。
  if (espSerial.available()) { // 如果軟體序列埠有可讀取的資料。
    static char udp_buffer[128]; // 靜態緩衝區，用於儲存 UDP 資料。
    static int udp_buffer_idx = 0; // 緩衝區索引。
    char c = espSerial.read(); // 讀取一個字元。
    
    if (c != '\n' && udp_buffer_idx < (int)sizeof(udp_buffer) - 1) { // 如果不是換行符且緩衝區未滿。
      udp_buffer[udp_buffer_idx++] = c; // 將字元存入緩衝區並增加索引。
    } else { // 如果是換行符或緩衝區已滿。
      udp_buffer[udp_buffer_idx] = '\0'; // 在緩衝區末尾添加空字元，使其成為有效的字串。
      udp_buffer_idx = 0; // 重置緩衝區索引。

      // 範例 UDP 訊息格式: +IPD,0,20:MINIAUTO_SERVER_IP:192.168.1.100:8000
      char* data_start = strstr(udp_buffer, "MINIAUTO_SERVER_IP:"); // 查找伺服器 IP 標識。
      if (data_start) { // 如果找到標識。
        data_start += strlen("MINIAUTO_SERVER_IP:"); // 跳過標識，指向 IP 位址的開頭。
        char* port_start = strrchr(data_start, ':'); // 從 IP 位址字串的末尾查找埠號分隔符。
        if (port_start) { // 如果找到埠號分隔符。
          *port_start = '\0'; // 將埠號分隔符替換為空字元，以截斷 IP 位址字串。
          int discovered_port = atoi(port_start + 1); // 將埠號字串轉換為整數。
          
          if (strcmp(g_server_ip, data_start) != 0 || g_server_port != discovered_port) { // 如果發現的 IP 或埠號與當前儲存的不同。
            strncpy(g_server_ip, data_start, sizeof(g_server_ip) - 1); // 複製發現的 IP 位址到全域變數。
            g_server_ip[sizeof(g_server_ip) - 1] = '\0'; // 確保字串以空字元結尾。
            g_server_port = discovered_port; // 更新全域埠號。
            
            Serial.print(F("發現伺服器 IP: ")); // 列印發現的伺服器 IP。
            Serial.print(g_server_ip);
            Serial.print(F(", 埠號: ")); // 列印埠號。
            Serial.println(g_server_port);
          }
        }
      }
    }
  }
}

// --- ESP-01S 設定 ---
void setupEsp01s() {
  Serial.println(F("--- 正在設定 ESP-01S ---")); // 列印設定訊息。
  
  sendAtCommand(AT_CMD, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 發送 AT 命令，檢查模組是否回應。
  delay(1000); // 延遲 1 秒。

  sendAtCommand(AT_RST, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 發送重置命令。
  delay(2000); // 延遲 2 秒。

  sendAtCommand(AT_CWMODE, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 設定 WiFi 模式為站點模式。
  delay(1000); // 延遲 1 秒。

  sendAtCommand(AT_CIPMUX, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 設定多連接模式。
  delay(1000); // 延遲 1 秒。

  sendAtCommand(AT_CIPSTART_UDP, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 啟動 UDP 連接。
  delay(1000); // 延遲 1 秒。

  // 連接到 WiFi。
  char cmd_buffer[128]; // 命令緩衝區。
  char temp_ssid[32]; // 臨時 SSID 緩衝區。
  char temp_pass[32]; // 臨時密碼緩衝區。
  strcpy_P(temp_ssid, ssid); // 從 Flash 記憶體複製 SSID 到臨時緩衝區。
  strcpy_P(temp_pass, password); // 從 Flash 記憶體複製密碼到臨時緩衝區。
  snprintf_P(cmd_buffer, sizeof(cmd_buffer), PSTR("%S%s%S%s%S"), AT_CWJAP_PART1, temp_ssid, AT_CWJAP_PART2, temp_pass, AT_CWJAP_PART3); // 格式化連接 WiFi 命令。
  sendAtCommand(cmd_buffer, 7000, esp_response_buffer, sizeof(esp_response_buffer), false); // 發送連接 WiFi 命令，等待 7 秒。
  delay(5000); // 延遲 5 秒，等待連接建立。

  // 檢查是否連接成功。
  sendAtCommand(AT_CIFSR, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 獲取本地 IP 位址，檢查連接狀態。
  if (strstr(esp_response_buffer, "ERROR") == NULL && strstr(esp_response_buffer, "STAIP") != NULL) { // 如果回應中沒有 "ERROR" 且包含 "STAIP"。
    Serial.println(F("ESP-01S 已連接到 WiFi！")); // 列印連接成功訊息。
    g_is_wifi_connected = true; // 設定 WiFi 連接狀態為 true。
  } else { // 如果連接失敗。
    Serial.println(F("ESP-01S 連接到 WiFi 失敗。")); // 列印連接失敗訊息。
    g_is_wifi_connected = false; // 設定 WiFi 連接狀態為 false。
  }
  Serial.println(F("---------------------------")); // 列印分隔線。
}

// --- LED 與蜂鳴器控制 ---
void Rgb_Show(uint8_t rValue, uint8_t gValue, uint8_t bValue) { // 設定 RGB LED 的顏色。
  rgbs[0].setRGB(rValue, gValue, bValue); // 設定第一個 LED 的 RGB 值。
  FastLED.show(); // 更新 LED 顯示。
}

void setLedStatus(uint8_t led_code) { // 根據代碼設定 LED 狀態。
  switch (led_code) { // 根據 led_code 的值執行不同操作。
    case 0: Rgb_Show(0, 0, 0); break;       // 關閉 LED (黑色)
    case 1: Rgb_Show(0, 255, 0); break;     // 綠色 LED
    case 2: Rgb_Show(255, 0, 0); break;     // 紅色 LED
    case 3: Rgb_Show(0, 0, 255); break;     // 藍色 LED
    default: Rgb_Show(0, 0, 0); break; // 預設情況下關閉 LED。
  }
}

void controlBuzzer(uint8_t buzzer_code) { // 控制蜂鳴器。
  static unsigned long lastBuzzerActionTime = 0; // 靜態變數，記錄上次蜂鳴器動作的時間。
  static bool buzzerState = false; // 靜態變數，記錄蜂鳴器狀態。
  const unsigned long SHORT_BEEP_DURATION = 100; // 短蜂鳴持續時間。
  const unsigned long LONG_BEEP_DURATION = 500; // 長蜂鳴持續時間。
  const unsigned long CONTINUOUS_BEEP_INTERVAL = 1000; // 連續蜂鳴間隔。

  unsigned long currentTime = millis(); // 獲取當前時間。

  switch (buzzer_code) { // 根據 buzzer_code 的值執行不同操作。
    case 0: // 關閉蜂鳴器。
      noTone(buzzerPin); // 停止發聲。
      buzzerState = false; // 設定蜂鳴器狀態為關閉。
      break;
    case 1: // 短蜂鳴 (一次性觸發)。
      if (currentTime - lastBuzzerActionTime > SHORT_BEEP_DURATION) { // 如果距離上次動作時間超過短蜂鳴持續時間。
        tone(buzzerPin, 1000, SHORT_BEEP_DURATION); // 發出 1000 Hz 的聲音，持續短蜂鳴時間。
        lastBuzzerActionTime = currentTime; // 更新上次動作時間。
      }
      break;
    case 2: // 長蜂鳴 (一次性觸發)。
      if (currentTime - lastBuzzerActionTime > LONG_BEEP_DURATION) { // 如果距離上次動作時間超過長蜂鳴持續時間。
        tone(buzzerPin, 1000, LONG_BEEP_DURATION); // 發出 1000 Hz 的聲音，持續長蜂鳴時間。
        lastBuzzerActionTime = currentTime; // 更新上次動作時間。
      }
      break;
    case 3: // 連續蜂鳴 (切換)。
      if (currentTime - lastBuzzerActionTime >= CONTINUOUS_BEEP_INTERVAL / 2) { // 如果距離上次動作時間超過連續蜂鳴間隔的一半。
        if (buzzerState) { // 如果蜂鳴器正在發聲。
          noTone(buzzerPin); // 停止發聲。
        } else { // 如果蜂鳴器未發聲。
          tone(buzzerPin, 1000); // 發出 1000 Hz 的聲音。
        }
        buzzerState = !buzzerState; // 切換蜂鳴器狀態。
        lastBuzzerActionTime = currentTime; // 更新上次動作時間。
      }
      break;
    default:
      noTone(buzzerPin); // 預設情況下停止發聲。
      buzzerState = false; // 設定蜂鳴器狀態為關閉。
      break;
  }
}

// --- 狀態計算 ---
uint8_t getBatteryLevelCode() { // 獲取電池電量代碼。
  const float VOLTAGE_CONVERSION_FACTOR = 0.02989; // 電壓轉換係數。
  const float BATTERY_FULL_VOLTAGE = 8.4; // 電池滿電電壓。
  const float BATTERY_EMPTY_VOLTAGE = 7.0; // 電池空電電壓。

  int rawVoltage = analogRead(A3); // 讀取 A3 引腳的原始類比電壓值。
  float voltage = rawVoltage * VOLTAGE_CONVERSION_FACTOR; // 將原始值轉換為實際電壓。
  g_current_voltage_mv = (int)(voltage * 100); // 將電壓轉換為毫伏並儲存。

  float percentage = ((voltage - BATTERY_EMPTY_VOLTAGE) / (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE)) * 100.0; // 計算電池電量百分比。
  
  if (percentage > 100.0) percentage = 100.0; // 限制百分比不超過 100%。
  if (percentage < 0.0) percentage = 0.0; // 限制百分比不低於 0%。

  if (percentage >= 80.0) return 3; // 11: 健康 (Healthy)
  if (percentage >= 40.0) return 2; // 10: 正常 (Okay)
  if (percentage >= 15.0) return 1; // 01: 低電量 (Low)
  return 0; // 00: 危急 (Critical)
}

uint8_t getErrorCode() { // 獲取錯誤代碼。
  if (g_motor_error) return 3; // 如果馬達有錯誤，返回 3。
  if (g_thermal_sensor_error || g_vision_module_error) return 2; // 如果熱像儀或視覺模組有錯誤，返回 2。
  if (g_communication_error) return 1; // 如果通訊有錯誤，返回 1。
  return 0; // 沒有錯誤，返回 0。
}

uint8_t getLedStatusCode(bool is_wifi_connected, uint8_t current_error_code, bool is_busy) { // 獲取 LED 狀態代碼。
  if (!is_wifi_connected) return 0; // 如果 WiFi 未連接，LED 關閉。
  if (current_error_code != 0) return 2; // 如果有錯誤，LED 顯示紅色。
  if (is_busy) return 3; // 如果忙碌，LED 顯示藍色。
  return 1; // 預設情況下，LED 顯示綠色。
}

// --- I2C 通訊 ---
bool getEsp32IpAddress(char* ip_buffer, int buffer_len) { // 獲取 ESP32 的 IP 位址。
  Wire.requestFrom((uint8_t)ESP32_I2C_SLAVE_ADDRESS, (uint8_t)(buffer_len - 1)); // 從 ESP32 從機請求資料。
  long startTime = millis(); // 記錄開始時間。
  int i = 0; // 緩衝區索引。
  ip_buffer[0] = '\0'; // 初始化緩衝區為空字串。
  while (Wire.available() && (millis() - startTime < 100)) { // 當有資料可用且未超時。
    ip_buffer[i++] = Wire.read(); // 讀取資料並存入緩衝區。
  }
  ip_buffer[i] = '\0'; // 在緩衝區末尾添加空字元。

  if (i > 7 && strchr(ip_buffer, '.') != NULL) { // 如果讀取到的字元數大於 7 且包含點號（基本 IP 格式檢查）。
    Serial.print(F("透過 I2C 接收到 ESP32 IP: ")); // 列印接收到的 IP。
    Serial.println(ip_buffer);
    return true; // 返回 true 表示成功。
  } else { // 如果 IP 無效。
    Serial.println(F("透過 I2C 獲取有效 ESP32 IP 失敗。")); // 列印失敗訊息。
    ip_buffer[0] = '\0'; // 清空緩衝區。
    return false; // 返回 false 表示失敗。
  }
}

// --- 主同步函數 ---
void syncWithServer() { // 與伺服器同步資料。
  // 1. 發送 POST 請求並處理回應。
  if (httpPost("/api/sync", http_response_buffer, sizeof(http_response_buffer))) { // 如果 HTTP POST 請求成功。
    Serial.print(F("接收到回應: ")); // 列印接收到的回應。
    Serial.println(http_response_buffer);
    g_communication_error = false; // 清除通訊錯誤標誌。

    // 手動解析 JSON 回應。
    uint8_t command_byte = get_json_int_value(http_response_buffer, "\"c\":"); // 獲取命令位元組。
    int motor_speed = get_json_int_value(http_response_buffer, "\"m\":"); // 獲取馬達速度。
    int direction_angle = get_json_int_value(http_response_buffer, "\"d\":"); // 獲取方向角度。
    int servo_angle = get_json_int_value(http_response_buffer, "\"a\":"); // 獲取舵機角度。

    controlBuzzer(command_byte & 0b11); // 控制蜂鳴器，使用命令位元組的低兩位。
    Velocity_Controller(direction_angle, motor_speed, 0); // 控制車輛速度和方向。
    myservo.write(servo_angle); // 設定舵機角度。

    uint8_t error_code = getErrorCode(); // 獲取當前錯誤代碼。
    uint8_t override_led_code = (command_byte >> 2) & 0b11; // 從命令位元組中提取覆蓋 LED 代碼。
    uint8_t final_led_code = override_led_code != 0 ? override_led_code : getLedStatusCode(g_is_wifi_connected, error_code, false); // 根據覆蓋代碼或當前狀態獲取最終 LED 代碼。
    setLedStatus(final_led_code); // 設定 LED 狀態。

  } else { // 如果 HTTP POST 請求失敗。
    Serial.println(F("沒有回應或 HTTP POST 失敗。")); // 列印失敗訊息。
    g_communication_error = true; // 設定通訊錯誤標誌。
    setLedStatus(0); // 失敗時關閉 LED。
  }
  Serial.println(F("---------------------------")); // 列印分隔線。
}

// --- 網路通訊 ---
bool httpPost(const char* path, char* response_buffer, int buffer_len) { // 發送 HTTP POST 請求。
  // 1. 收集狀態資料。
  uint8_t status_byte = 0; // 初始化狀態位元組。
  char current_esp32_ip[16]; // ESP32 IP 緩衝區。
  status_byte |= getBatteryLevelCode(); // 將電池電量代碼添加到狀態位元組。
  int ultrasonic_distance = ultrasound.GetDistance(); // 讀取超音波距離。
  g_vision_module_error = !getEsp32IpAddress(current_esp32_ip, sizeof(current_esp32_ip)); // 獲取 ESP32 IP 並更新視覺模組錯誤狀態。
  if (!g_vision_module_error) status_byte |= (1 << 3); // 如果視覺模組沒有錯誤，設定狀態位元組的第 3 位。

  bool include_thermal = false; // 是否包含熱像儀資料的標誌。
  static unsigned long lastThermalSendTime = 0; // 上次發送熱像儀資料的時間。
  if (millis() - lastThermalSendTime >= 1000) { // 如果距離上次發送熱像儀資料超過 1 秒。
    if (sensor.updatePixelMatrix() == 0) { // 如果熱像儀資料更新成功。
      g_thermal_sensor_error = false; // 清除熱像儀錯誤標誌。
      include_thermal = true; // 設定包含熱像儀資料。
      lastThermalSendTime = millis(); // 更新上次發送時間。
    } else { // 如果熱像儀資料更新失敗。
      g_thermal_sensor_error = true; // 設定熱像儀錯誤標誌。
    }
  }
  if (!g_thermal_sensor_error) status_byte |= (1 << 2); // 如果熱像儀沒有錯誤，設定狀態位元組的第 2 位。

  uint8_t error_code = getErrorCode(); // 獲取錯誤代碼。
  status_byte |= (error_code << 4); // 將錯誤代碼添加到狀態位元組的第 4 位開始。
  status_byte |= (0 << 6); // 佔位符。

  // 2. 計算內容長度。
  char temp_buf[10]; // 臨時緩衝區。
  int payload_len = 0; // 有效負載長度。
  payload_len += sprintf(temp_buf, "{\"s\":%d,\"v\":%d", status_byte, g_current_voltage_mv); // 計算狀態和電壓的長度。
  if (ultrasonic_distance > 0) { // 如果超音波距離有效。
    payload_len += sprintf(temp_buf, ",\"u\":%d", ultrasonic_distance); // 添加超音波距離的長度。
  }
  static char last_sent_esp32_ip[16] = ""; // 上次發送的 ESP32 IP。
  if (!g_vision_module_error && strcmp(current_esp32_ip, last_sent_esp32_ip) != 0) { // 如果視覺模組沒有錯誤且 IP 發生變化。
    payload_len += sprintf(temp_buf, ",\"i\":\"%s\"", current_esp32_ip); // 添加 IP 位址的長度。
  }
  if (include_thermal) { // 如果包含熱像儀資料。
    payload_len += 2 + 64 * 5; // "t":[[...]] 大約每個像素 5 個字元 (例如 -1024,)
  }
  payload_len += 1; // 用於閉合大括號。

  // 3. 建立 TCP 連接。
  char cmd_buffer[128]; // 命令緩衝區。
  snprintf_P(cmd_buffer, sizeof(cmd_buffer), PSTR("%S%s\",%d"), AT_CIPSTART_TCP, g_server_ip, g_server_port); // 格式化 TCP 連接命令。
  if (!sendAtCommand(cmd_buffer, 5000, esp_response_buffer, sizeof(esp_response_buffer), false)) { // 發送 TCP 連接命令。
    Serial.println(F("建立 TCP 連接失敗。")); // 列印失敗訊息。
    sendAtCommand(AT_CIPCLOSE, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 關閉連接。
    return false; // 返回 false 表示失敗。
  }

  // 4. 準備 CIPSEND 命令。
  int total_len = strlen_P(PSTR("POST ")) + strlen(path) + strlen_P(PSTR(" HTTP/1.1\r\nHost: ")) + strlen(g_server_ip) + strlen_P(PSTR("\r\nContent-Type: application/json\r\nContent-Length: ")) + 5 + strlen_P(PSTR("\r\nConnection: close\r\n\r\n")) + payload_len; // 計算總請求長度。
  snprintf_P(cmd_buffer, sizeof(cmd_buffer), PSTR("%S%d"), AT_CIPSEND, total_len); // 格式化 CIPSEND 命令。
  if (!sendAtCommand(cmd_buffer, 2000, esp_response_buffer, sizeof(esp_response_buffer), false) || strstr(esp_response_buffer, ">") == NULL) {
      Serial.println(F("CIPSEND 命令失敗。")); // 列印失敗訊息。
      sendAtCommand(AT_CIPCLOSE, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 關閉連接。
      return false; // 返回 false 表示失敗。
  }

  // 5. 發送 HTTP 請求 (串流 JSON 有效負載)。
  espSerial.print(F("POST ")); espSerial.print(path); espSerial.print(F(" HTTP/1.1\r\n")); // 發送 POST 行。
  espSerial.print(F("Host: ")); espSerial.print(g_server_ip); espSerial.print(F("\r\n")); // 發送 Host 頭。
  espSerial.print(F("Content-Type: application/json\r\n")); // 發送 Content-Type 頭。
  espSerial.print(F("Content-Length: ")); espSerial.print(payload_len); espSerial.print(F("\r\n")); // 發送 Content-Length 頭。
  espSerial.print(F("Connection: close\r\n\r\n")); // 發送 Connection 頭和空行，表示頭部結束。
  
  // 手動串流 JSON 有效負載。
  espSerial.print(F("{\"s\":")); espSerial.print(status_byte); espSerial.print(F(",\"v\":")); espSerial.print(g_current_voltage_mv); // 發送狀態和電壓。
  if (ultrasonic_distance > 0) { // 如果超音波距離有效。
    espSerial.print(F(",\"u\":")); espSerial.print(ultrasonic_distance); // 發送超音波距離。
  }
  if (!g_vision_module_error && strcmp(current_esp32_ip, last_sent_esp32_ip) != 0) { // 如果視覺模組沒有錯誤且 IP 發生變化。
    espSerial.print(F(",\"i\":\"")); espSerial.print(current_esp32_ip); espSerial.print(F("\"")); // 發送 IP 位址。
    strcpy(last_sent_esp32_ip, current_esp32_ip); // 更新上次發送的 IP。
  }
  if (include_thermal) { // 如果包含熱像儀資料。
    espSerial.print(F(",\"t\":[")); // 發送熱像儀資料開頭。
    for (int i = 0; i < 8; i++) { // 遍歷熱像儀矩陣的行。
      espSerial.print(F("[")); // 發送行開頭。
      for (int j = 0; j < 8; j++) { // 遍歷熱像儀矩陣的列。
        espSerial.print(sensor.pixelMatrix[i][j]); // 發送像素值。
        if (j < 7) espSerial.print(F(",")); // 如果不是最後一個像素，添加逗號。
      }
      espSerial.print(F("]")); // 發送行結尾。
      if (i < 7) espSerial.print(F(",")); // 如果不是最後一行，添加逗號。
    }
    espSerial.print(F("]")); // 發送熱像儀資料結尾。
  }
  espSerial.print(F("}")); // 發送 JSON 有效負載結尾。

  // 6. 讀取回應。
  unsigned long startTime = millis(); // 記錄開始時間。
  int idx = 0; // 緩衝區索引。
  memset(response_buffer, 0, buffer_len); // 清空回應緩衝區。
  while (millis() - startTime < 5000 && idx < buffer_len - 1) { // 當未超時且緩衝區未滿。
    if (espSerial.available()) { // 如果軟體序列埠有可讀取的資料。
      response_buffer[idx++] = espSerial.read(); // 讀取資料並存入緩衝區。
    }
  }
  
  sendAtCommand(AT_CIPCLOSE, 2000, esp_response_buffer, sizeof(esp_response_buffer)); // 關閉 TCP 連接。

  // 7. 檢查有效回應並提取主體。
  char* body_start = strstr(response_buffer, "\r\n\r\n"); // 查找 HTTP 頭部和主體之間的分隔符。
  if (strstr(response_buffer, "200 OK") != NULL && body_start != NULL) { // 如果回應包含 "200 OK" 且找到主體。
    Serial.println(F("HTTP POST 成功。")); // 列印成功訊息。
    memmove(response_buffer, body_start + 4, strlen(body_start + 4) + 1); // 將主體內容移動到緩衝區開頭。
    return true; // 返回 true 表示成功。
  } else { // 如果 HTTP POST 失敗或沒有 "200 OK"。
    Serial.println(F("HTTP POST 失敗或沒有 200 OK。")); // 列印失敗訊息。
    return false; // 返回 false 表示失敗。
  }
}

bool sendAtCommand(const char* command, const int timeout, char* response_buffer, int buffer_len, bool is_progmem) { // 發送 AT 命令。
  memset(response_buffer, 0, buffer_len); // 清空回應緩衝區。
  
  Serial.print(F("--- AT 命令 ---\n發送: ")); // 列印發送的命令。
  if (is_progmem) { // 如果命令儲存在 Flash 記憶體中。
    char pgm_buffer[128]; // 臨時緩衝區。
    strcpy_P(pgm_buffer, command); // 從 Flash 記憶體複製命令。
    Serial.println(pgm_buffer);
    espSerial.println((const __FlashStringHelper*)command); // 透過軟體序列埠發送命令。
  } else { // 如果命令儲存在 SRAM 中。
    Serial.println(command);
    espSerial.println(command); // 透過軟體序列埠發送命令。
  }

  int idx = 0; // 緩衝區索引。
  unsigned long startTime = millis(); // 記錄開始時間。
  while (millis() - startTime < timeout) { // 當未超時。
    if (espSerial.available() && idx < buffer_len - 1) { // 如果軟體序列埠有可讀取的資料且緩衝區未滿。
      response_buffer[idx++] = espSerial.read(); // 讀取資料並存入緩衝區。
    }
  }
  
  Serial.print(F("接收: ")); // 列印接收到的回應。
  Serial.println(response_buffer);
  Serial.println(F("--------------------")); // 列印分隔線。
  
  return (strstr(response_buffer, "OK") != NULL || strstr(response_buffer, "SEND OK") != NULL || strstr(response_buffer, ">") != NULL || strstr(response_buffer, "no change") != NULL); // 檢查回應是否包含成功標識。
}

// --- 馬達控制函數 ---
void Motor_Init(void) { // 馬達初始化。
  for(uint8_t i = 0; i < 4; i++) { // 遍歷所有馬達。
    pinMode(motordirectionPin[i], OUTPUT); // 設定馬達方向引腳為輸出模式。
    pinMode(motorpwmPin[i], OUTPUT); // 設定馬達 PWM 引腳為輸出模式。
  }
  Velocity_Controller( 0, 0, 0); // 初始設定馬達停止。
}

void Velocity_Controller(uint16_t angle, uint8_t velocity,int8_t rot) { // 速度控制器。
  int8_t velocity_0, velocity_1, velocity_2, velocity_3; // 四個馬達的速度變數。
  float speed = 1; // 速度係數。
  angle += 90; // 調整角度。
  float rad = angle * PI / 180; // 將角度轉換為弧度。
  if (rot == 0) speed = 1; // 如果沒有旋轉，速度係數為 1。
  else speed = 0.5; // 如果有旋轉，速度係數為 0.5。
  velocity /= sqrt(2); // 調整速度。
  velocity_0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot * speed; // 計算馬達 0 的速度。
  velocity_1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot * speed; // 計算馬達 1 的速度。
  velocity_2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot * speed; // 計算馬達 2 的速度。
  velocity_3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot * speed; // 計算馬達 3 的速度。
  Motors_Set(velocity_0, velocity_1, velocity_2, velocity_3); // 設定馬達速度。
}
 
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3) { // 設定馬達速度和方向。
  int8_t pwm_set[4]; // PWM 設定值陣列。
  int8_t motors[4] = { Motor_0, Motor_1, Motor_2, Motor_3}; // 馬達速度陣列。
  bool direction[4] = { 1, 0, 0, 1}; // 馬達方向陣列。
  for(uint8_t i = 0; i < 4; ++i) { // 遍歷所有馬達。
    if(motors[i] < 0) direction[i] = !direction[i]; // 如果速度為負，反轉方向。
    
    pwm_set[i] = abs(motors[i]); // 將速度的絕對值作為 PWM 設定值。

    digitalWrite(motordirectionPin[i], direction[i]); // 設定馬達方向引腳。
    PWM_Out(motorpwmPin[i], pwm_set[i]); // 輸出 PWM 信號。
  }
}

void PWM_Out(uint8_t PWM_Pin, int8_t DutyCycle) { // PWM 輸出函數。
  uint32_t currentTime_us = micros(); // 獲取當前時間（微秒）。
  int highTime = (period/100) * DutyCycle; // 計算高電平時間。

  if ((currentTime_us - previousTime_us) <= (unsigned long)highTime) { // 如果當前時間減去上次時間小於等於高電平時間。
    digitalWrite(PWM_Pin, HIGH); // 設定引腳為高電平。
  } else { // 否則。
    digitalWrite(PWM_Pin, LOW); // 設定引腳為低電平。
  }
  if (currentTime_us - previousTime_us >= (unsigned long)period) { // 如果當前時間減去上次時間大於等於 PWM 週期。
    previousTime_us = currentTime_us; // 更新上次時間。
  }
}

// --- 工具函數 ---
int get_json_int_value(const char* json, const char* key) { // 從 JSON 字串中獲取整數值。
    const char* key_ptr = strstr(json, key); // 查找鍵的指標。
    if (key_ptr == NULL) { // 如果未找到鍵。
        return 0; // 返回 0 表示未找到鍵。
    }
    key_ptr += strlen(key); // 跳過鍵的長度，指向值的開頭。
    return atoi(key_ptr); // 將值字串轉換為整數並返回。
}

