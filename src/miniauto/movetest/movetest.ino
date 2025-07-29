#include "MotorController.h"

MotorController motor;

void setup() {
  Serial.begin(9600);
  motor.init();

  Serial.println("控制鍵說明:");
  Serial.println("w/a/s/d 移動, q/e 旋轉, x 停止");
}

void loop() {
  if (Serial.available()) {
    char key = Serial.read();
    if (key == '\r' || key == '\n') return;
    
    motor.move(key);
  }
}