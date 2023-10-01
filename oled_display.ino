#include <Wire.h>
#include <Adafruit_SSD1306.h>
#include <endless_runner.h>

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define BUTTON_PIN_A GPIO_NUM_19


// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup() {
  Serial.begin(9600);
  pinMode(BUTTON_PIN_A, INPUT);
  // put your setup code here, to run once:
  // initialize OLED display with address 0x3C for 128x64
  Serial.println(F("SSD1306 OLED test"));
  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  delay(2000); // wait for initializing
  oled.clearDisplay(); // clear display
  oled.display(); 
}

void loop() {
  static double deltaTime = 0;
  static endless_runner runner(BUTTON_PIN_A, SCREEN_WIDTH, SCREEN_HEIGHT, oled);
  int start = millis();
  bool gameOver = runner.play(deltaTime);
  deltaTime = (millis() - start) / 1000.0;
  if (gameOver) {
    deltaTime = 0;
  }
}