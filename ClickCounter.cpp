#include <click_counter.h>
#include <utils.h>

ClickCounter::ClickCounter(Adafruit_SSD1306* oled, int screenWidth, int screenHeight, int clickButtonPin, int resetButtonPin, int quitButtonPin) {
    this->oled = *oled;
    this->screenWidth = screenWidth;
    this->screenHeight = screenHeight;
    this->clickButtonPin = clickButtonPin;
    this->resetButtonPin = resetButtonPin;
    this->quitButtonPin = quitButtonPin;
    this->reset();
}

PlayerState ClickCounter::play(double deltaTime) {
    PlayerState result = this->update(deltaTime);
    bool procceed = this->draw(result);
    if (procceed) {
        return result;
    } else {
        return PlayerState::still_playing;
    }
}

PlayerState ClickCounter::update(double deltaTime) {
    int buttonPressed = getAnyButtonPress(new int[3] {this->clickButtonPin, this->resetButtonPin, this->quitButtonPin}, 3);
    if (buttonPressed == this->clickButtonPin && !this->wasButtonPressedLastFrame) {
        this->clicks++;
        this->wasButtonPressedLastFrame = true;
    } else if (buttonPressed == this->clickButtonPin && this->wasButtonPressedLastFrame) {
        // do nothing
    }else if (buttonPressed == this->resetButtonPin) {
        waitForButtonRelease(buttonPressed);
        return PlayerState::restart;
    } else if (buttonPressed == this->quitButtonPin) {
        waitForButtonRelease(buttonPressed);
        return PlayerState::quit;
    } else {
        this->wasButtonPressedLastFrame = false;
    }
    return PlayerState::still_playing;
}

bool ClickCounter::draw(PlayerState state) {
    if (state == PlayerState::quit || state == PlayerState::restart) {
        this->oled.clearDisplay();
        this->oled.setCursor(this->screenWidth / 2 - 20, this->screenHeight / 2 - 10);
        this->oled.setTextSize(2);
        this->oled.setTextColor(SSD1306_WHITE);
        this->oled.println(F("Sure?"));
        this->oled.setCursor(this->screenWidth / 2 - 20, this->screenHeight / 2 + 10);
        this->oled.setTextSize(1);
        this->oled.println(F("A: Yes"));
        this->oled.setCursor(this->screenWidth / 2 - 20, this->screenHeight / 2 + 20);
        this->oled.println(F("B: No"));
        this->oled.display();
        int buttonPressed = waitForAnyButtonPress(new int[2] {this->clickButtonPin, this->resetButtonPin}, 2);
        waitForButtonRelease(buttonPressed);
        if (buttonPressed == this->clickButtonPin) {
            return true;
        }
    }
        

    this->oled.clearDisplay();
    this->oled.setCursor(this->screenWidth / 2 - 5, this->screenHeight / 2 - 10);
    this->oled.setTextSize(3);
    this->oled.setTextColor(SSD1306_WHITE);
    this->oled.println(this->clicks);
    this->oled.display();
    return false;
}

void ClickCounter::reset() {
    this->clicks = 0;
    this->wasButtonPressedLastFrame = false;
    this->isObjectSpawned = false;
}