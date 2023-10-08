#include <utils.h>

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

/// @brief waits for one of the buttons to pressed
/// @param pinA the pin of the first button
/// @param pinB the pin of the second button
/// @return which button was pressed
int waitForEitherButtonPress(int pinA, int pinB) {
  int aPressed = digitalRead(pinA);
  int bPressed = digitalRead(pinB);
  while (true) {
    aPressed = digitalRead(pinA);
    bPressed = digitalRead(pinB);
    if (aPressed == HIGH) {
      return pinA;
    } else if (bPressed == HIGH) {
      return pinB;
    }
  }
  return pinA;
}