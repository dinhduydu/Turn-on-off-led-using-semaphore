/*
Code không sử dụng cảm biến siêu âm để tránh vật cản
**********************************************************************/
#include <Arduino.h>
#include <Arduino_FreeRTOS.h>
#include <task.h>
#include <limits.h>
#include <SoftwareSerial.h>
#include <queue.h>

const byte RX = 2; 
const byte TX = 3; 
SoftwareSerial mySerial(RX,TX); 

uint32_t ulNotifiedValue, ulNotifiedValue1, ulNotifiedValue2;
int check_point = 0; // biến lưu zone 
int check_point_uart = 0; // biến đưa giá trị zone qua node mcu để đưa lên blynk

/*Khai báo sử dụng L298*/
#define PWM_MOTOR_1 6
#define IN1 9
#define IN2 8
#define IN3 7
#define IN4 10
#define PWM_MOTOR_2 5

#define IR_sensor A2 // use IR sensor to check point

/* Declare a variable that is used to hold the handle of driveTask . */
static TaskHandle_t driveTaskHandle = NULL; 
/* Declare a variable that is used to hold the handle of checkTask . */
static TaskHandle_t checkTaskHandle = NULL; 
/* Declare a variable that is used to hold the handle of communicateTask . */
static TaskHandle_t communicate_senderTaskHandle = NULL;
static TaskHandle_t communicate_receiverTaskHandle = NULL;
/* Declare a variable that is used to hold the handle of imuTask . */
static TaskHandle_t imuTaskHandle = NULL;

QueueHandle_t Queue_zone = NULL;
QueueHandle_t Queue_side = NULL;

void driveTask(void *pvParameters); 
void checkTask(void *pvParameters); 
void communicateTask_receiver(void *pvParameters); 
void communicateTask_sender(void *pvParameters); 
void imuTask(void *pvParameters); 

void setup() 
{  
    //Serial Begin at 9600 Baud
    mySerial.begin(9600); 
    Serial.begin(9600);// Use Serial Monitor
    pinMode(IR_sensor,INPUT); 
    pinMode(PWM_MOTOR_1, OUTPUT); 
    pinMode(IN1, OUTPUT); 
    pinMode(IN2, OUTPUT); 
    pinMode(IN3, OUTPUT); 
    pinMode(IN4, OUTPUT); 
    pinMode(PWM_MOTOR_2, OUTPUT); 

    Queue_zone=xQueueCreate(1, //Queue length
                            sizeof(int)); //Queue item size
    Queue_side=xQueueCreate(1, //Queue length
                            sizeof(int)); //Queue item size
    /*Tạo communicate trước, sau đó chờ node mcu gửi giá trị đến*/
    xTaskCreate( 
        communicateTask_receiver
        , "Communicate_r" 
        , 128 
        , NULL
        , 4 
        , &communicate_receiverTaskHandle); /* This task uses the task handle. */

    if(Queue_side!=NULL)
    {
        xTaskCreate( 
        driveTask
        , "Drive" 
        , 128 
        , NULL
        , 4 
        , &driveTaskHandle); /* This task uses the task handle. */

        xTaskCreate( 
        checkTask
        , "Check" 
        , 128 
        , NULL
        , 4 
        , &checkTaskHandle); /* This task uses the task handle. */

        xTaskCreate( 
        imuTask
        , "imu" 
        , 128 
        , NULL
        , 4 
        , &imuTaskHandle); /* This task uses the task handle. */

        xTaskCreate( 
        communicateTask_sender
        , "Communicate_s" 
        , 128 
        , NULL
        , 4 
        , &communicate_senderTaskHandle); /* This task uses the task handle. */

        /* Start the scheduler so the tasks start executing. */ 
        vTaskStartScheduler();/* Start the created tasks running. */
    } 
}

void loop() 
{ 
// empty
}

/*
 * driveTask runs the state machine:
 * Go FORWARD until Zone 3 is detected 
 * (blocks in FORWARD state until notified from distanceTask that obstacle is 
ahead) 
 * then change to REVERSE state and reverse
 * for about 1 second and enter TURN state. Do a short turn in TURN state and 
return to FORWARD state.
 */
void driveTask(void *pvParameters __attribute__((unused))) 
{ 
    for(;;)
    {
        int check_side = 0;
        while(1)
        {
            // Nhận giá trị từ hàng chờ để đưa vào biến check_side, sau đó kiểm tra check_size bằng 1 hay bằng 2
            if(xQueueReceive(Queue_side,
                        &check_side,
                        portMAX_DELAY)  == pdPASS )
                        {
                            Serial.print(check_point_uart);
                        }
            else break;
        }
        if(check_side = 1)// Start A = 1
        {   
            Serial.println("system turn left");
            system_turn_left(); 
        }   
        else if (check_side = 2)// Start B = 2
        { 
            Serial.println("system turn right");
            system_turn_right(); 
        } 
    }
 
    // 
    xTaskNotifyWaitIndexed( 1, /* "allway waits for notifications on index 1" OR "Wait for 1th notification"*/
                            ULONG_MAX, /* Clear all notification bits on entry. */
                            ULONG_MAX, /* Reset the notification value to 0 on exit. */
                            &ulNotifiedValue,/* Notified value pass out in ulNotifiedValue. */
                            portMAX_DELAY /* Block indefinitely. */
                            );
    _stop(); 
} 


void system_turn_left() 
{ 
    enum States{FORWARD,REVERSE,TURN}; 
    States state=FORWARD; 
    int Start; 
    if((Start == 1)) 
    { 
    _start(); 
    } 
 /* As per most tasks, this task is implemented in an infinite loop. */
    for(;;) // A Task shall never return or exit. 
    { 
        switch(state) // state là 1 giá trị hằng, có thể là biểu thức nhưng kết quả phải là hằng số
        { 
            case FORWARD: 
            forward(); 
            setSpeed(110); 
            xTaskNotifyWaitIndexed( 0, // "allway waits for notifications on index 0" OR "Wait for 0th notification"
                                    ULONG_MAX, /* Clear all notification bits on entry. */
                                    ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                    &ulNotifiedValue1,/* Notified value pass out in ulNotifiedValue. */
                                    portMAX_DELAY /* Block indefinitely. */
                                    ); 
            state=REVERSE; 
            break; 
 
            case REVERSE: 
            reverse(); 
            setSpeed(255); 
            vTaskDelay(pdMS_TO_TICKS((1000+random(-500, 50)))); 
            
            state=TURN; 
            break; 
            case TURN: 
            turnLeft(); 
            setSpeed(110); 
            vTaskDelay(pdMS_TO_TICKS((1000+random(-500, 50)))); 
 
            state=FORWARD; 
            break; 
        } 
    } 
} 

void system_turn_right() 
{ 
    enum States{FORWARD,REVERSE,TURN}; 
    States state=FORWARD; 
    int Start; 
    if((Start == 1)) 
    { 
        _start(); 
    } 
 /* As per most tasks, this task is implemented in an infinite loop. */
    for(;;) // A Task shall never return or exit. 
    { 
        switch(state) // state là 1 giá trị hằng, có thể là biểu thức nhưng kết quả phải là hằng số
        { 
            case FORWARD: 
            forward(); 
            setSpeed(110); 
            xTaskNotifyWaitIndexed( 0, // "allway waits for notifications on index 0" OR "Wait for 0th notification"
                                    ULONG_MAX, /* Clear all notification bits on entry. */
                                    ULONG_MAX, /* Reset the notification value to 0 on exit. */
                                    &ulNotifiedValue2,/* Notified value pass out in ulNotifiedValue. */
                                    portMAX_DELAY /* Block indefinitely. */
                                    ); 
    
            state=REVERSE; 
            break; 
    
            case REVERSE: 
            reverse(); 
            setSpeed(255); 
            vTaskDelay(pdMS_TO_TICKS((1000+random(-500, 50)))); 
            
            state=TURN; 
            break; 
            
            case TURN: 
            turnRight(); 
            setSpeed(110); 
            vTaskDelay(pdMS_TO_TICKS((1000+random(-500, 50)))); 
    
            state=FORWARD; 
            break; 
        } 
    }   
} 

/******************CHECK ZONE*****************************/
void checkTask(void *pvParameters __attribute__((unused))) 
{    
 /* As per most tasks, this task is implemented in an infinite loop. */
    for(;;) 
    { 
        while (1)
        {
            if((analogRead(IR_sensor) >1000)) 
        { 
            check_point++; 
            vTaskDelay(pdMS_TO_TICKS((1000+random(-500, 50)))); 
            xQueueSend(Queue_zone,
                        &check_point,
                        portMAX_DELAY);
            // One tick delay (15ms) in between reads for stability
            vTaskDelay(1);
            vTaskDelay(5000); 
        }
        else break; 
        }
        
        while(1)
        {
            if (check_point = 3) // Zone 3
            { 
            xTaskNotify(driveTaskHandle, // Việc xử lý Task sẽ được thông báo
                        0, // Dữ liệu được gửi với thông báo
                        eNoAction /* Task nhận được thông báo mà giá trị thông báo của nó không được cập nhật
                                    ulValue is not used and xTaskNotify() always returns pdPASS in this case
                                    --> luôn gửi noti thành công*/ 
                        ); 
            } 
            else break;
        }
        
        while(1)
        {
            if(check_point=5) //Zone 5
            { 
                _stop(); 

                // Then move driveTask to Block State
                xTaskNotify(driveTaskHandle, // Việc xử lý Task sẽ được thông báo
                            1, // Dữ liệu được gửi với thông báo
                            eNoAction /* Task nhận được thông báo mà giá trị thông báo của nó không được cập nhật
                                        ulValue is not used and xTaskNotify() always returns pdPASS in this case
                                        --> luôn gửi noti thành công*/ 
                            ); 
            } 
            else break;
        }
        
    } 
} 

void communicateTask_receiver(void *pvParameters __attribute__((unused))) 
{
    for (;;)
    {
        while(1)
        {
            // Lệnh đọc dữ liệu từ chân Rx (từ ESP8266)
            if (mySerial.available()) 
            {// Có dữ liệu gửi đến 
                while(mySerial.available()) 
                { 
                    int RxBuffer = mySerial.read();// Đọc dữ liệu nhận được
                    xQueueSend(Queue_side,
                                &RxBuffer,
                                portMAX_DELAY);
                } 
            } 
            else break;
        }
        
        while (1)
        {
            if(xQueueReceive(Queue_zone,
                        &check_point_uart,
                        portMAX_DELAY)  == pdPASS )
            {
                mySerial.print(check_point_uart);
            }
            else break;
        }                   
    }
}

void communicateTask_sender(void *pvParameters __attribute__((unused))) 
{
    for (;;)
    {
         while (1)
        {
            if(xQueueReceive(Queue_zone,
                        &check_point_uart,
                        portMAX_DELAY)  == pdPASS )
            {
                mySerial.print(check_point_uart);
            }
            else break;
        }       
    }
}

void imuTask (void *pvParameters __attribute__((unused))) 
{

}
/*Chương trình con phục vụ chương trình chính*/
void _start() 
{ 
    digitalWrite(PWM_MOTOR_1,HIGH); 
    digitalWrite(PWM_MOTOR_2,HIGH); 
} 
void _stop() 
{ 
    digitalWrite(PWM_MOTOR_1,LOW); 
    digitalWrite(PWM_MOTOR_2,LOW); 
} 
void forward() 
{ 
    //_start();
    digitalWrite(IN1,HIGH); 
    digitalWrite(IN2,LOW); 
    digitalWrite(IN3,HIGH); 
    digitalWrite(IN4,LOW); 
} 

void reverse() 
{ 
    //_start();
    digitalWrite(IN1,LOW); 
    digitalWrite(IN2,HIGH); 
    digitalWrite(IN3,LOW); 
    digitalWrite(IN4,HIGH); 
} 
void turnRight() 
{ 
    //_start();
    digitalWrite(IN1,HIGH); 
    digitalWrite(IN2,LOW); 
    digitalWrite(IN3,LOW); 
    digitalWrite(IN4,HIGH); 
} 
void turnLeft() 
{ 
    //_start();
    digitalWrite(IN1,LOW); 
    digitalWrite(IN2,HIGH); 
    digitalWrite(IN3,HIGH); 
    digitalWrite(IN4,LOW); 
} 

void setSpeed(int s) 
{ 
    analogWrite(PWM_MOTOR_1,s); 
    analogWrite(PWM_MOTOR_2,s); 
} 



