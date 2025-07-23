/*
  ESP32-S3-WROOM Camera Web Server for Miniauto

  This sketch provides an MJPEG video stream from a GC2145 camera on an ESP32-S3 module.
  It is designed to work within the Arduino framework and acts as an I2C slave to provide
  its IP address to the main Arduino UNO controller.

  - Based on official Espressif esp32-camera examples.
  - Uses Arduino WebServer library for the MJPEG stream.
  - Provides IP address over I2C.
*/

#include "esp_camera.h"
#include "WiFi.h"
#include "WebServer.h"
#include "Wire.h"

// --- WiFi Configuration ---
const char* ssid = "Hcedu01";
const char* password = "035260089";

// --- I2C Slave Configuration ---
#define I2C_SLAVE_ADDRESS 0x53
#define I2C_SDA_PIN 21
#define I2C_SCL_PIN 22

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

void setup_camera() {
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
  config.frame_size = FRAMESIZE_HVGA; // 設定影像解析度為 HVGA (480x320)
  config.jpeg_quality = 60; // 設定 JPEG 影像品質為 60 (較高)
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

  // Initialize I2C as slave
  Wire.begin(I2C_SLAVE_ADDRESS, I2C_SDA_PIN, I2C_SCL_PIN);
  Wire.onRequest(sendIpAddress);
  Serial.println("I2C Slave initialized.");
}

void loop() {
  server.handleClient();
}

void sendIpAddress() {
  String ipAddress = WiFi.localIP().toString();
  Serial.print("Sending IP via I2C: ");
  Serial.println(ipAddress);
  for (char c : ipAddress) {
    Wire.write((uint8_t)c);
  }
}
