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

unsigned int avg1[6];
unsigned int avg5[30];
unsigned int avg15[90];

// State
unsigned int CO2;
bool windowOpen = false;
unsigned int a1;
unsigned int a5;
unsigned int a15;
int temperature;
unsigned int humidity;


// Helper functions
unsigned int getAvg(unsigned int values[], int sizeOfArray) {
  if (counter == 0) {
    return values[0];
  }
  unsigned int sum = 0;
  int iMax = sizeOfArray;
  if (counter < sizeOfArray)
    iMax = counter;

  for (int i = 0; i < iMax; i++) {
    sum += values[i];
  }
  
  return sum / iMax;
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
  //gfx->setFont(&FreeMono24pt7b);
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, backlightState);

  displayStartupScreen();

  // Input buttons
  setupButtons();

  // Output MOSFET
  pinMode(OPEN_OUT, OUTPUT);
  pinMode(CLOSE_OUT, OUTPUT);
  digitalWrite(OPEN_OUT, LOW);
  digitalWrite(CLOSE_OUT, LOW);
}

// Main loop
void loop()
{
  // Update time display  
  if (millis() - secondTimer >= 800) {
    refreshTime();
    secondTimer = millis();
  }

  // Read sensors and upate display
  if (millis() - statsTimer >= 10000) {
    CO2 = myMHZ19.getCO2();

    // Store min and max CO2
    if (CO2 > 0 && CO2 < minCO)
      minCO = CO2;
    else if (CO2 > maxCO)
      maxCO = CO2;

    // Store CO2 for last minute, 5 minutes and 15 minutes
    avg1[counter % 6] = CO2;
    avg5[counter % (30)] = CO2;
    avg15[counter % (90)] = CO2;

    // Calculate average
    a1 = getAvg(avg1, 6);
    a5 = getAvg(avg5, 30);
    a15 = getAvg(avg15, 90);

    if (a1 > co2Threshold) {
      digitalWrite(LED, HIGH);
    } else {
      digitalWrite(LED, LOW);
    }

    // Read temperature and humidity from DHT sensor
    float newT = dht.readTemperature();
    float newH = dht.readHumidity();
    
    if (!isnan(newT)) {
      temperature = (newT + temperatureOffset) / 100 * 100;
    }
    
    if (!isnan(newH)) {
      humidity = newH + humidityOffset;
    }

    serialLog();

    refreshDisplay();
    
    counter++;
    statsTimer = millis();
  }

  pinDebouncer.update();
}
