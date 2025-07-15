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
  config.pixel_format = PIXFORMAT_JPEG;
  config.frame_size = FRAMESIZE_SVGA;
  config.jpeg_quality = 10;
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
  String response = "--" + boundary + "\r\n";
  response += "Content-Type: multipart/x-mixed-replace; boundary=" + boundary + "\r\n\r\n";
  server.sendContent(response);

  while (client.connected()) {
    camera_fb_t *fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      continue;
    }

    client.print("--" + boundary + "\r\n");
    client.print("Content-Type: image/jpeg\r\n");
    client.print("Content-Length: " + String(fb->len) + "\r\n\r\n");
    client.write(fb->buf, fb->len);
    client.print("\r\n");
    
    esp_camera_fb_return(fb);
  }
}

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
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
