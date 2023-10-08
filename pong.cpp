#include <pong.h>
#include <utils.h>
#include <Arduino.h>
#include <collision.h>

Pong::Pong(Adafruit_SSD1306* oled, int screenWidth, int screenHeight, int upButtonPin, int downButtonPin, int acceptButtonPin) {
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    this->oled = *oled;
    this->upButtonPin = upButtonPin;
    this->downButtonPin = downButtonPin;
    this->acceptButtonPin = acceptButtonPin;
    this-> ballX = this->screenWidth / 2 - this->ballWidth / 2;
    this->ballY = this->screenHeight / 2 - this->ballHeight / 2;

    this->oled.setTextSize(1);
    this->oled.setTextColor(SSD1306_WHITE);

    while (true) {
        this->oled.clearDisplay();
        this->oled.setCursor(0, screenHeight / 2 - 10);
        String message = "";
        message.concat(F("Play until score: "));
        message.concat(this->playUntilScore);
        this->oled.println(message);
        this->oled.display();
        int buttons[] = {this->upButtonPin, this->downButtonPin, this->acceptButtonPin};
        int buttonPressed = waitForAnyButtonPress(buttons, 3);
        waitForButtonRelease(buttonPressed);
        if (buttonPressed == this->upButtonPin) {
            this->playUntilScore++;
        } else if (buttonPressed == this->downButtonPin) {
            this->playUntilScore--;
        } else if (buttonPressed == this->acceptButtonPin) {
            break;
        }
    }
    Serial.println(this->playUntilScore);
}

/// @brief does calculations for the current frame
/// @param deltaTime time since last frame
/// @return if the game is over
Winner Pong::update(double deltaTime) {
    // control player 1 (human) movement
    
    int upButtonPressed = digitalRead(this->upButtonPin);
    int downButtonPressed = digitalRead(this->downButtonPin);
    if (upButtonPressed == HIGH) {
        this->player1Y -= this->playerSpeed * deltaTime;
    } else if (downButtonPressed == HIGH) {
        this->player1Y += this->playerSpeed * deltaTime;
    }
    if (this->player1Y < 0) {
        this->player1Y = 0;
    } else if (this->player1Y + this->playerHeight > this->screenHeight) {
        this->player1Y = this->screenHeight - this->playerHeight;
    }

    //update ball position

    this->ballX += this->ballVelocityX * this->ballSpeed * deltaTime;
    this->ballY += this->ballVelocityY * this->ballSpeed * deltaTime;
    bool isCollingWithPlayer1 = checkAABB(this->ballX, this->ballY, this->ballWidth, this->ballHeight, this->player1X, this->player1Y, this->playerWidth, this->playerHeight);
    bool isCollingWithPlayer2 = checkAABB(this->ballX, this->ballY, this->ballWidth, this->ballHeight, this->player2X, this->player2Y, this->playerWidth, this->playerHeight);
    bool isCollingWithTop = this->ballY < 0;
    bool isCollingWithBottom = this->ballY + this->ballHeight > this->screenHeight;
    bool passedPlayer1 = this->ballX < this->player1X + this->playerWidth;
    bool passedPlayer2 = this->ballX + this->ballWidth > this->player2X;
    if (isCollingWithTop || isCollingWithBottom) {
        this->ballVelocityY *= -1;
        if (isCollingWithTop) {
            this->ballY = 0;
        } else if (isCollingWithBottom) {
            this->ballY = this->screenHeight - this->ballHeight;
        }
    }
    if (isCollingWithPlayer1 || isCollingWithPlayer2) {
        // calculate how from the center of the paddle the ball hit
        float paddleCenter = 0;
        float ballCenter = 0;
        if (isCollingWithPlayer1) {
            paddleCenter = this->player1Y + this->playerHeight / 2;
            ballCenter = this->ballY + this->ballHeight / 2;
            this->ballX = this->player1X + this->playerWidth + 1; // move the ball out of the paddle to avoid glitching
        } else {
            paddleCenter = this->player2Y + this->playerHeight / 2;
            ballCenter = this->ballY + this->ballHeight / 2;
            this->ballX = this->player2X - this->ballWidth - 1; // move the ball out of the paddle to avoid glitching
        }
        float distanceFromCenter = paddleCenter - ballCenter;
        float normalizedDistance = distanceFromCenter / (this->playerHeight / 2); // normalize the distance to be between -1 and 1
        this->ballVelocityY = normalizedDistance;
        this->ballVelocityX *= -1;
    } else if (passedPlayer1) {
        this->player2Score++;
        this->ballX = this->screenWidth / 2 - this->ballWidth / 2;
        this->ballY = this->screenHeight / 2 - this->ballHeight / 2;
        this->ballVelocityX *= -1;
        this->ballVelocityY = 0;
    } else if (passedPlayer2) {
        this->player1Score++;
        this->ballX = this->screenWidth / 2 - this->ballWidth / 2;
        this->ballY = this->screenHeight / 2 - this->ballHeight / 2;
        this->ballVelocityX *= -1;
        this->ballVelocityY = 0;
    }

    // control player 2 (AI) movement. The AI will try to move to the center of the ball. 
    // The AI will only move if the ball is moving towards it
    // To make this fair, There is a 20% chance that the AI will not move

    int random_number = random(0, 100);
    if (this->ballVelocityX > 0 && random_number > 40 && this->ballX > this->screenWidth / 2) {
        float ballCenter = this->ballY + this->ballHeight / 2;
        float paddleCenter = this->player2Y + this->playerHeight / 2;
        float distanceFromCenter = paddleCenter - ballCenter;
        float normalizedDistance = distanceFromCenter / (this->playerHeight / 2); // normalize the distance to be between -1 and 1
        this->player2Y -= normalizedDistance * this->playerSpeed * deltaTime;
    } else if (random_number >= 20 && random_number <= 40) {
        // go in the opposite direction of the ball
        float ballCenter = this->ballY + this->ballHeight / 2;
        float paddleCenter = this->player2Y + this->playerHeight / 2;
        float distanceFromCenter = paddleCenter - ballCenter;
        float normalizedDistance = distanceFromCenter / (this->playerHeight / 2); // normalize the distance to be between -1 and 1
        this->player2Y += normalizedDistance * this->playerSpeed * deltaTime;
    }
    if (this->player2Y < 0) {
        this->player2Y = 0;
    } else if (this->player2Y + this->playerHeight > this->screenHeight) {
        this->player2Y = this->screenHeight - this->playerHeight;
    }
    if (this->player1Score >= this->playUntilScore) {
        return Winner::player1;
    } else if (this->player2Score >= this->playUntilScore) {
        return Winner::player2;
    } else {
        return Winner::none;
    }
}

/// @brief draws the current frame
/// @param gameOver to know if the game is over so it can draw the game over screen
PlayerState Pong::draw(Winner winner) {
    this->oled.clearDisplay();

    if (winner != Winner::none) {
        this->oled.setCursor(0, 0);
        this->oled.println(F("Game Over"));
        this->oled.setCursor(0, 10);
        this->oled.println(F("Press A to restart"));
        this->oled.setCursor(0, 20);
        this->oled.println(F("Press B to quit"));
        this->oled.setCursor(0, 30);
        if (winner == Winner::player1) {
            this->oled.println(F("Player 1 wins!"));
        } else {
            this->oled.println(F("Player 2 wins!"));
        }
        this->oled.display();
        int buttons[] = {this->upButtonPin, this->downButtonPin, this->acceptButtonPin};
        int buttonPressed = waitForAnyButtonPress(buttons, 3);
        waitForButtonRelease(buttonPressed);
        if (buttonPressed == this->acceptButtonPin) {
            return PlayerState::quit;
        } else if (buttonPressed == this->upButtonPin) {
            return PlayerState::restart;
        }
    }

    // draw player 1
    this->oled.fillRect(this->player1X, this->player1Y, this->playerWidth, this->playerHeight, SSD1306_WHITE);

    // draw player 2
    this->oled.fillRect(this->player2X, this->player2Y, this->playerWidth, this->playerHeight, SSD1306_WHITE);

    // draw ball
    this->oled.fillRect(this->ballX, this->ballY, this->ballWidth, this->ballHeight, SSD1306_WHITE);

    // draw a dashed line in the middle of the screen
    for (int i = 0; i < this->screenHeight; i += 2) {
        this->oled.drawPixel(this->screenWidth / 2, i, SSD1306_WHITE);
    }

    //draw a score counter on each side of the line 
    this->oled.setCursor(this->screenWidth / 2 - 10, 0);
    this->oled.println(this->player1Score);
    this->oled.setCursor(this->screenWidth / 2 + 10, 0);
    this->oled.println(this->player2Score);

    this->oled.display();
    return PlayerState::still_playing;
}

PlayerState Pong::play(double deltaTime) {
    Winner winner = Pong::update(deltaTime);
    return Pong::draw(winner);
}

void Pong::reset() {
    this->player1Y = this->screenHeight / 2 - this->playerHeight / 2;
    this->player2Y = this->screenHeight / 2 - this->playerHeight / 2;
    this->ballX = this->screenWidth / 2 - this->ballWidth / 2;
    this->ballY = this->screenHeight / 2 - this->ballHeight / 2;
    this->ballVelocityX = -1;
    this->ballVelocityY = 0;
    this->player1Score = 0;
    this->player2Score = 0;
}