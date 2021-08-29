#include <Arduino.h>

#include <FlexCAN.h>
#include <kinetis_flexcan.h>

#include <FreeRTOS_ARM.h> //Required due to our modificaions to Teensduino while installing FreeRTOS
//(had to manually include timers.h in FreeRTOS_ARM.h document)
#include "basic_io_arm.h"

//Another FreeRTOS port version attempted, but unseccessfully...
//#include <arduino_freertos.h> //https://github.com/tsandmann/freertos-teensy/issues/3


//#include <TeensyTimerTool.h> //timer substitute since freeRTOS timer doesn work...

int32_t timer_id;
//TimerHandle_t xTimer;

//using namespace TeensyTimerTool;

// Redefine AVR Flash string macro as nop for ARM (moves constant strings to program flash memory)
#undef F
#define F(str) str

//Quick check to make sure correct board is plugged in
#ifndef __MK66FX1M0__
  #error "Teensy 3.6 with dual CAN bus is required to run this example"
#endif

static CAN_message_t msg; //Initializing global variable "msg" -> CAN_message_t class type

//const int ledPin = 13; //LED is pin 13 on the teensy 3.6

uint8_t bytes_1[sizeof(float)]; //variable used for transferring float into bytes in CAN message
uint8_t bytes_2[sizeof(float)]; //variable used for transferring float into bytes in CAN message


const uint16_t angleCAN_ID = 0x700; //CAN message ID (11 - 29 bit ID number)
const uint16_t thrustCAN_ID = angleCAN_ID + 1;

//Setting up handles for use with "Direct to Task Notifications" (form of coopertative multitasking (???))
static void angleThread(void *arg); //"prototypes" of the two tasks being created
static void thrustThread(void *arg);

static TaskHandle_t xAngleThread = NULL, xThrustThread = NULL; 

void setup() {//---------------------------------------------------------
   // Insure malloc works in tasks
  //__malloc_heap_end = (char*)RAMEND;
  
  portBASE_TYPE s1, s2; //s3; //declaring threads "s1" and "s2"
  
  Serial.begin(9600);
  //delay(2500);
  Can0.begin(500000); //initializes this CAN on bus at 500kb/s (can be 20k - 1M)
  
  while(!Serial){} //loop to wait until serial monitor is open
  
  Serial.println(F("Setup of Teensy 3.6 CAN + FreeRTOS Server Test"));

  //pinMode(ledPin,OUTPUT); //just to know its turned on 
  //digitalWrite(ledPin,HIGH);

  //sem = xSemaphoreCreateBinary(); //creates a binary semaphore (similar to mutex) - initializes empty
  //Serial.println(String(sem));

  //Creation of the tasks (threads). pdPASS is returned if succesful, otherwise errCOULD_NOT_ALLOCATE_REQUIRED_MEMORY is returned
  s1 = xTaskCreate( angleThread,   /* Pointer to the function that implements the task. */
    "angleThread", /* Text name for the task.  This is to facilitate debugging only. */
    configMINIMAL_STACK_SIZE,    /* Stack depth - variable provided for teensy3.6 RTOS - original configMINIMAL_STACK_SIZE*/
    NULL,   /* Spme parameter passed into the task */
    tskIDLE_PRIORITY + 1,      /* This task will run at priority 1 (idle at 0). */
    &xAngleThread );   /* We are not using the task handle. */
    
  //Creation of the task (thread) for thrust sending
  s2 = xTaskCreate( thrustThread, 
    "thrustThread", 
    configMINIMAL_STACK_SIZE, 
    (void*)1, 
    tskIDLE_PRIORITY + 1, 
    &xThrustThread );

  //xTimer = xTimerCreate("Signal Timer", 1, pdTRUE, NULL, NULL);
  // check for creation errors...
  if (s1 != pdPASS || s2 != pdPASS ) { //(sem== NULL || ...)
    Serial.println(("Creation problem"));
    while(1){}
  }

  vTaskStartScheduler();              //start the scheduler kernel
  Serial.println("Insufficient RAM"); //will only skip starting if memory issue

  //infinite loop if it fails to start scheduler...
  while(1){} 
}

//-----------------------------------------
// Declare the thread function for angleThread (sends out angle desired on CAN)
static void angleThread(void *arg) { 
  
  //Motor angle and thrust arrays used for testing:
  const float motorPosition_1[10] = {-29.99, -22.56, -15.64, -8.27, 0, 7.72, 16.34, 23.5, 29.51};
  const float motorPosition_2[10] = {29.84, 22.56, 15.64, 8.27, 0, -7.72, -16.34, -23.5, -29.51};
  
  static uint8_t i;

  //start of thread infinite looop...
  while(1) {//-------------------------------
    
    memcpy(bytes_1, &motorPosition_1[i], sizeof(motorPosition_1[i])); //converts the float value into 4 bytes
    memcpy(bytes_2, &motorPosition_2[i], sizeof(motorPosition_2[i]));

    //message structure parameters...
    msg.ext = 0;          //extension fo ID (not needed, 0 default)
    msg.id = angleCAN_ID; //Message ID: Any number 0-4095
    msg.len = 8;          //Message Length: number of data bytes in the frame (8 is max)
    msg.buf[0] = bytes_1[0];  //bytes for the motor 1 postiion...
    msg.buf[1] = bytes_1[1];
    msg.buf[2] = bytes_1[2];
    msg.buf[3] = bytes_1[3];
    msg.buf[4] = bytes_2[0];  //bytes for the motor 2 postiion...
    msg.buf[5] = bytes_2[1];
    msg.buf[6] = bytes_2[2];
    msg.buf[7] = bytes_2[3];
    
    Can0.write(msg); //send the msg out
    
    Serial.println(F("Motor 1 angle - sent: "));
    //Serial.println(motorPosition_1[i]);

    Serial.println(F("Motor 2 angle - sent: "));
    //Serial.println(motorPosition_2[i]);

    i++; //increment the counter that cycles through array
    
    if (i == 9){ //resets the counter to loop back through
      i=0;
    }

    //"Direct to task notification" method functions for context switching
    xTaskNotifyGive(xThrustThread); //Notifies thrustThread that it can proceed
    vTaskDelay(500);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //Blocks thrustThread indefinitely (portMax_Delay) until notified by thrustThread
  }
}

//-------------------------------------------
// Declare the thread function for thread 2.
static void thrustThread(void *arg) {

  const int8_t motorThrust_1[10] = {55, 49, 42, 37, 32, 27, 13, 9, 0, -23}; //[lbF]
  const int8_t motorThrust_2[10] = {-17, 0, 7, 18, 22, 29, 34, 39, 46, 51};

  static uint8_t j;

  //if ((uint32_t) arg==1){
    //Serial.println("arg = 1 was passed");
    //ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
  //}

  //start of thread infinite looop...
  while(1) {//-------------------------------
    
    //message structure parameters...
    msg.ext = 0;          //extension fo ID (not needed, 0 default)
    msg.id = thrustCAN_ID; //Message ID: Any number 0-4095
    msg.len = 8;          //Message Length: number of data bytes in the frame (8 is max)
    msg.buf[0] = motorThrust_1[j];  //bytes for the motor 1 thrust...(only 1 byte each)
    msg.buf[1] = motorThrust_2[j];
    msg.buf[2] = 0;
    msg.buf[3] = 0;
    msg.buf[4] = 0;
    msg.buf[5] = 0;
    msg.buf[6] = 0;
    msg.buf[7] = 0;
    
    Can0.write(msg); //send the msg out

    Serial.println(F("Motor 1 thrust - sent: "));
    //Serial.println(motorThrust_1[j]);

    Serial.println(F("Motor 2 thrust - sent: "));
    //Serial.println(motorThrust_2[j]);

    j++; //increment the counter that cycles through array
    
    if (j == 9){ //resets the counter to loop back through
      j=0;
    }
    xTaskNotifyGive(xAngleThread); //Notifies thrustThread that it can proceed
    vTaskDelay(500);
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY); //Blocks thrustThread indefinitely (portMax_Delay) until notified by thrustThread
  }
}

//-----------------------------------------------------
void loop() {
  // Not used - idle loop has a very small, configMINIMAL_STACK_SIZE, stack
  // loop must never block
}