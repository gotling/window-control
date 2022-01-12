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

void printHeader(char *text, unsigned int x, unsigned int y) {
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

void printTextLarge(char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_30);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

void printTextSmall(char *text, unsigned int x, unsigned int y) {
  gfx->setFont(&URW_Gothic_L_Book_16);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setTextSize(1);
  gfx->setCursor(x, y);
  gfx->println(text);
}

void printSmallValueText(char *text, unsigned int x, unsigned int y) {
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

  sprintf(textDisplay, "%d \xB0 %cC\0", temperature, (char) 247);
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
  gfx->print((char)247);
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