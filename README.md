# HK2021_AutoDocking_TeensyTesting
Code and discussion of initial testing and environment setup for Teensy 3.6 boards used on our Candela AutoDocking project. 

## BEGINNGING COMMENTS:
- All initial testing code written on Ubuntu 16.04, developed on Arduino IDE and then migrated onto Platform IO (extension in Visual Code).
  * Platform IO is a much more robust IDE than arduino - allows Github integration, syntax grep, debugging, etc...I would highly recommend using this IDE instead.
- Got FreeRTOS to kind of work, but the available port no longer has support and is buggy...discussion at end.

## SETUP:
 ### Base Teensyduino w/ Platform IO:
 ### FreeRTOS Library:
 ### FlexCAN Library:
  - Super easy to use library that comes included with teensyduino installer and specifically ported to Teensy 3.6.
  - Simply "sketch -> include library -> contributed libraries"  in Arduino environment or just type in header
  - In Platform IO, it works the same as long as you do the standard #include <Arduino.h>
  - For API, reference this git repository: https://github.com/collin80/FlexCAN_Library
 ### Setting up Github in Platform IO:
  - Watch this youtube video: https://www.youtube.com/watch?v=13wmV6gcb2A&t=4s&ab_channel=VehlaEngineer_LogitechProjectsSolutions
  - Should be able to just clone my repo of this PlatoformIO workspace, but let me know if you run into issues

## SUMMARY OF INCLUDED FILES:

- NOTE: I tried to comment through most of main scripts themselves, but ran out of time...let me know if anything needs clarrication.

 ### 1. CANTest_Example_Edit: 
   - Basically just a copy of the included FlexCAN library example , but wihth my comments.
   - Uses CAN0 to send and CAN1 to receive on the same teensy 3.6 board.
   - Good demonstrations on the buffers in particular.
   - Requires two CAN transceivers (MCP2562-H/P/ND) and terminating 120 ohm resistors on both ends to run (see pic in repo).
   
 ### 2. directToTask_withTiming:
   - This shows the use of FreeRTOS software timers with "direct to task" notifications.
   - Direct to task notifcations are a lightweight alternative to binary semaphore use for context switching.
     * Read more on it here: https://www.freertos.org/RTOS-task-notification-API.html
     * Works in a similar tway to osSignalWait/Set that we used in our embedded systems course.
   - timers.h is not originally included in the FreeRTOS_ARM.h file for some reason...*Make sure to manually add this yourself (seems to work).*
   
 ### 3. FlexCAN_Example_Client:
   - Demonstration of FlexCAN being used to transfer data between two seperate boards.
   - Does NOT include FreeRTOS (just main loop with some standard teensyduino delays).
   - This particular script just *receives* information (in this case, a realistic range of motor parameter data types) over CAN.
   - Also worked in conjunction with "testing_RTOSwithCAN_Server" to show that info was being received (despite no print debugging possible on server side - FreeRTOS error with floats).
   
 ### 4. FlexCAN_Example_Server:
   - Pretty much same idea as FlexCAN_Example_Client, but that *sends* the messages instead.
   
 ### 5. freeRTOS_blink_Example:
   - Basically the standard example included with the FreeRTOS_ARM library, but with a few things moved around and my comments.
   - Good example of using binary semaphores for context switching rather than "direct to task" notifications.
     * API for semaphores/mutex in FreeRTOS: https://www.freertos.org/a00113.html
     
 ### 6. FreeRTOS_DirectToToTask_Testing:
   - similar to the "directoTask_withTiming", but that which relies purely on the "direct to task" notifications and delays
   - I was using to see the functionality of Give/Take funtions
     * Conclusion is that Give promotes from block->ready or ready->run (depending on task priority of course)
     
 ### 7. testing_RTOSwithCAN_Server (Hard Fault Found Here)
   - This was planned to encompass all aspects (FreeRTOS, FlexCAN, timers, etc.), but limitations were found
   - It is determined that FreeRTOS on Teensy does not allow printing of floats in threads due to the way dynamic memory is handeled (https://forum.pjrc.com/threads/57850-ChibiOS-RT-RTOS-for-Teensy-4-0)
   - See included screenshots.

## CONCLUSIONS:

- Teensy 3.6 is NOT ideal for working with FreeRTOS...
  1. FreeRTOS in teensduino environment produces hard fault when printing float in threads
    - https://forum.pjrc.com/threads/40915-Serial-println-Float-doesn-t-run
  2. Doesnt include timers.h standard...robustness is questionalble
  3. Even creator of port claims that trying to get it to work properly is a lost cause an he no longer supports it
  4. No debugger on board and getting one to work requires de-soldering the board
    * Thus its hard to really know whats going on...logic analyser would be nice
    * Lots of effort to get it to work... https://mcuoneclipse.com/2017/04/29/modifying-the-teensy-3-5-and-3-6-for-arm-swd-debugging/
    
- We can proceed in a couple ways...
  1. Stay the course - be weary of printing floats and hope no further issues arise.
  2. Continue to use the teensy, but do away with FreeRTOS and use better supported task managment libraries
    * Stock Teensy Timing Functions: https://www.pjrc.com/teensy/td_timing.html
    * TeensyThreads: https://forum.pjrc.com/threads/41504-Teensy-3-x-multithreading-library-first-release
    * TeensyTimerTool: https://github.com/luni64/TeensyTimerTool, https://forum.pjrc.com/threads/59112-TeensyTimerTool
  3. Try to get teensy to work in MCUXpresso (but that seems like a ton of work...Paul, the creator of Teensy, even condemned the idea...https://forum.pjrc.com/threads/65737-Making-a-proof-of-concept-device)
    * https://www.stupid-projects.com/using-nxp-sdk-with-teensy-4-0/
    * 	https://community.nxp.com/t5/Kinetis-Software-Development-Kit/How-to-create-an-FreeRTOS-project-with-MCUXpresso-IDE/ta-p/1107710
    * 	https://forum.pjrc.com/threads/48701-Getting-Kinetis-SDK-examples-to-work-on-the-Teensy
  4. Buy a similar performing STM32 board and include FreeRTOS in CubeMX (which is greatly supported).
    * STM32F429 nucleo or discovery boards - > Similar clock speed, flash, and ram with the ST link debugger.
  5. Buy a NXP development board that uses the same chip as Candela/Teensy and program in MCUXpressso IDE and SDK (which also supports FreeRTOS better)
    * FRDM-K66F NXP development board (Uses same chip as Teensy/Candela - MK66FX1M0(VMD18))
    * https://www.nxp.com/docs/en/data-sheet/K66P144M180SF5V2.pdf
