/**
 * @file    main.c
 * @brief   main file where the scheduler is configured and run.
 *
 * In this file is the configuration of the scheduler to manage the tasks that handle message
 * processing and clock updates.  
 *
 */
#include "bsp.h"

#define TASKS_N         1u          /*!< Number of tasks registered in the scheduler */
#define TICK_VAL        5u          /*!< Tick value to scheduler */
#define PERIOD_SERIAL_TASK  10u     /*!< Serial task periodicity */

static AppSched_Scheduler Scheduler;


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
    HAL_Init( );

    static AppSched_Task tasks[ TASKS_N ];
    
    /*Scheduler config*/
    Scheduler.tick      = TICK_VAL;
    Scheduler.tasks     = TASKS_N;
    Scheduler.taskPtr   = tasks;
    AppSched_initScheduler( &Scheduler );
    /*Register serial task*/
    (void) AppSched_registerTask( &Scheduler, Serial_InitTask, Serial_PeriodicTask, PERIOD_SERIAL_TASK );
    
    AppSched_startScheduler( &Scheduler );


    return 0u;
}
