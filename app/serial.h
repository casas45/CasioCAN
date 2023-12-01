#ifndef SERIAL_H__
#define SERIAL_H__

typedef enum _App_state
{
    IDLE = 0,
    MESSAGE,
    TIME,
    DATE,
    ALARM,
    OK,
    ERROR_
} APP_state;

void Serial_InitTask( void );

void Serial_PeriodicTask( void );

#endif