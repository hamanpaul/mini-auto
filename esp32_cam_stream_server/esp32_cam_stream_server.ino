#include "esp_camera.h"
#include "esp_timer.h"
#include "img_converters.h"
#include "fb_gfx.h"
#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "dl_lib.h"
#include "esp_http_server.h"

#include "WiFi.h"

// --- WiFi Configuration ---
const char* ssid = "YOUR_WIFI_SSID";         // <<<<<<< CHANGE THIS
const char* password = "YOUR_WIFI_PASSWORD"; // <<<<<<< CHANGE THIS

// --- Camera Pin Definitions for ESP32-S3-WROOM (GC2415 compatible) ---
// These pins are derived from the esp32-camera/examples/camera_example/main/take_picture.c
#define CAM_PIN_PWDN 38
#define CAM_PIN_RESET -1   // software reset will be performed
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

// --- Camera Configuration ---
camera_config_t config;

void setup_camera() {
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
  config.pin_sccb_sda = CAM_PIN_SIOD;
  config.pin_sccb_scl = CAM_PIN_SIOC;
  config.pin_pwdn = CAM_PIN_PWDN;
  config.pin_reset = CAM_PIN_RESET;
  config.xclk_freq_hz = 20000000; // 20MHz XCLK
  config.pixel_format = PIXFORMAT_JPEG; // Use JPEG for streaming efficiency
  config.frame_size = FRAMESIZE_SVGA; // SVGA (800x600) is a good balance
  config.jpeg_quality = 10; // 0-63, lower means higher quality
  config.fb_count = 2; // Use 2 frame buffers for smoother streaming
  config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
  config.fb_location = CAMERA_FB_IN_PSRAM; // Assuming PSRAM is available on S3-CAM

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    ESP.restart();
  }

  // Adjust camera settings for better image quality (optional)
  sensor_t *s = esp_camera_sensor_get();
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_hmirror(s, 1); // flip it back
  }
  // For GC2145/GC2415, you might need specific settings here if image is inverted or mirrored
  // s->set_vflip(s, 1); // Example: s->set_vflip(s, 1); 
  // s->set_hmirror(s, 1); // Example: s->set_hmirror(s, 1);
}

// --- Stream Server Handlers ---

// Handler for the main stream endpoint
esp_err_t stream_handler(httpd_req_t *req) {
  camera_fb_t *fb = NULL;
  esp_err_t res = ESP_OK;
  size_t _jpg_buf_len = 0;
  uint8_t *_jpg_buf = NULL;
  char *part_buf[64];
  static int64_t last_frame = 0;
  if (!last_frame) {
    last_frame = esp_timer_get_time();
  }

  res = httpd_resp_set_type(req, "multipart/x-mixed-replace;boundary=123456789000000000000987654321");
  if (res != ESP_OK) {
    return res;
  }

  while (true) {
    fb = esp_camera_fb_get();
    if (!fb) {
      Serial.println("Camera capture failed");
      res = ESP_FAIL;
    } else {
      if (fb->format != PIXFORMAT_JPEG) {
        bool jpeg_converted = frame2jpg(fb, 80, &_jpg_buf, &_jpg_buf_len);
        esp_camera_fb_return(fb);
        fb = NULL;
        if (!jpeg_converted) {
          Serial.println("JPEG compression failed");
          res = ESP_FAIL;
        }
      } else {
        _jpg_buf = fb->buf;
        _jpg_buf_len = fb->len;
      }
    }
    if (res == ESP_OK) {
      size_t hlen = snprintf((char *)part_buf, 64, "--123456789000000000000987654321\nContent-Type: image/jpeg\nContent-Length: %u\n\n", _jpg_buf_len);
      httpd_resp_send_chunk(req, (const char *)part_buf, hlen);
      httpd_resp_send_chunk(req, (const char *)_jpg_buf, _jpg_buf_len);
      httpd_resp_send_chunk(req, "\n", 1);
    }
    if (fb) {
      esp_camera_fb_return(fb);
      fb = NULL;
      _jpg_buf = NULL;
    } else if (_jpg_buf) {
      free(_jpg_buf);
      _jpg_buf = NULL;
    }
    if (res != ESP_OK) {
      break;
    }
    int64_t fr_end = esp_timer_get_time();
    int64_t frame_time = fr_end - last_frame;
    last_frame = fr_end;
    frame_time /= 1000;
    Serial.printf("MJPEG: %uB %ums (%.1ffps)\n", (uint32_t)_jpg_buf_len, (uint32_t)frame_time, 1000.0 / (uint32_t)frame_time);
  }

  last_frame = 0;
  return res;
}

// --- Web Server Setup ---
httpd_handle_t start_webserver(void) {
  httpd_config_t config = HTTPD_DEFAULT_CONFIG();
  httpd_handle_t server = NULL;
  config.uri_httpd_enable_basic_auth = false;

  httpd_uri_t stream_uri = {
    .uri = "/stream",
    .method = HTTP_GET,
    .handler = stream_handler,
    .user_ctx = NULL
  };

  Serial.printf("Starting web server on port: '%d'\n", config.server_port);
  if (httpd_start(&server, &config) == ESP_OK) {
    httpd_register_uri_handler(server, &stream_uri);
  }
  return server;
}

// --- Arduino Setup and Loop ---
void setup() {
  WRITE_PERI_REG(RTC_CNTL_BROWN_OUT_REG, 0); // disable brownout detector
  Serial.begin(115200);
  Serial.setDebugOutput(false);

  // Connect to WiFi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("Camera Stream Ready! Go to http://");
  Serial.print(WiFi.localIP());
  Serial.println("/stream");

  // Setup Camera
  setup_camera();

  // Start Web Server
  start_webserver();
}

void loop() {
  // Nothing much to do in loop, as the web server runs in the background
  delay(10);
}
