/*
 * 
 * https://github.com/WifWaf/MH-Z19/blob/master/examples/RetrieveDeviceInfo/RetrieveDeviceInfo.ino
 * https://github.com/moononournation/Arduino_GFX/blob/master/examples/HelloWorld/HelloWorld.ino
 */

#include <Arduino.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial
#include <Arduino_GFX_Library.h>

#define RX_PIN 16                                          // Rx pin which the MHZ19 Tx pin is attached to
#define TX_PIN 17                                          // Tx pin which the MHZ19 Rx pin is attached to
#define BAUDRATE 9600                                      // Device to MH-Z19 Serial baudrate (should not be changed)


MHZ19 myMHZ19;                                             // Constructor for library
SoftwareSerial mySerial(RX_PIN, TX_PIN);                   // (Uno example) create device to MH-Z19 serial

/* More data bus class: https://github.com/moononournation/Arduino_GFX/wiki/Data-Bus-Class */
Arduino_DataBus *bus = new Arduino_ESP32SPI(27 /* DC */, -1 /* CS */, 18 /* SCK */, 23 /* MOSI */, -1 /* MISO */, VSPI /* spi_num */);
Arduino_GFX *gfx = new Arduino_ST7789(bus, 33 /* RST */, 2 /* rotation */, true /* IPS */,
                                      240 /* width */, 240 /* height */, 0 /* col offset 1 */, 80 /* row offset 1 */);
#define TFT_BL 22

#define LED 19
unsigned int co2Threshold = 1000;

unsigned long getDataTimer = 0;
unsigned long statsTimer = 0;
unsigned long counter = 0;

unsigned int minCO = 5000;
unsigned int maxCO = 0;

unsigned int avg1[30];
unsigned int avg5[30 * 5];
unsigned int avg15[30 * 15];

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

void setup()
{
  Serial.begin(115200);                                     // Device to serial monitor feedback

  mySerial.begin(BAUDRATE);                               // (Uno example) device to MH-Z19 serial start
  myMHZ19.begin(mySerial);                                // *Serial(Stream) refence must be passed to library begin().


  /*
      getVersion(char array[]) returns version number to the argument. The first 2 char are the major
      version, and second 2 bytes the minor version. e.g 02.11
  */

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

  // LED
  pinMode(LED, OUTPUT);
  digitalWrite(LED, LOW);

  // Display

  gfx->begin();
  gfx->fillScreen(BLACK);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH);

  gfx->setCursor(100, 100);
  gfx->setTextColor(WHITE);
  gfx->setTextSize(8, 8, 2);
  gfx->println(":)");
}

void loop()
{
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

    getDataTimer = millis();
    counter++;
  }

  // Print stats every 10 seconds
  if (millis() - statsTimer >= 10000) {
    unsigned int a1 = getAvg(avg1, 30);
    unsigned int a5 = getAvg(avg5, 30 * 5);
    unsigned int a15 = getAvg(avg15, 30 * 15);
    unsigned int temp;
    unsigned int CO2;
    temp = myMHZ19.getTemperature();                     // Request Temperature (as Celsius)
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
    Serial.print(temp);
    Serial.println(" °C");

    int x = 0;
    int y = 0;

    gfx->fillScreen(BLACK);
    gfx->setCursor(x, y);
    gfx->setTextColor(NAVY);
    gfx->setTextSize(2, 3, 0);
    gfx->println("CO2 (ppm)");

    y += 30;

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

    gfx->println("Temperature");

    y += 20;

    gfx->setTextColor(LIGHTGREY);
    gfx->setTextSize(3, 4, 0);
    gfx->setCursor(x, y);
    gfx->print(temp);
    gfx->print(" ");
    gfx->print((char)247);
    gfx->println("C");

    statsTimer = millis();
  }
}
