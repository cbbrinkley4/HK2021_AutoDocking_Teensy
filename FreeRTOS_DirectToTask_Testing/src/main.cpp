#include <Arduino.h>

#include <FreeRTOS_ARM.h>
#include <basic_io_arm.h>

//#include <timers.h> //(timers.h from FreeRTOS doesnt work with Teensy!!!)
//#include <TeensyTimerTool.h> //alternative to FreeRTOS timers https://github.com/luni64/TeensyTimerTool

static void prvTask1(void *pvParameters);
static void prvTask2(void *pvParameters);

static TaskHandle_t xTask1 = NULL, xTask2 = NULL;

const int ledPin = 13; //LED is pin 13 on the teensy 3.6

/* prvTask1() uses the 'indexed' version of the API. */
static void prvTask1( void *pvParameters )
{
    for( ;; )
    {
        Serial.println("Start Task 1 Loop -> xTaskNotifyGive to Task 2...");
        delay(200);
        
        /* Send notification to prvTask2(), bringing it out of the 
        Blocked state. */
        xTaskNotifyGive( xTask2);
        Serial.println("Task 1 TaskNotifyGiven -> TaskNotifyTake Task 1...");
        delay(200);
        
        /* Block to wait for prvTask2() to notify this task. */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY );
        Serial.println("End of Task 1 loop, TaskNotifyTaken Task:");
        delay(200);
    }
}
/*-----------------------------------------------------------*/

/* prvTask2() uses the original version of the API (without the 
'Indexed'). */
static void prvTask2( void *pvParameters )
{
    for( ;; )
    {
        Serial.println("Start Task 2 Loop - TaskNotifyTake...");
        delay(200);
        
        /* Block to wait for prvTask1() to notify this task. */
        ulTaskNotifyTake( pdTRUE, portMAX_DELAY);
        Serial.println("Do stuff in task 2 -> TaskNotifyGive...");
        delay(200);
        
        /* Send a notification to prvTask1(), bringing it out of the 
        Blocked state. */
        xTaskNotifyGive(xTask1);
        Serial.println("End of task 2 loop, TaskNotifyGiven to Task 1");
        delay(200);
    }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT); //just to know its turned on 
  digitalWrite(ledPin,HIGH);
  while(!Serial){} //loop to wait until serial monitor is open

  
  xTaskCreate( prvTask1, "Task1", 200, NULL, tskIDLE_PRIORITY+1, &xTask1 );
  xTaskCreate( prvTask2, "Task2", 200, NULL, tskIDLE_PRIORITY+1, &xTask2 );
  Serial.println("Starting scheduler now...");
  vTaskStartScheduler();
}

void loop() {
  // NOT USED - idle
}