/**
 * @file    clock.h
 * 
 * @brief   header file where are the functions prototypes of clock driver.
*/
#ifndef CLOCK_H__
#define CLOCK_H__

void Clock_InitTask( void );

void Clock_PeriodicTask( void );

void ClockUpdate_Callback( void );

void TimerAlarmOneSecond_Callback( void );

void TimerDeactivateAlarm_Callback( void );

#endif