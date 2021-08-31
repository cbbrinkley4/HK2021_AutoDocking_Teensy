# HK2021_AutoDocking_TeensyTesting
Code and discussion of initial testing and environment setup for Teensy 3.6 boards used on our Candela AutoDocking project. 

## BEGINNGING COMMENTS:
- All initial testing code written on Ubuntu 16.04, developed on Arduino IDE and then migrated onto Platform IO (extension in Visual Code).
  * Platform IO is a much more robust IDE than arduino - allows Github integration, syntax grep, debugging, etc...I would highly recommend using this IDE instead.
- Got FreeRTOS to kind of work, but the available port no longer has support and is buggy...discussion at end.

## SETUP:
 ### Teensyduino and Platform IO IDE:
 Basically, just follow instructions here: https://wiki.uiowa.edu/display/teensymacos/Getting+Started#GettingStarted-Option1:ArduinoIDE
 1. Download Arduino IDE for respective OS https://www.arduino.cc/en/software
 2. Download Teensyduino bootloader and follow instructions on site: https://www.pjrc.com/teensy/td_download.html (make sure to copy over udev.rules per the link if using linux!!)
 3. Download Visual Studio Code: https://code.visualstudio.com/ (NOT to be confused with standard "Visual Studio"
 4. In VS Code, go to extensions and add PlatformIO (make sure you have python 3.6 + installed and have the default path set to this!)

When using PlatformIO, coding is pretty much the same as Arduino, you just have to make a new project, name the main script "main", and #include <Arduino.h> as a header (though it should automatically do this for you if you set up the project with arduino being the selected framework).

API for teensyduino: https://www.pjrc.com/teensy/teensyduino.html
PlatformIO documentation on teensy boards:  https://docs.platformio.org/en/latest/platforms/teensy.html

 ### FreeRTOS Library:
 There are a few git repos for FreeRTOS on teensy, but the only one that I was able to make work comes from "greiman" (who is also quite active on PJRC forums)
 
 FreeRTOS in Arduino IDE
 1. Download Git repo: https://github.com/greiman/FreeRTOS-Arduino/blob/master/libraries/FreeRTOS_ARM/examples/frBlink/frBlink.ino (there are a couple 
 2. Add extracted files to the Arduino/libary folders on your home or documents (not the programs folder - installation should generate this other folder. program folder should remain for standard included libraries)
 3. IMPORTANT: You must comment out *systick_isr* and *pendablesrvreq_isr* functions in EventResponder.cpp: https://forum.pjrc.com/threads/57308-FreeRTOS-compile-error-saying-Teensy-3-5-board-is-AVR-architecture

 FreeRTOS in PlatformIO...
 1. In the PlatformIO extension home page, go to Libraries/Registry and then search for FreeRTOS_ARM and include it with your project
 2. To include timers, same thing as Arduino. You will find 
 3. Add the #include <timers.h> line into "FreeRTOS_ARM.h" file if you want to use software timers. You will find the "FreeRTOS_ARM.h" file in the explorer to the left...project/.pio/libdeps/src/FreeRTOS_ARM.h
 4. You must do this to EVERY new project (may be a more advanced way of tweaking paths, but I dont know it yet)

Another helpful set of vidoeos/code come from DigiKey (they use an ESP32 though): https://github.com/ShawnHymel/introduction-to-rtos/blob/main/08-software-timer/esp32-freertos-08-demo-software-timer/esp32-freertos-08-demo-software-timer.ino, https://www.youtube.com/watch?v=F321087yYy4&list=PLeA_VFTcoXuuPU-8e4-cnQZfMlipd3-y1&index=6&t=20s&ab_channel=Digi-Key

Another series (again, not our teensy though): https://www.youtube.com/watch?v=PcE-2VjtYF4&list=PLeA_VFTcoXuuPU-8e4-cnQZfMlipd3-y1&index=1&t=394s&ab_channel=ProgrammingKnowledge

### FlexCAN Library:
  - Super easy to use library that comes included with teensyduino installer and specifically ported to Teensy 3.6.
  - Simply "sketch -> include library -> contributed libraries"  in Arduino environment or just type in header
  - In Platform IO, it works the same as long as you do the standard #include <Arduino.h>
  - For API, reference this git repository: https://github.com/collin80/FlexCAN_Library
  
### Setting up Github in Platform IO:
  - Watch this youtube video: https://www.youtube.com/watch?v=13wmV6gcb2A&t=4s&ab_channel=VehlaEngineer_LogitechProjectsSolutions
  - Should be able to just clone my repo of this PlatoformIO workspace, but let me know if you run into issues
    * *You may have to include FreeRTOS into each project manually because I didnt see them get included in my push for some reason*
    
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
   - Helpful example: https://github.com/skpang/Teensy32_CAN-Bus_BME280_temperature_and_humidity/blob/master/Teensy32_CAN-Bus_BME280_temperature_and_humidity.ino, https://github.com/skpang/Teensy32_OLED_BME280_temperature_display, https://www.youtube.com/watch?v=8-0dSIkvCeI&ab_channel=SKPang
   
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
    * https://forum.pjrc.com/threads/40915-Serial-println-Float-doesn-t-run
    * https://forum.pjrc.com/threads/49211-Does-teensy-3-5-3-6-support-freeRTOS
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
