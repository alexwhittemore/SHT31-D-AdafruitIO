// Built on heltec ESP32 kit samples

// Based on Adafruit IO Publish Example
//
// Adafruit invests time and resources providing this open source code.
// Please support Adafruit and open source hardware by purchasing
// products from Adafruit!
//
// Written by Todd Treece for Adafruit Industries
// Copyright (c) 2016 Adafruit Industries
// Licensed under the MIT license.
//
// All text above must be included in any redistribution.

/************************** Configuration ***********************************/

// edit the config.h tab and enter your Adafruit IO credentials
// and any additional configuration needed for WiFi, cellular,
// or ethernet clients.
#include "config.h"
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "Adafruit_SHT31.h"
#include <Math.h>


// set up the feeds
AdafruitIO_Feed *esp32temp = io.feed("esp32temp");
AdafruitIO_Feed *esp32humidity  = io.feed("esp32humidity");
AdafruitIO_Feed *esp32dew = io.feed("esp32dew");

const int led = 13;

//OLED pins to ESP32 GPIOs via this connection:
//OLED_SDA -- GPIO4
//OLED_SCL -- GPIO15
//OLED_RST -- GPIO16

#define SDA    4
#define SCL   15
#define RST   16 //RST must be set by software

#define PROG 0

//#define V2     1
//
//#ifdef V2 //WIFI Kit series V1 not support Vext control
//  #define Vext  21
//#endif

SSD1306  display(0x3c, SDA, SCL, RST);

Adafruit_SHT31 sht31 = Adafruit_SHT31();

float calcDewpoint(float humi, float temp) {
  float k;
  k = log(humi/100) + (17.62 * temp) / (243.12 + temp);
  return 243.12 * k / (17.62 - k);
}

float t_c;
float t_f;
float h;
float dp_f;
String tempStr;
String humidityStr;
String dewpointStr;
String tupleString;

void get_vals(){
  t_c = sht31.readTemperature();
  t_f = t_c*1.8+32;
  h = sht31.readHumidity();
  dp_f = calcDewpoint(h, t_c)*1.8+32;
  tempStr = String(t_f, 1) + " F";
  humidityStr = String(h, 1) + " %";
  dewpointStr = String(dp_f, 1) + " F";
  tupleString = String("(") + String(t_f, 1) + ", " + String(h, 1) + ", " + String(dp_f, 1) + ", )";
}

int oldMillis;
int newMillis;

bool time_to_update(){
  bool gt_1s = false;
  newMillis = millis();
  if(newMillis-oldMillis > 1000){
    gt_1s = true;
    oldMillis = newMillis;
  }
  return gt_1s;
}

int oldMillisAdafruit;
int newMillisAdafruit;

bool time_to_update_adafruit(){
  bool gt_10s = false;
  newMillisAdafruit = millis();
  if(newMillisAdafruit-oldMillisAdafruit > 10000){
    gt_10s = true;
    oldMillisAdafruit = newMillisAdafruit;
  }
  return gt_10s;
}

void setup() {
  pinMode(PROG,INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  pinMode(RST,OUTPUT);
//  pinMode(Vext, OUTPUT);
//  digitalWrite(Vext, HIGH);    // OLED USE Vext as power supply, must turn ON Vext before OLED init
  delay(50);

  // Initialising the UI will init the display too.
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  Serial.begin(115200);
  // wait for serial monitor to open
  while(! Serial);
  
  Serial.print("Connecting to Adafruit IO");

  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.drawString(0, 0, "Connecting to Adafruit");
  display.display();

  // connect to io.adafruit.com
  io.connect();

  // wait for a connection
  bool toggle = true;
  while(io.status() < AIO_CONNECTED) {
    Serial.print(".");
    display.clear();
    display.setTextAlignment(TEXT_ALIGN_LEFT);
    if (toggle) {
      display.drawString(0, 0, "Connecting to Adafruit...");
    } else {
      display.drawString(0, 0, "Connecting to Adafruit");
    }
    display.display();
    toggle = !toggle;
    delay(500);
  }

  // we are connected
  Serial.println();
  Serial.println(io.statusText());

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
}

#define VALUE_MARGIN 70

void drawVals(String temp, String humidity, String dewpoint) {
    // Font Demo1
    // create more fonts at http://oleddisplay.squix.ch/
    display.clear();
    display.setFont(ArialMT_Plain_16);

    display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 0, "Temp:");
    //display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(VALUE_MARGIN, 0, temp);

    //display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 18, "Humidity:");
    //display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(VALUE_MARGIN, 18, humidity);

    //display.setTextAlignment(TEXT_ALIGN_LEFT);
    display.drawString(0, 36, "Dew pt.:");
    //display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.drawString(VALUE_MARGIN, 36, dewpoint);

    display.display();
}

int display_en = true;

void loop() {
  io.run();
  if(time_to_update()){
    get_vals();
  
    Serial.println(tupleString);
    if(digitalRead(PROG) == LOW){
      display_en = !display_en;
    }
    if(display_en){
      drawVals(tempStr, humidityStr, dewpointStr);
    }else{
      display.clear();
      display.display();
    }
  }
  if(time_to_update_adafruit()) {
    Serial.print("Updating Adafruit with: ");
    Serial.print( round(t_f*10)/10.0 );
    Serial.print( ", ");
    Serial.print( round(h*10)/10.0 );
    Serial.print( ", ");
    Serial.print( round(dp_f*10)/10.0 );
    Serial.print( ", ");
    esp32temp->save( round(t_f*10)/10.0 );
    esp32humidity->save( round(h*10)/10.0 );
    esp32dew->save( round(dp_f*10)/10.0 );
  }

//  if (! isnan(t_c)) {  // check if 'is not a number'
//    Serial.print("Temp *F = "); Serial.println(t_f);
//  } else { 
//    Serial.println("Failed to read temperature");
//  }
//  
//  if (! isnan(h)) {  // check if 'is not a number'
//    Serial.print("Hum. % = "); Serial.println(h);
//  } else { 
//    Serial.println("Failed to read humidity");
//  }
//
//  if (! isnan(dp_f)) {  // check if 'is not a number'
//    Serial.print("Dewpoint *F = "); Serial.println(dp_f);
//  } else { 
//    Serial.println("Can't calculate dewpoint" );
//  }
//  
//  Serial.println();
}
