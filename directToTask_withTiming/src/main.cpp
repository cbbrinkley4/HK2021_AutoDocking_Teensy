#include <Arduino.h>

//Example of using a timer callback in conjunction with "directToTask" functions (similar use case to binary semaphores for context switching)

/*NOTES:
 * - Inspired by DigiKey's FreeRTOS videos on YouTube/Git..
 *      https://github.com/ShawnHymel/introduction-to-rtos/tree/main/08-software-timer
 *      https://www.youtube.com/watch?v=b1f1Iex0Tso&list=RDCMUCclJCqMDAkyVGsm5oFOTXIQ&index=8&ab_channel=Digi-Key
 * 
 * - timers.h not included in FreeRTOS_ARM.h for some reason...added " #include "utility/timers.h" " to FreeRTOS_ARM.h file
 *      Since it was not incorporate from OP, robustness is questionable
 *      
 * - FreeRTOS timer API: https://www.freertos.org/FreeRTOS-Software-Timer-API-Functions.html
 * 
 * - CANNOT print floats in FreeRTOS threads -> issue with boards use of DRAM
 * 
 * - A better supported timing tool (but then not FreeRTOS...) is TeensyTimerTool: https://github.com/luni64/TeensyTimerTool
 *      Standard Teensy "IntervalTimer" function also offers microsecond timing capability (but again...not FreeRTOS)
*/

#include <FreeRTOS_ARM.h>
#include <basic_io_arm.h>

static void prvTask1(void *pvParameters);
static void prvTask2(void *pvParameters);

static TaskHandle_t xTask1 = NULL, xTask2 = NULL;

const int ledPin = 13; //LED is pin 13 on the teensy 3.6

//Initializing the FreeRTOS timer handler (created in setup())
static TimerHandle_t periodTimer = NULL;

/*-----------------------------------------------------------*/
//Callback function for the timer:
void myTimerCallback(TimerHandle_t xTimer){
  if ((uint32_t)pvTimerGetTimerID(xTimer) == 1){
    Serial.println("Auto-Reload Periodic Timer expired");
    xTaskNotifyGive(xTask1);
  }
}
/*-----------------------------------------------------------*/
//Theoretical angle sending function:
static void prvTask1( void *pvParameters ){
    for( ;; )
    {
        /* Blocks indefinitely to wait for myTimerCallback() to protmote to "ready" state*/
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY); 
        Serial.println("Start Task 1 Loop");
        
        Serial.println("Sending angle 1, 2, 3...");
        
        /* Send notification to prvTask2(), bringing it out of the 
        Blocked state and into Ready. */
        xTaskNotifyGive( xTask2);
        //vTaskDelay(500); //delay function used in FreeRTOS (blocks thread for duration)
    }
}

/*-----------------------------------------------------------*/
//Theoretical thrust sending function:
static void prvTask2( void *pvParameters ){
    for( ;; )
    {
        /* Block to wait for prvTask1() to notify this task. */
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        
        Serial.println("Start Task 2 Loop");
        Serial.println("Sending thrust 1, 2, 3...");
    }
}
/*-----------------------------------------------------------*/
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT); //turn on LED to know where live
  digitalWrite(ledPin,HIGH);
  
  while(!Serial){} //loop to wait until serial monitor is open

  //Creating the Tasks (aka FreeRTOS Threads) - tasks are created automatically in "ready" state
  xTaskCreate( prvTask1, "Task1", 200, NULL, tskIDLE_PRIORITY+1, &xTask1 );
  xTaskCreate( prvTask2, "Task2", 200, NULL, tskIDLE_PRIORITY+1, &xTask2 );

  //Creating the timer that c
  periodTimer = xTimerCreate(
                  "Period Callback Timer",  //Name of timer (used for debugging)
                  1000/portTICK_PERIOD_MS,  //Period of timer (in ticks) -> 1 sec = 1000/portTICK_PERIOD_MS (SysTick for teensy generates an interrupt every 1 ms)
                  pdTRUE,                   //Auto-Reload Option (t
                  (void *)1,                //Timer ID that is passed to function
                  myTimerCallback);          //Call back function in which ID gets passed to

  if (periodTimer == NULL){
    Serial.println("Could not create one of the timers");
  }
  else{
  Serial.println("Starting timer now...");
  xTimerStart(periodTimer, portMAX_DELAY); //starts the timer
  }
  
  Serial.println("Starting scheduler now...");
  vTaskStartScheduler();//starts the kernel and thus leaves setup() loop 
}

/*-----------------------------------------------------------*/
void loop() {
  // NOT USED - idle
}