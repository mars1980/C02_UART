//MH-Z14 CO2 Module from Zhengzhou Winsen Electronics Technology Co., Ltd
//http://www.adafruit.com/products/878
//simple sketch to get sensor readings via UART and print them to Adafruit 7 segment backpack
//Michael Doherty - Bitponics
//Crys Moore
//2.7.12
//update 2.14.12


#include <SoftwareSerial.h>
#include <Wire.h>
#include "Adafruit_LEDBackpack.h"
#include "Adafruit_GFX.h"

//using softserial to send/recieve data over analog pins 0 & 1
SoftwareSerial mySerial(A0, A1); // RX, TX
Adafruit_7segment matrix = Adafruit_7segment();

byte cmd[9] = {0xFF,0x01,0x86,0x00,0x00,0x00,0x00,0x00,0x79}; 
char response[9]; 



void setup() 
{
  Serial.begin(9600);
  mySerial.begin(9600);
  matrix.begin(0x70);
  matrix.setBrightness(10); //0-15
}

void loop() {
  //、Read concentration and temperature value of the sensor
  mySerial.write(cmd,9);
  //Format of data returned by subsidiary detector
  mySerial.readBytes(response, 9);

  int responseHigh = (int) response[2];
  int responseLow = (int) response[3];
  int ppm = (256*responseHigh)+responseLow;
  //Serial.println(ppm,DEC);


  //delay(1000);  
}

