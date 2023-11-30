#ifndef SCHEDULER_H_
#define SCHEDULER_H_

#define TRUE    1u
#define FALSE   0u

typedef struct _task
{
    unsigned long period;       //How often the task should run in ms
    unsigned long elapsed;      //the current elapsed time
    void (*initFunc)(void);     //pointer to init task function
    void (*taskFunc)(void);     //pointer to task function
    unsigned char runTask;      //indicate the task is stoped or not
}AppSched_Task;

typedef struct _AppSched_Timer
{
    unsigned long timeout;       /*!< timer timeout to decrement and reload when the timer is re-started */
    unsigned long count;         /*!< actual timer decrement count */
    unsigned char startFlag;     /*!< flag to start timer count */
    void(*callbackPtr)(void);  /*!< pointer to callback function function */
} AppSched_Timer;

typedef struct _scheduler
{
    unsigned char tasks;        //number of task to handle
    unsigned long tick;         //the time base in ms
    unsigned char tasksCount;   //internal task counter
    AppSched_Task *taskPtr;     //Pointer to buffer for the TCB tasks
    unsigned char timers;        /*number of software timer to use*/
    AppSched_Timer *timerPtr;       /*Pointer to buffer timer array*/
    unsigned char timersCount;
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