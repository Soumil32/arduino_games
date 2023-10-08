#ifndef GAME_BASE_CLASS_H
#define GAME_BASE_CLASS_H

#include <Arduino.h>

#ifndef GAME_H
#define GAME_H

class Game {
public:
    virtual void reset() = 0;
    virtual uint8_t play(double deltaTime) = 0;
};

#endif // GAME_H

#endif // GAME_BASE_CLASS_H
