#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <HTTPClient.h>
#include <SPI.h>
#include <TFT_eSPI.h>
#include <WiFi.h>
#include <Wire.h>

#include "config.h"

#ifndef TFT_DISPOFF
#define TFT_DISPOFF 0x28
#endif

#ifndef TFT_SLPIN
#define TFT_SLPIN   0x10
#endif

#define TFT_MOSI            19
#define TFT_SCLK            18
#define TFT_CS              5
#define TFT_DC              16
#define TFT_RST             23

#define TFT_BL          4  // Display backlight control pin

TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library

// temp/humidity sensor
Adafruit_BME280 bme; // I2C

// ThingSpeak settings
HTTPClient http;

String lastTemperature = "";
String lastHumidity = "";

void setup() {
    Serial.begin(115200);
    Serial.println("Temperature sensor starting...");

    // set TTGO I2C pins (SDA, SCL)
    Wire.begin(21, 22);
    
    // BME280 init
    bool status = bme.begin(0x76);  
    if (!status) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
    } else {
      Serial.println("BME280 initialised");
    }

    // display init
    tft.init();
    tft.setRotation(1);
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(TEXT_SIZE);
    tft.setTextFont(TEXT_FONT);
    
    tft.setCursor(30, 40);
    tft.print("Starting..."); 

    // backlight settings
    if (TFT_BL > 0) { // TFT_BL has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
       pinMode(TFT_BL, OUTPUT); // Set backlight pin to output mode
       digitalWrite(TFT_BL, TFT_BACKLIGHT_ON); // Turn backlight on. TFT_BACKLIGHT_ON has been set in the TFT_eSPI library in the User Setup file TTGO_T_Display.h
    }

    Serial.println("Connecting to WiFi");
    // set up wifi connection for sending data
    connectWiFi();
}


int n = 0;
void loop() {
    float temperature = bme.readTemperature();
    float humidity = bme.readHumidity();
    String temperatureString = String(temperature, DECIMAL_PLACES);
    String humidityString = String(humidity, DECIMAL_PLACES);

    // redraw the result only if it is different pro prevent screen blinking one after every measurement
    if (!temperatureString.equals(lastTemperature) || !humidityString.equals(lastHumidity)) {
      tft.fillScreen(TFT_BLACK);
      tft.setTextFont(TEXT_FONT);
      
      tft.setCursor(OFFSET_X, 40);
      tft.print("Teplota: " + temperatureString + (char)247 + "C");
      
      tft.setCursor(OFFSET_X, 85);
      tft.print("Vlhkost: " + humidityString + " %");
    }
    
    Serial.print("Temp: " + String(temperature));
    Serial.println(", Humidity: " + String(humidity));

    if (n % SAMPLING_RATE == 0) {
      // send the first measurement and each n-th measurement
      sendData(temperature, humidity);      
    }
    
    lastTemperature = temperatureString;
    lastHumidity = humidityString;
    n++;
    
    if (n >= SAMPLING_RATE) {
      // reset counter
      n = 0;
    }

    delay(MEASURE_INTERVAL);
}


void connectWiFi(){
    while (WiFi.status() != WL_CONNECTED){
        WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
        delay(3000);
    }

    // Display a notification that the connection is successful. 
    Serial.println("Connected to WiFi");
    
    tft.fillScreen(TFT_BLACK);
    tft.setCursor(5, 40);
    tft.print("Connected to:"); 
    tft.setCursor(5, 85);
    tft.print(WIFI_SSID);
    delay(1000);
}

void sendData(float temperature, float humidity) {
  if (WiFi.status() != WL_CONNECTED) { 
    connectWiFi();
  }
   
  // Create data string to send to ThingSpeak.
  String data = "api_key="+String(API_KEY)+"&field1=" + String(temperature) + "&field2=" + String(humidity); 
  String url = "http://" + String(SERVER) + "/update?" + data;
  http.begin(url);
  int httpCode = http.GET();

  Serial.println(url);
  
  if(httpCode > 0) {
    // HTTP header has been send and Server response header has been handled
    Serial.println("[HTTP] GET... code: " + String(httpCode));

    // file found at server
    if(httpCode == HTTP_CODE_OK) {
        String payload = http.getString();
        Serial.println(payload);
    }
  } else {
    Serial.println("[HTTP] GET... failed, error: " + String(http.errorToString(httpCode)));
  }
  
  http.end();
}
