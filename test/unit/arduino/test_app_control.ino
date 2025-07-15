#include <ArduinoUnit.h>
#include "src/miniauto/app_control.ino"

void setup() {
  Serial.begin(9600);
}

void loop() {
  Test::run();
}

test(example_test) {
  // Example test: check if a basic function from app_control.ino works as expected
  // For demonstration, let's assume app_control.ino has a function like `int add(int a, int b)`
  // If not, you'll need to adapt this to an actual function you want to test.
  // For now, we'll just assert true.
  assertEqual(1 + 1, 2);
}
