/**
 * @file    scheduler.c
 * @brief   Scheduler to run different tasks according their periodicity.
 *
 * This driver help to manage a structure that contain the tasks to run and the software timers. It has
 * functions to initialize the scheduler, register task/timer, start task/timer, stop task/timer, to 
 * change their periodicity and the most important to run the scheduler.
 *  
 */

#include "scheduler.h"
#include <stddef.h>
#include "bsp.h"



/**
 * @brief   Interface to initialize the scheduler.
 *
 * In this fuction initialize the scheduler, to do that is just necessary set the tasks and timers count to
 * zero.
 *
 * @param   scheduler [in] It's the memory address of the scheduler to access the elements.
 *
 *
 * @note Before using this function it's mandatory initialized the elements: tick, tasks, taskPtr, timeout
 * timers and timerPtr.
 */
void AppSched_initScheduler( AppSched_Scheduler *scheduler)
{
    scheduler->tasksCount = 0;
    scheduler->timersCount = 0;
}

/**
 * @brief   Register a task if the period is valid.
 *
 * This function register a task, to do that verify if the period is a multiple of tick, in the TRUE case,
 * register the task setting the init and task fuctions in the corresponding TCB position with its 
 * periodicity, the period is assigned to the eleapsed time, so the tasks will be run just after start the
 * scheduler, and also set the runTask flag to TRUE, finally increment the taskCount by one to return it.
 *
 * @param   scheduler[in] It's the memory address of the scheduler to access the elements.
 * @param   initPtr[in] Memory address of the init function, it could be NULL.
 * @param   taskPtr[in] Memory address of the function to run periodically.
 * @param   period[in] The periodicity with wich the task will be run.
 *
 * @retval  Returns the taskID if the task has been registered succesfully, and returns a zero if not.
 * 
 * @note    The taskID's will be from 1 to the number of tasks.
 *
 */
unsigned char AppSched_registerTask( AppSched_Scheduler *scheduler, void (*initPtr)(void), void (*taskPtr)(void), unsigned long period )
{
    unsigned char varRetRt = 0;

    if ( ( period % scheduler->tick ) == 0u )
    {
        scheduler->taskPtr[ scheduler->tasksCount ].initFunc = initPtr;
        scheduler->taskPtr[ scheduler->tasksCount ].taskFunc = taskPtr;
        scheduler->taskPtr[ scheduler->tasksCount ].period = period;
        scheduler->taskPtr[ scheduler->tasksCount ].elapsed = period;
        scheduler->taskPtr[ scheduler->tasksCount ].runTask = TRUE;
        scheduler->tasksCount++;

        varRetRt = scheduler->tasksCount;
    }

    return varRetRt;   
}

/**
 * @brief   Stop a task that was previously registered.
 * 
 * To know if the argument task is a valid taskID the function verify if it's in the range bettwen 
 * 1 and the tasksCount, in a TRUE case set the runTask flag to FALSE doing an offset to the position
 * in the TCB block to access the right task.
 * 
 * @param   scheduler[in] It's the memory address of the scheduler to access the elements.
 * @param   task[in] the taskID of the task to stop.
 * 
 * 
 * @retval  Return the success of the action, TRUE is the task was stopped and FALSE if was not.
 * 
 * 
*/
unsigned char AppSched_stopTask( AppSched_Scheduler *scheduler, unsigned char task )
{
    unsigned char retStopTask = FALSE;

    if ( ( task > 0u ) && (task <= scheduler->tasksCount) )
    {
        scheduler->taskPtr[ task - 1u ].runTask = FALSE;
        retStopTask = TRUE;
    }

    return retStopTask;
}

/**
 * @brief   Start a task that was previously registered.
 * 
 * To know if the argument task is a valid taskID the function verify if it's in the range bettwen 
 * 1 and the tasksCount, in a TRUE case set the runTask flag to FALSE doing an offset to the position
 * in the TCB block to access the right task.
 * 
 * @param   scheduler[in] It's the memory address of the scheduler to access the elements.
 * @param   task[in] the taskID of the task to stop.
 * 
 * 
 * @retval  Return the success of the action, TRUE is the task was stopped and FALSE if was not.
 * 
*/
unsigned char AppSched_startTask( AppSched_Scheduler *scheduler, unsigned char task )
{
    unsigned char retStartTask = FALSE;

    if ( ( task > 0u ) && ( task <= scheduler->tasksCount ) )
    {
        scheduler->taskPtr[ task - 1u ].runTask = TRUE;
        retStartTask = TRUE;
    }

    return retStartTask;
}

/**
 * @brief   This function sets a new period to a task that has been registered before.
 * 
 * To know if the argument task is a valid taskID the function verify if it's in the range bettwen 
 * 1 and the tasksCount, and also verify if the period is valid that means it's lower or equal to
 * the timeout and it's a tick multiple, in a TRUE case just set the new value to the period element
 * of the TCB in the right position.
 * 
 * @param   scheduler[in] Memory address of the scheduler to access the elements.
 * @param   task[in] the taskID of the task to change the period.
 * @param   period[in] the new period of the task.
 * 
 * @retval  Return the action success, TRUE if it's a valid taskID and period, and false if not. 
*/
unsigned char AppSched_periodTask( AppSched_Scheduler *scheduler, unsigned char task, unsigned long period)
{
    unsigned char varRetpT = FALSE;

    if ( ( ( period % scheduler->tick ) == 0u ) && ( task > 0u ) && ( task <= scheduler->tasksCount ) )
    {
        scheduler->taskPtr[ task - 1u ].period = period;
        varRetpT = TRUE;
    }

    return varRetpT;
}

/**
 * @brief This function runs the scheduler for the time set in timeout.
 * 
 * When the function is called runs the init functions if there are, then enter in a while loop until
 * the timeout has elapsed, the base of time is the number of ticks, that is checked using the function
 * miliseconds. In the cycle every time a tick happens check all the tasks and timers to know if it's
 * time to run the corresponding function.
 * 
 * @param scheduler[in] Memory address of the scheduler to access the elements.
 * 
 * 
 * @note 
*/

void AppSched_startScheduler( AppSched_Scheduler *scheduler )
{
    unsigned long status = TRUE;

    unsigned long tickstart = HAL_GetTick(); 
    static unsigned long countTicks = 0;  //variable to count ticks

    for (unsigned char i = 0; i < scheduler->tasksCount; i++)   //cicle for init tasks
    {
        if ( scheduler->taskPtr[i].initFunc != NULL )
        {
            scheduler->taskPtr[i].initFunc();
        }
    }
    
    while ( status )
    {

        if( ( HAL_GetTick() - tickstart ) >= ( scheduler->tick * countTicks ) )    //if to know tick happens
        {
            for (unsigned char i = 0; i < scheduler->tasksCount; i++)   //run all tasks if its time
            {
                    if ( ( scheduler->taskPtr[i].elapsed >= scheduler->taskPtr[i].period ) && ( scheduler->taskPtr[i].runTask == TRUE ) )
                    {
                        scheduler->taskPtr[i].taskFunc();
                        scheduler->taskPtr[i].elapsed = 0;          //reset elapsed time
                    }
                
                scheduler->taskPtr[i].elapsed += scheduler->tick; 
            }
             
            for (unsigned char j = 0; j < scheduler->timersCount; j++)
            {
                if ( scheduler->timerPtr[j].startFlag == TRUE )
                {
                    scheduler->timerPtr[j].count -= scheduler->tick;    //decrement the count timer

                    if ( ( AppSched_getTimer( scheduler, j + 1u ) == 0u ) && ( scheduler->timerPtr[j].callbackPtr != NULL ) )
                    {
                        scheduler->timerPtr[j].startFlag = FALSE;
                        scheduler->timerPtr[j].callbackPtr();
                    }
                }
            }

            ++countTicks;       //increment the tick.
        
        }   
        
    }
    
}


/**
 * @brief   Register a timer if the timeout is valid.
 * 
 * This function register a timer, to do that verify if the timeout is a multiple of tick, in the TRUE case,
 * register the timer setting the callback fuction in the corresponding TCB position with its timeout, the
 * timeout is assigned to the actual count, and the startFlag is set FALSE and the timersCount increment by
 * one.
 * 
 * @param   scheduler[in] Memory address of the scheduler to access the elements.
 * @param   timeout[in] Timer timeout to decrement. 
 * @param   callbackPtr[in] Memory address of the function to be run when the count down to zero.
 * 
 * @retval  Returns the timerID if the timer has been registered succesfully, and returns a zero if not.
*/
unsigned char AppSched_registerTimer( AppSched_Scheduler *scheduler, unsigned long timeout, void (*callbackPtr)(void) )
{
    unsigned char timerID = 0;

    if ( ( timeout % scheduler->tick ) == 0u )
    {
        scheduler->timerPtr[ scheduler->timersCount ].timeout = timeout;
        scheduler->timerPtr[ scheduler->timersCount ].callbackPtr = callbackPtr;
        scheduler->timerPtr[ scheduler->timersCount ].count = scheduler->timerPtr[ scheduler->timersCount ].timeout;
        scheduler->timerPtr[ scheduler->timersCount ].startFlag = FALSE;
        scheduler->timersCount++;
        timerID = scheduler->timersCount;
    }

    return timerID;
}

/**
 * @brief   Function to get the actual timer count.
 * 
 * This function help to know the actual count of the timer, to do that before check if the parameter timer
 * is valid, it means that is in the range between 1 and the actual timersCount value, if it's valid use an
 * auxiliary variable to return the count of the respective timer, if the timerID doesn't exist returns 0. 
 * 
 * @param   scheduler[in] Memory address of the scheduler to access the elements.
 * @param   timer[in] the timerID of the timer to get the count.
 * 
 * @retval  Returns the actual count of the timer if the timerID is valid, if not returns zero.
 * 
*/
unsigned long AppSched_getTimer( AppSched_Scheduler *scheduler, unsigned char timer )
{
    unsigned long retGtimer = 0;

    if ( ( timer > 0u ) && ( timer <= scheduler->timersCount ) )
    {
        retGtimer = scheduler->timerPtr[ timer - 1u ].count;
    }

    return retGtimer;
}

/**
 * @brief   Function to start the timer setting a new timeout.
 * 
 * This function set a new timeout value, first verify the timerID is valid and if the timeout is a multiple 
 * of tick, in a TRUE case set the new value in the elements count and timeout and start the timer setting
 * the startFlag to TRUE, in the FALSE case the function just return a zero.
 * 
 * 
 * @param   scheduler[in] Memory address of the scheduler to access the elements.
 * @param   timer[in] the timerID of the timer to get the count.
 * @param   timeout[in] the new value of the timer timeout.
 * 
 * @retval  Returns the success of the the function, TRUE when a new timeout was set and FALSE when not.
*/

unsigned char AppSched_reloadTimer( AppSched_Scheduler *scheduler, unsigned char timer, unsigned long timeout )
{
    unsigned char varRetReload = FALSE;

    if ( ( timer > 0u ) && ( timer <= scheduler->timersCount ) && ( ( timeout % scheduler->tick ) == 0u ) )
    {
        scheduler->timerPtr[ timer - 1u ].timeout = timeout;
        scheduler->timerPtr[ timer - 1u ].count = timeout;
        scheduler->timerPtr[ timer - 1u ].startFlag = TRUE;
        varRetReload = TRUE;
    }

    return varRetReload;
}

/**
 * @brief   Function to start the timer and reset the count.
 * 
 * To start the timer first check if the timerID is valid, if it is then set the value of the timeout
 * to the actual count and set the startFlag to TRUE, and if the timer doesn't exist it returns FALSE.
 * 
 * @param   scheduler[in] Memory address of the scheduler to access the elements.
 * @param   timer[in] the timerID of the timer to start.
 * 
 * @retval  Returns the success, TRUE if the timer was started and FALSE if the timer don't exist.
 * 
 * 
 * @note    This function is also used to restart the timer, using it in the callback function.
*/

unsigned char AppSched_startTimer( AppSched_Scheduler *scheduler, unsigned char timer )
{
    unsigned char retStart = FALSE;

    if ( ( timer > 0u ) && ( timer <= scheduler->timersCount ) )
    {
        scheduler->timerPtr[ timer - 1u ].count = scheduler->timerPtr[timer - 1u].timeout;
        scheduler->timerPtr[ timer - 1u ].startFlag = TRUE;
        retStart = TRUE;
    }

    return retStart;
}

/**
 * @brief   Function to stop a existing timer.
 * 
 * This function is used to set the startFlag to FALSE, it first check for the existence of the timer, if exists
 * it sets the flag and if not, it returns FALSE.
 * 
 * @param   scheduler[in] Memory address of the scheduler to access the elements.
 * @param   timer[in] the timerID of the timer to stop.
 * 
 * @retval  Returns the success, TRUE if the timer was stopped and FALSE if the timer don't exist.
 * 
*/
unsigned char AppSched_stopTimer( AppSched_Scheduler *scheduler, unsigned char timer )
{
    unsigned char retStop = FALSE;

    if ( ( timer > 0u ) && ( timer <= scheduler->timersCount ) )
    {
        scheduler->timerPtr[ timer - 1u ].startFlag = FALSE;
        retStop = TRUE;
    }

    return retStop;
}
