char timeDisplay[8];

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

// Most stats
void refreshDisplay() {
  int x = 0;
  int y = 0;

  gfx->fillScreen(BLACK);

  gfx->setTextColor(PURPLE);
  gfx->setTextSize(2, 2, 0);
  gfx->setCursor(x, y);
  gfx->println("Current");
  gfx->setCursor(x + 90, y);
  gfx->println("Min");
  gfx->setCursor(x + 160, y);
  gfx->println("Max");

  y += 20;

  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(3, 4, 0);
  gfx->println(CO2);
  gfx->setCursor(x + 90, y);
  gfx->println(minCO);
  gfx->setCursor(x + 160, y);
  gfx->println(maxCO);

  y += 40;

  gfx->setCursor(x, y);
  gfx->setTextColor(PURPLE);
  gfx->setTextSize(2, 2, 0);

  gfx->println("Average");

  y += 20;

  gfx->setCursor(x, y);
  gfx->println("1 min");
  gfx->setCursor(x + 80, y);
  gfx->println("5 min");
  gfx->setCursor(x + 160, y);
  gfx->println("15 min");

  y += 20;

  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(3, 4, 0);
  gfx->println(a1);
  gfx->setCursor(x + 80, y);
  gfx->println(a5);
  gfx->setCursor(x + 160, y);
  gfx->println(a15);

  y += 40;

  gfx->setCursor(x, y);
  gfx->setTextColor(PURPLE);
  gfx->setTextSize(2, 2, 0);

  gfx->println("Temp");

  gfx->setCursor(x + 120, y);
  gfx->println("Humidity");

  y += 20;

  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(2, 2, 0);
  gfx->setCursor(x, y);
  gfx->print(temperature);
  gfx->print(" ");
  gfx->print((char)247);
  gfx->print("C");
  
  gfx->setCursor(x + 120, y);
  gfx->print(humidity);
  gfx->print(" %");

  refreshTime();
}

// Window open display
void windowOpenDisplay() {
  int x = 0;
  int y = 0;

  gfx->fillScreen(BLACK);

  gfx->setTextColor(CYAN);
  gfx->setTextSize(3, 4, 0);
  gfx->setCursor(x, y);
  gfx->println("Window Open");

  y += 40;
  
  gfx->setCursor(x, y);
  gfx->setTextSize(2, 2, 0);
  gfx->setTextColor(PURPLE);
  gfx->println("Time");

  y += 20;

  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(3, 4, 0);
  displayTime((millis() - openTime) / 1000, x, y, 3, 4);

  y += 40;

  gfx->setCursor(x, y);
  gfx->setTextSize(2, 2, 0);
  gfx->setTextColor(PURPLE);
  gfx->println("CO2");

  y += 20;

  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(3, 4, 0);
  gfx->print(a1);
  gfx->println(" ppm");

  y += 40;

  gfx->setCursor(x, y);
  gfx->setTextSize(2, 2, 0);
  gfx->setTextColor(PURPLE);
  gfx->println("Temperature");

  y += 20;

  gfx->setCursor(x, y);
  gfx->setTextColor(LIGHTGREY);
  gfx->setTextSize(3, 4, 0);
  gfx->print(temperature);
  gfx->print(" ");
  gfx->print((char)247);
  gfx->println("C");
}

void windowClosingDisplay() {
  int x = 0;
  int y = 100;

  gfx->fillScreen(BLACK);

  gfx->setTextColor(CYAN);
  gfx->setTextSize(3, 4, 0);
  gfx->setCursor(x, y);
  gfx->println("Closing");
  gfx->println("window..");
}

// Refresh that happens often
void refreshTime() {
  long diff;

  if (displayState == displayStats) {  
    unsigned int x = 0;
    unsigned int y = 200;

    gfx->setCursor(x, y);
    gfx->setTextColor(PURPLE, BLACK);
    gfx->setTextSize(2, 2, 0);
    
    if (windowOpen) {
      gfx->print("Window open  ");
      diff = (millis() - openTime) / 1000;
      displayTime(diff, x, y + 20, 2, 2);
    } else {
      gfx->print("Window closed");
      diff = (millis() - closeTime) / 1000;
      displayTime(diff, x, y + 20, 2, 2);
    }
  } else if (displayState == displayWindowOpen) {
    diff = (millis() - openTime) / 1000;
    displayTime(diff, 0, 60, 3, 4);
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

  gfx->setTextSize(width, height, 0);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setCursor(x, y);
  gfx->print(timeDisplay);
}