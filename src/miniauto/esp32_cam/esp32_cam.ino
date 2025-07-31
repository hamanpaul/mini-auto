#define HAS_UNO_I2C 0 // Define to enable UNO I2C communication and sensor data check
#define USE_UDP_DISCOVERY 1 // Set to 1 to enable UDP discovery, 0 to use manual IP

// Manual Backend Server IP and Port (only used if USE_UDP_DISCOVERY is 0)
const char* MANUAL_BACKEND_SERVER_IP = "192.168.0.100"; // <<< CHANGE THIS TO YOUR BACKEND SERVER IP
const int MANUAL_BACKEND_SERVER_PORT = 8000; // <<< CHANGE THIS TO YOUR BACKEND SERVER PORT

/*
  ESP32-S3-WROOM Camera Web Server and Autonomous Sync Agent for Miniauto

  This sketch runs on the ESP32-S3 module, providing an MJPEG video stream from a GC2145 camera.
  It acts as an I2C slave to communicate with the Arduino UNO, receiving sensor data and sending control commands.
  Additionally, it functions as an autonomous sync agent, managing independent HTTP communication with the backend server.

  - Based on official Espressif esp32-camera examples.
  - Uses Arduino WebServer library for the MJPEG stream.
  - Implements I2C slave for communication with Arduino UNO.
  - Manages independent HTTP synchronization with the backend server.
*/

#include "esp_camera.h"
#include "WiFi.h"
#include "WebServer.h"
#include "Wire.h"
#include <VehicleData.h>
#include <HTTPClient.h> // 用於發送 HTTP 請求到後端伺服器
#include <ArduinoJson.h> // 用於處理 JSON 數據
#include <AsyncUDP.h> // 用於監聽 UDP 廣播，發現後端伺服器 IP

// --- WiFi Configuration ---
const char* ssid = "Hcedu01";
const char* password = "035260089";

// --- I2C Slave Configuration ---
#define I2C_SLAVE_ADDRESS 0x53
#define I2C_SDA_PIN 47
#define I2C_SCL_PIN 48



// 計算結構體大小
const size_t SENSOR_DATA_SIZE = sizeof(SensorData_t);
const size_t COMMAND_DATA_SIZE = sizeof(CommandData_t);

// --- 全域變數 ---
volatile SensorData_t receivedSensorData; // 用於儲存從 UNO 接收的感測器數據
volatile CommandData_t currentCommandData; // 用於儲存要發送給 UNO 的控制指令
volatile bool newSensorDataAvailable = false; // 標誌，指示是否有新感測器數據從 UNO 傳來
String old_response = "";

// 用於 I2C 通訊除錯，追蹤數據變化
SensorData_t lastReceivedSensorDataFromUNO = {};
CommandData_t lastSentCommandDataToUNO = {};
bool isFirstI2CComm = true;
bool httpSyncTimerStarted = false; // 標誌，指示 HTTP 同步定時器是否已啟動
bool cameraRegistered = false; // 標誌，指示攝影機是否已註冊
bool serverIpConfirmed = false; // 標誌，指示是否已確認伺服器 IP

// 全域變數，用於儲存上次發送的感測器數據，以及第一次同步的標誌
SensorData_t lastSentSensorData;
bool firstSync = true;

// 後端伺服器 IP 和埠號
String backendServerIp = "";
int backendServerPort = 8000;

// UDP 廣播相關
AsyncUDP udp; // UDP 物件
const int UDP_BROADCAST_PORT = 5005; // 監聽 UDP 廣播的埠號

// HTTP 同步定時器相關
esp_timer_handle_t http_sync_timer; // 定時器句柄
const int HTTP_SYNC_INTERVAL_MS = 200; // HTTP 同步間隔，單位毫秒

// --- Pin Definitions for ESP32-S3-WROOM (matches official example) ---
#define CAM_PIN_PWDN 38
#define CAM_PIN_RESET -1
#define CAM_PIN_VSYNC 6
#define CAM_PIN_HREF 7
#define CAM_PIN_PCLK 13
#define CAM_PIN_XCLK 15
#define CAM_PIN_SIOD 4     // SCCB SDA
#define CAM_PIN_SIOC 5     // SCCB SCL
#define CAM_PIN_D0 11
#define CAM_PIN_D1 9
#define CAM_PIN_D2 8
#define CAM_PIN_D3 10
#define CAM_PIN_D4 12
#define CAM_PIN_D5 18
#define CAM_PIN_D6 17
#define CAM_PIN_D7 16

WebServer server(80);

void receiveEvent(int howMany) {
  if (howMany == SENSOR_DATA_SIZE) {
    byte buffer[SENSOR_DATA_SIZE];
    Wire.readBytes(buffer, SENSOR_DATA_SIZE);

    if (isFirstI2CComm || memcmp(buffer, &lastReceivedSensorDataFromUNO, SENSOR_DATA_SIZE) != 0) {
      memcpy((void*)&receivedSensorData, buffer, SENSOR_DATA_SIZE);
      memcpy(&lastReceivedSensorDataFromUNO, buffer, SENSOR_DATA_SIZE);
    }
    newSensorDataAvailable = true;

  } else {
    Serial.print("I2C RX Error: Received unexpected data size: ");
    Serial.println(howMany);
    while(Wire.available()) {
      Wire.read();
    }
  }
}

void requestEvent() {
  if (isFirstI2CComm || memcmp((void*)&currentCommandData, &lastSentCommandDataToUNO, COMMAND_DATA_SIZE) != 0) {
    Wire.write((byte*)&currentCommandData, COMMAND_DATA_SIZE);
    memcpy(&lastSentCommandDataToUNO, (void*)&currentCommandData, COMMAND_DATA_SIZE);
    Serial.println("I2C TX -> Sent updated command data to UNO.");
  } else {
    Wire.write((byte*)&currentCommandData, COMMAND_DATA_SIZE); // 即使數據未變，也要回應主機的請求
  }
  isFirstI2CComm = false; // 第一次通訊完成
}

void http_sync_callback(void* arg) {
  // 複製當前感測器數據到臨時變數，避免直接操作 volatile packed 數據
  SensorData_t currentSensorDataCopy;
  memcpy(&currentSensorDataCopy, (const void*)&receivedSensorData, sizeof(SensorData_t));

  // 檢查數據是否與上次發送的數據相同，如果相同且不是第一次同步，則跳過詳細日誌
 // if (!firstSync && memcmp(&currentSensorDataCopy, &lastSentSensorData, sizeof(SensorData_t)) == 0) {
//    Serial.print("^");
 //   return;
//  }

  // 更新 lastSentSensorData
  memcpy(&lastSentSensorData, &currentSensorDataCopy, sizeof(SensorData_t));
  firstSync = false;

#if HAS_UNO_I2C
  // 檢查是否有新的感測器數據可用
  if (!newSensorDataAvailable) {
    return;
  }

  // 重置標誌
  newSensorDataAvailable = false;
#endif

  // 確保後端伺服器 IP 已知
  if (backendServerIp.length() == 0) {
    Serial.println("Backend server IP not discovered yet. Skipping HTTP sync.");
    return;
  }

  HTTPClient http;
  String serverPath = "http://" + backendServerIp + ":" + String(backendServerPort) + "/api/sync";

  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");

  // 構建 JSON Payload
  StaticJsonDocument<256> doc; // 調整大小以容納新的特徵值

  // 將 volatile packed 數據複製到臨時變數
  uint8_t status_byte_temp = currentSensorDataCopy.status_byte;
  uint16_t voltage_mv_temp = currentSensorDataCopy.voltage_mv;
  int16_t ultrasonic_distance_cm_temp = currentSensorDataCopy.ultrasonic_distance_cm;
  int16_t thermal_max_temp_temp = currentSensorDataCopy.thermal_max_temp;
  int16_t thermal_min_temp_temp = currentSensorDataCopy.thermal_min_temp;
  uint8_t thermal_hotspot_x_temp = currentSensorDataCopy.thermal_hotspot_x;
  uint8_t thermal_hotspot_y_temp = currentSensorDataCopy.thermal_hotspot_y;

  doc["s"] = status_byte_temp;
  doc["v"] = voltage_mv_temp;
  
  if (ultrasonic_distance_cm_temp != -1) { // 只有當有效時才發送
    doc["u"] = ultrasonic_distance_cm_temp;
  }

  // 添加熱成像特徵值
  doc["t_max"] = thermal_max_temp_temp;
  doc["t_min"] = thermal_min_temp_temp;
  doc["t_hx"] = thermal_hotspot_x_temp;
  doc["t_hy"] = thermal_hotspot_y_temp;

  String requestBody;
  serializeJson(doc, requestBody);

  // 根據數據是否變化來決定是否列印詳細的 Request Body
  if (firstSync || memcmp(&currentSensorDataCopy, &lastSentSensorData, sizeof(SensorData_t)) != 0) {
    Serial.print("Request Body: ");
    Serial.println(requestBody);
  }

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    String response = http.getString();
    if(response.compareTo(old_response) !=0 )
      Serial.println("HTTP Response: " + response);
      old_response = response;

    // 解析回應 JSON
    StaticJsonDocument<128> response_doc; // 根據 CommandData_t 的大小調整
    DeserializationError error = deserializeJson(response_doc, response);

    if (!error) {
      currentCommandData.command_byte = response_doc["c"] | 0;
      currentCommandData.motor_speed = response_doc["m"] | 0;
      currentCommandData.direction_angle = response_doc["d"] | 0;
      currentCommandData.servo_angle = response_doc["a"] | 0;
      currentCommandData.rotation_speed = response_doc["r"] | 0; // 新增：解析旋轉速度
    } else {
      Serial.print("Failed to parse JSON response: ");
      Serial.println(error.c_str());
    }
  } else {
    Serial.printf("HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}void setup_camera() {
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = CAM_PIN_D0;
  config.pin_d1 = CAM_PIN_D1;
  config.pin_d2 = CAM_PIN_D2;
  config.pin_d3 = CAM_PIN_D3;
  config.pin_d4 = CAM_PIN_D4;
  config.pin_d5 = CAM_PIN_D5;
  config.pin_d6 = CAM_PIN_D6;
  config.pin_d7 = CAM_PIN_D7;
  config.pin_xclk = CAM_PIN_XCLK;
  config.pin_pclk = CAM_PIN_PCLK;
  config.pin_vsync = CAM_PIN_VSYNC;
  config.pin_href = CAM_PIN_HREF;
  config.pin_sccb_sda = CAM_PIN_SIOD; // Correct field name
  config.pin_sccb_scl = CAM_PIN_SIOC; // Correct field name
  config.pin_pwdn = CAM_PIN_PWDN;
  config.pin_reset = CAM_PIN_RESET;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_RGB565; // 改回 RGB565，因為 RGB888 不被支援
  config.frame_size = FRAMESIZE_QVGA; // 設定影像解析度為 HVGA (480x320)
  config.jpeg_quality = 40; // 設定 JPEG 影像品質為 60 (較高)
  config.fb_count = 2;
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM;

  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }
  Serial.println("Camera initialized successfully.");
}

void handle_stream() {
  WiFiClient client = server.client();
  String boundary = "123456789000000000000987654321";

  // 手動發送 HTTP 標頭，確保使用 HTTP/1.1 協定
  client.print("HTTP/1.1 200 OK\r\n");
  client.print("Content-Type: multipart/x-mixed-replace; boundary=" + boundary + "\r\n");
  client.print("Connection: keep-alive\r\n"); // 對於串流很重要
  client.print("\r\n"); // 標頭結束的空行

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      continue;
    }

    // 將 RGB565 幀轉換為 JPEG 格式
    uint8_t *jpg_buf = NULL;
    size_t jpg_buf_len = 0;
    bool jpeg_converted = fmt2jpg(fb->buf, fb->len, fb->width, fb->height, PIXFORMAT_RGB565, 60, &jpg_buf, &jpg_buf_len);
    esp_camera_fb_return(fb); // 釋放原始幀緩衝區
    fb = NULL;

    if (!jpeg_converted) {
      Serial.println("JPEG 轉換失敗");
      if (jpg_buf) free(jpg_buf); // 確保在失敗時也釋放記憶體
      continue;
    }

    // 發送多部分串流的邊界和影像標頭
    client.print("--" + boundary + "\r\n");
    client.print("Content-Type: image/jpeg\r\n");

    // 使用 snprintf 安全地格式化 Content-Length 標頭
    char contentLengthHeader[64]; // 足夠的緩衝區用於 "Content-Length: XXXXX\r\n"
    snprintf(contentLengthHeader, sizeof(contentLengthHeader), "Content-Length: %u\r\n", jpg_buf_len);
    client.print(contentLengthHeader);
    

    client.print("\r\n"); // 影像標頭結束的空行

    // 發送影像資料
    client.write(jpg_buf, jpg_buf_len);
    client.print("\r\n"); // 影像資料結束的空行
    
    if (jpg_buf) {
      free(jpg_buf); // 釋放 JPEG 緩衝區
    }
  }
}

void registerCamera() {
  if (backendServerIp.length() == 0) {
    Serial.println("Backend server IP not discovered yet. Cannot register camera.");
    return;
  }

  HTTPClient http;
  String serverPath = "http://" + backendServerIp + ":" + String(backendServerPort) + "/api/register_camera";

  http.begin(serverPath);
  http.addHeader("Content-Type", "application/json");

  StaticJsonDocument<256> doc; // Adjust size as needed for registration payload
  doc["i"] = WiFi.localIP().toString(); // 將鍵名改為 'i'

  String requestBody;
  serializeJson(doc, requestBody);

  Serial.print("Sending camera registration to: ");
  Serial.println(serverPath);
  Serial.print("Request Body: ");
  Serial.println(requestBody);

  int httpResponseCode = http.POST(requestBody);

  if (httpResponseCode > 0) {
    Serial.printf("Camera registration HTTP Response code: %d\n", httpResponseCode);
    String response = http.getString();
    Serial.println("Camera registration HTTP Response: " + response);
    // You might want to parse the response here to confirm successful registration
  } else {
    Serial.printf("Camera registration HTTP Error: %s\n", http.errorToString(httpResponseCode).c_str());
  }

  http.end();
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(false);
  Serial.println();

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi connected");
  Serial.print("Camera Stream Ready! Go to http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");

  // Setup Camera
  setup_camera();

  // Start Web Server
  server.on("/stream", HTTP_GET, handle_stream);
  server.begin();
  Serial.println("Web server started.");

  // 初始化 UDP 監聽
#if USE_UDP_DISCOVERY
  if (udp.listen(UDP_BROADCAST_PORT)) {
    Serial.print("UDP listening on port ");
    Serial.println(UDP_BROADCAST_PORT);
    udp.onPacket([](AsyncUDPPacket packet) {
      if (serverIpConfirmed) return; // 如果已經確認 IP，則直接忽略後續的廣播包

      Serial.print("UDP Packet Type: ");
      Serial.print(packet.isBroadcast() ? "Broadcast" : packet.isMulticast() ? "Multicast" : "Unicast");
      Serial.print(", From: ");
      Serial.print(packet.remoteIP());
      Serial.print(":");
      Serial.print(packet.remotePort());
      Serial.print(", Length: ");
      Serial.print(packet.length());
      Serial.print(", Data: ");
      String data = (char*)packet.data();
      Serial.println(data);
      Serial.print("Raw UDP Packet Data (Hex): ");
      for (int i = 0; i < packet.length(); i++) {
        if (packet.data()[i] < 16) Serial.print("0");
        Serial.print(packet.data()[i], HEX);
        Serial.print(" ");
      }
      Serial.println();

      // 解析廣播訊息，尋找伺服器 IP
      if (data.startsWith("MINIAUTO_SERVER_IP:")) {
        int firstColon = data.indexOf(':');
        int secondColon = data.indexOf(':', firstColon + 1);
        if (firstColon != -1 && secondColon != -1) {
          backendServerIp = data.substring(firstColon + 1, secondColon);
          backendServerPort = data.substring(secondColon + 1).toInt();
          Serial.print("Discovered Backend Server IP: ");
          Serial.print(backendServerIp);
          Serial.print(", Port: ");
          Serial.println(backendServerPort);
          serverIpConfirmed = true; // 標記為已確認

          // 如果 HTTP 同步定時器尚未啟動，則啟動它
          if (!httpSyncTimerStarted) {
            const esp_timer_create_args_t http_sync_timer_args = {
              .callback = &http_sync_callback, // 定時器回調函數
              .name = "http_sync_timer" // 定時器名稱
            };
            ESP_ERROR_CHECK(esp_timer_create(&http_sync_timer_args, &http_sync_timer));
            ESP_ERROR_CHECK(esp_timer_start_periodic(http_sync_timer, HTTP_SYNC_INTERVAL_MS * 1000)); // 啟動定時器，單位微秒
            Serial.println("HTTP Sync Timer initialized and started.");
            httpSyncTimerStarted = true;
          }

          // 如果攝影機尚未註冊，則註冊它
          if (!cameraRegistered) {
            registerCamera();
            cameraRegistered = true;
          }
        }
      }
    });
  } else {
    Serial.println("Failed to start UDP listener!");
  }
#else // USE_UDP_DISCOVERY is 0
  backendServerIp = MANUAL_BACKEND_SERVER_IP;
  backendServerPort = MANUAL_BACKEND_SERVER_PORT;
  Serial.print("Using manual Backend Server IP: ");
  Serial.print(backendServerIp);
  Serial.print(", Port: ");
  Serial.println(backendServerPort);

  // 啟動 HTTP 同步定時器
  const esp_timer_create_args_t http_sync_timer_args = {
    .callback = &http_sync_callback, // 定時器回調函數
    .name = "http_sync_timer" // 定時器名稱
  };
  ESP_ERROR_CHECK(esp_timer_create(&http_sync_timer_args, &http_sync_timer));
  ESP_ERROR_CHECK(esp_timer_start_periodic(http_sync_timer, HTTP_SYNC_INTERVAL_MS * 1000)); // 啟動定時器，單位微秒
  Serial.println("HTTP Sync Timer initialized and started.");
  httpSyncTimerStarted = true;

  // 註冊攝影機
  registerCamera();
  cameraRegistered = true;
#endif

  // Initialize I2C as slave
  Wire.begin(I2C_SLAVE_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN, 100000U);
  Wire.onReceive(receiveEvent); // 註冊接收事件回調
  Wire.onRequest(requestEvent); // 註冊請求事件回調
  Serial.println("I2C Slave initialized on SDA=47, SCL=48 at 100kHz.");

  // 初始化命令數據為停止狀態
  currentCommandData.command_byte = 0;
  currentCommandData.motor_speed = 0;
  currentCommandData.direction_angle = 0;
  currentCommandData.servo_angle = 90;
}

void loop() {
  server.handleClient();
  // esp_timer 會在背景運行，不需要在 loop 中額外呼叫
  // AsyncUDP 也在背景運行，不需要在 loop 中額外呼叫

  // Check for serial input to confirm ESP32 is alive
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    command.trim(); // Remove any whitespace
    if (command.length() > 0) {
      Serial.println("ESP32 is alive!");
    }
  }
}