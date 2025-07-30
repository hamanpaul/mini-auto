#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include <Arduino.h>

class MotorController {
public:
  MotorController();
  void init();
  void move(uint16_t angle, uint8_t velocity, int8_t rot_input);
  void move(char direction);
  void stop();

private:
  void setMotors(int8_t m0, int8_t m1, int8_t m2, int8_t m3);
  const static uint8_t pwm_min = 2;
  const static uint8_t motorpwmPin[4];
  const static uint8_t motordirectionPin[4];
};

#endif // MOTOR_CONTROLLER_H