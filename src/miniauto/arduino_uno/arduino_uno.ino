/*
  Arduino API Client for Vehicle Control (Memory Optimized)

  這個程式碼（sketch）運行在 Arduino UNO 上，透過 I2C 與 ESP32 模組通訊，
  與 Python FastAPI 伺服器進行互動。它負責發送車輛狀態更新並接收移動命令。

  此版本經過重構，旨在最小化 SRAM 使用量，方法包括：
  1. 使用 F() 巨集將常數字串儲存在 Flash 記憶體中，以節省 SRAM。
  2. 將動態 'String' 物件替換為靜態 'char' 陣列，避免記憶體碎片化。
  3. 透過 I2C 傳輸結構化數據，減少通訊開銷。
  4. 專注於硬體控制，將網路通訊任務轉移至 ESP32。
*/

// --- 引入函式庫 ---
#include <Wire.h> // 引入 Wire 函式庫，用於 I2C 通訊，與熱像儀和 ESP32-CAM 模組通訊。
#include <Melopero_AMG8833.h> // 引入 Melopero_AMG8833 函式庫，用於控制 AMG8833 熱像儀。

#include <FastLED.h> // 引入 FastLED 函式庫，用於控制 WS2812B RGB LED。
#include <Servo.h> // 引入 Servo 函式庫，用於控制舵機。
#include <math.h> // 引入 math 函式庫，提供數學函數，例如三角函數。
#include <Ultrasound.h> // 引入 Ultrasound 函式庫，用於超音波感測器。

// --- 常數定義 ---
#define SERIAL_TEST_MODE 1 // 設定為 1 啟用 Serial 測試模式，0 禁用
#define BUZZER_ENABLE 0
#define IR_IMG_ENABLE 0

// --- I2C 從機位址 ---
#define ESP32_I2C_SLAVE_ADDRESS 0x53 // 定義 ESP32 模組的 I2C 從機位址。

// --- I2C 數據結構定義 (與 ESP32 保持一致) ---
// 定義 UNO 感測器數據的結構體
// 使用 __attribute__((packed)) 確保結構體成員緊密排列，沒有填充位元組
typedef struct __attribute__((packed)) {
  uint8_t status_byte;      // 狀態位元組 (s)
  uint16_t voltage_mv;      // 電壓 (v)，單位毫伏
  int16_t ultrasonic_distance_cm; // 超音波距離 (u)，單位厘米，-1 表示無效
  // 熱成像數據的特徵值
  int16_t thermal_max_temp; // 最高溫度 * 100
  int16_t thermal_min_temp; // 最低溫度 * 100
  uint8_t thermal_hotspot_x; // 最熱點的 X 座標 (0-7)
  uint8_t thermal_hotspot_y; // 最熱點的 Y 座標 (0-7)
} SensorData_t;

// 定義後端控制指令的結構體
typedef struct __attribute__((packed)) {
  uint8_t command_byte;     // 命令位元組 (c)
  int16_t motor_speed;      // 馬達速度 (m)
  int16_t direction_angle;  // 方向角度 (d)
  int16_t servo_angle;      // 舵機角度 (a)
} CommandData_t;

// 計算結構體大小
const size_t SENSOR_DATA_SIZE = sizeof(SensorData_t);
const size_t COMMAND_DATA_SIZE = sizeof(CommandData_t);

// --- 全域變數 ---
SensorData_t mySensorData; // 用於儲存 UNO 自己的感測器數據
CommandData_t receivedCommand; // 用於儲存從 ESP32 接收的控制指令

// --- 引腳定義 ---
const static uint8_t ledPin = 2; // 定義 RGB LED 的資料引腳。
const static uint8_t buzzerPin = 3; // 定義蜂鳴器的引腳。
const static uint8_t servoPin = 5; // 定義舵機的控制引腳。
const static uint8_t motorpwmPin[4] = {10, 9, 6, 11}; // 定義四個馬達的 PWM 控制引腳。
const static uint8_t motordirectionPin[4] = {12, 8, 7, 13}; // 定義四個馬達的方向控制引腳。





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



// --- 全域狀態變數 ---
// 用於比較 I2C 數據是否變化的全域變數
SensorData_t lastSentSensorData = {};
CommandData_t lastReceivedCommand = {};
bool isFirstSync = true;

int g_current_voltage_mv = 0; // 當前電壓，單位毫伏。

bool g_thermal_sensor_error = false; // 熱像儀感測器錯誤狀態。
bool g_vision_module_error = false; // 視覺模組錯誤狀態。
bool g_motor_error = false; // 馬達錯誤狀態。
bool g_communication_error = false; // 通訊錯誤狀態。



// --- 函數宣告 ---
void Motor_Init(void); // 馬達初始化函數。
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot); // 速度控制器函數。
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3); // 設定馬達速度和方向函數。
void PWM_Out(uint8_t PWM_Pin, int8_t DutyCycle); // PWM 輸出函數。




void setLedStatus(uint8_t led_code); // 設定 LED 狀態函數。
void controlBuzzer(uint8_t buzzer_code); // 控制蜂鳴器函數。
uint8_t getBatteryLevelCode(); // 獲取電池電量代碼函數。
uint8_t getErrorCode(); // 獲取錯誤代碼函數。
uint8_t getLedStatusCode(uint8_t current_error_code, bool is_busy); // 獲取 LED 狀態代碼函數。
void syncWithServer(); // 與伺服器同步函數。
void Rgb_Show(uint8_t rValue, uint8_t gValue, uint8_t bValue); // 顯示 RGB 顏色函數。
void handleSerialCommand(); // 處理序列埠命令函數。

// --- 設定 (Setup) ---
void setup() {
  Serial.begin(9600); // 初始化硬體序列埠，波特率為 9600，用於與電腦通訊。
  while (!Serial) {} // 等待序列埠連接。
  Serial.println(F("Arduino UNO 已準備就緒。")); // 列印準備就緒訊息。

  

  Motor_Init(); // 初始化馬達。
  myservo.attach(servoPin); // 將舵機連接到指定的引腳。
  myservo.write(90); // 將舵機設置到 90 度（通常是中間位置）。

#if BUZZER_ENABLE
  tone(buzzerPin, 1200, 100); // 蜂鳴器發出 1200 Hz 的聲音，持續 100 毫秒。
#endif

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


}

// --- 主迴圈 (Main Loop) ---
void loop() {
  unsigned long currentTime = millis(); // 獲取當前時間（毫秒）。

#if SERIAL_TEST_MODE
  handleSerialCommand(); // 處理序列埠命令 (測試模式)
#endif

  if (currentTime - lastCommandPollTime >= commandPollInterval) { // 如果距離上次命令輪詢時間超過設定間隔。
    syncWithServer(); // 與伺服器同步資料。
    lastCommandPollTime = currentTime; // 更新上次命令輪詢時間。
  }
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
#if BUZZER_ENABLE
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
  #endif
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

uint8_t getLedStatusCode(uint8_t current_error_code, bool is_busy) { // 獲取 LED 狀態代碼。
  if (current_error_code != 0) return 2; // 如果有錯誤，LED 顯示紅色。
  if (is_busy) return 3; // 如果忙碌，LED 顯示藍色。
  return 1; // 預設情況下，LED 顯示綠色。
}

// --- I2C 通訊 ---

  // --- I2C 通訊 ---

// --- 主同步函數 ---
void syncWithServer() { // 與 ESP32 進行數據同步
  // 1. 收集感測器數據
  mySensorData.status_byte = 0;
  mySensorData.status_byte |= getBatteryLevelCode();
  mySensorData.ultrasonic_distance_cm = ultrasound.GetDistance();

#if IR_IMG_ENABLE
  bool include_thermal = false;
  static unsigned long lastThermalSendTime = 0;
  if (millis() - lastThermalSendTime >= 1000) {
    if (sensor.updatePixelMatrix() == 0) {
      g_thermal_sensor_error = false;
      include_thermal = true;
      lastThermalSendTime = millis();
      for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
          mySensorData.thermal_matrix_flat[i * 8 + j] = sensor.pixelMatrix[i][j];
        }
      }
    } else {
      g_thermal_sensor_error = true;
    }
  }
#else
  g_thermal_sensor_error = true;
#endif

  if (!g_thermal_sensor_error) mySensorData.status_byte |= (1 << 2);
  uint8_t error_code = getErrorCode();
  mySensorData.status_byte |= (error_code << 4);

  // 2. 透過 I2C 分塊推送感測器數據給 ESP32
  Wire.beginTransmission(ESP32_I2C_SLAVE_ADDRESS);
  byte *dataPtr = (byte*)&mySensorData;
  size_t bytesSent = Wire.write(dataPtr, SENSOR_DATA_SIZE);
  byte i2c_tx_status = Wire.endTransmission();

  if (i2c_tx_status == 0 && bytesSent == SENSOR_DATA_SIZE) {
    if (isFirstSync || memcmp(&mySensorData, &lastSentSensorData, SENSOR_DATA_SIZE) != 0) {
      Serial.println(F("I2C TX -> Sent updated sensor data to ESP32."));
      memcpy(&lastSentSensorData, &mySensorData, SENSOR_DATA_SIZE);
    } else {
      Serial.print(F("t")); // 數據未變，精簡輸出
    }
  } else {
    Serial.print(F("I2C TX -> Error: ")); Serial.print(i2c_tx_status);
    Serial.print(F(", Bytes Sent: ")); Serial.println(bytesSent);
  }

  // 3. 透過 I2C 從 ESP32 拉取控制指令
  uint8_t bytesRead = Wire.requestFrom(ESP32_I2C_SLAVE_ADDRESS, COMMAND_DATA_SIZE);

  if (bytesRead == COMMAND_DATA_SIZE) {
    byte buffer[COMMAND_DATA_SIZE];
    Wire.readBytes(buffer, COMMAND_DATA_SIZE);
    
    if (isFirstSync || memcmp(buffer, &lastReceivedCommand, COMMAND_DATA_SIZE) != 0) {
      memcpy(&receivedCommand, buffer, COMMAND_DATA_SIZE);
      memcpy(&lastReceivedCommand, buffer, COMMAND_DATA_SIZE);

      Serial.println();
      Serial.print(F("I2C RX <- Received new command: c=")); Serial.print(receivedCommand.command_byte);
      Serial.print(F(", m=")); Serial.print(receivedCommand.motor_speed);
      Serial.print(F(", d=")); Serial.print(receivedCommand.direction_angle);
      Serial.print(F(", a=")); Serial.println(receivedCommand.servo_angle);

      // 執行指令
      controlBuzzer(receivedCommand.command_byte & 0b11);
      Velocity_Controller(receivedCommand.direction_angle, receivedCommand.motor_speed, 0);
      myservo.write(receivedCommand.servo_angle);
      uint8_t override_led_code = (receivedCommand.command_byte >> 2) & 0b11;
      uint8_t final_led_code = override_led_code != 0 ? override_led_code : getLedStatusCode(error_code, false);
      setLedStatus(final_led_code);
    } else {
      Serial.print(F("r")); // 數據未變，精簡輸出
    }
  } else {
    Serial.print(F("E")); // 錯誤
  }
  
  isFirstSync = false; // 第一次同步完成
  Serial.println(); // 換行保持輸出整潔
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

// --- 序列埠命令處理函數 (僅用於測試) ---
void handleSerialCommand() {
  if (Serial.available()) {
    String commandString = Serial.readStringUntil('\n');
    commandString.trim();
    Serial.print(F("Received serial command: "));
    Serial.println(commandString);

    // 解析命令格式: c,m,d,a (command_byte, motor_speed, direction_angle, servo_angle)
    int firstComma = commandString.indexOf(',');
    int secondComma = commandString.indexOf(',', firstComma + 1);
    int thirdComma = commandString.indexOf(',', secondComma + 1);

    if (firstComma != -1 && secondComma != -1 && thirdComma != -1) {
      receivedCommand.command_byte = commandString.substring(0, firstComma).toInt();
      receivedCommand.motor_speed = commandString.substring(firstComma + 1, secondComma).toInt();
      receivedCommand.direction_angle = commandString.substring(secondComma + 1, thirdComma).toInt();
      receivedCommand.servo_angle = commandString.substring(thirdComma + 1).toInt();

      Serial.print(F("Parsed command: c=")); Serial.print(receivedCommand.command_byte);
      Serial.print(F(", m=")); Serial.print(receivedCommand.motor_speed);
      Serial.print(F(", d=")); Serial.print(receivedCommand.direction_angle);
      Serial.print(F(", a=")); Serial.println(receivedCommand.servo_angle);

      // 執行指令 (與 I2C 接收到的指令處理邏輯相同)
      controlBuzzer(receivedCommand.command_byte & 0b11);
      Velocity_Controller(receivedCommand.direction_angle, receivedCommand.motor_speed, 0);
      myservo.write(receivedCommand.servo_angle);

      uint8_t error_code = getErrorCode(); // 獲取當前錯誤代碼
      uint8_t override_led_code = (receivedCommand.command_byte >> 2) & 0b11;
      uint8_t final_led_code = override_led_code != 0 ? override_led_code : getLedStatusCode(error_code, false);
      setLedStatus(final_led_code);

    } else {
      Serial.println(F("Invalid serial command format. Expected c,m,d,a"));
    }
  }
}