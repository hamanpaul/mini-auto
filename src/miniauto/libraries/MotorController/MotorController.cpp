#include "MotorController.h"

const uint8_t MotorController::motorpwmPin[4] = {10, 9, 6, 11};
const uint8_t MotorController::motordirectionPin[4] = {12, 8, 7, 13};

MotorController::MotorController() {}

void MotorController::init() {
  for (uint8_t i = 0; i < 4; i++) {
    pinMode(motordirectionPin[i], OUTPUT);
  }
  stop();
}

void MotorController::move(uint16_t angle, uint8_t velocity, int8_t rot_input) {
  int16_t v0, v1, v2, v3;
  float speed = (rot_input == 0) ? 1 : 1;
  // 移除覆蓋 rot 值的程式碼
  // rot=(angle == 270)? 100 :(angle == 90)? -100 : 0; 
  if (velocity == 0) {
    velocity = 0;
  } else {
    velocity=(velocity < 40 )? 40 : (velocity > 140 )? 140 : velocity;
  }
  angle += 90;
  float rad = angle * PI / 180;
  velocity /= sqrt(2);
  
  Serial.print("a:");
  Serial.print(angle);
  Serial.print("\tv:");
  Serial.print(velocity);
  Serial.print("\t");
  Serial.print("\tr_input:");
  Serial.print(rot_input);
  Serial.println();
  

  v0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot_input * speed;
  v1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot_input * speed;
  v2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot_input * speed;
  v3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot_input * speed;

  // 正規化馬達速度到 0-100 範圍
  int16_t max_abs_v = 0;
  max_abs_v = max((int16_t)max_abs_v, (int16_t)abs(v0));
  max_abs_v = max((int16_t)max_abs_v, (int16_t)abs(v1));
  max_abs_v = max((int16_t)max_abs_v, (int16_t)abs(v2));
  max_abs_v = max((int16_t)max_abs_v, (int16_t)abs(v3));

  if (max_abs_v > 100) { // 如果有任何一個值超過 100，則進行縮放
    v0 = map(v0, -max_abs_v, max_abs_v, -100, 100);
    v1 = map(v1, -max_abs_v, max_abs_v, -100, 100);
    v2 = map(v2, -max_abs_v, max_abs_v, -100, 100);
    v3 = map(v3, -max_abs_v, max_abs_v, -100, 100);
  }

  setMotors(v0, v1, v2, v3);
}

void MotorController::move(char direction) {
  switch (direction) {
    case 'w':
      move(0, 60, 0);
      break;
    case 's':
      move(180, 60, 0);
      break;
    case 'a':
      move(0, 0, 50);
      break;
    case 'd':
      move(0, 0, -50);
      break;
    case 'q':
      move(0, 0, 50);
      break;
    case 'e':
      move(0, 0, -50);
      break;
    case 'x':
      stop();
      break;
  }
}

void MotorController::stop() {
  setMotors(0, 0, 0, 0);
}

void MotorController::setMotors(int16_t m0, int16_t m1, int16_t m2, int16_t m3) {
  int16_t motors[4] = { m0, m1, m2, m3 };
  int8_t pwm_set[4];
  bool direction[4] = { 1, 0, 0, 1 }; // 預設方向

  for (uint8_t i = 0; i < 4; ++i) {
	Serial.print("m[");
	Serial.print(i);
	Serial.print("]:");
	Serial.print(motors[i]);
	Serial.print("\t");
    if (motors[i] < 0) direction[i] = !direction[i];
    pwm_set[i] = (motors[i] == 0) ? 0 : map(abs(motors[i]), 0, 100, pwm_min, 255);
    Serial.print("pwm_set[");
    Serial.print(i);
    Serial.print("]:");
    Serial.print(pwm_set[i]);
    Serial.print("\t");
    digitalWrite(motordirectionPin[i], direction[i]);
    analogWrite(motorpwmPin[i], pwm_set[i]);
  }
  Serial.println();
}
