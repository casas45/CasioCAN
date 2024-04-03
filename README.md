CasioCAN - ModularMX {#mainpage}
============

CasioCAN description.
-------------

Luis David Casas Hernandez - ModularMX.
luis_david_casas@hotmail.com

The CasioCAN project is part of the ModularMX embedded bootcamp.
This project consists of a serial programable clock through CAN messages, where a round-robin scheduler is used to run the different tasks that integrate the system, and a queueing algorithm for the communication among tasks.

There are three essential parts of the application:
**Serial event machine:** in charge of processing the CAN messages to know if the message has valid parameters and communicate with other tasks to change the time, date, or alarm on the RTC.
**Clock event machine:** handles the RTC actions like setting time, date, or an alarm, and the alarm interrupt to activate a PWM signal which sounds the buzzer, and communicates with the display task each second to update the LCD.
**Display event machine**: uses the LCD driver to control the display and show the information needed, like time and date, a message if the alarm is active, and control the backlight and its intensity.

Other drivers were developed, one of them is the LCD driver which was implemented to control the alphanumeric display NHD-C0216CZ-NSW-BBW-3V3  with the controller ST7032, this was done using SPI communication. The other implemented driver was used to read analog signals using the ADC module, these signals come from two external potentiometers to control the backlight intensity and contrast of the LCD, and another channel is used to read the internal temperature sensor of the MCU.

Another important feature is the implementation of the WWDG to know if a deadlock occurred in the system.

Also, some Functional Safety requirements were added to the project, including checking the returned values by the used functions, a scheduler monitoring flow checking if the tasks were running with their corresponding periodicity, and reading the potentiometers with extra channels to check if the values are equal with an error range of 10% if an inconsistency occurs the program is sent to a safe state where the modules are disabled and a respective error code is shown using eight LEDs.

The entire project was versioned using git, and each feature was developed in a separate branch, documented using Doxygen, and compliance with MISRA standards was ensured using cppcheck.


> The project is implemented in the STM32-G0B1RE Nucleo board.



