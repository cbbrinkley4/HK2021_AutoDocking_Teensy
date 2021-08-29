# HK2021_AutoDocking_TeensyTesting
Code for initial testing of our Teensy 3.6 boards used on the Candela AutoDocking project. 

## BEGINNGING COMMENTS:
- All initial testing code written on Ubuntu 16.04, developed on Arduino IDE and then migrated onto Platform IO
  * Platform IO is a much more versatile IDE than arduino - allows Github inteegration, syntax grep, debugging
- Got FreeRTOS to kind of work, but the available port no longer has support an

## INITIAL ENVIRONMENT SETUP:
- Beging by downloading 
  * asdfdf
## CONCLUSIONS
- FreeRTOS is janky on the Teensy and there isn't much of a solution unless we want to work real hard to port ourselves or try to get it to work in MCUXpresso
- We can proceed in a couple ways...
  1. Stay the course - be weary of printing floats and hope no issues arise
  2. By a similar performing STM32 board and include FreeRTOS in CubeMX
