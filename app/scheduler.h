/**
 * @file scheduler.h
 * 
 * @brief Here is defined the Timer, Task and Scheduler structs, and the functions prototypes of
 * the scheduler.c file 
*/
#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#ifndef TEST_L
#define FOREVER() 1             /*!< MACRO defined with test purposes */  
#else 
extern unsigned char numLoops;
#define FOREVER() numLoops--
#endif

/** 
  * @defgroup BooleanValues This define are used to avoid magical nmumbers 0 and 1
  @{ */
#define TRUE    1u      /*!< Boolean value TRUE (1) */
#define FALSE   0u      /*!< Boolean value FALSE (0) */
/**
  @} */

/**
 * @brief This struct is the task control block (TCB).
*/
typedef struct _task
{
    unsigned long period;       /*!< How often the task should run in ms */
    unsigned long elapsed;      /*!< the current elapsed time */
    void (*initFunc)(void);     /*!< pointer to init task function */
    void (*taskFunc)(void);     /*!< pointer to task function */
    unsigned char runTask;      /*!< indicate the task is stopped or not*/
}AppSched_Task;

/**
 * @brief Struct to control the software timers.
*/
typedef struct _AppSched_Timer
{
    unsigned long timeout;       /*!< Timer timeout to decrement and reload when the timer is re-started. */
    unsigned long count;         /*!< Actual timer decrement count. */
    unsigned char startFlag;     /*!< Flag to start timer count. */
    void(*callbackPtr)(void);    /*!< Pointer to callback function function. */
} AppSched_Timer;

/**
 * @brief Struct to control the scheduler.
*/
typedef struct _scheduler
{
    unsigned char tasks;        /*!< Number of task to handle. */ 
    unsigned long tick;         /*!< The time base in ms. */
    unsigned char tasksCount;   /*!< Internal task counter. */ 
    AppSched_Task *taskPtr;     /*!< Pointer to buffer for the TCB tasks */
    unsigned char timers;       /*!< Number of software timer to use */ 
    AppSched_Timer *timerPtr;   /*!< Pointer to buffer timer array */    
    unsigned char timersCount;  /*!< Internal timer counter. */
}AppSched_Scheduler;


void AppSched_initScheduler( AppSched_Scheduler *scheduler);
unsigned char AppSched_registerTask( AppSched_Scheduler *scheduler, void (*initPtr)(void), void (*taskPtr)(void), unsigned long period );
unsigned char AppSched_stopTask( AppSched_Scheduler *scheduler, unsigned char task );
unsigned char AppSched_startTask( AppSched_Scheduler *scheduler, unsigned char task );
unsigned char AppSched_periodTask( AppSched_Scheduler *scheduler, unsigned char task, unsigned long period);
void AppSched_startScheduler( AppSched_Scheduler *scheduler );

unsigned char AppSched_registerTimer( AppSched_Scheduler *scheduler, unsigned long timeout, void (*callbackPtr)(void) );
unsigned long AppSched_getTimer( AppSched_Scheduler *scheduler, unsigned char timer );
unsigned char AppSched_reloadTimer( AppSched_Scheduler *scheduler, unsigned char timer, unsigned long timeout );
unsigned char AppSched_startTimer( AppSched_Scheduler *scheduler, unsigned char timer );
unsigned char AppSched_stopTimer( AppSched_Scheduler *scheduler, unsigned char timer );



#endif