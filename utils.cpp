#include <utils.h>

int getAnyButtonPress(int* pins, int numOfPins) {
  for (int i = 0; i < numOfPins; i++) {
    if (digitalRead(pins[i]) == HIGH) {
      return pins[i];
    }
  }
  return -1;
}

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

/// @brief waits for any of the buttons to be pressed
/// @param pins the pins of the buttons
/// @param numOfPins the number of pins
/// @return which button was pressed
int waitForAnyButtonPress(int* pins, int numOfPins) {
  int pressed = -1;
  while (pressed == -1) {
    for (int i = 0; i < numOfPins; i++) {
      if (digitalRead(pins[i]) == HIGH) {
        pressed = pins[i];
      }
    }
  }
  return pressed;
}