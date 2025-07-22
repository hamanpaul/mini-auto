#include <Arduino.h>
#include "Ultrasound.h"


// 模組物件 (需搭配原廠 Ultrasound 類別)
Ultrasound ultrasound;
// 馬達控制腳位
const static uint8_t pwm_min = 2;
const static uint8_t motorpwmPin[4] = {10, 9, 6, 11};
const static uint8_t motordirectionPin[4] = {12, 8, 7, 13};

// 超音波濾波設定
#define FILTER_N 3
extern uint16_t filter_buf[FILTER_N + 1];

// 車輛狀態控制變數
uint16_t car_derection = 0;
uint8_t speed_data = 0;
int8_t car_rot = 0;
bool obstacle_avoid_mode = false;


// 函式宣告
void Motor_Init(void);
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot);
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot, bool drift);
void Motors_Set(int8_t Motor_0, int8_t Motor_1, int8_t Motor_2, int8_t Motor_3);
int Filter();
uint16_t ultrasonic_distance();

void setup() {
  Serial.begin(9600);
  Serial.setTimeout(500);
  Motor_Init();

  Serial.println("控制鍵說明:");
  Serial.println("w/a/s/d 移動, q/e 旋轉, x 停止");
  Serial.println("1 開啟避障, 2 關閉避障");
}

void loop() {
  if (Serial.available()) {
    char key = Serial.read();
    if (key == '\r' || key == '\n') return;
    
    switch (key) {
      case 'w':
        car_derection = 0;
        speed_data = 60;
        car_rot = 0;
        Serial.println("動作：前進");
        break;
      case 's':
        car_derection = 180;
        speed_data = 60;
        car_rot = 0;
        Serial.println("動作：後退");
        break;
      case 'a':
        car_derection = 90;
        speed_data = 60;
        car_rot = 0;
        Serial.println("動作：左移");
        break;
      case 'd':
        car_derection = 270;
        speed_data = 60;
        car_rot = 0;
        Serial.println("動作：右移");
        break;
      case 'q':
        speed_data = 0;
        car_rot = 50;
        Serial.println("動作：左轉（原地）");
        break;
      case 'e':
        speed_data = 0;
        car_rot = -50;
        Serial.println("動作：右轉（原地）");
        break;
      case 'x':
        speed_data = 0;
        car_rot = 0;
        Serial.println("動作：停止");
        break;
      case '1':
        obstacle_avoid_mode = true;
        Serial.println("已開啟避障模式");
        break;
      case '2':
        obstacle_avoid_mode = false;
        Serial.println("已關閉避障模式");
        break;
      default:
        Serial.println("未知指令");
        return;
    }

    // 印出目前控制參數
    Serial.print("方向："); Serial.print(car_derection); Serial.print(" 度，");
    Serial.print("速度："); Serial.print(speed_data); Serial.print("，");
    Serial.print("自轉："); Serial.println(car_rot);
  }

  if (obstacle_avoid_mode) {
    uint16_t distance = ultrasonic_distance();
    if (distance > 0 && distance <= 180) {
      speed_data = 0;
      car_rot = -50;
      Serial.println("避障觸發：停止移動並轉向");
    }
  }

  Velocity_Controller(car_derection, speed_data, car_rot);
  delay(20);
}

// 簡化呼叫版本
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot) {
  Velocity_Controller(angle, velocity, rot, false);
}

// 電機初始化
void Motor_Init(void) {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(motordirectionPin[i], OUTPUT);
  }
  Velocity_Controller(0, 0, 0);
}

// 主控制函式
void Velocity_Controller(uint16_t angle, uint8_t velocity, int8_t rot, bool drift) {
  int8_t v0, v1, v2, v3;
  float speed = (rot == 0) ? 1 : 0.5;
  angle += 90;
  float rad = angle * PI / 180;
  velocity /= sqrt(2);

  if (drift) {
    v0 = (velocity * sin(rad) - velocity * cos(rad)) * speed;
    v1 = (velocity * sin(rad) + velocity * cos(rad)) * speed;
    v2 = v0 - rot * speed * 2;
    v3 = v1 + rot * speed * 2;
  } else {
    v0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot * speed;
    v1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot * speed;
    v2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot * speed;
    v3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot * speed;
  }

  Motors_Set(v0, v1, v2, v3);
}

// 實際驅動馬達
void Motors_Set(int8_t m0, int8_t m1, int8_t m2, int8_t m3) {
  int8_t motors[4] = { m0, m1, m2, m3 };
  int8_t pwm_set[4];
  bool direction[4] = { 1, 0, 0, 1 }; // 預設方向

  for (uint8_t i = 0; i < 4; ++i) {
    if (motors[i] < 0) direction[i] = !direction[i];
    pwm_set[i] = (motors[i] == 0) ? 0 : map(abs(motors[i]), 0, 100, pwm_min, 255);
    digitalWrite(motordirectionPin[i], direction[i]);
    analogWrite(motorpwmPin[i], pwm_set[i]);
  }
}

// 超音波濾波
int Filter() {
  int sum = 0;
  filter_buf[FILTER_N] = ultrasound.GetDistance();
  for (int i = 0; i < FILTER_N; i++) {
    filter_buf[i] = filter_buf[i + 1];
    sum += filter_buf[i];
  }
  return sum / FILTER_N;
}

// 超音波 + RGB 效果
uint16_t ultrasonic_distance() {
  uint8_t s;
  uint16_t distance = Filter();

  Serial.print("Distance: ");
  Serial.print(distance);
  Serial.println(" mm");

  if (distance > 0 && distance <= 80) {
    ultrasound.Breathing(1, 0, 0, 1, 0, 0); // 紅色呼吸
  } else if (distance <= 180) {
    s = map(distance, 80, 180, 0, 255);
    ultrasound.Color(255 - s, 0, 0, 255 - s, 0, 0);
  } else if (distance <= 320) {
    s = map(distance, 180, 320, 0, 255);
    ultrasound.Color(0, 0, s, 0, 0, s);
  } else if (distance <= 500) {
    s = map(distance, 320, 500, 0, 255);
    ultrasound.Color(0, s, 255 - s, 0, s, 255 - s);
  } else {
    ultrasound.Color(0, 255, 0, 0, 255, 0);
  }

  return distance;
}
