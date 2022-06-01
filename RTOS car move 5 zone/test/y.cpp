#include<Arduino.h>
#include <Arduino_FreeRTOS.h>

#include<SoftwareSerial.h>
SoftwareSerial ArduinoUno (5,6);

void setup() {
  //Serial Begin at 9600 Baud
  Serial.begin(9600);
  ArduinoUno.begin(9600);
}

void loop() 
{
  ArduinoUno.print("hello bạn nhỏ");
  delay(500);

  String msg = ArduinoUno.readString();
  Serial.println(msg);
  
} 
