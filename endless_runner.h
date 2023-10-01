#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <collision.h>
#include <utils.h>

struct Obstacle {
  int x;
  int y;
  int width;
  int height;
  bool is_active;
};

class endless_runner
{
private:
    /* data */
    int buttonPinA;
    Obstacle obstacles[10]; // An static pool of `n` Obstacle objects
    int playerWidth = 10;
    int playerHeight = 10;
    double playerY; // the y position of the player
    short playerState = 0; // 0 = idle, 1 = jumping, 2 = falling
    int timeSinceLastObjectSpawn = 0; // this is the amount of time since an object has been spawned in milliseconds
    double playerAccelerationAdded = 0; // the amount of acceleration added to the player
    bool gameOver = false; // wether the game is over or not
    float score = 0; // the score of the player
    int screenWidth;
    int screenHeight;
    Adafruit_SSD1306 oled;

    /* Constants */
    const int playerXOffset = 15;
    const int playerYOffset = 5;
    const int playerJumpPower = 35;
    const int msToJump = 1700;
    const int obstacleSpeed = 10;
    const int obstacleSpawnRateMax = 2500;
    const int obstacleSpawnRateMin = 2000;

    /* private functions */
    bool check_if_out_of_bounds(int x, int y, int width, int height);
    double get_acceleration(int msToJump, int deltaTime) {
        // the acceleration per millisecond. this will only have to be calculated once for the program
        static double accelerationPerMs = (double)playerJumpPower / (double)msToJump;
        return accelerationPerMs * deltaTime; // multiplied by how many milliseconds have passed
    }
public:
    endless_runner(int buttonPinA, int screenWidth, int screenHeight, Adafruit_SSD1306 oled);
    bool play(double deltaTime);
    ~endless_runner();
    void reset() {
        this->playerY = this->screenHeight - this->playerHeight - this->playerYOffset;
        this->playerState = 0;
        this->timeSinceLastObjectSpawn = 0;
        this->playerAccelerationAdded = 0;
        this->gameOver = false;
        this->score = 0;
        for (int i = 0; i < 10; i++) {
            Obstacle obstacle = obstacles[i];
            obstacle.is_active = false;
            obstacles[i] = obstacle;
        }
    }
};

endless_runner::endless_runner(int buttonPinA, int screenWidth, int screenHeight, Adafruit_SSD1306 oled)
{
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    this->playerY = screenHeight - playerHeight - playerYOffset;
    this->buttonPinA = buttonPinA;
    this->oled = oled;
}

endless_runner::~endless_runner()
{
}

bool endless_runner::check_if_out_of_bounds(int x, int y, int width, int height) {
  if ((x) >= this->screenWidth) {
      return true;
  } else if ((y + height) >= this->screenHeight) {
      return true;
  } else if (x <= 0) {
      return true;
  } else if (y <= 0) {
      return true;
  } else {
      return false;
  }
}

/// @brief plays a frame of the game
/// @param deltaTime the time since the start of the last frame in milliseconds
/// @return if the game is over or not
bool endless_runner::play(double deltaTime) {
    int pressed = digitalRead(this->buttonPinA);
    this->oled.clearDisplay();
    if (pressed == HIGH && this->playerState != 2) { // if the button is pressed and the player is not falling
        this->playerState = 1;
    }
    if (this->playerState == 1) {
        double player_acceleration_this_frame = get_acceleration(this->msToJump / 2 , deltaTime * 1000);
        this->playerY -= player_acceleration_this_frame;
        this->playerAccelerationAdded += player_acceleration_this_frame;
        if (this->playerAccelerationAdded >= this->playerJumpPower) {
        this->playerState = 2;
        }
    } else if (this->playerState == 2) {
        double player_acceleration_this_frame = get_acceleration(this->msToJump / 2, deltaTime * 1000 );
        this->playerY += player_acceleration_this_frame;
        this->playerAccelerationAdded -= player_acceleration_this_frame;
        if (this->playerAccelerationAdded <= 0) {
        this->playerState = 0;
        this->playerAccelerationAdded = 0;
        this->playerY = this->screenHeight - this->playerHeight - this->playerYOffset;
        }
    }

    int timeToWait = random(this->obstacleSpawnRateMin, this->obstacleSpawnRateMax);
    if (this->timeSinceLastObjectSpawn >= timeToWait) {
        // generate a random number which will decide if an obstacle will be spawned
        int random_number = random(0, 3);
        if (random_number == 1) {
        int obstacle_width = random(8, 11);
        int obstacle_height = random(8, 15);
        int obstacle_x = this->screenWidth - obstacle_width;
        int obstacle_y = this->screenHeight - obstacle_height - this->playerYOffset;
        Obstacle obstacle = {obstacle_x, obstacle_y, obstacle_width, obstacle_height, true};
        for (int i = 0; i < 10; i++) {
            if (!obstacles[i].is_active) {
            obstacles[i] = obstacle;
            break;
            }
        }
        this->timeSinceLastObjectSpawn = 0;
        }
    } else {
        this->timeSinceLastObjectSpawn += deltaTime * 1000;
    }

    for (int i = 0; i < 10; i++) {
        Obstacle obstacle = obstacles[i];
        if (obstacle.is_active) {
        obstacle.x -= this->obstacleSpeed * deltaTime;
        // do some AABB collision detection
        if (checkAABB(this->playerXOffset, this->playerY, this->playerWidth, this->playerHeight, obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
            if (this->playerXOffset + this->playerWidth >= obstacle.x && this->playerXOffset <= obstacle.x + obstacle.width) {
            this->gameOver = true;
            }
        }
        if (check_if_out_of_bounds(obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
            obstacle.is_active = false;
            obstacles[i] = obstacle;
            continue;
        }
        obstacles[i] = obstacle;
        this->oled.drawRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height, SSD1306_WHITE);
        }
    }

    this->oled.drawRect(this->playerXOffset, this->playerY, this->playerWidth, this->playerHeight, SSD1306_WHITE);
    this->oled.drawFastHLine(0, this->screenHeight - this->playerYOffset, this->screenWidth, SSD1306_WHITE);
    if (this->gameOver) {
        this->oled.clearDisplay();
        this->oled.setTextSize(2);
        this->oled.setCursor(10, 0);
        this->oled.setTextColor(SSD1306_WHITE);
        this->oled.println(F("Game Over"));
        this->oled.setTextSize(1);
        this->oled.setCursor(10, this->screenHeight / 2 - 10);
        String score_string = "";
        score_string.concat(F("Score: "));
        score_string.concat(int(floor(score)));
        score_string.concat(F(" points"));
        this->oled.println(score_string);
        this->oled.setCursor(10, this->screenHeight / 2 + 10);
        this->oled.println(F("Press to restart..."));
        this->oled.display();
        waitForButtonPress(this->buttonPinA);
        waitForButtonRelease(this->buttonPinA);
        this->reset();
        return true;
    } else {
        score += 2 * deltaTime;
        this->oled.setTextSize(0);
        this->oled.setCursor(this->screenWidth / 2 - 10, 0);
        this->oled.setTextColor(SSD1306_WHITE);
        this->oled.println("Score: " + String(int(floor(score))));
        this->oled.display();
        return false;
    }
}