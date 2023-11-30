/**
 * @file    main.c
 * @brief   **Template Application entry point**
 *
 * The main file is the entry point of the application or any user code, please provide the
 * proper description of this file according to your own implementation
 * This Demo app only blinks an LED connected to PortA Pin 5
 *
 * @note    Only the files inside folder app will be take them into account when the
 *          doxygen runs by typing "make docs", index page is generated in
 *          Build/doxigen/html/index.html
 */
#include "scheduler.h"
#include "queue.h"
#include "bsp.h"
#include "serial.h"

#define TASKS_N         1u
#define TICK_VAL        5u
#define PERIOD_SERIAL_TASK  10u

static AppSched_Task tasks[ TASKS_N ];
static AppSched_Scheduler Scheduler;



/**
 * @brief   **Application entry point**
 *
 * Ptovide the proper description for function main according to your own
 * implementation
 *
 * @retval  None
 */
int main( void )
{
    /*Scheduler config*/
    Scheduler.tick      = TICK_VAL;
    Scheduler.tasks     = TASKS_N;
    Scheduler.taskPtr   = tasks;
    AppSched_initScheduler( &Scheduler );
    /*Register serial task*/
    AppSched_registerTask( &Scheduler, Serial_InitTask, Serial_PeriodicTask, PERIOD_SERIAL_TASK );

    AppSched_startScheduler( &Scheduler );


    return 0u;
}
