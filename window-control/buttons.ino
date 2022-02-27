#define CLOSE_IN 35
#define CLOSE_IN_2 34
#define OPEN_IN 32
#define OPEN_IN_2 33

#define BTN_DOWN 18
#define BTN_MIDDLE 5
#define BTN_UP 19

void setupButtons() {
  pinDebouncer.addPin(CLOSE_IN, HIGH);
  pinDebouncer.addPin(CLOSE_IN_2, HIGH);
  pinDebouncer.addPin(OPEN_IN, HIGH);
  pinDebouncer.addPin(OPEN_IN_2, HIGH);
  
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
        // case BTN_UP:
        //   openWindow();
        //   break;
        // case BTN_DOWN:
        //   closeWindow();
        //   break;
        case BTN_MIDDLE:
          preferencesDisplay();
          break;
      }
      break;
    // case displayWindowOpen:
    //   switch (pinNumber) {
    //     case BTN_DOWN:
    //       closeWindow();
    //       break;
    //   }
    //   break;
    case displayPreferences:
      preferencesHandleButtons(pinNumber);
      break;
  }

  switch (pinNumber) {
    case CLOSE_IN:
      closeTime = millis();
      windowOpen = false;
      windowClosingDisplay();
      
      mqttSendEvent(RECEIVE_CLOSE);
      break;
    case CLOSE_IN_2:
      closeTime2 = millis();
      windowOpen2 = false;
      windowClosingDisplay();
      
      mqttSendEvent(RECEIVE_CLOSE_2);
      break;
    case OPEN_IN:
      openTime = millis();
      windowOpen = true;
      windowOpenDisplay();

      mqttSendEvent(RECEIVE_OPEN);
      break;
    case OPEN_IN_2:
      openTime2 = millis();
      windowOpen2 = true;
      windowOpenDisplay();
      
      mqttSendEvent(RECEIVE_OPEN_2);
      break;
  }
}

// Button released
void onPinDeactivated(int pinNumber) {
  Serial.print("buttonRelease: ");
  Serial.println(pinNumber);
}