char message[80];
String clientId = "ESP32WindowControl";

// FIXME: WiFiManager will show default values, not what is persisted
WiFiManagerParameter paramMqttServer("mqttserver", "MQTT server", mqttServer.c_str(), 40);
WiFiManagerParameter paramMqttPort("mqttport", "MQTT port", mqttPort.c_str(), 6);
WiFiManagerParameter paramMqttUser("mqttuser", "MQTT user", mqttUser.c_str(), 40);
WiFiManagerParameter paramMqttPassword("mqttpassword", "MQTT password", mqttPassword.c_str(), 40);
WiFiManagerParameter paramMqttTopic("mqtttopic", "MQTT topic", mqttTopic.c_str(), 40);

//callback notifying us of the need to save config
void saveConfigCallback () { 
  mqttServer = paramMqttServer.getValue();
  mqttPort = paramMqttPort.getValue();
  mqttUser = paramMqttUser.getValue();
  mqttPassword = paramMqttPassword.getValue();
  mqttTopic = paramMqttTopic.getValue();

  preferences.begin("mqtt", false);
  preferences.putString("server", mqttServer);
  preferences.putString("port", mqttPort);
  preferences.putString("user", mqttUser);
  preferences.putString("password", mqttPassword);
  preferences.putString("topic", mqttTopic);
  preferences.end();
  Serial.println("MQTT Preferences saved");
}

void setupWiFi() {
  // WiFi & Config
  WiFi.mode(WIFI_STA);
  //wm.resetSettings();
  wm.setConfigPortalBlocking(false);
  wm.setConfigPortalTimeout(60);
  wm.setParamsPage(true);

  // invert theme, dark
  wm.setDarkMode(true);
  // set Hostname
  //wm.setHostname("WindowControl");
  wm.setBreakAfterConfig(true);

  if (wm.autoConnect("Window Control")) {
      Serial.println("Connected to WiFi");
  }
  else {
      Serial.println("Configuration portal running");
  }
  //wm.startConfigPortal("Window Control");

  clientId = "ESP32Client-";
  clientId += String(random(0xffffff), HEX);
  Serial.print("Client ID: ");
  Serial.println(clientId.c_str());
  Serial.println("DEBUG MQTT");
  Serial.println(mqttServer);
  Serial.println(mqttPort);
  Serial.println(mqttUser);
  Serial.println(mqttPassword);
  Serial.println(mqttTopic);
  Serial.println("END DEBUG MQTT");

  mqtt.setServer(mqttServer.c_str(), mqttPort.toInt());
}

void setupCustomParameters() {
  wm.addParameter(&paramMqttServer);
  wm.addParameter(&paramMqttPort);
  wm.addParameter(&paramMqttUser);
  wm.addParameter(&paramMqttPassword);
  wm.addParameter(&paramMqttTopic);
  wm.setSaveConfigCallback(saveConfigCallback);
  wm.setPreSaveConfigCallback(saveConfigCallback);
}

bool mqttConnect() {
  client.setInsecure();

  if (mqtt.connected()) {
    return true;
  }

  Serial.print("Connecting to MQTT... ");
  if (mqtt.connect(clientId.c_str(), mqttUser.c_str(), mqttPassword.c_str())) {
    Serial.println("MQTT Connected!");
    return true;
  } else {
    Serial.print("MQTT connection failed, rc=");
    Serial.println(mqtt.state());
    return false;
  }
}

void mqttSend() {
  if (a1 == 0)
    return;

  sprintf(message, "co2,%d\nco2-a1,%d\nco2-a5,%d\nco2-a15,%d\ntemperature,%.1f\nhumidity,%d", CO2, a1, a5, a15, temperature, humidity);

  if (mqttConnect()) {
    if (mqtt.publish(mqttTopic.c_str(), message)) {
      Serial.println("MQTT send OK!");
    } else {
      Serial.println("MQTT send Failed"); 
    }
  }
}

void mqttSendEvent(Event event) {
  switch (event) {
    case RECEIVE_OPEN:
      sprintf(message, "in,1\n");
      break;
    case RECEIVE_OPEN_2:
      sprintf(message, "in_2,1\n");
      break;
    case RECEIVE_CLOSE:
      sprintf(message, "in,0\n");
      break;
    case RECEIVE_CLOSE_2:
      sprintf(message, "in_2,0\n");
      break;
    case RECEIVE_STOP:
      sprintf(message, "in,0.5\n");
      break;
    case RECEIVE_STOP_2:
      sprintf(message, "in_2,0.5\n");
      break;
    case SEND_CLOSE:
      sprintf(message, "out,0\n");
      break;
    case SEND_CLOSE_2:
      sprintf(message, "out_2,0\n");
      break;
    default:
      Serial.println("MQTT send event: Unknown Event");      
  }
  
  if (mqttConnect()) {
    if (mqtt.publish(mqttTopic.c_str(), message)) {
      Serial.print("MQTT send event OK! ");
      Serial.print(message);      
    } else {
      Serial.println("MQTT send event Failed"); 
    }
  }
}