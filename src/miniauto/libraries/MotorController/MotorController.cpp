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

void MotorController::move(uint16_t angle, uint8_t velocity, int8_t rot) {
  int8_t v0, v1, v2, v3;
  float speed = (rot == 0) ? 1 : 0.5;
  angle += 90;
  float rad = angle * PI / 180;
  velocity /= sqrt(2);

  v0 = (velocity * sin(rad) - velocity * cos(rad)) * speed + rot * speed;
  v1 = (velocity * sin(rad) + velocity * cos(rad)) * speed - rot * speed;
  v2 = (velocity * sin(rad) - velocity * cos(rad)) * speed - rot * speed;
  v3 = (velocity * sin(rad) + velocity * cos(rad)) * speed + rot * speed;

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
      move(90, 60, 0);
      break;
    case 'd':
      move(270, 60, 0);
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

void MotorController::setMotors(int8_t m0, int8_t m1, int8_t m2, int8_t m3) {
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
