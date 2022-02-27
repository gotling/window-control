#define CLOSE_IN 35
#define CLOSE_IN_2 34
#define OPEN_IN 32
#define OPEN_IN_2 33

#define BTN_DOWN 18
#define BTN_MIDDLE 5
#define BTN_UP 19

// Figure out if both open and close are pressed at the same time to stop windows in current position
bool active = false;
bool active2 = false;
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
      if (active)
        stop = true;
      else
        active = true;

      closeTime = millis();
      windowOpen = false;
      windowClosingDisplay();
      
      mqttSendEvent(RECEIVE_CLOSE);
      break;
    case CLOSE_IN_2:
      if (active2)
        stop2 = true;
      else
        active2 = true;
      
      closeTime2 = millis();
      windowOpen2 = false;
      windowClosingDisplay();
      
      mqttSendEvent(RECEIVE_CLOSE_2);
      break;
    case OPEN_IN:
      if (active)
        stop = true;
      else
        active = true;

      openTime = millis();
      windowOpen = true;
      windowOpenDisplay();

      mqttSendEvent(RECEIVE_OPEN);
      break;
    case OPEN_IN_2:
      if (active2)
        stop2 = true;
      else
        active2 = true;
      
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

  // If button released after both buttons pressed, treat windows as closed
  switch (pinNumber) {
    case CLOSE_IN:
      active = false;
      break;
    case CLOSE_IN_2:
      active2 = false;
      break;
    case OPEN_IN:
      active = false;
      if (stop)
        windowOpen = false;
      stop = false;
      break;
    case OPEN_IN_2:
      if (stop2)
        windowOpen2 = false;
      stop2 = false;
      break;
  }
}