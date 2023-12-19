CasioCan project {#mainpage}
============

Project description.
-------------

This project consists in a programmable clock using messages that are sent and received through FDCAN module. 

### Serial Message Processing.

The processing of the messages that arrive to configure the clock is made through a queue, where a serial event machine is implemented. Each writtten message indicates which event needs to be executed.


### Clock State Machine.

This state machine is in charge of the message processing from the serial task and the message that indicate when it's time to update the display.

> The project is implemented in the STM32-G0B1RE Nucleo board.



