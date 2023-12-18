/**
 * @file    main.c
 * @brief   main file where the scheduler is configured and run.
 *
 * In this file is the configuration of the scheduler to manage the tasks that handle message
 * processing and clock updates.  
 *
 */
#include "bsp.h"

#define TASKS_N             2u          /*!< Number of tasks registered in the scheduler */
#define TIMERS_N            1u          /*!< Number of timers registered in the scheduler */
#define TICK_VAL            5u          /*!< Tick value to scheduler */
#define PERIOD_SERIAL_TASK  10u         /*!< Serial task periodicity */
#define PERIOD_CLOCK_TASK   100u        /*!< Clock task periodicity */
#define ONE_SECOND          1000u       /*!< Value of 1000 ms*/

extern void initialise_monitor_handles(void);

AppSched_Scheduler Scheduler;

/** @brief  Variable to save the update timer ID */
uint8_t UpdateTimerID;

/**
 * @brief   **Application entry point**
 *
 * This function is the application entry point. It initializes the HAL library and configure the
 * scheduler, the tasks in charge of message processing and clock are registered with their periodicity
 * and starts the same scheduler.
 *
 * @retval  None
 */
int main( void )
{
    initialise_monitor_handles();
    
    HAL_Init( );
    
    /*Scheduler config*/
    static AppSched_Task tasks[ TASKS_N ];
    static AppSched_Timer timers[ TIMERS_N ];

    Scheduler.tick      = TICK_VAL;
    Scheduler.tasks     = TASKS_N;
    Scheduler.taskPtr   = tasks;
    Scheduler.timers    = TIMERS_N;
    Scheduler.timerPtr  = timers;

    AppSched_initScheduler( &Scheduler );
    /*Register serial task*/
    (void) AppSched_registerTask( &Scheduler, Serial_InitTask, Serial_PeriodicTask, PERIOD_SERIAL_TASK );
    (void) AppSched_registerTask( &Scheduler, Clock_InitTask, Clock_PeriodicTask, PERIOD_CLOCK_TASK );
    
    /*Software timer register to update time and date in display*/
    UpdateTimerID = AppSched_registerTimer( &Scheduler, ONE_SECOND, ClockUpdate_Callback );
    (void) AppSched_startTimer( &Scheduler, UpdateTimerID );

    AppSched_startScheduler( &Scheduler );

    return 0u;
}
