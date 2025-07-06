

/*
  Arduino API Client for Vehicle Control

  This sketch runs on an Arduino UNO and controls an ESP-01S module via AT commands
  to interact with a Python FastAPI server. It sends vehicle status updates and 
  receives movement commands.

  HARDWARE:
  - Arduino UNO
  - ESP-01S WiFi Module
  - AMG8833 Thermal Sensor
  - Motors, Servos, etc.

  LIBRARIES:
  - SoftwareSerial
  - Arduino_JSON

  SETUP:
  1. Connect ESP-01S TX to Arduino Pin 2 (RX) and RX to Arduino Pin 3 (TX).
  2. Update the `ssid`, `password`, and `serverIp` variables below.
  3. Open the Serial Monitor at 9600 baud to see debug output.
  4. Set the ESP-01S baud rate to 9600 if not already set.
*/

// --- status_byte Definition ---
// This byte encapsulates key vehicle status information for efficient transmission.
// Each bit or bit pair represents a specific state or value range.
//
// Bit 0 & 1: Battery Level (2 bits)
//   00: Critical (0-14%)
//   01: Low (15-39%)
//   10: Okay (40-79%)
//   11: Healthy (80-100%)
//   Associated Behavior: Affects LED status (Red) and buzzer warning (short beeps).
//
// Bit 2: Thermal Sensor Status (1 bit)
//   0: Error, 1: OK
//   Associated Behavior: Affects error code (Sensor Error) and LED status (Red).
//
// Bit 3: Vision Module Status (1 bit)
//   0: Error, 1: OK
//   Associated Behavior: Affects error code (Sensor Error) and LED status (Red).
//
// Bit 4 & 5: Error Code (2 bits)
//   00: No Error
//   01: Communication Error
//   10: Sensor Error
//   11: Motor/Actuator Error
//   Associated Behavior: Affects LED status (Red) and buzzer warning (specific patterns).
//
// Bit 6 & 7: LED Status (2 bits) - This is the *reported* LED state, not direct control.
//   00: Off / Standby / Not Connected (Highest Priority)
//     - Trigger: System not connected to Wi-Fi, or connection abnormal.
//     - Behavior: Set LED to (0, 0, 0) - OFF.
//   01: Green / Normal Operation (Lowest Priority)
//     - Trigger: System connected, no errors/warnings, no commands executing (idle).
//     - Behavior: Set LED to (0, 255, 0) - BRIGHT GREEN.
//   10: Red / Error or Warning (Second Highest Priority)
//     - Trigger: Any internal error or severe warning (e.g., critical battery, sensor error, comms loss, motor error).
//     - Behavior: Set LED to (255, 0, 0) - BRIGHT RED. Trigger specific buzzer patterns.
//   11: Blue / Busy or Processing (Third Highest Priority)
//     - Trigger: System executing a time-consuming task (e.g., complex command, firmware update, internal computation in avoidance mode).
//     - Behavior: Set LED to (0, 0, 255) - BRIGHT BLUE.

// --- command_byte Definition ---
// This byte encapsulates control commands from the server.
//
// Bit 0 & 1: Buzzer Control (2 bits)
//   00: Off
//   01: Short Beep (e.g., 100ms)
//   10: Long Beep (e.g., 500ms)
//   11: Continuous Beep (e.g., 1 second on, 1 second off)
//
// Bit 2 & 3: LED Override (2 bits)
//   00: No Override (LED controlled by internal status_byte logic)
//   01: Force Green
//   10: Force Red
//   11: Force Blue
//
// Bit 4-7: Reserved for other commands

#include <Wire.h>
#include <Melopero_AMG8833.h>
#include <SoftwareSerial.h>
#include <Arduino_JSON.h>
#include <FastLED.h>
#include <Servo.h> // Added for Servo
#include <math.h>  // Added for math functions like sin, cos, sqrt, PI
#include <Ultrasound.h> // Added for Ultrasound sensor

// --- I2C Slave Addresses ---
#define ESP32_I2C_SLAVE_ADDRESS 0x53 // As per GEMINI.md

// --- Sensor Objects ---
Melopero_AMG8833 sensor;

// --- LED Objects ---
const static uint8_t ledPin = 2; // Assuming LED is on Pin 2 as per app_control.ino
static CRGB rgbs[1];

// --- Configuration ---
const char* ssid = "Hcedu01";         // Your WiFi network SSID
const char* password = "YOUR_WIFI_PASSWORD"; // Your WiFi network password
const char* serverIp = "192.168.1.100";      // The IP address of your computer running the Python server
const int serverPort = 8000;

// --- Pin Definitions (from app_control.ino) ---
const static uint8_t buzzerPin = 3;
const static uint8_t servoPin = 5;
const static uint8_t motorpwmPin[4] = {9,6,11,10} ;
const static uint8_t motordirectionPin[4] = {12,7,13,8};

// --- Hardware Objects ---
Servo myservo;          // 实例化舵机
Ultrasound ultrasound;  // 实例化超声波

// --- Timing Control ---
unsigned long lastThermalUpdateTime = 0;
unsigned long lastCommandPollTime = 0;
const long thermalUpdateInterval = 1000; // Milliseconds for thermal data update
const long commandPollInterval = 200;    // Milliseconds for command polling

// Define pins for SoftwareSerial to communicate with ESP-01S
// UNO RX = Pin 2, UNO TX = Pin 3
SoftwareSerial espSerial(2, 3); 

// --- Global Variables ---
int g_current_voltage_mv = 0; // Current battery voltage in mV (e.g., 785 for 7.85V)
bool g_is_wifi_connected = false; // Track WiFi connection status
bool g_thermal_sensor_error = false; // Track thermal sensor error
bool g_vision_module_error = false; // Track vision module error
bool g_motor_error = false; // Track motor error (placeholder for now)
bool g_communication_error = false; // Track communication error with backend

// --- Function Declarations ---
void setupEsp01s();
bool httpPost(String path, String jsonPayload);
String httpGet(String path);
String sendAtCommand(String command, const int timeout);


void setup() {
  // Start serial communication with the computer for debugging
  Serial.begin(9600);
  while (!Serial) { }
  Serial.println("Arduino UNO is ready.");

  // Start serial communication with the ESP-01S module
  espSerial.begin(9600); // Common baud rate for ESP-01S
  Serial.println("ESP-01S Serial started at 9600.");

  // Initialize hardware (motors, sensors, etc.) here
  Motor_Init(); // Initialize motor pins
  pinMode(servoPin, OUTPUT);
  myservo.attach(servoPin); // Attach servo to pin
  myservo.write(90); // Set initial servo angle to 90 degrees (center)
  
  // Play startup tone
  tone(buzzerPin, 1200); 
  delay(100);
  noTone(buzzerPin);

  FastLED.addLeds<WS2812, ledPin, RGB>(rgbs, 1);
  FastLED.setBrightness(50); // Set a default brightness

  // Initialize I2C and AMG8833 Sensor
  Serial.println("Initializing I2C and AMG8833 sensor...");
  Wire.begin();
  int sensorStatus = sensor.initI2C(AMG8833_I2C_ADDRESS_B, Wire);
  if (sensorStatus != 0) {
    Serial.print("Sensor init failed with error: ");
    Serial.println(sensor.getErrorDescription(sensorStatus));
  } else {
    Serial.println("Sensor init success.");
    sensor.setFPSMode(FPS_MODE::FPS_10);
  }

  // Setup ESP-01S module (connect to WiFi, etc.)
  setupEsp01s();
}

void loop() {
  unsigned long currentTime = millis();

  // Task: Periodically synchronize with server
  if (currentTime - lastCommandPollTime >= commandPollInterval) {
    syncWithServer();
    lastCommandPollTime = currentTime;
  }
}
 /**  * @brief Reads thermal data from the AMG8833, formats it as JSON, and POSTs it to the server.  */ void handleThermalUpdate() {  Serial.println("\n--- Handling Thermal Update ---");  int statusCode = sensor.updatePixelMatrix();  if (statusCode != 0) {    Serial.print("Failed to read pixel matrix. Error: ");    Serial.println(sensor.getErrorDescription(statusCode));    return;  }  // Create JSON object  JSONVar thermalPayload;    // Create a JSON array for the 8x8 matrix  JSONArray matrix = JSONArray();  for (int i = 0; i < 8; i++) {    JSONArray row = JSONArray();    for (int j = 0; j < 8; j++) {      row[j] = sensor.pixelMatrix[i][j];    }    matrix[i] = row;  }  thermalPayload["thermal_matrix"] = matrix;  // TODO: Add other sensor data like voltage here  // thermalPayload["voltage"] = analogRead(A3) * 0.02989;  String jsonString = JSON.stringify(thermalPayload);    Serial.println("Sending thermal data...");  httpPost("/api/thermal_update", jsonString);} /** 

/**
 * @brief Sends a series of AT commands to set up the ESP-01S module.
 */
void setupEsp01s() {
  Serial.println("--- Setting up ESP-01S ---");
  
  // Test AT communication
  sendAtCommand("AT", 2000);
  delay(1000);

  // Reset the module
  sendAtCommand("AT+RST", 2000);
  delay(2000);

  // Set mode to Station mode
  sendAtCommand("AT+CWMODE=1", 2000);
  delay(1000);

  // Connect to WiFi
  String connect_command = "AT+CWJAP=\"" + String(ssid) + "\",\"" + String(password) + "\"";
  sendAtCommand(connect_command, 7000); // WiFi connection can take longer
  delay(5000);

  // Check if connected
  String response = sendAtCommand("AT+CIFSR", 2000); // Get IP address
  if (response.indexOf("ERROR") == -1 && response.indexOf("STAIP") != -1) {
    Serial.println("ESP-01S Connected to WiFi!");
    g_is_wifi_connected = true;
  } else {
    Serial.println("ESP-01S Failed to connect to WiFi.");
    g_is_wifi_connected = false;
  }
  Serial.println("---------------------------");
}

/**
 * @brief Sets the RGB color of the LED.
 * @param rValue Red component (0-255).
 * @param gValue Green component (0-255).
 * @param bValue Blue component (0-255).
 */
void Rgb_Show(uint8_t rValue, uint8_t gValue, uint8_t bValue) {
  rgbs[0].r = rValue;
  rgbs[0].g = gValue;
  rgbs[0].b = bValue;
  FastLED.show();
}

/**
 * @brief Calculates the battery level code (2-bit) based on voltage.
 * @return 2-bit battery level code (00:Critical, 01:Low, 10:Okay, 11:Healthy).
 */
uint8_t getBatteryLevelCode() {
  // Constants for voltage conversion (from app_control.ino)
  const float VOLTAGE_CONVERSION_FACTOR = 0.02989; 
  const float BATTERY_FULL_VOLTAGE = 8.4; // Example for 2S LiPo
  const float BATTERY_EMPTY_VOLTAGE = 7.0; // Example for 2S LiPo (safe cutoff)

  // Read raw analog value from A3
  int rawVoltage = analogRead(A3);
  // Convert to actual voltage
  float voltage = rawVoltage * VOLTAGE_CONVERSION_FACTOR;
  g_current_voltage_mv = (int)(voltage * 100); // Store voltage in mV (e.g., 7.85V -> 785)

  // Convert voltage to percentage (linear approximation)
  float percentage = ((voltage - BATTERY_EMPTY_VOLTAGE) / (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE)) * 100.0;
  
  // Clamp percentage to 0-100 range
  if (percentage > 100.0) percentage = 100.0;
  if (percentage < 0.0) percentage = 0.0;

  // Map percentage to 2-bit code
  if (percentage >= 80.0) {
    return 3; // 11: Healthy (80-100%)
  } else if (percentage >= 40.0) {
    return 2; // 10: Okay (40-79%)
  } else if (percentage >= 15.0) {
    return 1; // 01: Low (15-39%)
  } else {
    return 0; // 00: Critical (0-14%)
  }
}

/**
 * @brief Determines the current error code (2-bit).
 * @return 2-bit error code (00:No Error, 01:Comm Error, 10:Sensor Error, 11:Motor Error).
 */
uint8_t getErrorCode() {
  // Priority: Motor/Actuator > Sensor > Communication > No Error

  if (g_motor_error) {
    return 3; // 11: Motor/Actuator Error
  }
  
  if (g_thermal_sensor_error || g_vision_module_error) {
    return 2; // 10: Sensor Error
  }

  if (g_communication_error) {
    return 1; // 01: Communication Error
  }

  return 0; // 00: No Error
}

/**
 * @brief Determines the LED status code (2-bit) based on priority.
 * @param is_wifi_connected Boolean indicating WiFi connection status.
 * @param current_error_code The 2-bit error code from getErrorCode().
 * @param is_busy Boolean indicating if the system is currently busy.
 * @return 2-bit LED status code (00:Off, 01:Green, 10:Red, 11:Blue).
 */
uint8_t getLedStatusCode(bool is_wifi_connected, uint8_t current_error_code, bool is_busy) {
  // Priority 1: 00 (Off / Not Connected)
  if (!is_wifi_connected) {
    return 0; // 00: Off
  }

  // Priority 2: 10 (Red / Error)
  if (current_error_code != 0) {
    return 2; // 10: Red
  }

  // Priority 3: 11 (Blue / Busy)
  if (is_busy) {
    return 3; // 11: Blue
  }

  // Priority 4: 01 (Green / Normal Operation)
  return 1; // 01: Green
}

  Serial.println("---------------------------");
}

// --- Motor Control Functions (Ported from app_control.ino) ---

/* 电机初始化函数 */
void Motor_Init(void)
{
  for(uint8_t i = 0; i < 4; i++) {
    pinMode(motordirectionPin[i], OUTPUT);
    pinMode(motorpwmPin[i], OUTPUT);
  }
  Velocity_Controller( 0, 0, 0);
}

/**
 * @brief 速度控制函数
 * @param angle   用于控制小车的运动方向，小车以车头为0度方向，逆时针为正方向。
 *                取值为0~359
 * @param velocity   用于控制小车速度，取值为0~100。
 * @param rot     用于控制小车的自转速度，取值为-100~100，若大于0小车有一个逆
 *                 时针的自转速度，若小于0则有一个顺时针的自转速度。
 * @param drift   用于决定小车是否开启漂移功能，取值为0或1，若为0则开启，反之关闭。
 * @retval None
 */
void Velocity_Controller(uint16_t angle, uint8_t velocity,int8_t rot)
{
  int8_t velocity_0, velocity_1, velocity_2, velocity_3;
  float speed = 1;
  angle += 90;
  float rad = angle * PI / 180;
  if (rot == 0) speed = 1;///< 速度因子
  else speed = 0.5;
  velocity /= sqrt(2);
  velocity_0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot * speed;
  velocity_1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot * speed;
  velocity_2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot * speed;
  velocity_3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot * speed;
  Motors_Set(velocity_0, velocity_1, velocity_2, velocity_3);
}

/**
 * @brief PWM与轮子转向设置函数
 * @param Motor_x   作为PWM与电机转向的控制数值。根据麦克纳姆轮的运动学分析求得。
 * @retval None
 */
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3)
{
  int8_t pwm_set[4];
  int8_t motors[4] = { Motor_0, Motor_1, Motor_2, Motor_3};
  bool direction[4] = { 1, 0, 0, 1};///< 前进 左1 右0
  for(uint8_t i = 0; i < 4; ++i) // Added initialization for i
  {
    if(motors[i] < 0) direction[i] = !direction[i];
    else direction[i] = direction[i];

    if(motors[i] == 0) pwm_set[i] = 0;
    else pwm_set[i] = abs(motors[i]);

    digitalWrite(motordirectionPin[i], direction[i]);
    PWM_Out(motorpwmPin[i], pwm_set[i]);
  }
}

/* 模拟PWM输出 */
void PWM_Out(uint8_t PWM_Pin, int8_t DutyCycle)
{
  // This function needs to be non-blocking and handle its own timing.
  // The original app_control.ino uses a global previousTime_us for this.
  // We need to ensure this is handled correctly in our non-blocking loop.
  // For now, we'll use a simple analogWrite if available, or a placeholder.
  // A proper non-blocking software PWM would be more complex.

  // For simplicity and initial testing, using analogWrite if pin supports PWM
  // Note: Not all pins support analogWrite (PWM) on Arduino Uno.
  // Pins 3, 5, 6, 9, 10, and 11 support PWM.
  // Our motorpwmPin are 9, 6, 11, 10 - all support PWM.
  analogWrite(PWM_Pin, map(DutyCycle, 0, 100, 0, 255));
}

/**
 * @brief Requests the ESP32-S3's IP address via I2C.
 * @return The IP address as a String, or an empty string if failed.
 */
String getEsp32IpAddress() {
  String ipAddress = "";
  Wire.requestFrom(ESP32_I2C_SLAVE_ADDRESS, 16); // Request up to 16 bytes (max IP string length)

  long startTime = millis();
  while (Wire.available() && (millis() - startTime < 100)) { // Read within 100ms timeout
    char c = Wire.read();
    ipAddress += c;
  }
  
  // Basic validation: check if it looks like an IP address
  if (ipAddress.length() > 7 && ipAddress.indexOf(".") != -1) {
    Serial.print("Received ESP32 IP via I2C: ");
    Serial.println(ipAddress);
    return ipAddress;
  } else {
    Serial.println("Failed to get valid ESP32 IP via I2C.");
    return "";
  }
}

/**
 * @brief Sets the LED color based on a 2-bit status code.
 * @param led_code 2-bit code: 00=Off, 01=Green, 10=Red, 11=Blue.
 */
void setLedStatus(uint8_t led_code) {
  switch (led_code) {
    case 0: // 00: Off
      Rgb_Show(0, 0, 0);
      break;
    case 1: // 01: Green
      Rgb_Show(0, 255, 0);
      break;
    case 2: // 10: Red
      Rgb_Show(255, 0, 0);
      break;
    case 3: // 11: Blue
      Rgb_Show(0, 0, 255);
      break;
    default:
      Rgb_Show(0, 0, 0); // Default to off for invalid codes
      break;
  }
}

/**
 * @brief Controls the buzzer based on a 2-bit code.
 * This function is non-blocking.
 * @param buzzer_code 2-bit code: 00=Off, 01=Short Beep, 10=Long Beep, 11=Continuous Beep.
 */
void controlBuzzer(uint8_t buzzer_code) {
  static unsigned long lastBuzzerToggleTime = 0;
  static bool buzzerState = false; // true = ON, false = OFF
  const unsigned long SHORT_BEEP_DURATION = 100; // ms
  const unsigned long LONG_BEEP_DURATION = 500;  // ms
  const unsigned long CONTINUOUS_BEEP_INTERVAL = 1000; // ms (on/off cycle)

  unsigned long currentTime = millis();

  switch (buzzer_code) {
    case 0: // 00: Off
      noTone(buzzerPin);
      buzzerState = false;
      break;
    case 1: // 01: Short Beep (one-shot)
      if (!buzzerState) { // Only trigger if not already beeping from this command
        tone(buzzerPin, 1000); // 1kHz tone
        lastBuzzerToggleTime = currentTime;
        buzzerState = true;
      }
      if (buzzerState && (currentTime - lastBuzzerToggleTime >= SHORT_BEEP_DURATION)) {
        noTone(buzzerPin);
        buzzerState = false; // Reset state after beep
      }
      break;
    case 2: // 10: Long Beep (one-shot)
      if (!buzzerState) {
        tone(buzzerPin, 1000);
        lastBuzzerToggleTime = currentTime;
        buzzerState = true;
      }
      if (buzzerState && (currentTime - lastBuzzerToggleTime >= LONG_BEEP_DURATION)) {
        noTone(buzzerPin);
        buzzerState = false;
      }
      break;
    case 3: // 11: Continuous Beep (toggling)
      if (currentTime - lastBuzzerToggleTime >= CONTINUOUS_BEEP_INTERVAL / 2) {
        if (buzzerState) {
          noTone(buzzerPin);
        } else {
          tone(buzzerPin, 1000);
        }
        buzzerState = !buzzerState;
        lastBuzzerToggleTime = currentTime;
      }
      break;
    default:
      noTone(buzzerPin);
      buzzerState = false;
      break;
  }
}

/**
 * @brief Sends a POST request to the server via the ESP-01S and returns the response body.
 * This function is used for the main sync endpoint.
 * @param path The API path for the request (e.g., "/api/sync").
 * @param jsonPayload The JSON string to send as the request body.
 * @return A String containing the body of the HTTP response if successful, otherwise an empty string.
 * 
 * Expected Request Payload (from Arduino to Server):
 *   {
 *     "s": <status_byte>,       // uint8_t: Encoded vehicle status (battery, sensor health, error, LED state)
 *     "v": <voltage_int>,       // int: Current battery voltage in mV (e.g., 785 for 7.85V)
 *     "t": [<thermal_array>]    // Optional: Array of 64 int (temp * 100), sent every 1 second
 *   }
 * 
 * Expected Response Payload (from Server to Arduino):
 *   {
 *     "c": <command_byte>,      // uint8_t: Encoded control commands (buzzer, LED override, etc.)
 *     "m": <motor_speed>,       // int: Motor speed (-100 to 100)
 *     "d": <direction_angle>,   // int: Direction angle (0-359)
 *     "a": <servo_angle>        // int: Servo angle (0-180)
 *   }
 */
String httpPost(String path, String jsonPayload) {
  String httpRequest = "POST " + path + " HTTP/1.1
";
  httpRequest += "Host: " + String(serverIp) + "
";
  httpRequest += "Content-Type: application/json
";
  httpRequest += "Content-Length: " + String(jsonPayload.length()) + "
";
  httpRequest += "Connection: close

";
  httpRequest += jsonPayload;

  // Establish TCP connection
  String response = sendAtCommand("AT+CIPSTART=\"TCP\",\"" + String(serverIp) + "\"," + String(serverPort), 5000);
  if (response.indexOf("ERROR") != -1) {
    Serial.println("Failed to establish TCP connection.");
    return "";
  }

  // Send the HTTP request
  sendAtCommand("AT+CIPSEND=" + String(httpRequest.length()), 2000);
  response = sendAtCommand(httpRequest, 5000);

  // Close the connection
  sendAtCommand("AT+CIPCLOSE", 2000);

  // Check for a valid response and extract the body
  int bodyStartIndex = response.indexOf("

");
  if (response.indexOf("200 OK") != -1 && bodyStartIndex != -1) {
    Serial.println("HTTP POST successful.");
    // Return the response body, which is after the double CRLF
    return response.substring(bodyStartIndex + 4);
  } else {
    Serial.println("HTTP POST failed or no 200 OK.");
    return "";
  }
}

/**
 * @brief Synchronizes vehicle status with the server and processes commands.
 * This is the main communication function called periodically.
 */
void syncWithServer() {
  Serial.println("\n--- Syncing with Server ---");

  // --- 1. Collect Status Data ---
  uint8_t status_byte = 0;

  // Battery Level (Bit 0 & 1)
  uint8_t battery_code = getBatteryLevelCode();
  status_byte |= battery_code; // Directly set Bit 0 & 1

  // Thermal Sensor Status (Bit 2)
  // Check thermal sensor error during update
  if (statusCode != 0) {
    g_thermal_sensor_error = true;
  } else {
    g_thermal_sensor_error = false;
  }
  if (!g_thermal_sensor_error) status_byte |= (1 << 2);

  // Vision Module Status (Bit 3)
  // Check vision module error during IP retrieval
  if (current_esp32_ip == "") {
    g_vision_module_error = true;
  } else {
    g_vision_module_error = false;
  }
  if (!g_vision_module_error) status_byte |= (1 << 3);

  // Error Code (Bit 4 & 5)
  uint8_t error_code = getErrorCode();
  status_byte |= (error_code << 4); // Shift error code to Bit 4 & 5

  // LED Status (Bit 6 & 7) - Determined by priority, will be set after processing commands
  // For now, we'll send a placeholder or current internal LED state.
  // The actual LED color will be set based on the command from the server or internal logic.
  // We'll update this part after command processing.
  uint8_t current_led_status_code = 0; // Placeholder for now
  status_byte |= (current_led_status_code << 6); // Shift LED status to Bit 6 & 7

  // --- 2. Build JSON Payload ---
  JSONVar payload;
  payload["s"] = status_byte;
  payload["v"] = g_current_voltage_mv;

  // ESP32-S3 IP Address (sent periodically or on change)
  static String esp32_ip = "";
  // String current_esp32_ip = getEsp32IpAddress(); // Already called above
  if (current_esp32_ip != "" && current_esp32_ip != esp32_ip) {
    esp32_ip = current_esp32_ip;
    payload["i"] = esp32_ip; // "i" for IP
  }

  // Thermal Matrix (Optional: sent every 1 second)
  static unsigned long lastThermalSendTime = 0;
  const unsigned long THERMAL_SEND_INTERVAL = 1000; // 1 second
  if (millis() - lastThermalSendTime >= THERMAL_SEND_INTERVAL) {
    int statusCode = sensor.updatePixelMatrix();
    if (statusCode == 0) {
      JSONArray matrix = JSONArray();
      for (int i = 0; i < 8; i++) {
        JSONArray row = JSONArray();
        for (int j = 0; j < 8; j++) {
          row[j] = (int)(sensor.pixelMatrix[i][j] * 100); // Convert to int * 100
        }
        matrix[i] = row;
      }
      payload["t"] = matrix;
      lastThermalSendTime = millis();
    } else {
      Serial.print("Failed to read pixel matrix for sync. Error: ");
      Serial.println(sensor.getErrorDescription(statusCode));
      g_thermal_sensor_error = true; // Set thermal sensor error
    }
  }

  String jsonString = JSON.stringify(payload);
  Serial.print("Sending payload: ");
  Serial.println(jsonString);

  // --- 3. Send POST Request and Process Response ---
  String responseBody = httpPost("/api/sync", jsonString);

  if (responseBody != "") {
    Serial.print("Received response: ");
    Serial.println(responseBody);

    JSONVar responseJson = JSON.parse(responseBody);
    if (JSON.typeof(responseJson) == "undefined") {
      Serial.println("Failed to parse server response.");
      g_communication_error = true; // Set communication error
      return;
    } else {
      g_communication_error = false; // Clear communication error if response is valid
    }

    // --- Process Commands ---
    // Command Byte (c)
    if (responseJson.hasOwnProperty("c")) {
      uint8_t command_byte = (uint8_t)((int)responseJson["c"]);
      
      // Extract buzzer control (Bit 0 & 1)
      uint8_t buzzer_code = command_byte & 0b00000011; // Get Bit 0 & 1
      controlBuzzer(buzzer_code);
      Serial.print("Command byte: ");
      Serial.println(command_byte);

      // TODO: Implement LED override based on command_byte (Bit 2 & 3)
    }

    // Motor Speed (m)
    if (responseJson.hasOwnProperty("m")) {
      int motor_speed = (int)responseJson["m"];
      // Direction Angle (d)
      int direction_angle = 0; // Default to 0
      if (responseJson.hasOwnProperty("d")) {
        direction_angle = (int)responseJson["d"];
      }
      Velocity_Controller(direction_angle, motor_speed, 0); // Control motors
      Serial.print("Motor speed: ");
      Serial.print(motor_speed);
      Serial.print(", Direction angle: ");
      Serial.println(direction_angle);
    }

    // Servo Angle (a)
    if (responseJson.hasOwnProperty("a")) {
      int servo_angle = (int)responseJson["a"];
      myservo.write(servo_angle); // Control servo
      Serial.print("Servo angle: ");
      Serial.println(servo_angle);
    }

    // Update LED status based on processed commands or internal state
    // This is where the LED status in status_byte (Bit 6 & 7) would be determined
    // based on the priority logic (connected, error, busy, idle).
    
    // Determine internal LED state
    bool is_busy = false; // TODO: Implement actual busy state tracking
    uint8_t internal_led_code = getLedStatusCode(g_is_wifi_connected, error_code, is_busy);

    // Extract LED override from command_byte (Bit 2 & 3)
    uint8_t override_led_code = (command_byte >> 2) & 0b00000011; // Get Bit 2 & 3

    uint8_t final_led_code;

    // Apply LED priority logic
    if (!g_is_wifi_connected) { // Priority 1: Not Connected
      final_led_code = 0; // Force Off
    } else if (override_led_code != 0) { // Priority 2: Backend Override
      final_led_code = override_led_code;
    } else { // Priority 3: Internal Status
      final_led_code = internal_led_code;
    }

    setLedStatus(final_led_code); // Set the actual LED color

    // Update status_byte with the final LED code that is actually being displayed
    status_byte &= ~((1 << 6) | (1 << 7)); // Clear Bit 6 & 7
    status_byte |= (final_led_code << 6); // Set Bit 6 & 7

  } else { // No response or HTTP POST failed.
    Serial.println("No response or HTTP POST failed.");
    g_communication_error = true; // Set communication error

    // If communication fails, force LED to Off (Priority 1: Not Connected)
    setLedStatus(0); 
    // Update status_byte with the final LED code (Off)
    status_byte &= ~((1 << 6) | (1 << 7)); // Clear Bit 6 & 7
    status_byte |= (0 << 6); // Set Bit 6 & 7 to 00 (Off)
  }
  Serial.println("---------------------------");
}

/**
 * @brief Sends a GET request to the server via the ESP-01S.
 * @param path The API path for the request (e.g., "/api/command").
 * @return A String containing the body of the HTTP response.
 */
String httpGet(String path) {
  String httpRequest = "GET " + path + " HTTP/1.1\r\n";
  httpRequest += "Host: " + String(serverIp) + "\r\n";
  httpRequest += "Connection: close\r\n\r\n";

  // Establish TCP connection
  String response = sendAtCommand("AT+CIPSTART=\"TCP\",\"" + String(serverIp) + "\",\"" + String(serverPort), 5000);
  if (response.indexOf("ERROR") != -1) {
    Serial.println("Failed to establish TCP connection.");
    return "";
  }

  // Send the HTTP request
  sendAtCommand("AT+CIPSEND=" + String(httpRequest.length()), 2000);
  response = sendAtCommand(httpRequest, 5000);

  // Close the connection
  sendAtCommand("AT+CIPCLOSE", 2000);

  // Check for a valid response and extract the body
  int bodyStartIndex = response.indexOf("\r\n\r\n");
  if (response.indexOf("200 OK") != -1 && bodyStartIndex != -1) {
    Serial.println("HTTP GET successful.");
    // Return the response body, which is after the double CRLF
    return response.substring(bodyStartIndex + 4);
  } else {
    Serial.println("HTTP GET failed.");
    return "";
  }
}

/**
 * @brief A utility function to send an AT command and get the response.
 * @param command The AT command to send (without \r\n).
 * @param timeout The maximum time to wait for a response in milliseconds.
 * @return The response from the ESP-01S as a String.
 */
String sendAtCommand(String command, const int timeout) {
  String response = "";
  
  espSerial.println(command); // Send the AT command to the ESP-01S
  
  long int startTime = millis();
  while ((startTime + timeout) > millis()) {
    while (espSerial.available()) {
      char c = espSerial.read();
      response += c;
    }
  }
  
  // Print command and response to the main serial for debugging
  Serial.print("--- AT Command ---\nSent: ");
  Serial.println(command);
  Serial.print("Recv: ");
  Serial.println(response);
  Serial.println("--------------------");
  
  return response;
}

