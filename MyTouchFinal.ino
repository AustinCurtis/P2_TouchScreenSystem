//This example implements a simple sliding On/Off button. The example
// demonstrates drawing and touch operations.
//
//Thanks to Adafruit forums member Asteroid for the original sketch!
//
#include <Adafruit_GFX.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_ILI9341.h>
#include <Adafruit_FT6206.h>
#include <OneWire.h>

// The FT6206 uses hardware I2C (SCL/SDA)
Adafruit_FT6206 ts = Adafruit_FT6206();

// This is calibration data for the raw touch data to the screen coordinates
#define TS_MINX 150
#define TS_MINY 130
#define TS_MAXX 3800
#define TS_MAXY 4000
//
#define TFT_CS 10
#define TFT_DC 9
Adafruit_ILI9341 tft = Adafruit_ILI9341(TFT_CS, TFT_DC);

boolean RecordOn = false;
boolean isHome = true;
boolean isPitch = false;
boolean isRoll = false;
boolean isTemp = false;

int DS18S20_Pin = 2;
OneWire ds(DS18S20_Pin);


int xRead;
int xAng;

unsigned long previousMillis = 0;
long interval = 2000;

void homePage(){
  Serial.println("home");
  tft.fillScreen(ILI9341_BLACK);
  tft.drawRoundRect(25, 25, 122.5, 70, 5, ILI9341_WHITE);
  tft.fillRoundRect(25, 25, 122.5, 70, 5, ILI9341_GREEN);
  tft.setCursor(45, 45);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(3);
  tft.print("Pitch");
  tft.drawRoundRect(172.5, 25, 122.5, 70, 5, ILI9341_WHITE);
  tft.fillRoundRect(172.5, 25, 122.5, 70, 5, ILI9341_BLUE);
  tft.setCursor(200, 45);
  tft.print("Roll");
  tft.drawRoundRect(25, 145, 270, 70, 5, ILI9341_WHITE);
  tft.fillRoundRect(25, 145, 270, 70, 5, ILI9341_RED);
  tft.setCursor(65, 165);
  tft.print("Temperature");
}


void disPitch(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(80, 80);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(12);
  xRead = analogRead(A0);
  xAng = map(xRead, 250, 420, 90, -90);
  //278
  tft.print(xAng);
  tft.drawTriangle(0, 20, 40, 0, 40, 40, ILI9341_WHITE);
  tft.fillTriangle(0, 20, 40, 0, 40, 40, ILI9341_WHITE);
}

void disRoll(){
  tft.fillScreen(ILI9341_BLACK);
  tft.setCursor(80, 80);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(12);
  xRead = analogRead(A1);
  xAng = map(xRead, 239, 420, -90, 90);
  //272
  tft.print(xAng);
  tft.drawTriangle(0, 20, 40, 0, 40, 40, ILI9341_WHITE);
  tft.fillTriangle(0, 20, 40, 0, 40, 40, ILI9341_WHITE);
}

float getTemp(){
  //returns the temperature from one DS18S20 in DEG Celsius

  byte data[12];
  byte addr[8];

  if ( !ds.search(addr)) {
      //no more sensors on chain, reset search
      ds.reset_search();
      return -1000;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      Serial.println("CRC is not valid!");
      return -1000;
  }

  if ( addr[0] != 0x10 && addr[0] != 0x28) {
      Serial.print("Device is not recognized");
      return -1000;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44,1); // start conversion, with parasite power on at the end

  byte present = ds.reset();
  ds.select(addr);    
  ds.write(0xBE); // Read Scratchpad

  for (int i = 0; i < 9; i++) { // we need 9 bytes
    data[i] = ds.read();
  }

  ds.reset_search();

  byte MSB = data[1];
  byte LSB = data[0];

  float tempRead = ((MSB << 8) | LSB); //using two's compliment
  float TemperatureSum = tempRead / 16;

  return (TemperatureSum * 18 + 5)/10 + 32;
}

//return (TemperatureSum * 18 + 5)/10 + 32;


void disTemp(){
  tft.fillScreen(ILI9341_BLACK);
  float temperature = getTemp();
  //Serial.println(String(temperature));
  tft.setCursor(5, 80);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(10);
  tft.print(temperature);
  tft.drawTriangle(0, 20, 40, 0, 40, 40, ILI9341_WHITE);
  tft.fillTriangle(0, 20, 40, 0, 40, 40, ILI9341_WHITE);
}



void setup(void)
{
  Serial.println("Start");
  Serial.begin(9600);
  tft.begin();
  if (!ts.begin(40)) { 
    Serial.println("Unable to start touchscreen.");
  } 
  else { 
    Serial.println("Touchscreen started."); 
  }

  tft.fillScreen(ILI9341_BLACK);
  // origin = left,top landscape (USB left upper)
  tft.setRotation(3); 
  homePage();
  pinMode(6, OUTPUT);
}

void loop()
{
  //float temperature = getTemp();
  //Serial.println(String(temperature));
  unsigned long currentMillis = millis();
  // See if there's any  touch data for us

  if (ts.touched()){
    TS_Point p = ts.getPoint();
    //p.x = map(p.x, 0, 240, 240, 0);
    //p.y = map(p.y, 0, 320, 320, 0);
    int y = tft.height() - p.x;
    int x = p.y;

    if(isHome){
      if((x>25)&&(x<147.5)&&(y>25)&&(y<95)){
        tone(6, 1500, 200);
        disPitch();
        isPitch = true;
        isHome = false;
      }
      else if((x>172.5)&&(x<295)&&(y>25)&&(y<95)){
        tone(6, 1500, 200);
        disRoll();
        isHome = false;
        isRoll = true;
      }
      else if((x>25)&&(x<295)&&(y>145)&&(y<215)){
        tone(6, 1500, 200);
        //25 95
        disTemp();
        isHome = false;
        isTemp = true;
      }
    } else {
      if((x<40)&&(y<40)){
        tone(6, 1500, 200);
        homePage();
        isHome = true;
        isTemp = false;
        isRoll = false;
        isPitch = false;
      }
    }

    Serial.println(y);
  }

  if(!isHome){
    if(isPitch){
      if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        disPitch();
      }
    } else if(isRoll){
      if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        disRoll();
      }
    }else if(isTemp){
      if(currentMillis - previousMillis > interval) {
        previousMillis = currentMillis;
        disTemp();
      }
    }
  }



}
