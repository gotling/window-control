#define CLOSE_IN 35
#define CLOSE_IN_2 34
#define OPEN_IN 32
#define OPEN_IN_2 33

#define BTN_DOWN 18
#define BTN_MIDDLE 5
#define BTN_UP 19

bool stop = false;
bool stop2 = false;

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
  }
}

void windowClosePress(int hall) {
  Serial.print("Window close press, Hall ");
  Serial.println(hall);
  stop = false;
  stop2 = false;
  
  if (hall == 1) {
    windowOpen = false;
    closeTime = millis();
    windowClosingDisplay(1);
    mqttSendEvent(RECEIVE_CLOSE);
  } else {
    windowOpen2 = false;
    closeTime2 = millis();
    windowClosingDisplay(2);
    mqttSendEvent(RECEIVE_CLOSE_2);
  } 
}

void windowOpenPress(int hall) {
  Serial.print("Window open press, Hall ");
  Serial.println(hall);
  stop = false;
  stop2 = false;
  
  if (hall == 1) {
    windowOpen = true;
    openTime = millis();
    windowOpeningDisplay(1);
    mqttSendEvent(RECEIVE_OPEN);
  } else {
    openTime2 = millis();
    windowOpen2 = true;
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
      if (stop) {
        windowOpen = false;
        closeTime = millis();
        refreshDisplay();
        mqttSendEvent(RECEIVE_STOP);
      }

      stop = true;
      break;
    case CLOSE_IN_2:
      if (stop2) {
        windowOpen2 = false;
        closeTime2 = millis();
        refreshDisplay();
        mqttSendEvent(RECEIVE_STOP_2);
      }

      stop2 = true;
      break;
    case OPEN_IN:
      if (stop) {
        windowOpen = false;
        closeTime = millis();
        refreshDisplay();
        mqttSendEvent(RECEIVE_STOP);
      }

      stop = true;
      break;
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