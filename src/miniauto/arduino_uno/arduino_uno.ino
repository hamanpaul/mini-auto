/*
  Arduino API Client for Vehicle Control (Memory Optimized)

  This sketch runs on an Arduino UNO and controls an ESP-01S module via AT commands
  to interact with a Python FastAPI server. It sends vehicle status updates and 
  receives movement commands.

  This version is refactored to minimize SRAM usage by:
  1. Using F() macro to store constant strings in Flash memory.
  2. Replacing dynamic 'String' objects with static 'char' arrays.
  3. Streaming HTTP requests instead of building them in memory.
  4. Centralizing AT command definitions for easier maintenance.
  5. Manually building and parsing JSON to completely remove Arduino_JSON library overhead.
*/

// --- Includes ---
#include <Wire.h>
#include <Melopero_AMG8833.h>
#include <SoftwareSerial.h>
#include <FastLED.h>
#include <Servo.h>
#include <math.h>
#include <Ultrasound.h>

// --- Constant Definitions ---

// --- I2C Slave Addresses ---
#define ESP32_I2C_SLAVE_ADDRESS 0x53

// --- Pin Definitions ---
const static uint8_t ledPin = 2;
const static uint8_t buzzerPin = 3;
const static uint8_t servoPin = 5;
const static uint8_t motorpwmPin[4] = {10, 9, 6, 11};
const static uint8_t motordirectionPin[4] = {12, 8, 7, 13};

// --- Network Configuration ---
const char ssid[] PROGMEM = "Hcedu01";
const char password[] PROGMEM = "035260089";
char g_server_ip[16] = ""; // Buffer for server IP address (e.g., "192.168.1.100")
int g_server_port = 8000;

// --- AT Command Definitions (for ESP-01S) ---
// Stored in PROGMEM (Flash) to save SRAM
const char AT_CMD[] PROGMEM = "AT";
const char AT_RST[] PROGMEM = "AT+RST";
const char AT_CWMODE[] PROGMEM = "AT+CWMODE=1";
const char AT_CIPMUX[] PROGMEM = "AT+CIPMUX=1";
const char AT_CIPSTART_UDP[] PROGMEM = "AT+CIPSTART=0,\"UDP\",\"0.0.0.0\",0,5005,0";
const char AT_CWJAP_PART1[] PROGMEM = "AT+CWJAP=\"";
const char AT_CWJAP_PART2[] PROGMEM = "\",\"";
const char AT_CWJAP_PART3[] PROGMEM = "\"";
const char AT_CIFSR[] PROGMEM = "AT+CIFSR";
const char AT_CIPSTART_TCP[] PROGMEM = "AT+CIPSTART=\"TCP\",\""; // Note: Partial command
const char AT_CIPSEND[] PROGMEM = "AT+CIPSEND=";
const char AT_CIPCLOSE[] PROGMEM = "AT+CIPCLOSE";

// --- Hardware & Sensor Objects ---
Melopero_AMG8833 sensor;
Servo myservo;
Ultrasound ultrasound;
static CRGB rgbs[1];

// --- Timing Control ---
const static int pwmFrequency = 500;                /* PWM频率，单位是赫兹 */
const static int period = 1000000 / pwmFrequency;  /* PWM周期，单位是微秒 */
static uint32_t previousTime_us = 0;          /* 上一次的微秒计数时间间隔 用于非阻塞延时 */
unsigned long lastCommandPollTime = 0;
const long commandPollInterval = 200; // Milliseconds

// --- Software Serial for ESP-01S ---
// UNO RX = Pin 2, UNO TX = Pin 3
SoftwareSerial espSerial(2, 3);

// --- Global State Variables ---
int g_current_voltage_mv = 0;
bool g_is_wifi_connected = false;
bool g_thermal_sensor_error = false;
bool g_vision_module_error = false;
bool g_motor_error = false;
bool g_communication_error = false;

// --- Buffers ---
// Global buffers to avoid stack allocation in loop and reduce fragmentation
char esp_response_buffer[96]; // For AT command responses
char http_response_buffer[96]; // For parsing HTTP response from server

// --- Function Declarations ---
void Motor_Init(void);
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot);
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3);
void PWM_Out(uint8_t PWM_Pin, int8_t DutyCycle);
void setupEsp01s();
bool httpPost(const char* path, char* response_buffer, int buffer_len);
bool sendAtCommand(const char* command, const int timeout, char* response_buffer, int buffer_len, bool is_progmem = true);
bool getEsp32IpAddress(char* ip_buffer, int buffer_len);
void setLedStatus(uint8_t led_code);
void controlBuzzer(uint8_t buzzer_code);
uint8_t getBatteryLevelCode();
uint8_t getErrorCode();
uint8_t getLedStatusCode(bool is_wifi_connected, uint8_t current_error_code, bool is_busy);
void syncWithServer();
void Rgb_Show(uint8_t rValue, uint8_t gValue, uint8_t bValue);
int get_json_int_value(const char* json, const char* key);

// --- Setup ---
void setup() {
  Serial.begin(9600);
  while (!Serial) {}
  Serial.println(F("Arduino UNO is ready."));

  espSerial.begin(9600);
  Serial.println(F("ESP-01S Serial started at 9600."));

  Motor_Init();
  myservo.attach(servoPin);
  myservo.write(90);

  tone(buzzerPin, 1200, 100);

  FastLED.addLeds<WS2812, ledPin, RGB>(rgbs, 1);
  FastLED.setBrightness(50);

  Serial.println(F("Initializing I2C and AMG8833 sensor..."));
  Wire.begin();
  int sensorStatus = sensor.initI2C(AMG8833_I2C_ADDRESS_B, Wire);
  if (sensorStatus != 0) {
    Serial.print(F("Sensor init failed with error: "));
    Serial.println(sensor.getErrorDescription(sensorStatus));
  } else {
    Serial.println(F("Sensor init success."));
    sensor.setFPSMode(FPS_MODE::FPS_10);
  }

  setupEsp01s();
}

// --- Main Loop ---
void loop() {
  unsigned long currentTime = millis();

  if (currentTime - lastCommandPollTime >= commandPollInterval) {
    if (g_server_ip[0] != '\0') {
      syncWithServer();
    } else {
      Serial.println(F("Server IP not discovered yet. Waiting..."));
    }
    lastCommandPollTime = currentTime;
  }

  // Check for incoming UDP broadcast for server discovery
  if (espSerial.available()) {
    static char udp_buffer[128];
    static int udp_buffer_idx = 0;
    char c = espSerial.read();
    
    if (c != '\n' && udp_buffer_idx < (int)sizeof(udp_buffer) - 1) {
      udp_buffer[udp_buffer_idx++] = c;
    } else {
      udp_buffer[udp_buffer_idx] = '\0';
      udp_buffer_idx = 0;

      // Example: +IPD,0,20:MINIAUTO_SERVER_IP:192.168.1.100:8000
      char* data_start = strstr(udp_buffer, "MINIAUTO_SERVER_IP:");
      if (data_start) {
        data_start += strlen("MINIAUTO_SERVER_IP:");
        char* port_start = strrchr(data_start, ':');
        if (port_start) {
          *port_start = '\0'; // Null-terminate the IP address
          int discovered_port = atoi(port_start + 1);
          
          if (strcmp(g_server_ip, data_start) != 0 || g_server_port != discovered_port) {
            strncpy(g_server_ip, data_start, sizeof(g_server_ip) - 1);
            g_server_ip[sizeof(g_server_ip) - 1] = '\0';
            g_server_port = discovered_port;
            
            Serial.print(F("Discovered Server IP: "));
            Serial.print(g_server_ip);
            Serial.print(F(", Port: "));
            Serial.println(g_server_port);
          }
        }
      }
    }
  }
}

// --- ESP-01S Setup ---
void setupEsp01s() {
  Serial.println(F("--- Setting up ESP-01S ---"));
  
  sendAtCommand(AT_CMD, 2000, esp_response_buffer, sizeof(esp_response_buffer));
  delay(1000);

  sendAtCommand(AT_RST, 2000, esp_response_buffer, sizeof(esp_response_buffer));
  delay(2000);

  sendAtCommand(AT_CWMODE, 2000, esp_response_buffer, sizeof(esp_response_buffer));
  delay(1000);

  sendAtCommand(AT_CIPMUX, 2000, esp_response_buffer, sizeof(esp_response_buffer));
  delay(1000);

  sendAtCommand(AT_CIPSTART_UDP, 2000, esp_response_buffer, sizeof(esp_response_buffer));
  delay(1000);

  // Connect to WiFi
  char cmd_buffer[128];
  char temp_ssid[32];
  char temp_pass[32];
  strcpy_P(temp_ssid, ssid);
  strcpy_P(temp_pass, password);
  snprintf_P(cmd_buffer, sizeof(cmd_buffer), PSTR("%S%s%S%s%S"), AT_CWJAP_PART1, temp_ssid, AT_CWJAP_PART2, temp_pass, AT_CWJAP_PART3);
  sendAtCommand(cmd_buffer, 7000, esp_response_buffer, sizeof(esp_response_buffer), false);
  delay(5000);

  // Check if connected
  sendAtCommand(AT_CIFSR, 2000, esp_response_buffer, sizeof(esp_response_buffer));
  if (strstr(esp_response_buffer, "ERROR") == NULL && strstr(esp_response_buffer, "STAIP") != NULL) {
    Serial.println(F("ESP-01S Connected to WiFi!"));
    g_is_wifi_connected = true;
  } else {
    Serial.println(F("ESP-01S Failed to connect to WiFi."));
    g_is_wifi_connected = false;
  }
  Serial.println(F("---------------------------"));
}

// --- LED & Buzzer Control ---
void Rgb_Show(uint8_t rValue, uint8_t gValue, uint8_t bValue) {
  rgbs[0].setRGB(rValue, gValue, bValue);
  FastLED.show();
}

void setLedStatus(uint8_t led_code) {
  switch (led_code) {
    case 0: Rgb_Show(0, 0, 0); break;       // Off
    case 1: Rgb_Show(0, 255, 0); break;     // Green
    case 2: Rgb_Show(255, 0, 0); break;     // Red
    case 3: Rgb_Show(0, 0, 255); break;     // Blue
    default: Rgb_Show(0, 0, 0); break;
  }
}

void controlBuzzer(uint8_t buzzer_code) {
  static unsigned long lastBuzzerActionTime = 0;
  static bool buzzerState = false;
  const unsigned long SHORT_BEEP_DURATION = 100;
  const unsigned long LONG_BEEP_DURATION = 500;
  const unsigned long CONTINUOUS_BEEP_INTERVAL = 1000;

  unsigned long currentTime = millis();

  switch (buzzer_code) {
    case 0: // Off
      noTone(buzzerPin);
      buzzerState = false;
      break;
    case 1: // Short Beep (one-shot)
      if (currentTime - lastBuzzerActionTime > SHORT_BEEP_DURATION) {
        tone(buzzerPin, 1000, SHORT_BEEP_DURATION);
        lastBuzzerActionTime = currentTime;
      }
      break;
    case 2: // Long Beep (one-shot)
      if (currentTime - lastBuzzerActionTime > LONG_BEEP_DURATION) {
        tone(buzzerPin, 1000, LONG_BEEP_DURATION);
        lastBuzzerActionTime = currentTime;
      }
      break;
    case 3: // Continuous Beep (toggling)
      if (currentTime - lastBuzzerActionTime >= CONTINUOUS_BEEP_INTERVAL / 2) {
        if (buzzerState) {
          noTone(buzzerPin);
        } else {
          tone(buzzerPin, 1000);
        }
        buzzerState = !buzzerState;
        lastBuzzerActionTime = currentTime;
      }
      break;
    default:
      noTone(buzzerPin);
      buzzerState = false;
      break;
  }
}

// --- Status Calculation ---
uint8_t getBatteryLevelCode() {
  const float VOLTAGE_CONVERSION_FACTOR = 0.02989; 
  const float BATTERY_FULL_VOLTAGE = 8.4;
  const float BATTERY_EMPTY_VOLTAGE = 7.0;

  int rawVoltage = analogRead(A3);
  float voltage = rawVoltage * VOLTAGE_CONVERSION_FACTOR;
  g_current_voltage_mv = (int)(voltage * 100);

  float percentage = ((voltage - BATTERY_EMPTY_VOLTAGE) / (BATTERY_FULL_VOLTAGE - BATTERY_EMPTY_VOLTAGE)) * 100.0;
  
  if (percentage > 100.0) percentage = 100.0;
  if (percentage < 0.0) percentage = 0.0;

  if (percentage >= 80.0) return 3; // 11: Healthy
  if (percentage >= 40.0) return 2; // 10: Okay
  if (percentage >= 15.0) return 1; // 01: Low
  return 0; // 00: Critical
}

uint8_t getErrorCode() {
  if (g_motor_error) return 3;
  if (g_thermal_sensor_error || g_vision_module_error) return 2;
  if (g_communication_error) return 1;
  return 0;
}

uint8_t getLedStatusCode(bool is_wifi_connected, uint8_t current_error_code, bool is_busy) {
  if (!is_wifi_connected) return 0; // Off
  if (current_error_code != 0) return 2; // Red
  if (is_busy) return 3; // Blue
  return 1; // Green
}

// --- I2C Communication ---
bool getEsp32IpAddress(char* ip_buffer, int buffer_len) {
  Wire.requestFrom((uint8_t)ESP32_I2C_SLAVE_ADDRESS, (uint8_t)(buffer_len - 1));
  long startTime = millis();
  int i = 0;
  ip_buffer[0] = '\0';
  while (Wire.available() && (millis() - startTime < 100)) {
    ip_buffer[i++] = Wire.read();
  }
  ip_buffer[i] = '\0';

  if (i > 7 && strchr(ip_buffer, '.') != NULL) {
    Serial.print(F("Received ESP32 IP via I2C: "));
    Serial.println(ip_buffer);
    return true;
  } else {
    Serial.println(F("Failed to get valid ESP32 IP via I2C."));
    ip_buffer[0] = '\0';
    return false;
  }
}

// --- Main Sync Function ---
void syncWithServer() {
  // 1. Send POST and Process Response
  if (httpPost("/api/sync", http_response_buffer, sizeof(http_response_buffer))) {
    Serial.print(F("Received response: "));
    Serial.println(http_response_buffer);
    g_communication_error = false;

    // Manually parse the JSON response
    uint8_t command_byte = get_json_int_value(http_response_buffer, "\"c\":");
    int motor_speed = get_json_int_value(http_response_buffer, "\"m\":");
    int direction_angle = get_json_int_value(http_response_buffer, "\"d\":");
    int servo_angle = get_json_int_value(http_response_buffer, "\"a\":");

    controlBuzzer(command_byte & 0b11);
    Velocity_Controller(direction_angle, motor_speed, 0);
    myservo.write(servo_angle);

    uint8_t error_code = getErrorCode();
    uint8_t override_led_code = (command_byte >> 2) & 0b11;
    uint8_t final_led_code = override_led_code != 0 ? override_led_code : getLedStatusCode(g_is_wifi_connected, error_code, false);
    setLedStatus(final_led_code);

  } else {
    Serial.println(F("No response or HTTP POST failed."));
    g_communication_error = true;
    setLedStatus(0); // Turn off LED on failure
  }
  Serial.println(F("---------------------------"));
}

// --- Network Communication ---
bool httpPost(const char* path, char* response_buffer, int buffer_len) {
  // 1. Collect Status Data
  uint8_t status_byte = 0;
  char current_esp32_ip[16];
  status_byte |= getBatteryLevelCode();
  int ultrasonic_distance = ultrasound.GetDistance(); // Read ultrasonic distance
  g_vision_module_error = !getEsp32IpAddress(current_esp32_ip, sizeof(current_esp32_ip));
  if (!g_vision_module_error) status_byte |= (1 << 3);

  bool include_thermal = false;
  static unsigned long lastThermalSendTime = 0;
  if (millis() - lastThermalSendTime >= 1000) {
    if (sensor.updatePixelMatrix() == 0) {
      g_thermal_sensor_error = false;
      include_thermal = true;
      lastThermalSendTime = millis();
    } else {
      g_thermal_sensor_error = true;
    }
  }
  if (!g_thermal_sensor_error) status_byte |= (1 << 2);

  uint8_t error_code = getErrorCode();
  status_byte |= (error_code << 4);
  status_byte |= (0 << 6); // Placeholder

  // 2. Calculate Content Length
  char temp_buf[10];
  int payload_len = 0;
  payload_len += sprintf(temp_buf, "{\"s\":%d,\"v\":%d", status_byte, g_current_voltage_mv);
  if (ultrasonic_distance > 0) { // Only include if valid distance
    payload_len += sprintf(temp_buf, ",\"u\":%d", ultrasonic_distance);
  }
  static char last_sent_esp32_ip[16] = "";
  if (!g_vision_module_error && strcmp(current_esp32_ip, last_sent_esp32_ip) != 0) {
    payload_len += sprintf(temp_buf, ",\"i\":\"%s\"", current_esp32_ip);
  }
  if (include_thermal) {
    payload_len += 2 + 64 * 5; // "t":[[...]] roughly 5 chars per pixel (e.g. -1024,)
  }
  payload_len += 1; // for closing brace

  // 3. Establish TCP connection
  char cmd_buffer[128];
  snprintf_P(cmd_buffer, sizeof(cmd_buffer), PSTR("%S%s\",%d"), AT_CIPSTART_TCP, g_server_ip, g_server_port);
  if (!sendAtCommand(cmd_buffer, 5000, esp_response_buffer, sizeof(esp_response_buffer), false)) {
    Serial.println(F("Failed to establish TCP connection."));
    sendAtCommand(AT_CIPCLOSE, 2000, esp_response_buffer, sizeof(esp_response_buffer));
    return false;
  }

  // 4. Prepare CIPSEND command
  int total_len = strlen_P(PSTR("POST ")) + strlen(path) + strlen_P(PSTR(" HTTP/1.1\r\nHost: ")) + strlen(g_server_ip) + strlen_P(PSTR("\r\nContent-Type: application/json\r\nContent-Length: ")) + 5 + strlen_P(PSTR("\r\nConnection: close\r\n\r\n")) + payload_len;
  snprintf_P(cmd_buffer, sizeof(cmd_buffer), PSTR("%S%d"), AT_CIPSEND, total_len);
  if (!sendAtCommand(cmd_buffer, 2000, esp_response_buffer, sizeof(esp_response_buffer), false) || strstr(esp_response_buffer, ">") == NULL) {
      Serial.println(F("CIPSEND command failed."));
      sendAtCommand(AT_CIPCLOSE, 2000, esp_response_buffer, sizeof(esp_response_buffer));
      return false;
  }

  // 5. Send HTTP Request (streaming JSON payload)
  espSerial.print(F("POST ")); espSerial.print(path); espSerial.print(F(" HTTP/1.1\r\n"));
  espSerial.print(F("Host: ")); espSerial.print(g_server_ip); espSerial.print(F("\r\n"));
  espSerial.print(F("Content-Type: application/json\r\n"));
  espSerial.print(F("Content-Length: ")); espSerial.print(payload_len); espSerial.print(F("\r\n"));
  espSerial.print(F("Connection: close\r\n\r\n"));
  
  // Stream the JSON payload manually
  espSerial.print(F("{\"s\":")); espSerial.print(status_byte); espSerial.print(F(",\"v\":")); espSerial.print(g_current_voltage_mv);
  if (ultrasonic_distance > 0) {
    espSerial.print(F(",\"u\":")); espSerial.print(ultrasonic_distance);
  }
  if (!g_vision_module_error && strcmp(current_esp32_ip, last_sent_esp32_ip) != 0) {
    espSerial.print(F(",\"i\":\"")); espSerial.print(current_esp32_ip); espSerial.print(F("\""));
    strcpy(last_sent_esp32_ip, current_esp32_ip);
  }
  if (include_thermal) {
    espSerial.print(F(",\"t\":["));
    for (int i = 0; i < 8; i++) {
      espSerial.print(F("["));
      for (int j = 0; j < 8; j++) {
        espSerial.print(sensor.pixelMatrix[i][j]);
        if (j < 7) espSerial.print(F(","));
      }
      espSerial.print(F("]"));
      if (i < 7) espSerial.print(F(","));
    }
    espSerial.print(F("]"));
  }
  espSerial.print(F("}"));

  // 6. Read response
  unsigned long startTime = millis();
  int idx = 0;
  memset(response_buffer, 0, buffer_len);
  while (millis() - startTime < 5000 && idx < buffer_len - 1) {
    if (espSerial.available()) {
      response_buffer[idx++] = espSerial.read();
    }
  }
  
  sendAtCommand(AT_CIPCLOSE, 2000, esp_response_buffer, sizeof(esp_response_buffer));

  // 7. Check for valid response and extract body
  char* body_start = strstr(response_buffer, "\r\n\r\n");
  if (strstr(response_buffer, "200 OK") != NULL && body_start != NULL) {
    Serial.println(F("HTTP POST successful."));
    memmove(response_buffer, body_start + 4, strlen(body_start + 4) + 1);
    return true;
  } else {
    Serial.println(F("HTTP POST failed or no 200 OK."));
    return false;
  }
}

bool sendAtCommand(const char* command, const int timeout, char* response_buffer, int buffer_len, bool is_progmem) {
  memset(response_buffer, 0, buffer_len); // Clear buffer
  
  Serial.print(F("--- AT Command ---\nSent: "));
  if (is_progmem) {
    char pgm_buffer[128];
    strcpy_P(pgm_buffer, command);
    Serial.println(pgm_buffer);
    espSerial.println((const __FlashStringHelper*)command);
  } else {
    Serial.println(command);
    espSerial.println(command);
  }

  int idx = 0;
  unsigned long startTime = millis();
  while (millis() - startTime < timeout) {
    if (espSerial.available() && idx < buffer_len - 1) {
      response_buffer[idx++] = espSerial.read();
    }
  }
  
  Serial.print(F("Recv: "));
  Serial.println(response_buffer);
  Serial.println(F("--------------------"));
  
  return (strstr(response_buffer, "OK") != NULL || strstr(response_buffer, "SEND OK") != NULL || strstr(response_buffer, ">") != NULL || strstr(response_buffer, "no change") != NULL);
}

// --- Motor Control Functions ---
void Motor_Init(void) {
  for(uint8_t i = 0; i < 4; i++) {
    pinMode(motordirectionPin[i], OUTPUT);
    pinMode(motorpwmPin[i], OUTPUT);
  }
  Velocity_Controller( 0, 0, 0);
}

void Velocity_Controller(uint16_t angle, uint8_t velocity,int8_t rot) {
  int8_t velocity_0, velocity_1, velocity_2, velocity_3;
  float speed = 1;
  angle += 90;
  float rad = angle * PI / 180;
  if (rot == 0) speed = 1;
  else speed = 0.5; 
  velocity /= sqrt(2);
  velocity_0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot * speed;
  velocity_1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot * speed;
  velocity_2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot * speed;
  velocity_3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot * speed;
  Motors_Set(velocity_0, velocity_1, velocity_2, velocity_3);
}
 
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3) {
  int8_t pwm_set[4];
  int8_t motors[4] = { Motor_0, Motor_1, Motor_2, Motor_3};
  bool direction[4] = { 1, 0, 0, 1};
  for(uint8_t i = 0; i < 4; ++i) { // BUGFIX: Initialized i=0
    if(motors[i] < 0) direction[i] = !direction[i];
    
    pwm_set[i] = abs(motors[i]);

    digitalWrite(motordirectionPin[i], direction[i]); 
    PWM_Out(motorpwmPin[i], pwm_set[i]);
  }
}

void PWM_Out(uint8_t PWM_Pin, int8_t DutyCycle) { 
  uint32_t currentTime_us = micros();
  int highTime = (period/100) * DutyCycle;

  if ((currentTime_us - previousTime_us) <= (unsigned long)highTime) {  
    digitalWrite(PWM_Pin, HIGH);
  } else {
    digitalWrite(PWM_Pin, LOW);
  }
  if (currentTime_us - previousTime_us >= (unsigned long)period) {
    previousTime_us = currentTime_us;
  }
}

// --- Utility Functions ---
int get_json_int_value(const char* json, const char* key) {
    const char* key_ptr = strstr(json, key);
    if (key_ptr == NULL) {
        return 0; // Key not found
    }
    key_ptr += strlen(key);
    return atoi(key_ptr);
}
