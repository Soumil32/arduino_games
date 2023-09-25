#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUTTON_PIN 6
#define PLAYER_JUMP_POWER 35
#define FRAMES_TO_JUMP 15
#define OBSTACLE_SPEED 10 // this is in pixels per second
#define OBSTACLE_SPAWN_RATE_MAX 2000 // this is the max amount of time to wait in milliseconds
#define OBSTACLE_SPAWN_RATE_MIN 1200 // this is the min amount of time to wait in milliseconds
#define PLAYER_X_OFFSET 15

typedef void (*gameLoopFunc)(float);

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
  static float deltaTime = 0;
  static gameLoopFunc gameLoop = makeGameLoopFunc();
  long currentTime = millis();
  gameLoop(deltaTime);
  deltaTime = (millis() - currentTime) / 1000.0;
}

gameLoopFunc makeGameLoopFunc() {
  return gameLoop;
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

// get acceleration for each frame when there are 5 frames to jump
double get_acceleration_5(int frame, int jump_power) {
  double frame1 = jump_power / 3;
  if (frame == 1) {
    return frame1;
  }
  double frame3 = jump_power / 5;
  if (frame == 3) {
    return frame3;
  }
  if (frame == 2) {
    double frame2 = (frame1 + frame3) / 2;
    // if frame 2 is not an integer, then it will be rounded up
    // if frame 2 is a integer, then add 1 to it
    if (frame2 - (int)frame2 != 0) {
      return ceil(frame2);
    } else {
      return frame2 + 1;
    }
  }
  double frame5 = 1;
  if (frame == 5) {
    return frame5;
  }
  if (frame == 4) {
    double frame4 = (frame3 + frame5) / 2;
    if (frame4 - (int)frame4 != 0) {
      return ceil(frame4);
    } else {
      return frame4 + 1;
    }
  }
}

// get acceleration for each frame when there are 10 frames to jump
double get_acceleration_10(int frame, int jump_power) {
  const double divide_by_double = 4;
  const int divide_by_int = 4;
  if (frame / divide_by_int == 0) {
    return get_acceleration_5(1, jump_power) / divide_by_double;
  } else if (frame / divide_by_int == 1) {
    return get_acceleration_5(2, jump_power) / divide_by_double;
  } else if (frame / divide_by_int == 2) {
    return get_acceleration_5(3, jump_power) / divide_by_double;
  } else if (frame / divide_by_int == 3) {
    return get_acceleration_5(4, jump_power) / divide_by_double;
  } else if (frame / divide_by_int == 4) {
    return get_acceleration_5(5, jump_power) / divide_by_double;
  } else if (frame / divide_by_int == 5) { 
    return 0;
  } else {
    Serial.print("Error: frame is out of bounds: ");
    Serial.println(frame);
    return 0;
  }
}

/// @brief plays the game 
/// @param deltaTime delta time in seconds
void gameLoop(float deltaTime) {
  // put your main code here, to run repeatedly:
  // make a dynamically sized array of obstacles
  static Obstacle obstacles[10];
  static int width = 10;
  static int height = 10;
  static double player_acceleration_added = 0;
  static double player_y = SCREEN_HEIGHT - height - 5;
  static int player_state = 0; // 0 = idle, 1 = jumping, 2 = falling
  static int jump_frame = 1;
  static int time_since_last_spawn = 0; // this is in milliseconds
  static bool game_over = false;
  static float score = 0;
  int pressed = digitalRead(BUTTON_PIN);
  oled.clearDisplay();
  if (pressed == 1 && player_state != 2) { // if the button is pressed and the player is not falling
    player_state = 1;
  }
  if (player_state == 1) {
    double player_acceleration_this_frame = get_acceleration_10(jump_frame, PLAYER_JUMP_POWER);
    player_y -= player_acceleration_this_frame;
    player_acceleration_added += player_acceleration_this_frame;
    jump_frame++;
    if (player_acceleration_added >= PLAYER_JUMP_POWER) {
      player_state = 2;
    } else if (jump_frame > FRAMES_TO_JUMP) {
      player_state = 2;
    }
  } else if (player_state == 2) {
    double player_acceleration_this_frame = get_acceleration_10(jump_frame, PLAYER_JUMP_POWER);
    player_y += player_acceleration_this_frame;
    player_acceleration_added -= player_acceleration_this_frame;
    jump_frame--;
    if (player_acceleration_added <= 0) {
      player_state = 0;
      jump_frame = 1;
    }
  }
  
  int timeToWait = random(OBSTACLE_SPAWN_RATE_MIN, OBSTACLE_SPAWN_RATE_MAX);
  if (time_since_last_spawn >= timeToWait) {
    Serial.println(timeToWait);
    // generate a random number which will decide if an obstacle will be spawned
    int random_number = random(0, 3);
    if (random_number == 1) {
      int obstacle_width = random(10, 15);
      int obstacle_height = random(10, 18);
      int obstacle_x = SCREEN_WIDTH - obstacle_width;
      int obstacle_y = SCREEN_HEIGHT - obstacle_height - 5;
      Obstacle obstacle = {obstacle_x, obstacle_y, obstacle_width, obstacle_height, true};
      for (int i = 0; i < 5; i++) {
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
      bool is_player_above_obstacle = player_y + height < obstacle.y;
      bool is_right_wall_colliding = !is_player_above_obstacle && PLAYER_X_OFFSET + width > obstacle.x;
      bool is_player_bottom_colliding = player_y + height >= obstacle.y && player_y != SCREEN_HEIGHT - height - 5 && PLAYER_X_OFFSET + width > obstacle.x;
      bool is_left_wall_colliding = !is_player_above_obstacle && PLAYER_X_OFFSET > obstacle.x;
      if (check_if_out_of_bounds(obstacle.x, obstacle.y, obstacle.width, obstacle.height)) {
        obstacle.is_active = false;
        obstacles[i] = obstacle;
        continue;
      }
      if (is_right_wall_colliding || is_player_bottom_colliding || is_left_wall_colliding) {
        game_over = true;
      }
      obstacles[i] = obstacle;
      oled.drawRect(obstacle.x, obstacle.y, obstacle.width, obstacle.height, SSD1306_WHITE);
    }
  }

  oled.setTextSize(0.8);
  oled.setCursor(SCREEN_WIDTH / 2 - 10, 0);
  oled.setTextColor(SSD1306_WHITE);
  oled.println("Score: " + String(int(ceil(score))));

  oled.drawRect(PLAYER_X_OFFSET, player_y, width, height, SSD1306_WHITE);
  oled.drawFastHLine(0, SCREEN_HEIGHT - 5, SCREEN_WIDTH, SSD1306_WHITE);
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
    player_y = SCREEN_HEIGHT - height - 5;
    player_state = 0;
    jump_frame = 1;
    time_since_last_spawn = 0;
    game_over = false;
    score = 0;
    return;
  } else {
    score += 2 * deltaTime;
    oled.display();
  }
  delay(5);
} 