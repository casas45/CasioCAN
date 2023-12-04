#include "unity.h"
#include "scheduler.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bsp.h"

#include "mock_stm32g0xx_hal.h"

AppSched_Task tasks[ 2 ];
AppSched_Scheduler Sche;
AppSched_Timer timers1 [ 2 ];

AppSched_Task taskSche[2];
AppSched_Scheduler ScheWithTask;

AppSched_Task taskSche2[2];
AppSched_Scheduler ScheWithTimer;
AppSched_Timer timers2 [ 2 ];

AppSched_Task taskSche3[2];
AppSched_Scheduler ScheWithTimerStart;
AppSched_Timer timersStart [ 2 ];

void InitTask01(void)
{
}

void Task01(void)
{ 
}

void Task02(void)
{
}

void Task03(void)
{
    printf("printf task03\n");
}


void setUp(void)
{
    Sche.tasks = 2;
    Sche.tick = 100;
    Sche.taskPtr = tasks;
    Sche.timers = 2;
    Sche.timerPtr = timers1;
    AppSched_initScheduler( &Sche );

    /*Define a scheduler with two tasks*/
    ScheWithTask.tasks = 1;
    ScheWithTask.tick = 100;
    ScheWithTask.taskPtr = taskSche;
    AppSched_initScheduler( &ScheWithTask );
    AppSched_registerTask( &ScheWithTask, NULL, Task01, 500 );
    AppSched_registerTask( &ScheWithTask, NULL, Task02, 500 );
    AppSched_stopTask( &ScheWithTask, 2 );

    /*Define a scheduler with a timer*/
    ScheWithTimer.tasks = 1;
    ScheWithTimer.tick = 100;
    ScheWithTimer.taskPtr = taskSche2;
    ScheWithTimer.timers = 2;
    ScheWithTimer.timerPtr = timers2;
    AppSched_initScheduler( &ScheWithTimer );
    AppSched_registerTimer( &ScheWithTimer, 1000, Task01 );

    /*Define a scheduler with a timer started*/
    ScheWithTimerStart.tasks = 2;
    ScheWithTimerStart.tick = 100;
    ScheWithTimerStart.taskPtr = taskSche3;

    ScheWithTimerStart.timers = 2;
    ScheWithTimerStart.timerPtr = timersStart;

    AppSched_initScheduler( &ScheWithTimerStart );
    AppSched_registerTimer( &ScheWithTimerStart, 1000, Task01 );
    AppSched_registerTimer( &ScheWithTimerStart, 1000, NULL );
    AppSched_startTimer( &ScheWithTimerStart, 1 );
    AppSched_startTimer( &ScheWithTimerStart, 2 );
    AppSched_registerTask( &ScheWithTimerStart, InitTask01, Task03, 500 );
    AppSched_registerTask( &ScheWithTimerStart, NULL, Task02, 500 );
    
    HAL_GetTick_IgnoreAndReturn( 100 );
}

void tearDown(void)
{
    
}


void test__AppSched_initScheduler__scheduler(void)
{
    AppSched_initScheduler( &Sche );

    TEST_ASSERT_EQUAL( 0, Sche.tasksCount );
    TEST_ASSERT_EQUAL( 0, Sche.timersCount );
}

void test__AppSched_registerTask__register_a_Task_check_taskID(void)
{
    uint8_t taskID;

    taskID = AppSched_registerTask( &Sche, InitTask01, Task01, 500 );   //TEST TaskID = 1 -> First task registered

    TEST_ASSERT_EQUAL( 1, taskID ); 
}

void test__AppSched_registerTask__register_a_Task_check_startFlag(void)
{
    uint8_t taskID;

    taskID = AppSched_registerTask( &Sche, InitTask01, Task01, 500 );

    TEST_ASSERT_TRUE( Sche.taskPtr[ taskID - 1 ].runTask ); 
}

void test__AppSched_registerTask__register_a_Task_period_noValid(void)
{
    uint8_t varRet;

    varRet = AppSched_registerTask( &Sche, NULL, Task02, 150 ); //Return 0, period no valid, is not a multiple of Sche.tick=100

    TEST_ASSERT_FALSE( varRet );      
}

void test__AppSched_registerTask__register_two_tasks_check_taskCounter(void)
{
    /*Register two tasks*/
    AppSched_registerTask( &Sche, InitTask01, Task01, 500 );
    AppSched_registerTask( &Sche, NULL, Task02, 1000 );

    TEST_ASSERT_EQUAL( 2, Sche.tasksCount );    
}

void test__AppSched_stopTask__stop_a_existing_task(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTask( &ScheWithTask, 1 );  //Function should return true, if task exists

    TEST_ASSERT_TRUE( varRet );
    TEST_ASSERT_EQUAL( false, ScheWithTask.taskPtr[0].runTask );        //runTask flag should be false after stop the task
}

void test__AppSched_stopTask__stop_a_non_existing_task(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTask( &Sche, 1 );  //Function should return False, if task doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

void test__AppSched_stopTask__stop_a_non_existing_task_0(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTask( &Sche, 0 );  //Function should return False, if task doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

void test__AppSched_stopTask__stop_existing_task_check_runFlag(void)
{
    AppSched_stopTask( &ScheWithTask, 1 );      //stop the task

    TEST_ASSERT_FALSE( ScheWithTask.taskPtr[0].runTask );        //runTask flag should be false after stop the task
}

void test__AppSched_starTask__start_a_stopped_task_noValid_taskID(void)
{
    uint8_t varRet;

    varRet = AppSched_startTask( &ScheWithTask, 3 );    //this scheduler just have 2 task, timerID=3 doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

void test__AppSched_starTask__start_a_stopped_task_taskID_0(void)
{
    uint8_t varRet;

    varRet = AppSched_startTask( &ScheWithTask, 0 );    //this scheduler just have 2 task, timerID=3 doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

void test__AppSched_starTask__start_a_stopped_task_valid_taskID(void)
{
    uint8_t varRet;

    varRet = AppSched_startTask( &ScheWithTask, 2 );    //the task 2 has already stopped, with this fcn start again

    TEST_ASSERT_TRUE( varRet );
}

void test__AppSched_periodTask__change_period_non_existing_task(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 3, 300 );//Function should return false, if task doesn't exist

    TEST_ASSERT_FALSE(varRet);
}

void test__AppSched_periodTask__change_period_non_existing_task_0(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 0, 300 );//Function should return false, if task doesn't exist

    TEST_ASSERT_FALSE(varRet);
}

void test__AppSched_periodTask__change_period_no_multiple(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 1, 130 );  //Function should return false, if timeout isn't a multiple of period

    TEST_ASSERT_FALSE(varRet);
}


void test__AppSched_periodTask__change_period_valid_arguments(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 1, 300 );  //Function should return true, for valid task and period

    TEST_ASSERT_TRUE( varRet );
}

void test__AppSched_registerTimer__register_a_timer_no_valid_timeout(void)
{
    uint8_t varRet;

    varRet = AppSched_registerTimer( &Sche, 550, Task01 );   //return False the timeout isn't a multiple of tick

    TEST_ASSERT_FALSE( varRet );
}


void test__AppSched_registerTimer__register_a_timer_valid_timeout(void)
{
    uint8_t varRet;

    varRet = AppSched_registerTimer( &Sche, 1000, Task01 );   //return TimerID = 1 the timeout is valid

    TEST_ASSERT_EQUAL( 1, varRet );
}

void test__AppSched_getTimer__get_count_timer_noValid_timerID(void)
{
    unsigned long timeout;

    timeout = AppSched_getTimer( &ScheWithTimer, 3 );     //return 0 for no valid timerID 

    TEST_ASSERT_EQUAL( 0, timeout );      
}

void test__AppSched_getTimer__get_count_timer_timerID_0(void)
{
    unsigned long timeout;

    timeout = AppSched_getTimer( &ScheWithTimer, 0 );     //return 0 for no valid timerID 

    TEST_ASSERT_EQUAL( 0, timeout );      
}

void test__AppSched_getTimer__get_count_timer_valid_timerID(void)
{
    unsigned long timeout;

    timeout = AppSched_getTimer( &ScheWithTimer, 1 );

    TEST_ASSERT_EQUAL( 1000, timeout );   //the actual count is the timeout
}

void test__AppSched_reloadTimer__reload_timer_noValid_ID(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 2, 1000 ); //no valid timerID, return false

    TEST_ASSERT_FALSE( varRet );     
}

void test__AppSched_reloadTimer__reload_timer_noValid_timeout(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 1, 1250 );    //no valid new timeout, return false

    TEST_ASSERT_FALSE( varRet );  
}

void test__AppSched_reloadTimer__reload_timer_zeroID(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 0, 500 );    //no valid new timeout, return false

    TEST_ASSERT_FALSE( varRet );  
}

void test__AppSched_reloadTimer__reload_timer_valid_arguments(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 1, 500 );    //valid parameters return true

    TEST_ASSERT_TRUE( varRet );       
    TEST_ASSERT_EQUAL( 500, ScheWithTimer.timerPtr[0].timeout ); //New timeout == 500
}

void test__AppSched_startTimer__start_Timer_noValid_timerID(void)
{
    uint8_t retVar;

    retVar = AppSched_startTimer( &ScheWithTimer, 2 );   //no valid timerID, return false

    TEST_ASSERT_FALSE( retVar );     
}

void test__AppSched_startTimer__start_Timer_timerID_0(void)
{
    uint8_t retVar;

    retVar = AppSched_startTimer( &ScheWithTimer, 0 );   //no valid timerID, return false

    TEST_ASSERT_FALSE( retVar );     
}

void test__AppSched_startTimer__start_Timer_valid_timerID(void)
{
    uint8_t retVar;

    retVar = AppSched_startTimer( &ScheWithTimer, 1 );   //valid ID return true

    TEST_ASSERT_TRUE( retVar );
    TEST_ASSERT_TRUE( ScheWithTimer.timerPtr[0].startFlag );    //check start flag after starTimer function
}

void test__AppSched_stopTimer__stop_Timer_noValid_timerID(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTimer( &ScheWithTimerStart, 5 );    //no valid timerID, return false

    TEST_ASSERT_FALSE( varRet );
}

void test__AppSched_stopTimer__stop_Timer_timerID_0(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTimer( &ScheWithTimerStart, 0 );    //no valid timerID, return false

    TEST_ASSERT_FALSE( varRet );
}

void test__AppSched_stopTimer__stop_Timer_valid_arguments(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTimer( &ScheWithTimerStart, 1 );    //valid ID return true

    TEST_ASSERT_TRUE( varRet );
    TEST_ASSERT_FALSE( ScheWithTimerStart.timerPtr[0].startFlag  );  //check start flag after stopTimer function
}

/*
void test__AppSched_starScheduler__start_a_basic_scheduler_with_a_stopped_task(void)
{
    TEST_ASSERT_TRUE( AppSched_stopTask( &ScheWithTimerStart, 1 ) );

    AppSched_startScheduler( &ScheWithTimerStart );
    
    TEST_ASSERT_FALSE( ScheWithTimerStart.taskPtr[0].runTask );                                 
}
*/