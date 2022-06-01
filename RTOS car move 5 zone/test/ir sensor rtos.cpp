// Must define Arduino.h when we use Platform IO
#include<Arduino.h>
// Include Arduino FreeRTOS library
#include <Arduino_FreeRTOS.h>
// Include queue support
#include <queue.h>

// Define a count and read_count value
int send_count = 0;
int read_count;
//Function Declaration
void TaskBlink(void *pvParameters);
void Generator(void *pvParameters);
void Display(void *pvParameters);
//Declaring a global variable of type QueueHandle_t 
QueueHandle_t Queue;

void setup() {

Queue=xQueueCreate(10, //Queue length
                        sizeof(int)); //Queue item size
if(Queue!=NULL){

  // Create task that consumes the queue if it was created.
  xTaskCreate(Display,// Task function
              "Display",// Task name
              128,// Stack size 
              NULL,
              1,// Priority
              NULL);

  // Create task that publish data in the queue if it was created.
  xTaskCreate(Generator, // Task function
              "Generator",// Task name
              128,// Stack size 
              NULL,
              1,// Priority
              NULL);
              
   // Create other task that publish data in the queue if it was created.
 
   xTaskCreate(TaskBlink,// Task function
              "Blink", // Task name
              128,// Stack size 
              NULL,
              1,// Priority
              NULL);

}
}

void loop() {}

void Generator(void *pvParameters)
{

  pinMode(11, INPUT_PULLUP);
  for (;;)
  {
    if (analogRead(A0)>500)
    {
        send_count++;
        vTaskDelay(100);
    }
    
    
    if (send_count == 5)
    {
      send_count =0;
    }
    xQueueSend(Queue,&send_count,portMAX_DELAY);
    
  }
}

/**
 * Display task.
 * Prints the received items from the queue to the serial monitor.
 */
void Display(void *pvParameters){


  // Init Arduino serial
  Serial.begin(9600);

  // Wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  while (!Serial) {
    vTaskDelay(1);
  }
  
  for (;;){
    if(xQueueReceive(Queue,&read_count,portMAX_DELAY) == pdPASS ){
      Serial.print("Giá trị:");
      Serial.println(read_count);
    }
  }
}

/* 
 * Blink task. 
 * See Blink_AnalogRead example. 
 */
void TaskBlink(void *pvParameters){

  pinMode(12,OUTPUT);
  digitalWrite(12,LOW);
  for (;;){
    digitalWrite(12,HIGH);
    vTaskDelay(250);
    digitalWrite(12,LOW);
    vTaskDelay(250);
  }
}
