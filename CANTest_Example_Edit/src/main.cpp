#include <Arduino.h>
// -------------------------------------------------------------
// CANtest for Teensy 3.6 dual CAN bus
// by Collin Kidder, Based on CANTest by Pawelsky (based on CANtest by teachop)
//
// Both buses are left at default 250k speed and the second bus sends frames to the first
// to do this properly you should have the two buses linked together. This sketch
// also assumes that you need to set enable pins active. Comment out if not using
// enable pins or set them to your correct pins.
//
// This sketch tests both buses as well as interrupt driven Rx and Tx. There are only
// two Tx buffers by default so sending 5 at a time forces the interrupt driven system
// to buffer the final three and send them via interrupts. All the while all Rx frames
// are internally saved to a software buffer by the interrupt handler.
//

#include <FlexCAN.h>
#include <FreeRTOS_ARM.h> //Required due to our modificaions to Teensduino while installing FreeRTOS

//Quick check to make sure correct board is plugged in
#ifndef __MK66FX1M0__
  #error "Teensy 3.6 with dual CAN bus is required to run this example"
#endif

static CAN_message_t msg; //Initializing global variable "msg" -> CAN_message_t class type
static uint8_t hex[17] = "0123456789abcdef"; //code used for deciphering CAN message - saves an array of 17
const int ledPin = 13;

// -------------------------------------------------------------
static void hexDump(uint8_t dumpLen, uint8_t *bytePtr)
{
  uint8_t working; 
  while( dumpLen-- ) { //while loop that decrements lenth of msg.buf (8 in this case)
    working = *bytePtr++;
    
    //Serial.print(working); //prints the decimal number sent in msg.buf (*bytePtr for Msg.buf[0] = 10)
    //Serial.write(working); //doesnt print anything
    //Serial.write(hex[10]); //prints 'a'char per the hex[17] declared earlier
    //Serial.print(hex[10]); //displays the decimal value of char in hex[] global variable (ASCII table - a = 96)

    //converting the binary of Msg.buf[x] to hex using global hex[] array
    Serial.write( hex[ working>>4 ] ); //shifts the byte (0000 1010 for 10) to the right to read first half only - compares to hex[] chars
    Serial.write( hex[ working&15 ] ); //AND bitwise mask to see the values of the the first 4 (10 -> 1010 -> 'A' in hex)
    //Serial.print(working, HEX); //another way of writing, but removes the zeroes
  }
  Serial.write('\r'); //"carriage return", will write over whatevers on line
  Serial.write('\n');
}

// -------------------------------------------------------------
void setup(void)
{
  delay(1000);
  Serial.println(F("Hello Teensy 3.6 dual CAN Test."));

  pinMode(ledPin,OUTPUT);
  digitalWrite(ledPin,HIGH);
  
  Can0.begin(); //initializes this CAN on bus (default is 250k, but can be 20k 1Mb/s)
  Can1.begin();

  //if using enable pins on a transceiver they need to be set on
  //pinMode(2, OUTPUT);
  //pinMode(35, OUTPUT);

  //digitalWrite(2, HIGH);
  //digitalWrite(35, HIGH);

  //message structure parameters...
  msg.ext = 0;
  msg.id = 0x100; //Message ID: Any number 0-4095
  msg.len = 8; //Message Length: number of data bytes in the frame
  msg.buf[0] = 10; //Date byte 0: anything
  msg.buf[1] = 20;// ""
  msg.buf[2] = 0;
  msg.buf[3] = 100;
  msg.buf[4] = 128;
  msg.buf[5] = 64;
  msg.buf[6] = 32;
  msg.buf[7] = 16;
}

// -------------------------------------------------------------
void loop(void)
{
  CAN_message_t inMsg;//
  
  while (Can0.available()) 
  {
    Can0.read(inMsg); //
    Serial.print("CAN bus 0:"); 
    hexDump(8, inMsg.buf); //send the inMsg data frame buffers (8x, msg.buf[0]-[7]) to hexDump function
    
    //Serial.write(hex[*inMsg.buf]); Serial.write('\n');
  }

  // --> Testing of the tx/rx buffers as mentioned in the original comments (reasoning for so many increments and writes):
  msg.buf[0]++;
  Can1.write(msg);
  msg.buf[0]++;
  Can1.write(msg);
  msg.buf[0]++;
  Can1.write(msg);
  msg.buf[0]++;
  Can1.write(msg);
  msg.buf[0]++;
  Can1.write(msg);  
  delay(20);
}