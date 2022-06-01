#include<Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <MPU6050_tockn.h>
#include <Wire.h>

MPU6050 mpu6050(Wire);

void Di_Thang( void *pvParameters );
void TaskDemZone( void *pvParameters );

#define ENA 13
#define IN1 12
#define IN2 11
#define IN3 10
#define IN4 9
#define ENB 8

float z =0 ;


void setup() {

Serial.begin(9600);
  pinMode(A0, INPUT);
  pinMode(ENA, OUTPUT);
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);
  pinMode(ENB, OUTPUT);
  
    mpu6050.calcGyroOffsets(true);
    digitalWrite(ENA, HIGH);
    digitalWrite(IN1, LOW);
    analogWrite(IN2, 70);
    digitalWrite(ENB, HIGH);
    analogWrite(IN3, 70);
    digitalWrite(IN4, LOW);

  xTaskCreate(
  Di_Thang
  ,"DiThang"
  ,128
  ,NULL
  ,1
  ,NULL);

 xTaskCreate(
  TaskDemZone
  ,"DemZone"
  ,128
  ,NULL
  ,2
  ,NULL);
  
  //delay(500);
}

void Di_Thang( void *pvParameters )
{
    (void) pvParameters;
  Wire.begin();
  mpu6050.begin();



  for (;;)
  {
    mpu6050.update();
    z = mpu6050.getAngleZ();
    Serial.print(z);
    if (z > 5)
    {
      analogWrite(IN2, 200);
    }
    if (z > -5 and z < 5)
    {
      analogWrite(IN3, 70);
      analogWrite(IN3, 70);
    }
    if (z < -5)
    {
      analogWrite(IN3, 200);
    }
  }
}

int Zone=0;
void TaskDemZone( void *pvParameters )
{
  (void) pvParameters;

  while(1)
  { 
    
    if(analogRead(A0)>1000)
    { 
        Zone++;
    }
    if(Zone == 3)
    {
        analogWrite(IN3, 0);
        analogWrite(IN3, 0);
    }      
  }
}

void loop() {
  
      
    }
