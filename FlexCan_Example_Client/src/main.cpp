#include <Arduino.h>

//Code that simply tests out FlexCAN library API (Client)
//Refer to MCP2562-H/P-ND Data sheet and SKPang Schematic for CAN transceiver wiring 

#include <FlexCAN.h>
#include <kinetis_flexcan.h>

//Included ONLY due to 'systick_isr' modification to standard teensduino headers... 
//https://forum.pjrc.com/threads/57308-FreeRTOS-compile-error-saying-Teensy-3-5-board-is-AVR-architecture
#include <FreeRTOS_ARM.h> 
#include <basic_io_arm.h>

// Redefine AVR Flash string macro as nop for ARM (moves constant strings to program flash memory)
#undef F
#define F(str) str

const int ledPin = 13;

//FlexCAN myCAN(50000);
CAN_message_t msg, rxmsg;
const uint16_t angleCAN_ID = 0x700; //CAN message ID (11 - 29 bit ID number)
const uint16_t thrustCAN_ID = angleCAN_ID + 1;

volatile uint32_t count = 0;
//initializing 32 bit software timer that uses interupts to call a function (part of the standard Teensy API)
//https://www.pjrc.com/teensy/td_timing_IntervalTimer.html
IntervalTimer data_timer;
volatile uint32_t can_msg_count = 0;

unsigned char data_1[4]; //temp. receiving memory location from CAN rxmsg
unsigned char data_2[4];

float motorAngle_1; //new data storage location for data
float motorAngle_2;
int8_t motorThrust_1;
int8_t motorThrust_2;

uint8_t no_data1 = 0;
uint8_t no_data2 = 0;

void setup() {//------------------------------------------------------
  
  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,HIGH);
  //while(!Serial){} //loop to wait until serial monitor is open
  
  Serial.println(F("Initializing Test of Basic CAN Comm. - 2 Boards"));

  Can0.begin(500000); //initializes this CAN on bus at 500kb/s (can be 20k - 1M)
  data_timer.begin(data_count, 1000000); //start "no_data" counter at 1 sec periods (timer in microseconds)
}

void data_count(void){//---------------------------------------------
  
  no_data1++;
  no_data2++;
}


void loop() {//--------------------------------------------------------
  
  while(Can0.read(rxmsg)){
    if(rxmsg.id == angleCAN_ID){
      data_1[0] = rxmsg.buf[0];
      data_1[1] = rxmsg.buf[1];
      data_1[2] = rxmsg.buf[2];
      data_1[3] = rxmsg.buf[3];
      data_2[0] = rxmsg.buf[4];
      data_2[1] = rxmsg.buf[5];
      data_2[2] = rxmsg.buf[6];
      data_2[3] = rxmsg.buf[7];

      //converting bytes of data back into floats
      memcpy(&motorAngle_1, data_1, 4); //memcpy(dest. addr, src memory location, # of bytes)
      memcpy(&motorAngle_2, data_2, 4);

      Serial.print("Motor 1 Angle Receieved: ");
      Serial.println(motorAngle_1, 2);
      Serial.print("Motor 2 Angle Recieved: ");
      Serial.println(motorAngle_2, 2);
      Serial.print("\n");

      no_data1 = 0;
      
    }
    if(rxmsg.id == thrustCAN_ID){
      motorThrust_1 = rxmsg.buf[0]; //no need for memcpy b/c we only sent 1 byte 
      motorThrust_2 = rxmsg.buf[1];

      Serial.print("Motor 1 Thrust Receieved: ");
      Serial.println(motorThrust_1); 
      Serial.print("Motor 2 Thrust Recieved: ");
      Serial.println(motorThrust_2);
      Serial.print("\n");
      
      no_data2 = 0;
    }
  }
  
  if(no_data1>2){
    Serial.println("No angle values were recieved");
    Serial.print("\n");
    no_data1 = 3;
  }
  
  if(no_data2>2){
    Serial.println("No thrust varlues were recieved");
    Serial.print("\n");
    no_data2 = 3;
  }
}