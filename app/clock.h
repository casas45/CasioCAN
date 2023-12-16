/**
 * @file    clock.h
 * 
 * @brief   header file where are the functions prototypes of clock driver.
*/
#ifndef CLOCK_H__
#define CLOCK_H__

#ifndef UTEST
#define STATIC static       /*!< Macro to remove static keyword only for unit tests */
#else
#define STATIC
#endif

typedef enum
{
    IDLE = 0,
    EVALUATE_MSG,
    TIME,
    DATE,
    ALARM,
    DISPLAY,
    N_CLK_STATES
} ClkState;

void Clock_InitTask( void );

void Clock_PeriodicTask( void );

void ClockUpdate_Callback( void );

#endif