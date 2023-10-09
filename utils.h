#ifndef UTILS_H
#define UTILS_H

#include <Arduino.h>

enum class PlayerState {
    quit,
    restart,
    still_playing
};

void waitForButtonPress(int pin);
void waitForButtonRelease(int pin);
int waitForEitherButtonPress(int pinA, int pinB);
int waitForAnyButtonPress(int* pins, int numOfPins);
int getAnyButtonPress(int* pins, int numOfPins);

#endif // UTILS_H
