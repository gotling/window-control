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

void refreshTime() {
  long diff;
  unsigned int x = 0;
  unsigned int y = 200;

  gfx->setCursor(x, y);
  gfx->setTextColor(PURPLE, BLACK);
  gfx->setTextSize(2, 2, 0);
  
  if (windowOpen) {
    gfx->print("Windows open  ");
    diff = (millis() - openTime) / 1000;
    displayTime(diff, x, y + 20);
  } else {
    gfx->print("Windows closed");
    diff = (millis() - closeTime) / 1000;
    displayTime(diff, x, y + 20);
  }
}

void displayTime(unsigned long seconds, unsigned int x, unsigned int y) {
  if (seconds >= 3600) {
    sprintf(timeDisplay, "%02d:%02d:%02d\0", seconds / 3600, (seconds % 3600) / 60, seconds % 60);
  } else if (seconds >= 600) {
    sprintf(timeDisplay, "%02d:%02d   \0", seconds / 60, seconds % 60);
  } else {
    sprintf(timeDisplay, "%d:%02d    \0", seconds / 60, seconds % 60);
  }

  gfx->setTextSize(2, 2, 0);
  gfx->setTextColor(LIGHTGREY, BLACK);
  gfx->setCursor(x, y);
  gfx->print(timeDisplay);
}