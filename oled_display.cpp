#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <endless_runner.h>
#include <pong.h>
#include <click_counter.h>
#include <utils.h>
#include <game_base_class.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUTTON_PIN_A GPIO_NUM_19
#define BUTTON_PIN_B GPIO_NUM_34
#define BUTTON_PIN_C GPIO_NUM_23

enum game_option {
  endless_runner,
  pong,
  click_counter
};

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

Game* displayMenu(Adafruit_SSD1306 oled);
void setup();
void loop();

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN_A, INPUT);
  pinMode(BUTTON_PIN_B, INPUT);
  // put your setup code here, to run once:
  // initialize OLED display with address 0x3C for 128x64
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000); // wait for initializing
  oled.clearDisplay(); // clear display
  oled.display(); 
}

Game* displayMenu(Adafruit_SSD1306* oled) {
  (*oled).clearDisplay();
  (*oled).setTextSize(1);
  (*oled).setTextColor(SSD1306_WHITE);
  const game_option games[] = {endless_runner, pong, click_counter};
  const __FlashStringHelper *game_names[] = {F("Endless Runner"), F("Pong"), F("Click Counter")};
  game_option selected_game = endless_runner;
  while (true) {
    (*oled).clearDisplay();
    for (int i = 0; i < sizeof(games) / sizeof(game_option); i++) {
      (*oled).setCursor(SCREEN_WIDTH / 2 - 40, i * 10);
      if (selected_game == games[i]) {
        (*oled).setTextColor(SSD1306_BLACK, SSD1306_WHITE);
      } else {
        (*oled).setTextColor(SSD1306_WHITE);
      }
      (*oled).println(game_names[i]); 
    }
    (*oled).display();
    int buttonPressed = waitForEitherButtonPress(BUTTON_PIN_A, BUTTON_PIN_B);
    waitForButtonRelease(buttonPressed);
    if (buttonPressed == BUTTON_PIN_B) {
      // go to next game in the list
      selected_game = games[(selected_game + 1) % (sizeof(games) / sizeof(game_option))];
      continue;
    } else if (buttonPressed == BUTTON_PIN_A) {
      // start the selected game
      switch (selected_game) {
        case endless_runner: {
          Game* game = new EndlessRunner(BUTTON_PIN_A, BUTTON_PIN_B, SCREEN_WIDTH, SCREEN_HEIGHT, oled);
          return game;
        }
        case pong: {
          return new Pong(oled, SCREEN_WIDTH, SCREEN_HEIGHT, BUTTON_PIN_A, BUTTON_PIN_B, BUTTON_PIN_C);
        }
        case click_counter: {
          return new ClickCounter(oled, SCREEN_WIDTH, SCREEN_HEIGHT, BUTTON_PIN_A, BUTTON_PIN_B, BUTTON_PIN_C);
        }
      }
    }
  }
}

void loop() {
  while (true)
  {
    Game* game = displayMenu(&oled);
    double deltaTime = 0;
    PlayerState result = PlayerState::still_playing;
    while (result == PlayerState::still_playing || result == PlayerState::restart) {
      int start = millis();
      result = game->play(deltaTime);
      deltaTime = (millis() - start) / 1000.0;
      if (result == PlayerState::restart) {
        deltaTime = 0;
        game->reset();
        continue;
      }
    }
    if (result == PlayerState::quit) {
      delete game;
      continue;
    }
  }
  
}