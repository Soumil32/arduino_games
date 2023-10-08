#include <game_base_class.h>
#include <Adafruit_SSD1306.h>

enum Winner {
    player1,
    player2,
    none
};

class Pong : public Game {
    public:
        Pong(Adafruit_SSD1306* oled, int screenWidth, int screenHeight, int upButtonPin, int downButtonPin, int acceptButtonPin);
        PlayerState play(double deltaTime);
        void reset();
    private:
        /* game data */
        float player1Y = 0;
        float player2Y = 0;
        float ballX = 0;
        float ballY = 0;
        float ballVelocityX = -1;
        float ballVelocityY = 0;
        int player1Score = 0;
        int player2Score = 0;

        /* game constants */
        const int playerWidth = 5;
        const int playerHeight = 15;
        const int player1X = 5;
        const int player2X = 118;
        const int playerStartY = 25;
        const int ballWidth = 5;
        const int ballHeight = 5;
        const int ballSpeed = 50; // pixels per second
        const int playerSpeed = 50; // pixels per second

        /* additional data provided at instantiation */
        int screenWidth;
        int screenHeight;
        Adafruit_SSD1306 oled;
        int upButtonPin;
        int downButtonPin; 
        int acceptButtonPin;
        int playUntilScore = 1; // play until a player reaches this score

        /* proccess functions */
        Winner update(double deltaTime);
        PlayerState draw(Winner winner);
};