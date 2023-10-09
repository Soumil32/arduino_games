#include <game_base_class.h>
#include <Adafruit_SSD1306.h>

class ClickCounter : public Game {
    public:
        ClickCounter(Adafruit_SSD1306* oled, int screenWidth, int screenHeight, int clickButtonPin, int resetButtonPin, int quitButtonPin);
        PlayerState play(double deltaTime);
        void reset();

    private:
        int clickButtonPin;
        int resetButtonPin;
        int quitButtonPin;
        bool wasButtonPressedLastFrame; // so the player can't hold down the button
        bool isObjectSpawned;
        int clicks;
        int screenWidth;
        int screenHeight;
        Adafruit_SSD1306 oled;

        // constants
        const int ObjectAppearTime = 1000;

        // private methods
        PlayerState update(double deltaTime); // if the user pressed the button to reset or quit
        bool draw(PlayerState state); // if the user wants to go ahead with resetting or quitting
};