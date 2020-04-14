// Built on heltec ESP32 kit samples

#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include "SSD1306.h" // alias for `#include "SSD1306Wire.h"`
#include "Adafruit_SHT31.h"

// Web server includes
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include "credentials.h"

WebServer server(80);
const int led = 13;

//OLED pins to ESP32 GPIOs via this connecthin:
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

void handleRoot() {
  digitalWrite(led, 1);
  server.send(200, "text/plain", tupleString);
  digitalWrite(led, 0);
}

void handleNotFound() {
  digitalWrite(led, 1);
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  digitalWrite(led, 0);
}

int oldMillis;
int newMillis;

bool time_to_update(){
  bool gt_1s = false;
  newMillis = millis();
  if(newMillis-oldMillis > 1000){
    gt_1s = true;
  }
  oldMillis = newMillis;
  return gt_1s;
}

void setup() {
  pinMode(PROG,INPUT);
  pinMode(led, OUTPUT);
  digitalWrite(led, 0);
  pinMode(RST,OUTPUT);
//  pinMode(Vext, OUTPUT);
//  digitalWrite(Vext, HIGH);    // OLED USE Vext as power supply, must turn ON Vext before OLED init
  delay(50); 

  Serial.begin(115200);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);


  // Initialising the UI will init the display too.
  display.init();

  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);

  Serial.println("SHT31 test");
  if (! sht31.begin(0x44)) {   // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
    while (1) delay(1);
  }
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("esp32")) {
    Serial.println("MDNS responder started");
  }

  server.on("/", handleRoot);

  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });

  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("HTTP server started");
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
  server.handleClient();
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
