void serialLog() {
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
}