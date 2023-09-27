#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUTTON_PIN 6
#define PLAYER_JUMP_POWER 35.0
#define MS_TO_JUMP 1650
#define OBSTACLE_SPEED 10 // this is in pixels per second
#define OBSTACLE_SPAWN_RATE_MAX 2500 // this is the max amount of time to wait in milliseconds
#define OBSTACLE_SPAWN_RATE_MIN 2000 // this is the min amount of time to wait in milliseconds
#define PLAYER_X_OFFSET 15
#define PLAYER_Y_OFFSET 5

typedef void (*gameLoopFunc)(double);

struct Obstacle {
  int x;
  int y;
  int width;
  int height;
  bool is_active;
};


// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN, INPUT);
  // put your setup code here, to run once:
  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(200); // wait for initializing
  oled.clearDisplay(); // clear display
  oled.display(); 
}

void loop() {
  static double deltaTime = 0;
  //static gameLoopFunc gameLoop = makeGameLoopFunc();
  long currentTime = millis();
  gameLoop(deltaTime);
  deltaTime = (millis() - currentTime) / 1000.0;
}

bool check_if_out_of_bounds(int x, int y, int width, int height) {
  if ((x) >= SCREEN_WIDTH) {
      return true;
  } else if ((y + height) >= SCREEN_HEIGHT) {
      return true;
  } else if (x <= 0) {
      return true;
  } else if (y <= 0) {
      return true;
  } else {
      return false;
  }
}

/// @brief get get the acceleration of the player each frame
/// @param total_time The total time the player will be jumping for in milliseconds
/// @param delta_time the time since the last frame
/// @return returns the acceleration of the player this frame in pixels per millisecond
double get_acceleration(int total_time, float delta_time) {
  // jump at a constant rate for the entire duration
  // the amount will be evened out by the amount of time passed since the last frame
  double accelerationPerMillisecond = PLAYER_JUMP_POWER / total_time;
  return accelerationPerMillisecond * (delta_time * 1000);
}

/// @brief plays the game 
/// @param deltaTime delta time in seconds
void gameLoop(double deltaTime) {
  // put your main code here, to run repeatedly:
  // make a dynamically sized array of obstacles
  static Obstacle obstacles[10];
  static int width = 10;
  static int height = 10;
  static double player_acceleration_added = 0;
  static double player_y = SCREEN_HEIGHT - height - PLAYER_Y_OFFSET;
  static short int player_state = 0; // 0 = idle, 1 = jumping, 2 = falling
  static int time_since_last_spawn = 0; // this is in milliseconds
  static bool game_over = false;
  static float score = 0;
  int pressed = digitalRead(BUTTON_PIN);
  oled.clearDisplay();
  if (pressed == HIGH && player_state != 2) { // if the button is pressed and the player is not falling
    player_state = 1;
  }
  if (player_state == 1) {
    double player_acceleration_this_frame = get_acceleration(MS_TO_JUMP / 2 , deltaTime);
    player_y -= player_acceleration_this_frame;
    player_acceleration_added += player_acceleration_this_frame;
    if (player_acceleration_added >= PLAYER_JUMP_POWER) {
      player_state = 2;
    }
  } else if (player_state == 2) {
    double player_acceleration_this_frame = get_acceleration(MS_TO_JUMP / 2, deltaTime);
    player_y += player_acceleration_this_frame;
    player_acceleration_added -= player_acceleration_this_frame;
    if (player_acceleration_added <= 0) {
      player_state = 0;
      player_acceleration_added = 0;
      player_y = SCREEN_HEIGHT - height - PLAYER_Y_OFFSET;
    }
  }

  int timeToWait = random(OBSTACLE_SPAWN_RATE_MIN, OBSTACLE_SPAWN_RATE_MAX);
  if (time_since_last_spawn >= timeToWait) {
    // generate a random number which will decide if an obstacle will be spawned
    int random_number = random(0, 3);
    if (random_number == 1) {
      int obstacle_width = random(8, 12);
      int obstacle_height = random(8, 19);
      int obstacle_x = SCREEN_WIDTH - obstacle_width;
      int obstacle_y = SCREEN_HEIGHT - obstacle_height - PLAYER_Y_OFFSET;
      Obstacle obstacle = {obstacle_x, obstacle_y, obstacle_width, obstacle_height, true};
      for (int i = 0; i < 10; i++) {
        if (!obstacles[i].is_active) {
          obstacles[i] = obstacle;
          break;
        }
      }
      time_since_last_spawn = 0;
    }
  } else {
    time_since_last_spawn += deltaTime * 1000;
  }

  for (int i = 0; i < 10; i++) {
    Obstacle obstacle = obstacles[i];
    if (obstacle.is_active) {
      obstacle.x -= OBSTACLE_SPEED * deltaTime;
      // do some AABB collision detection
      if (player_y <= obstacle.y + obstacle.height && player_y + height >= obstacle.y) {
        if (PLAYER_X_OFFSET + width >= obstacle.x && PLAYER_X_OFFSET <= obstacle.x + obstacle.width) {
          game_over = true;
        }
      }
      if (check_if_out_of_bounds(obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
        obstacle.is_active = false;
        obstacles[i] = obstacle;
        continue;
      }
      obstacles[i] = obstacle;
      oled.drawRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height, SSD1306_WHITE);
    }
  }

  oled.setTextSize(0);
  oled.setCursor(SCREEN_WIDTH / 2 - 10, 0);
  oled.setTextColor(SSD1306_WHITE);
  oled.println("Score: " + String(int(floor(score))));

  oled.drawRect(PLAYER_X_OFFSET, player_y, width, height, SSD1306_WHITE);
  oled.drawFastHLine(0, SCREEN_HEIGHT - PLAYER_Y_OFFSET, SCREEN_WIDTH, SSD1306_WHITE);
  if (game_over) {
    oled.setTextSize(2);
    oled.setCursor(0, 0);
    oled.setTextColor(SSD1306_WHITE);
    oled.println("Game Over");
    oled.display();
    Serial.println("Game Over");
    delay(2000);
    for (int i = 0; i < 10; i++) {
      obstacles[i].is_active = false;
    }
    width = 10;
    height = 10;
    player_acceleration_added = 0;
    player_y = SCREEN_HEIGHT - height - PLAYER_Y_OFFSET;
    player_state = 0;
    time_since_last_spawn = 0;
    game_over = false;
    score = 0;
    return;
  } else {
    score += 2 * deltaTime;
    oled.display();
  }
  //delay(5);
} 