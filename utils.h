#include <Arduino.h>

void waitForButtonPress(int pin) {
  int pressed = digitalRead(pin);
  while (pressed == LOW) {
    pressed = digitalRead(pin);
  }
}

void waitForButtonRelease(int pin) {
  int pressed = digitalRead(pin);
  while (pressed == HIGH) {
    pressed = digitalRead(pin);
  }
}