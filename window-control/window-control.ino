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
#include <Preferences.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial
#include <Arduino_GFX_Library.h>
#include <DHT.h>
#include <FTDebouncer.h>

#include "fonts.h"

// CO2 sensor
#define RX_PIN 16                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 17                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)
MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

// DHT temperature and humidity
#define DHTPIN 15
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);
int temperatureOffset = 0;
int humidityOffset = 0;

// Display
Arduino_DataBus *bus = new Arduino_ESP32SPI(27 /* DC */, -1 /* CS */, 14 /* SCK */, 13 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 12 /* RST */, 3 /* rotation */, true /* IPS */,
                                      240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
#undef TFT_BL
#define TFT_BL 26
bool backlightState = true;
unsigned int toastTime = 2000;
#undef PURPLE
#define PURPLE 0xA45F
#undef CYAN
#define CYAN 0x4E3F

// setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;


// LED
#define LED 2

// Buttons
FTDebouncer pinDebouncer;

// Output MOSFET
#define OPEN_OUT 33
#define CLOSE_OUT 32

// Configuration
Preferences preferences;

unsigned int co2UpperThreshold = 1000;
unsigned int co2LowerThreshold = 900;
unsigned int openTimeUpperThreshold = 2; //120000;
unsigned int openTimeLowerThreshold = 1; //60000;

// Stats
enum {
  displayStats,
  displayWindowOpen,
  displayWindowClosing,
  displayPreferences
} displayState = displayStats;

unsigned long statsTimer = 0;
unsigned long secondTimer = 0;
unsigned long counter = 0;

unsigned int minCO = 5000;
unsigned int maxCO = 0;

unsigned int avg1[6];
unsigned int avg5[30];
unsigned int avg15[90];

unsigned long openTime = millis();
unsigned long closeTime = millis();

// State
unsigned int CO2;
bool windowOpen = false;
unsigned int a1;
unsigned int a5;
unsigned int a15;
float temperature;
unsigned int humidity;

void refreshLED() {
  if (a1 >= co2UpperThreshold) {
    digitalWrite(LED, HIGH);
  } else if (a1 <= co2LowerThreshold) {
    digitalWrite(LED, LOW);
  }
}

void openWindow() {
  Serial.println("OPEN OUT HIGH");
  windowOpen = true;
  openTime = millis();
  windowOpenDisplay();
  
  digitalWrite(OPEN_OUT, HIGH);
  delay(100);
  digitalWrite(OPEN_OUT, LOW);
}

void closeWindow() {
  Serial.println("CLOSE OUT HIGH");
  windowOpen = false;
  closeTime = millis();
  windowClosingDisplay();
  
  digitalWrite(CLOSE_OUT, HIGH);
  delay(100);
  digitalWrite(CLOSE_OUT, LOW);
}

void actionOnCO2() {
  if (windowOpen) {
    unsigned long diff = (millis() - openTime) / 60000;
    
    if (diff > openTimeUpperThreshold || (diff > openTimeLowerThreshold && a1 <= co2LowerThreshold))
      closeWindow();
  }
}

void readAndRefresh() {
  readSensors();

  serialLog();

  switch(displayState) {
    case displayStats:
      refreshDisplay();
      break;
    case displayWindowOpen:
      windowOpenDisplay();
      break;
    case displayWindowClosing:
      windowClosingDisplay();
      break;
    case displayPreferences:
      preferencesDisplay();
      break;      
    default:
      refreshDisplay();
      break;
  }

  refreshLED();
}

// Setup
void setup()
{
  Serial.begin(115200);

  // Configuration
  preferences.begin("settings", true);
  co2UpperThreshold = preferences.getUInt("co2Max", 1000);
  co2LowerThreshold = preferences.getUInt("co2Min", 900);
  openTimeUpperThreshold = preferences.getUInt("openMax", 2);
  openTimeLowerThreshold = preferences.getUInt("openMin", 1);
  preferences.end();

  // CO2 sensor
  setupMHZ19();

  // DHT
  dht.begin();

  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Display
  setupDisplay();

  // Set display brightness
  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(TFT_BL, ledChannel);
  ledcWrite(ledChannel, 128);

  displayStartupScreen();

  // Input buttons
  setupButtons();

  // Output MOSFET
  pinMode(OPEN_OUT, OUTPUT);
  pinMode(CLOSE_OUT, OUTPUT);
  digitalWrite(OPEN_OUT, LOW);
  digitalWrite(CLOSE_OUT, LOW);

  // Read sensor data, log to serial and update display
  readAndRefresh();
}

// Main loop
void loop()
{
  pinDebouncer.update();

  // Update time display  
  if (millis() - secondTimer >= 500) {
    refreshTime();

    secondTimer = millis();
  }

  // Read sensors and upate display
  if (millis() - statsTimer >= 10000) {
    readAndRefresh();

    actionOnCO2();
    
    statsTimer = millis();
  }

  // Reset display to default
  if (displayState == displayWindowClosing && (millis() - closeTime) > toastTime) {
    refreshDisplay();
  }
}
