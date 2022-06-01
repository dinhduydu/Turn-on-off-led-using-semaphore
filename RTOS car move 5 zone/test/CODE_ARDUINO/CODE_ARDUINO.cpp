#include<Arduino.h>
#include <SoftwareSerial.h>
 
SoftwareSerial to_nodemcu(2,3);//tx-2, rx-3
 
int IR_sensor = A0; // cảm biến hồng ngoại

int count = 0;

int sdata = 0;  // send data

String cdata; 
 
void setup()
{
  Serial.begin(9600); // Serial Monitor
  to_nodemcu.begin(9600); // Software Serial
 
  pinMode(IR_sensor, INPUT);
  pinMode(4, OUTPUT); // bat led
  digitalWrite(4, LOW); 
}
 
void loop()
{
    sdata = analogRead(IR_sensor);
     if (sdata1 > 1000 ) 
     {
       count ++;
       digitalWrite(4,HIGH);
    }
    else 
    {
      count = count;
      digitalWrite(4,LOW);
    }
    cdata = cdata + count;
   
    Serial.println(cdata); 
    to_nodemcu.println(cdata);
    delay(2000); // 100 milli seconds
    cdata = ""; 
}
