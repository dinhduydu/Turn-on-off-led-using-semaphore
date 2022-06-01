
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <semphr.h>
// Using Software Serial to avoid debug in RX TX pin of Arduino or Node MCU
#include <SoftwareSerial.h>
// Define pin RX TX by Software Serial
SoftwareSerial mySerial (6,7);




int sensorValue, abc;
int count =0;

void TaskSendNotify( void *pvParameters );
void TaskAnalogRead( void *pvParameters );
void TaskBlink( void *pvParameters );
/* Create Semaphore Handle*/
SemaphoreHandle_t xSerialSemaphore;

// the setup function runs once when you press reset or power the board; plz remember this
void setup() 
{
    pinMode(11,OUTPUT);
    digitalWrite(11,LOW);
    pinMode(12,OUTPUT);
    digitalWrite(12,LOW);
  // initialize serial communication at 9600 bits per second:
  // both of MCU need to config this baud bcuz this is UART protocol
    mySerial.begin(9600);
//  while (!mySerial) {
//    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
//  }
    Serial.begin(9600);
    
  // IR sensor is connected to Arduino Pin A0  
  pinMode(A0, INPUT);
  
  if ( xSerialSemaphore == NULL )  // Check to confirm that the Serial Semaphore has not already been created.
  {
     Serial.println("Mutex can not be created");
     xSerialSemaphore = xSemaphoreCreateMutex();
     if ( ( xSerialSemaphore ) != NULL )
      xSemaphoreGive( ( xSerialSemaphore ) );  // Make the Serial Port available for use, by "Giving" the Semaphore.
  }

  // Now set up two Tasks to run independently.
  xTaskCreate(
    TaskSendNotify
    ,  "SendNotify"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskAnalogRead
    ,  "AnalogRead"
    ,  128  // Stack size
    ,  NULL
    ,  3  // Priority
    ,  NULL );

  xTaskCreate(
    TaskBlink
    ,  "TaskBlink"  // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  1  // Priority
    ,  NULL );  
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskAnalogRead( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  for (;;)
  {
     sensorValue = analogRead(A0);
     //xSemaphoreTake( xSerialSemaphore, portMAX_DELAY );
  
    /* We are able to obtain the semaphore and can now access the share resource*/
    if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {
      abc = sensorValue;
      Serial.println("abc has been updated");
      /*We can finished accessing the shared resource. Release the Semaphore*/
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others.
        
      vTaskDelay(1);  // one tick delay (15ms) in between reads for stability
    }
  }
}

void TaskSendNotify( void *pvParameters __attribute__((unused)) )  // This is a Task.
{
  for (;;) // A Task shall never return or exit.
  {
     if ( xSemaphoreTake( xSerialSemaphore, ( TickType_t ) 5 ) == pdTRUE )
    {   
      
        if(abc> 500)
        {
          Serial.print("abc = analogRead(A0) is:");
          Serial.println(abc);
          count ++;   
          if (count > 5)
          {
            count =0;
          }
          Serial.println("Led12 Turn OFF");
          digitalWrite(12,LOW);
        }
        //xSemaphoreTake( xSerialSemaphore, portMAX_DELAY );
        
        if (abc< 300)
        {
          Serial.println("Led 12 Turn ON");
          digitalWrite(12,HIGH);
        }
        mySerial.print(count);
        /*Send value named count to Node MCU */
        Serial.print("sensor Value is: ");Serial.println(abc);
        Serial.print("count is: ");Serial.println(count);
        vTaskDelay(200);
      xSemaphoreGive( xSerialSemaphore ); // Now free or "Give" the Serial Port for others. 
    }
  }
}

void TaskBlink(void *pvParameters)
{
  pinMode(11,OUTPUT);
  digitalWrite(11,LOW);
  for (;;)
  {
    digitalWrite(11,HIGH);
    delay(200);
    digitalWrite(11,LOW);
    delay(200);
  }
}