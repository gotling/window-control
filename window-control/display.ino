char timeDisplay[8];
char textDisplay[40];

void setupDisplay() {
  gfx->begin();
}

void displayStartupScreen() {
  gfx->fillScreen(BLACK);
  gfx->setCursor(20, 60);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(16, 16, 2);
  gfx->println(":)");
}

void printHeader(const char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_16);
  gfx->setTextColor(PURPLE);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

void printValue(unsigned int value, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(value);
}

void printValueText(const char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

void printTextLarge(const char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

void printTextSmall(const char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_16);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

void printSmallValueText(const char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_16);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

// Most stats
void refreshDisplay() {
  displayState = displayStats;

  int x = 0;
  int y = 40;

  gfx->fillScreen(BLACK);

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY);
  gfx->setCursor(x, y);
  gfx->print("Hall ");
  gfx->println(1);

  y += 30;
  
  // printHeader("CO2", x, y);

  // y += 20;

  // printHeader("1 min", x, y);
  // printHeader("5 min", x + 80, y);
  // printHeader("15 min", x + 160, y);

  // y += 30;

  // printValue(a1, x, y);
  // printValue(a5, x + 80, y);
  // printValue(a15, x + 160, y);

  printHeader("CO2", x, y);
  printHeader("Temp", x + 120, y);

  y += 30;

  printValue(a1, x, y);

  sprintf(textDisplay, "%.1f %cC\0", temperature, (char) 0x7E);
  printValueText(textDisplay, x + 120, y);
  
  y += 20;
  printHeader("Humidity", x, y);

  y += 20;

  sprintf(textDisplay, "%d %%", humidity);
  printSmallValueText(textDisplay, x, y);

  y = 190;

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY);
  gfx->setCursor(x, y);
  gfx->print("Hall ");
  gfx->println(2);

  y = 230;
  gfx->setFont(&URW_Gothic_L_Book_16);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->print("Mode: ");

  if (mode == OFF) {
    gfx->setTextColor(RED);
    gfx->println("OFF");
  } else {
    gfx->setTextColor(GREEN);
    gfx->println("ON");
  }

  refreshTime();
}

// Window open display
void windowOpenDisplay() {
  displayState = displayWindowOpen;

  int x = 0;
  int y = 30;

  gfx->fillScreen(BLACK);

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(CYAN);
  gfx->setCursor(x, y);
  gfx->println("Window Open");

  y += 30;
  
  printHeader("Time", x, y);

  y += 30;

  displayTime((millis() - openTime) / 1000, x, y, 3, 4);

  y += 30;

  printHeader("CO2", x, y);

  y += 30;

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  gfx->print(a1);
  gfx->println(" ppm");

  y += 30;

  printHeader("Temperature", x, y);

  y += 30;

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  sprintf(textDisplay, "%.1f %cC\0", temperature, (char) 0x7E);
  gfx->println(textDisplay);
}

void windowOpeningDisplay(int hall) {
  displayState = displayWindowOpening;

  int x = 0;
  int y = 40;

  gfx->fillScreen(BLACK);
  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY);

  gfx->setCursor(x, y);
  gfx->print("Hall ");
  gfx->println(hall);
  
  y = 120;
  gfx->setTextColor(CYAN);
  gfx->setCursor(x, y);
  gfx->println("Window");
  gfx->println("opening..");
}

void windowClosingDisplay(int hall) {
  displayState = displayWindowClosing;

  int x = 0;
  int y = 40;

  gfx->fillScreen(BLACK);
  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY);

  gfx->setCursor(x, y);
  gfx->print("Hall ");
  gfx->println(hall);

  y = 120;
  gfx->setTextColor(CYAN);
  gfx->setCursor(x, y);
  gfx->println("Window");
  gfx->println("closing..");
}

unsigned short pIndex = 0;
unsigned short pSelected = false;
bool pChanged = false;
char buf[12];

typedef enum {
	BACK,
	TEXT,
  ENUM,
	NUMBER
} MenuItemType;

typedef struct {
  MenuItemType type;
	const char *name;
	int *value;
  const char *text;
} MenuItem;

MenuItem menuItems[] {
  {.type = BACK, .name = "Back", .value = NULL, .text = "Go Back"},
  {.type = ENUM, .name = "Mode", .value = &mode, .text = NULL},
  {.type = NUMBER, .name = "CO2 Warning", .value = &co2UpperThreshold, .text = NULL},
  {.type = NUMBER, .name = "CO2 Target", .value = &co2LowerThreshold, .text = NULL},
  {.type = NUMBER, .name = "Max Open Time (min)", .value = &openTimeUpperThreshold, .text = NULL},
  {.type = NUMBER, .name = "Min Open Time (min)", .value = &openTimeLowerThreshold, .text = NULL},
  {.type = NUMBER, .name = "Hall 2 Open Time (min)", .value = &openTime2Threshold, .text = NULL},
};

void enumToText(int value) {
  Serial.print("VALUE: ");
  Serial.println(value);
  if (value == 0)
    snprintf(buf, 12, "%s", "OFF");
  else if (value == 1)
    snprintf(buf, 12, "%s", "CO2+Time");
  else
    snprintf(buf, 12, "%s", "Unknown");
}

void preferencesDisplay() {
  displayState = displayPreferences;

  int x = 0;
  int y = 30;

  gfx->fillScreen(BLACK);

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(CYAN);
  gfx->setCursor(x, y);
  gfx->println("Preferences");

  int i = 0;  

  if (pIndex > 1) {
    i = pIndex-1;
  }

  for (i; i < sizeof(menuItems)/sizeof(MenuItem); i++) {
    y += 20;
    printHeader(menuItems[i].name, x, y);
    y += 30;
    if (menuItems[i].type == NUMBER)
      printValue(*menuItems[i].value, x, y);
    else if (menuItems[i].type == ENUM) {
      enumToText(*menuItems[i].value);
      printTextLarge(buf, x, y);
    } else
      printTextLarge(menuItems[i].text, x, y);

    if (i == pIndex) {
      if (pSelected) {
        printTextLarge("+", 200, 60);
        printTextLarge("!", 200, 140);
        printTextLarge("-", 200, 220);
      } else
        printTextLarge("<", 200, y);
    }
  }
}

void increaseValue() {
  int i = pIndex;
  
  if (menuItems[i].type != NUMBER && menuItems[i].type  != ENUM)
    return;
  
  int value = *menuItems[i].value;

  if (menuItems[i].type == ENUM) {
    *menuItems[i].value = value + 1;
    if (*menuItems[i].value > 1)
      *menuItems[i].value = 1;
  } else {
    if (value >= 100) {
      *menuItems[i].value = value + 100;    
    } else {
      *menuItems[i].value = value + 1;
    }
  }

  if (value != *menuItems[i].value)
    pChanged = true;
}

void decreaseValue() {
  int i = pIndex;

  if (menuItems[i].type != NUMBER && menuItems[i].type  != ENUM)
    return;

  int value = *menuItems[i].value;

  if (menuItems[i].type == ENUM) {
    *menuItems[i].value = value - 1;
    if (*menuItems[i].value < 0)
      *menuItems[i].value = 0;
  } else {
    if (value <= 0) {
      *menuItems[i].value = 0;
    } else if (value <= 100) {
      *menuItems[i].value = value - 1;    
    } else {
      *menuItems[i].value = value - 100;
    }
  }

  if (value != *menuItems[i].value)
    pChanged = true;
}

void preferencesSelectedButtons(int button) {
  switch (button) {
    case BTN_UP:
      // Increase
      increaseValue();
      break;
    case BTN_DOWN:
      // Decrease
      decreaseValue();
      break;
    case BTN_MIDDLE:
      // Save
      pSelected = false;
      break;
  }

  preferencesDisplay();
}

void preferencesSave() {
  if (pChanged) {
    preferences.begin("settings", false);
    preferences.putInt("mode", mode);
    preferences.putInt("co2Max", co2UpperThreshold);
    preferences.putInt("co2Min", co2LowerThreshold);
    preferences.putInt("openMax", openTimeUpperThreshold);
    preferences.putInt("openMin", openTimeLowerThreshold);
    preferences.putInt("open2", openTime2Threshold);
    preferences.end();
    pChanged = false;
    Serial.println("Preferences saved");
  }
}

void preferencesHandleButtons(int button) {
  if (pSelected)
    return preferencesSelectedButtons(button);

  switch (button) {
    case BTN_UP:
      if (pIndex > 0) {
        pIndex--;
        preferencesDisplay();
      }  
      break;
    case BTN_DOWN:
      if (pIndex < sizeof(menuItems)/sizeof(MenuItem) -1) {
        pIndex++;
        preferencesDisplay();
      }  
      break;
    case BTN_MIDDLE:
      if (pIndex == 0) {
        // Close preferences
        preferencesSave();
        refreshDisplay();
      } else {
        pSelected = !pSelected;
        preferencesDisplay();
      }
      break;
  }
}

void refreshTime() {
  displayTimeWithHeader(120, 20, 1);
  displayTimeWithHeader(120, 170, 2);
}


// Refresh that happens often
void displayTimeWithHeader(int x, int y, int hall) {
  long diff;

  if (displayState == displayStats) {      
    if (hall == 1) {
      if (windowOpen) {
        printHeader("Open  ", x, y);
        diff = (millis() - openTime) / 1000;
        displayTime(diff, x, y + 20, 2, 2);
      } else {
        printHeader("Closed", x, y);
        diff = (millis() - closeTime) / 1000;
        displayTime(diff, x, y + 20, 2, 2);
      }
    } else if (hall == 2) {
      if (windowOpen2) {
        printHeader("Open  ", x, y);
        diff = (millis() - openTime2) / 1000;
        displayTime(diff, x, y + 20, 2, 2);
      } else {
        printHeader("Closed", x, y);
        diff = (millis() - closeTime2) / 1000;
        displayTime(diff, x, y + 20, 2, 2);
      }
    }
    
  } else if (displayState == displayWindowOpen) {
    diff = (millis() - openTime) / 1000;
    displayTime(diff, 0, 90, 3, 4);
  }
}

void displayTime(unsigned long seconds, unsigned int x, unsigned int y, unsigned int width, unsigned int height) {
  if (seconds >= 3600) {
    sprintf(timeDisplay, "%02d:%02d:%02d\0", seconds / 3600, (seconds % 3600) / 60, seconds % 60);
  } else if (seconds >= 600) {
    sprintf(timeDisplay, "%02d:%02d   \0", seconds / 60, seconds % 60);
  } else {
    sprintf(timeDisplay, "%d:%02d    \0", seconds / 60, seconds % 60);
  }
  if (width > 2)
    printTextLarge(timeDisplay, x, y);
  else
    printTextSmall(timeDisplay, x, y);
}