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

#include "fonts.h"

// CO2 sensor
#define RX_PIN 16                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 17                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)
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
unsigned int toastTime = 2000;
#define PURPLE 0xA45F
#define CYAN 0x4E3F

// LED
#define LED 2

// Buttons
FTDebouncer pinDebouncer;

// Output MOSFET
#define OPEN_OUT 33
#define CLOSE_OUT 32

// Configuration
unsigned int co2UpperThreshold = 1000;
unsigned int co2LowerThreshold = 900;
unsigned long openTimeThreshold = 60000;

// Stats
enum {
  displayStats,
  displayWindowOpen,
  displayWindowClosing
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
int temperature;
unsigned int humidity;

void refreshLED() {
  if (a1 >= co2UpperThreshold) {
    digitalWrite(LED, HIGH);
  } else if (a1 <= co2LowerThreshold) {
    digitalWrite(LED, LOW);
  }
}

void closeWindows() {
  displayState = displayWindowClosing;
  windowClosingDisplay();

  digitalWrite(CLOSE_OUT, HIGH);
  delay(100);
  digitalWrite(CLOSE_OUT, LOW);
  Serial.println("Sent windows close trigger");
  windowOpen = false; // TODO: Remove this when read signal from button press which should trigger when we send close signal
  closeTime = millis();  
}

void actionOnCO2() {
  if (windowOpen && (millis() - openTime) > openTimeThreshold) {
    if (a1 <= co2LowerThreshold) {
      // Good CO2 level reached, close windows by sending button press
      closeWindows();
    }
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

  // CO2 sensor
  setupMHZ19();

  // DHT
  dht.begin();

  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Display
  setupDisplay();

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
    displayState = displayStats;
    refreshDisplay();
  }
}
