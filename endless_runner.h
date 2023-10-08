#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <collision.h>
#include <utils.h>
#include <game_base_class.h>
#include <Arduino.h>

#ifndef ENDLESS_RUNNER_H
#define ENDLESS_RUNNER_H
struct Obstacle {
  int x;
  int y;
  int width;
  int height;
  bool is_active;
};

class EndlessRunner : public Game
{
private:
    /* data */
    int buttonPinA;
    int buttonPinB;
    Obstacle obstacles[10]; // An static pool of `n` Obstacle objects
    int playerWidth = 10;
    int playerHeight = 10;
    double playerY; // the y position of the player
    short playerState = 0; // 0 = idle, 1 = jumping, 2 = falling
    int timeSinceLastObjectSpawn = 0; // this is the amount of time since an object has been spawned in milliseconds
    double playerAccelerationAdded = 0; // the amount of acceleration added to the player
    u_int8_t gameOver = false; // wether the game is over or not
    float score = 0; // the score of the player
    int screenWidth;
    int screenHeight;
    Adafruit_SSD1306 oled;

    /* Constants */
    const int playerXOffset = 15;
    const int playerYOffset = 5;
    const int playerJumpPower = 35;
    const int msToJump = 1400;
    const int obstacleSpeed = 10;
    const int obstacleSpawnRateMax = 2500;
    const int obstacleSpawnRateMin = 2000;

    /* private functions */
    bool check_if_out_of_bounds(int x, int y, int width, int height);
    double get_acceleration(int msToJump, int deltaTime);
public:
    EndlessRunner(int buttonPinA, int buttonPinB, int screenWidth, int screenHeight, Adafruit_SSD1306* oled);
    PlayerState play(double deltaTime);
    ~EndlessRunner();
    void reset();
};

#endif // ENDLESS_RUNNER_H