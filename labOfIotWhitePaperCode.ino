#include <WiFiNINA.h>
#include <DHT.h>
#include <ThingSpeak.h>
#include <SPI.h>

/*------------------------LCD-------------------------------*/
#include <Wire.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_I2Cexp.h>
hd44780_I2Cexp lcd;
const int LCD_COLS = 20;
const int LCD_ROWS = 4;
/*----------------------------------------------------------*/

/*------------------------DHT SENSOR------------------------*/
#define DHTPIN 2         // DHT data pin connected to Arduino pin 2
#define DHTTYPE DHT22     // DHT 22 (or AM2302)
DHT dht(DHTPIN, DHTTYPE); // Initialize the DHT sensor
/*----------------------------------------------------------*/

/*-----------------------ThingSpeak-------------------------*/
char thingspeakAddress[] = "api.thingspeak.com";
unsigned long channelID = 890503;
unsigned long commandsChannelID = 908059;
String apiKey = "BBOMQTFH9H3JK8XM"; //API write key from ThingSpeak data channel
const char* readAPIKey = "BQWVR26DB8S7F8FR"; //API read key from ThingSpeak data channel
const char* writeAPIKeyCommands = "UWP1VPP37FMUAOUE"; //API write key from ThingSpeak commands channel
const char* ssid = "RL"; //SSID of your wifi
const char* password = "zxcvbnm1605"; //password of your wifi
int duration=5;//delay between each data measure and uploading
/*----------------------------------------------------------*/

/*----------------------------------------------------------*/
unsigned int tempField = 1; //ThingSpeak Field ID
unsigned int humField = 2; //ThingSpeak Field ID
long lastReadTime = 0;
float responseValue = 0;
/*----------------------------------------------------------*/

int status = WL_IDLE_STATUS;
WiFiClient client; //Start client

void setup()
{
  Serial.begin(115200);

  dht.begin();

  lcd.begin(LCD_COLS, LCD_ROWS);

  ThingSpeak.begin(client);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv != "1.0.0") {
    Serial.println("Please upgrade the firmware");
  }

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, password); // Connect to WPA/WPA2 network. Change this line if using open or WEP network
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected");
  }
}

void loop()
{
  char t_buffer[10];
  char h_buffer[10];
  int statusCode = 0;
  //Read temperature and humidity values from DHT sensor:
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  String t=dtostrf(temp,0,5,t_buffer);
  String h=dtostrf(hum,0,5,h_buffer);
  //Update values on ThingSpeak platform
  updateThingSpeak("field1="+t+"&field2="+h);
  delay(1000);

  if (millis() - lastReadTime > 15000) {
    float value = ThingSpeak.readFloatField(commandsChannelID, 1); //read command to the thermostat from ThingSpeak
    Serial.println("New ThingSpeak command = " + String(value));
    if (value == 0) {
      lcd.clear();
      lcd.print("OFF");
    }
    else {
      lcd.clear();
      lcd.print("Thermostat is working...");
    }
    responseValue = value;
    lastReadTime = millis();
  }
}

void updateThingSpeak(String tsData)
{
    if (client.connect(thingspeakAddress, 80)){
    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(tsData.length());
    client.print("\n\n");
    client.print(tsData);
    if (client.connected())
    {
      Serial.println();
    }
    else {
      Serial.println("Connection to ThingSpeak failed.");
      Serial.println();
    }
  }
}
