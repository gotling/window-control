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
 * WiFiManager by tzapu
 * https://github.com/tzapu/WiFiManager
 * PubSubClient by Nick O'Leary
 * https://github.com/knolleary/pubsubclient
 */

#include <Arduino.h>
#include <Preferences.h>
#include "MHZ19.h"
#include <SoftwareSerial.h>                                // Remove if using HardwareSerial
#include <Arduino_GFX_Library.h>
#include <DHT.h>
#include <FTDebouncer.h>
#include <WiFiClientSecure.h>
#include <WiFiManager.h>
#include <PubSubClient.h>

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
#define TFT_BRIGHTNESS 128
short tftBrightness = TFT_BRIGHTNESS;
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
#define CLOSE_OUT 23
#define CLOSE_OUT_2 22

// Network
WiFiManager wm;
WiFiClientSecure client;
PubSubClient mqtt(client);

// Configuration
Preferences preferences;

int screenTimeout = 60000; // Milliseconds
int mode = 0;
int ledWarning = 1;
int co2UpperThreshold = 2000;
int co2LowerThreshold = 1000;
int openTimeUpperThreshold = 10;
int openTimeLowerThreshold = 5;
int openTime2Threshold = 10;
int tempLowerThreshold = 16;

#define MQTT_SERVER "io.adafruit.com"
#define MQTT_PORT "8883"
#define MQTT_USER ""
#define MQTT_PASSWORD ""
#define MQTT_TOPIC ""

String mqttServer = MQTT_SERVER;
String mqttPort = MQTT_PORT;
String mqttUser = MQTT_USER;
String mqttPassword = MQTT_PASSWORD;
String mqttTopic = MQTT_TOPIC;

// Stats
enum {
  displayStats,
  displayWindowOpen,
  displayWindowOpening,
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
unsigned long openTime2 = millis();
unsigned long closeTime = millis();
unsigned long closeTime2 = millis();
unsigned long lastActionTime = millis();
unsigned long mqttTime = -30000;

// State
unsigned int CO2;
bool windowOpen = false;
bool windowOpen2 = false;
unsigned int a1;
unsigned int a5;
unsigned int a15;
float temperature;
unsigned int humidity;

// For working out if both buttons were pressed at the same time to stop windows
#define STOP_PRESS_RESET 5000
bool stop = false;
bool stop2 = false;

typedef enum {
  OFF,
  CO2TIME,
  TEMPTIME
} ModeType;

typedef enum {
  RECEIVE_OPEN,
  RECEIVE_OPEN_2,
  RECEIVE_CLOSE,
  RECEIVE_CLOSE_2,
  RECEIVE_STOP,
  RECEIVE_STOP_2,
  SEND_OPEN,
  SEND_OPEN_2,
  SEND_CLOSE,
  SEND_CLOSE_2
} Event;

void refreshLED() {
  if (ledWarning) {
    if (a1 >= co2UpperThreshold) {
      digitalWrite(LED, HIGH);
    } else if (a1 < co2UpperThreshold) {
      digitalWrite(LED, LOW);
    }
  }
}

void closeWindow() {
  Serial.println("Closing window in main hall");
  windowClosingDisplay(1);
  
  digitalWrite(CLOSE_OUT, HIGH);
  delay(100);
  digitalWrite(CLOSE_OUT, LOW);

  mqttSendEvent(SEND_CLOSE);
}

void closeWindow2() {
  Serial.println("Closing window in secondary hall");
  windowClosingDisplay(2);
  
  digitalWrite(CLOSE_OUT_2, HIGH);
  delay(100);
  digitalWrite(CLOSE_OUT_2, LOW);

  mqttSendEvent(SEND_CLOSE_2);
}

void actionOnSensor() {
  int diff;
  
  if (mode != OFF && windowOpen) {
    diff = (millis() - openTime) / 60000;

    if (diff >= openTimeUpperThreshold)
      closeWindow();
    else if (temperature <= tempLowerThreshold)
      closeWindow();
    else if (diff >= openTimeLowerThreshold) {
      if ((mode == CO2TIME) && (a1 <= co2LowerThreshold)) {
        closeWindow();
      }
    }
  }

  if (mode != OFF && windowOpen2) {
    diff = (millis() - openTime2) / 60000;

    if (diff >= openTime2Threshold)
      closeWindow2();
  }
}

void readAndRefresh() {
  readSensors();

  serialLog();

  refreshLED();

  // Screens with refreshing data
  switch(displayState) {
    case displayStats:
      refreshDisplay();
      break;
    case displayWindowOpen:
      windowOpenDisplay();
      break;
  }
}

// Setup
void setup()
{
  Serial.begin(115200);

  // Configuration
  preferences.begin("settings", true);
  mode = preferences.getInt("mode", 0);
  co2UpperThreshold = preferences.getInt("co2Max", 2000);
  co2LowerThreshold = preferences.getInt("co2Min", 1000);
  openTimeUpperThreshold = preferences.getInt("openMax", 10);
  openTimeLowerThreshold = preferences.getInt("openMin", 5);
  openTime2Threshold = preferences.getInt("open2", 10);
  tempLowerThreshold = preferences.getInt("tempMin", 16.0);
  preferences.end();
  preferences.begin("mqtt", true);
  mqttServer = preferences.getString("server", mqttServer);
  mqttPort = preferences.getString("port", mqttPort);
  mqttUser = preferences.getString("user", mqttUser);
  mqttPassword = preferences.getString("password", mqttPassword);
  mqttTopic = preferences.getString("topic", mqttTopic);
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
  ledcWrite(ledChannel, TFT_BRIGHTNESS);

  displayStartupScreen();

  // Input buttons
  setupButtons();

  // Output MOSFET
  pinMode(CLOSE_OUT, OUTPUT);
  pinMode(CLOSE_OUT_2, OUTPUT);
  digitalWrite(CLOSE_OUT, LOW);
  digitalWrite(CLOSE_OUT_2, LOW);

  // Setup WiFiManager
  setupWiFi();
  setupCustomParameters();

  // Read sensor data, log to serial and update display
  readAndRefresh();
}

// Main loop
void loop()
{
  pinDebouncer.update();
  wm.process();
  mqtt.loop();

  // Screen timout / Blank screen
  if (tftBrightness > 0 && (millis() - lastActionTime >= screenTimeout)) {
    tftBrightness = 0;
    ledcWrite(ledChannel, tftBrightness);
  }

  // Reset state of mulitple buttons being pressed
  if (millis() - lastActionTime >= STOP_PRESS_RESET) {
    stop = false;
    stop2 = false;
  }

  // Update time display  
  if (millis() - secondTimer >= 500) {
    refreshTime();

    secondTimer = millis();
  }

  // Read sensors and update display
  if (millis() - statsTimer >= 10000) {
    readAndRefresh();

    actionOnSensor();
    
    statsTimer = millis();
  }

  // Send to MQTT server
  if (millis() - mqttTime >= 60000) {
    if (WiFi.status() == WL_CONNECTED)
      mqttSend();
    
    mqttTime = millis();
  }

  // Reset display to default
  if ((displayState == displayWindowClosing || displayState == displayWindowOpening) && (millis() - lastActionTime) > toastTime) {
    refreshDisplay();
  }

  // Return to main view automatically after windows opened if mode is off
  if (mode == OFF && displayState == displayWindowOpen && (millis() - openTime) > toastTime) {
    refreshDisplay();
  }
}
