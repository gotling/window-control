/*
 * # Libraries
 *
 * EspSoftwareSerial by Dirk Kaar, Peter Lerup
 * https://github.com/plerup/espsoftwareserial/
 * MH-Z19 by Jonathan Dempsey
 * https://github.com/WifWaf/MH-Z19/blob/master/examples/RetrieveDeviceInfo/RetrieveDeviceInfo.ino
 * GFX Library for Arduino by Moon On Our Nation
 * https://github.com/moononournation/Arduino_GFX/blob/master/examples/HelloWorld/HelloWorld.ino
 * FTDebouncer by Ubi de Feo
 * https://github.com/ubidefeo/FTDebouncer
 * DHT sensor library by Adafruit 
 * https://github.com/adafruit/DHT-sensor-library
 */

#include <Arduino.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial
#include <Arduino_GFX_Library.h>
#include <DHT.h>
#include <FTDebouncer.h>

#define RX_PIN 16                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 17                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)

// Serial log
#define RX0_PIN 3
#define TX0_PIN 1

// CO2 sensor
MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

// DHT temperature and humidity
#define DHTPIN 15
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
int temperatureOffset = 0;
int humidityOffset = 0;

// Display
Arduino_DataBus *bus = new Arduino_ESP32SPI(27 /* DC */, -1 /* CS */, 14 /* SCK */, 13 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 12 /* RST */, 3 /* rotation */, true /* IPS */,
                                      240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
#define TFT_BL 26
bool backlightState = true;
//#include "FreeMono24pt7b.h"

// LED
#define LED 2
unsigned int co2Threshold = 1000;

// Input buttons
#define BTN_OPEN 35
#define BTN_CLOSE 34
#define BTN_DOWN 18
#define BTN_MIDDLE 5
#define BTN_UP 19

unsigned long openTime = millis();
unsigned long closeTime = millis();
char timeDisplay[8];

FTDebouncer pinDebouncer;

// Output MOSFET
#define OPEN_OUT 33
#define CLOSE_OUT 32

// Stats
unsigned long getDataTimer = 0;
unsigned long statsTimer = 0;
unsigned long secondTimer = 0;
unsigned long inputTimer = 0;
unsigned long counter = 0;

unsigned int minCO = 5000;
unsigned int maxCO = 0;

unsigned int avg1[30];
unsigned int avg5[30 * 5];
unsigned int avg15[30 * 15];

// State
bool windowOpen = false;
unsigned int a1;
unsigned int a5;
unsigned int a15;
unsigned int CO2;
int temperature;
unsigned int humidity;


// Helper functions
unsigned int getAvg(unsigned int values[], int sizeOfArray) {
  unsigned int sum = 0;
  int iMax = sizeOfArray;
  if (counter < sizeOfArray)
    iMax = counter;

  for (int i = 0; i < iMax; i++) {
    sum += values[i];
  }
  return sum / iMax;
}

// Input
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
    case BTN_DOWN:
      Serial.println("OPEN OUT HIGH");
      digitalWrite(OPEN_OUT, HIGH);
      break;
    case BTN_UP:
      Serial.println("CLOSE OUT HIGH");
      digitalWrite(CLOSE_OUT, HIGH);
      break;
    case BTN_MIDDLE:
      Serial.println("CLOSE AND OPEN LOW");
      digitalWrite(OPEN_OUT, LOW);
      digitalWrite(CLOSE_OUT, LOW);
      break;
  }
}

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

// Setup
void setup()
{
  //Serial.begin(115200);                                     // Device to serial monitor feedback
  Serial.begin(115200, SERIAL_8N1, RX0_PIN, TX0_PIN);
  
  // CO2 sensor
  mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin().

  char myVersion[4];
  myMHZ19.getVersion(myVersion);

  Serial.print("\nFirmware Version: ");
  for (byte i = 0; i < 4; i++)
  {
    Serial.print(myVersion[i]);
    if (i == 1)
      Serial.print(".");
  }
  Serial.println("");

  Serial.print("Range: ");
  Serial.println(myMHZ19.getRange());
  Serial.print("Background CO2: ");
  Serial.println(myMHZ19.getBackgroundCO2());
  Serial.print("Temperature Cal: ");
  Serial.println(myMHZ19.getTempAdjustment());
  Serial.print("ABC Status: "); myMHZ19.getABC() ? Serial.println("ON") :  Serial.println("OFF");

  myMHZ19.autoCalibration();                              // Turn auto calibration ON (OFF autoCalibration(false))

  // DHT
  dht.begin();

  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Display
  gfx->begin();
  gfx->fillScreen(BLACK);
  //gfx->setFont(&FreeMono24pt7b);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, backlightState);

  gfx->setCursor(20, 60);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(16, 16, 2);
  gfx->println(":)");

  // Input buttons
  pinDebouncer.addPin(BTN_OPEN, LOW);
  pinDebouncer.addPin(BTN_CLOSE, LOW);
  
  pinDebouncer.addPin(BTN_DOWN, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_MIDDLE, HIGH, INPUT_PULLUP);
  pinDebouncer.addPin(BTN_UP, HIGH, INPUT_PULLUP);
  pinDebouncer.begin();

  // Output MOSFET
  pinMode(OPEN_OUT, OUTPUT);
  pinMode(CLOSE_OUT, OUTPUT);
  digitalWrite(OPEN_OUT, LOW);
  digitalWrite(CLOSE_OUT, LOW);
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

// Main loop
void loop()
{
  if (millis() - secondTimer >= 1000) {
    refreshTime();
    secondTimer = millis();
  }
  
  if (millis() - getDataTimer >= 2000)
  {
    unsigned int CO2;

    /* note: getCO2() default is command "CO2 Unlimited". This returns the correct CO2 reading even
      if below background CO2 levels or above range (useful to validate sensor). You can use the
      usual documented command with getCO2(false) */

    CO2 = myMHZ19.getCO2();                             // Request CO2 (as ppm)

    avg1[counter % 30] = CO2;
    avg5[counter % (30 * 5)] = CO2;
    avg15[counter % (30 * 15)] = CO2;

    if (CO2 > 0 && CO2 < minCO)
      minCO = CO2;
    else if (CO2 > maxCO)
      maxCO = CO2;

    // Read temperature and humidity from DHT sensor
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    
    if (!isnan(newT)) {
      temperature = (newT + temperatureOffset) / 100 * 100;
    }
    
    if (!isnan(newH)) {
      humidity = newH + humidityOffset;
    }

    getDataTimer = millis();
    counter++;
  }

  // Print stats every 10 seconds
  if (millis() - statsTimer >= 10000) {
    a1 = getAvg(avg1, 30);
    a5 = getAvg(avg5, 30 * 5);
    a15 = getAvg(avg15, 30 * 15);
    
    CO2 = myMHZ19.getCO2();

    if (a1 > co2Threshold) {
      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(LED, LOW);
    }

    Serial.print("CO2 Current/Min/Max: ");
    Serial.print(CO2);
    Serial.print("/");
    Serial.print(minCO);
    Serial.print("/");
    Serial.print(maxCO);

    Serial.print(" Avg 1/5/15 min: ");
    Serial.print(a1);
    Serial.print("/");
    Serial.print(a5);
    Serial.print("/");
    Serial.print(a15);

    Serial.print(" Temp: ");
    Serial.print(temperature);
    Serial.print(" °C");
    Serial.print(" Humidity: ");
    Serial.print(humidity);
    Serial.println(" %");

    refreshDisplay();
    
    statsTimer = millis();
  }

  pinDebouncer.update();
}
