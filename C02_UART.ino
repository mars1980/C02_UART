//running Arduino 1.0 IDE
//MH-Z14 CO2 Module from Zhengzhou Winsen Electronics Technology Co., Ltd
//Seeeduino Stalker v2.3

//Michael Doherty - Bitponics
//Crys Moore
//2.7.12
//update 3.38.12

// sketch to get sensor readings via UART and print them to Adafruit 7 segment backpack.
//Sensor readings are time stamped and logged to the SD card. 

/*************************************************** 
 * This is a library for our I2C LED Backpacks
 * 
 * Designed specifically to work with the Adafruit LED 7-Segment backpacks 
 * ----> http://www.adafruit.com/products/881
 * ----> http://www.adafruit.com/products/880
 * ----> http://www.adafruit.com/products/879
 * ----> http://www.adafruit.com/products/878
 * 
 * These displays use I2C to communicate, 2 pins are required to 
 * interface. There are multiple selectable I2C addresses. For backpacks
 * with 2 Address Select pins: 0x70, 0x71, 0x72 or 0x73. For backpacks
 * with 3 Address Select pins: 0x70 thru 0x77
 * 
 * Adafruit invests time and resources providing this open source code, 
 * please support Adafruit and open-source hardware by purchasing 
 * products from Adafruit!
 * 
 * Written by Limor Fried/Ladyada for Adafruit Industries.  
 * BSD license, all text above must be included in any redistribution
 ****************************************************/


#include <SoftwareSerial.h>
#include <Wire.h>
#include "RTClib.h"
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"
#include <SD.h>


//using softserial to send/recieve data over analog pins 0 & 1
SoftwareSerial mySerial(A0, A1); // RX, TX respectively
Adafruit_7segment matrix = Adafruit_7segment();
RTC_DS1307 RTC;
File dataFile = SD.open("datalog1.csv", FILE_WRITE);



byte cmd[9] = {
  0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
char response[9]; 
const int chipSelect = 10;
unsigned long time;
int calibrationTime = 10; //warm up time for C02 sensor (3min = 180sec)
float lipoCalibration=.685; //was 1.1 //.685
//reading at 3.5 on voltmeter; 3.8 from arduino
float voltage;
int BatteryValue;
float threshold = 4.0; //battery threshold




void setup() 
{
  Serial.begin(9600);
  analogReference(INTERNAL); 
  pinMode(A2,INPUT);
  mySerial.begin(9600);
  matrix.begin(0x70);
  Wire.begin();
  RTC.begin();
  matrix.setBrightness(5); //0-15 (11 drawing 50mA of current from the NCP1402-5V Step-Up Breakout)

  //give the sensor some time to calibrate and countdown to the 7seg
  Serial.print("calibrating sensor (3min) ");
  for(int i = 0; i < calibrationTime; i++){
    Serial.print(".");
    if((millis()/1000) < calibrationTime){
      matrix.println(calibrationTime -(millis()/1000));
      matrix.writeDisplay();
    }
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  delay(50);

  if (! RTC.isrunning())
  {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
  }
  else
  {
    RTC.adjust(DateTime(__DATE__, __TIME__)); 
  }

  // Serial.print("Initializing SD card...");
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    return;
  }
  //Serial.println("card initialized.");
  //readSd(); //fuction to read the contents of the SD
}
////////////////////////////////////////////////////////////END SETUP


void loop() 
{
  DateTime now = RTC.now();
  time = millis(); //1s = 1000

  //PING CO2, READ RESPONSE, GET READINGS AND CONVERT TO PPM
  mySerial.write(cmd,9);
  mySerial.readBytes(response, 9);
  int responseHigh = (int) response[2];
  int responseLow = (int) response[3];
  int ppm = (256*responseHigh)+responseLow;

  //read the battery and convert to voltage
  BatteryValue = analogRead(A2);
  voltage = BatteryValue * (lipoCalibration / 1024)* (10+2)/2;  //Voltage divider 
  //
  Serial.print("Battery Voltage -> ");
  Serial.print(voltage);
  Serial.print("V   ");
  Serial.println();





  if (voltage < threshold)
  {  //display the battery low msg 'Batt'
    matrix.print(0xBAFF, HEX);
    matrix.writeDisplay();
    Serial.println('do not record data');
  }
  else if (voltage > threshold)
  {
    // create a string for ppm, cast the data, open up the file
    String ppmString = "";
    ppmString = String(ppm);

    //PRINT c02 OUT ON THE SEVSEG BACKPACK & SERIAL
    //Serial.println(ppm,DEC);
    matrix.println(ppm);  
    matrix.writeDisplay();
    
    char timeStamp[20]; 
    sprintf(timeStamp,"%04i-%02i-%02iT%02i:%02i:%02i", now.year(), now.month(), now.day(), now.hour(), now.minute(), now.second()); 
    
    ///********************************read every .5 sec and log every 5 sec***********************************
    dataFile = SD.open("datalog1.csv", FILE_WRITE);
    if (dataFile) 
    {
      dataFile.print(ppmString);
      dataFile.print(',');
      dataFile.print(timeStamp);
      dataFile.print(','); 
      dataFile.print(voltage);
      dataFile.println();
      dataFile.close();
      // Serial.println(ppmString);
    }  
    else 
    {
      Serial.println("error opening datalog1.csv");
    }
    Serial.println("battery is all GOOD");
  }  
}
//******************add in the sd formmatter stuff************************

///function to read back the contents of the file via Serial
void readSd()
{

  File dataFile = SD.open("datalog1.csv");

  // if the file is available, write to it:
  if (dataFile) {
    while (dataFile.available()) {
      Serial.write(dataFile.read());
    }
    dataFile.close();
  }  
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog1.csv");
  }
}













