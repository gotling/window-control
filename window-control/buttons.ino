#define BTN_OPEN 35
#define BTN_CLOSE 34
#define BTN_DOWN 18
#define BTN_MIDDLE 5
#define BTN_UP 19

void setupButtons() {
  pinDebouncer.addPin(BTN_OPEN, LOW);
  pinDebouncer.addPin(BTN_CLOSE, LOW);
  
  pinDebouncer.addPin(BTN_DOWN, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_MIDDLE, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_UP, HIGH, INPUT_PULLUP);
  pinDebouncer.begin();
}

// Button pressed
void onPinActivated(int pinNumber){
  unsigned int x = 200;
  unsigned int y = 220;

  Serial.print("buttonPress: ");
  Serial.println(pinNumber);

  switch (pinNumber) {
    case BTN_OPEN:
      openTime = millis();
      windowOpen = true;
      gfx->fillCircle(x, y, 16, GREEN);
      break;
    case BTN_CLOSE:
      closeTime = millis();
      windowOpen = false;
      gfx->fillCircle(x + 20, y, 16, RED);
      break;
    case BTN_UP:
      Serial.println("OPEN OUT HIGH");
      windowOpen = true;
      openTime = millis();
      windowOpenDisplay();
      
      digitalWrite(OPEN_OUT, HIGH);
      delay(100);
      digitalWrite(OPEN_OUT, LOW);
      break;
    case BTN_DOWN:
      Serial.println("CLOSE OUT HIGH");
      windowOpen = false;
      closeTime = millis();
      windowClosingDisplay();
      
      digitalWrite(CLOSE_OUT, HIGH);
      delay(100);
      digitalWrite(CLOSE_OUT, LOW);
      break;
    case BTN_MIDDLE:
      Serial.println("CLOSE AND OPEN LOW");
      digitalWrite(OPEN_OUT, LOW);
      digitalWrite(CLOSE_OUT, LOW);
      break;
  }
}

// Button released
void onPinDeactivated(int pinNumber){
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