void setupMHZ19() {
  mySerial.begin(BAUDRATE);
  myMHZ19.begin(mySerial);

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

  myMHZ19.autoCalibration();
}

unsigned int getAvg(unsigned int values[], int sizeOfArray) {
  if (counter == 0) {
    return values[0];
  }
  unsigned int sum = 0;
  int iMax = sizeOfArray;
  if (counter + 1 < sizeOfArray)
    iMax = counter + 1;
  
  for (int i = 0; i < iMax; i++) {
    sum += values[i];
  }
  
  return sum / iMax;
}

void readSensors() {
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

  // Read temperature and humidity from DHT sensor
  float newT = dht.readTemperature();
  float newH = dht.readHumidity();
  
  if (!isnan(newT)) {
    temperature = (newT + temperatureOffset) / 100 * 100;
  }
  
  if (!isnan(newH)) {
    humidity = newH + humidityOffset;
  }

  counter++;
}