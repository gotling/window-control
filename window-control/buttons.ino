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
  lastActionTime = millis();
  Serial.print("buttonPress: ");
  Serial.println(pinNumber);

  switch (pinNumber) {
    case CLOSE_IN:
      windowClosePress(1);
      break;
    case CLOSE_IN_2:
      windowClosePress(2);
      break;
    case OPEN_IN:
      windowOpenPress(1);
      break;
    case OPEN_IN_2:
      windowOpenPress(2);
      break;
    case BTN_DOWN:
    case BTN_MIDDLE:
    case BTN_UP:
      switch(displayState) {
        case displayStats:
          switch (pinNumber) {
            case BTN_MIDDLE:
              preferencesDisplay();
              break;
          }
          break;
        case displayPreferences:
          preferencesHandleButtons(pinNumber);
          break;
      }

      break;
  }

  // Turn on display
  if (tftBrightness == 0) {
    tftBrightness = TFT_BRIGHTNESS;
    ledcWrite(ledChannel, tftBrightness);
    return; // THIS COULD BE THE PROBLEM WITH MISSING INPUT SIGNALS
  }
}

void windowClosePress(int hall) {
  Serial.print("Window close press, Hall ");
  Serial.println(hall);
  
  if (hall == 1) {
    stop = false;
    windowOpen = false;
    closeTime = millis();
    windowClosingDisplay(1);
    mqttSendEvent(RECEIVE_CLOSE);
  } else {
    stop2 = false;
    windowOpen2 = false;
    closeTime2 = millis();
    windowClosingDisplay(2);
    mqttSendEvent(RECEIVE_CLOSE_2);
  } 
}

void windowOpenPress(int hall) {
  Serial.print("Window open press, Hall ");
  Serial.println(hall);
  
  if (hall == 1) {
    stop = false;
    windowOpen = true;
    openTime = millis();
    windowOpeningDisplay(1);
    mqttSendEvent(RECEIVE_OPEN);
  } else {
    stop2 = false;
    windowOpen2 = true;
    openTime2 = millis();
    windowOpeningDisplay(2);
    mqttSendEvent(RECEIVE_OPEN_2);
  }
}

// Button released
void onPinDeactivated(int pinNumber) {
  Serial.print("buttonRelease: ");
  Serial.println(pinNumber);

  // If button released after both buttons pressed, treat windows as closed
  switch (pinNumber) {
    case CLOSE_IN:
    case OPEN_IN:
      if (stop) {
        windowOpen = false;
        closeTime = millis();
        refreshDisplay();
        mqttSendEvent(RECEIVE_STOP);
      }

      stop = true;
      break;
    case CLOSE_IN_2:
    case OPEN_IN_2:
      if (stop2) {
        windowOpen2 = false;
        closeTime2 = millis();
        refreshDisplay();
        mqttSendEvent(RECEIVE_STOP_2);
      }

      stop2 = true;
      break;
  }
}