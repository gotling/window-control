#define BTN_OPEN 35
#define BTN_CLOSE 34
#define BTN_DOWN 18
#define BTN_MIDDLE 5
#define BTN_UP 19

void setupButtons() {
  pinDebouncer.addPin(BTN_OPEN, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_CLOSE, HIGH, INPUT_PULLUP);
  
  pinDebouncer.addPin(BTN_DOWN, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_MIDDLE, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_UP, HIGH, INPUT_PULLUP);
  pinDebouncer.begin();
}

// Button pressed
void onPinActivated(int pinNumber) {
  unsigned int x = 200;
  unsigned int y = 220;

  lastActionTime = millis();
  if (tftBrightness == 0) {
    tftBrightness = TFT_BRIGHTNESS;
    ledcWrite(ledChannel, tftBrightness);
    return;
  }

  Serial.print("buttonPress: ");
  Serial.println(pinNumber);

  switch(displayState) {
    case displayStats:
      switch (pinNumber) {
        case BTN_UP:
          openWindow();
          break;
        case BTN_DOWN:
          closeWindow();
          break;
        case BTN_MIDDLE:
          preferencesDisplay();
          break;
      }
      break;
    case displayWindowOpen:
      switch (pinNumber) {
        case BTN_DOWN:
          closeWindow();
          break;
      }
      break;
    case displayPreferences:
      preferencesHandleButtons(pinNumber);
      break;
  }

  switch (pinNumber) {
    case BTN_OPEN:
      openTime = millis();
      windowOpen = true;
      windowOpenDisplay();
      gfx->fillCircle(x, y, 16, GREEN);
      break;
    case BTN_CLOSE:
      closeTime = millis();
      windowOpen = false;
      windowClosingDisplay();
      gfx->fillCircle(x + 20, y, 16, RED);
      break;
  }
}

// Button released
void onPinDeactivated(int pinNumber) {
  unsigned int x = 200;
  unsigned int y = 220;

  Serial.print("buttonRelease: ");
  Serial.println(pinNumber);

  switch (pinNumber) {
    case BTN_OPEN:
      gfx->fillCircle(x, y, 16, BLACK);
      gfx->drawCircle(x, y, 16, GREEN);
      break;
    case BTN_CLOSE:
      gfx->fillCircle(x + 20, y, 16, BLACK);
      gfx->drawCircle(x + 20, y, 16, RED);
      break;
  }
}