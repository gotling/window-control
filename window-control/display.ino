char timeDisplay[8];
char textDisplay[40];

void setupDisplay() {
  gfx->begin();
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, backlightState);
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
  int y = 16;

  gfx->fillScreen(BLACK);

  printHeader("Current", x, y);
  printHeader("Min", x + 80, y);
  printHeader("Max", x + 160, y);

  y += 30;

  printValue(CO2, x, y);
  printValue(minCO, x + 80, y);
  printValue(maxCO, x + 160, y);
  gfx->setFont(NULL);

  y += 20;

  printHeader("Average", x, y);

  y += 20;

  printHeader("1 min", x, y);
  printHeader("5 min", x + 80, y);
  printHeader("15 min", x + 160, y);

  y += 30;

  printValue(a1, x, y);
  printValue(a5, x + 80, y);
  printValue(a15, x + 160, y);

  y += 40;

  printHeader("Temp", x, y);
  printHeader("Humidity", x + 120, y);

  y += 20;

  sprintf(textDisplay, "%d %cC\0", temperature, (char) 0x7E);
  printSmallValueText(textDisplay, x, y);
  
  sprintf(textDisplay, "%d %%", humidity);
  printSmallValueText(textDisplay, x + 120, y);

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
  gfx->print(temperature);
  gfx->print(" ");
  gfx->print((char)0x7E);
  gfx->println("C");
}

void windowClosingDisplay() {
  displayState = displayWindowClosing;
  int x = 0;
  int y = 100;

  gfx->fillScreen(BLACK);

  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(CYAN);
  gfx->setCursor(x, y);
  gfx->println("Closing");
  gfx->println("window..");
}

// mode
// unsigned int co2UpperThreshold = 1000;
// unsigned int co2LowerThreshold = 900;
// unsigned long openTimeUpperThreshold = 120000;
// unsigned long openTimeLowerThreshold = 60000;

unsigned short pIndex = 0;
unsigned short pSelected = false;
bool pChanged = false;

typedef enum {
	BACK,
	TEXT,
	NUMBER
} MenuItemType;

typedef struct {
  MenuItemType type;
	const char *name;
	unsigned int *value;
  const char *text;
} MenuItem;

MenuItem menuItems[] {
  {.type = BACK, .name = "Back", .value = NULL, .text = "Go Back"},
  {.type = TEXT, .name = "Mode", .value = NULL, .text = "CO2+Time"},
  {.type = NUMBER, .name = "CO2 Upper", .value = &co2UpperThreshold, .text = NULL},
  {.type = NUMBER, .name = "CO2 Lower", .value = &co2LowerThreshold, .text = NULL},
  {.type = NUMBER, .name = "Max Open Time (min)", .value = &openTimeUpperThreshold, .text = NULL},
  {.type = NUMBER, .name = "Min Open Time (min)", .value = &openTimeLowerThreshold, .text = NULL},
};

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
    else
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
  
  if (menuItems[i].type != NUMBER)
    return;
  
  int value = *menuItems[i].value;  
  if (value >= 100) {
    *menuItems[i].value = value + 100;    
  } else {
    *menuItems[i].value = value + 1;
  }

  if (value != *menuItems[i].value)
    pChanged = true;
}

void decreaseValue() {
  int i = pIndex;

  if (menuItems[i].type != NUMBER)
    return;

  int value = *menuItems[i].value;  
  if (value <= 0) {
    *menuItems[i].value = 0;
  } else if (value <= 100) {
    *menuItems[i].value = value - 1;    
  } else {
    *menuItems[i].value = value - 100;
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
    preferences.putUInt("co2Max", co2UpperThreshold);
    preferences.putUInt("co2Min", co2LowerThreshold);
    preferences.putUInt("openMax", openTimeUpperThreshold);
    preferences.putUInt("openMin", openTimeLowerThreshold);
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

// Refresh that happens often
void refreshTime() {
  long diff;

  if (displayState == displayStats) {  
    unsigned int x = 0;
    unsigned int y = 210;
    
    if (windowOpen) {
      printHeader("Window open  ", x, y);
      diff = (millis() - openTime) / 1000;
      displayTime(diff, x, y + 20, 2, 2);
    } else {
      printHeader("Window closed", x, y);
      diff = (millis() - closeTime) / 1000;
      displayTime(diff, x, y + 20, 2, 2);
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