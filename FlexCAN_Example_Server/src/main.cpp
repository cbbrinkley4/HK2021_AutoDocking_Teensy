#include <Arduino.h>

//Code that simply tests out FlexCAN library API (Server)
//Refer to MCP2562-H/P-ND Data sheet and SKPang Schematic for CAN transceiver wiring 

#include <FlexCAN.h>
#include <kinetis_flexcan.h>

//Included ONLY due to 'systick_isr' modification to teensduino 
#include <FreeRTOS_ARM.h> 
#include <basic_io_arm.h>

// Redefine AVR Flash string macro as nop for ARM (moves constant strings to program flash memory)
#undef F
#define F(str) str

const int ledPin = 13; //LED is pin 13 on Teensy 3.6

//Motor angle and thrust arrays used for testing:
const float motorAngle_1[10] = {-29.99, -22.56, -15.64, -8.27, 0, 7.72, 16.34, 23.5, 29.51};
const float motorAngle_2[10] = {29.84, 22.56, 15.64, 8.27, 0, -7.72, -16.34, -23.5, -29.51};
const int8_t motorThrust_1[10] = {55, 49, 42, 37, 32, 27, 13, 9, 0, -23}; //[lbF]
const int8_t motorThrust_2[10] = {-17, 0, 7, 18, 22, 29, 34, 39, 46, 51};

uint8_t bytes_1[sizeof(float)]; //variable used for transferring float into bytes in CAN message
uint8_t bytes_2[sizeof(float)]; //variable used for transferring float into bytes in CAN message

//Initializing CAN message handler
CAN_message_t msg;
const uint16_t angleCAN_ID = 0x700; //CAN message ID (11 - 29 bit ID number)
const uint16_t thrustCAN_ID = angleCAN_ID + 1;

void setup() { //-----------------------------------------

  Serial.begin(9600);
  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,HIGH);
  while(!Serial){} //loop to wait until serial monitor is open
  
  Serial.println(F("Initializing Test of Basic CAN Comm. - 2 Boards"));

  Can0.begin(500000); //initializes this CAN on bus at 500kb/s (can be 20k - 1M)
}

void loop() {//--------------------------------------------

  //Counters for cycling through the sample values
  static uint8_t i;
  static uint8_t j;
  
  memcpy(bytes_1, &motorAngle_1[i], sizeof(motorAngle_1[i])); //converts the float value into 4 bytes
  memcpy(bytes_2, &motorAngle_2[i], sizeof(motorAngle_2[i]));

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
    
  Serial.print(F("Motor 1 Angle Sent: "));
  Serial.println(motorAngle_1[i]);
  Serial.print(F("Motor 2 Angle Sent: "));
  Serial.println(motorAngle_2[i]);
  Serial.print("\n");
  
  i++; //increment the counter that cycles through array
  if (i == 9){ //resets the counter to loop back through
    i=0;
  }
  
  //delay(500); //500 ms

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

  Serial.print(F("Motor 1 Thrust Sent: "));
  Serial.println(motorThrust_1[j]);
  Serial.print(F("Motor 2 Thrust Sent: "));
  Serial.println(motorThrust_2[j]);
  Serial.print("\n");
  j++; //increment the counter that cycles through array
  if (j == 9 ){ //resets the counter to loop back through
    j=0;
  }

  delay(1000); //send out every 1 sec
}