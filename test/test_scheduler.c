/**
 * @file    test_scheduler.c
 * 
 * This file contains the test cases for fucntions of scheduler file.
*/
#include "unity.h"
#include "scheduler.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "bsp.h"

#include "mock_stm32g0xx_hal.h"

/**
 *  @brief  global variable to indicate the number of times that the while loop will be run.
*/
uint8_t numLoops;

/** @brief  Scheduler without registered tasks */
AppSched_Scheduler Sche;
/** @brief  Task control block for Sche*/
AppSched_Task tasks[ 2 ];
/** @brief  Timer control block for Sche*/
AppSched_Timer timers1 [ 2 ];

/** @brief  Scheduler with registered tasks */
AppSched_Scheduler ScheWithTask;
/** @brief  Task control block for ScheWithTask*/
AppSched_Task taskSche[ 4 ];

/** @brief  Scheduler with registered timers */
AppSched_Scheduler ScheWithTimer;
/** @brief  Task control block for ScheWithTimer*/
AppSched_Task taskSche2[2];
/** @brief  Timer control block for ScheWithTimer*/
AppSched_Timer timers2 [ 2 ];

/** @brief  Scheduler with registered and started timers */
AppSched_Scheduler ScheWithTimerStart;
/** @brief  Task control block for ScheWithTimerStart*/
AppSched_Task taskSche3[2];
/** @brief  Timer control block for ScheWithTimerStart*/
AppSched_Timer timersStart [ 7 ];

/** @brief  init function for registered tasks */
void InitTask01(void)
{
}

/** @brief  function for registered tasks */
void Task01(void)
{ 
}

/** @brief  function for registered tasks */
void Task02(void)
{
}

/** @brief  function for registered tasks */
void Task03(void)
{
}

/**
 * @brief   function that is executed before any unit test function.
 * 
 * In this block are the scheduler configurations and also here the tasks and timers are registered
 * in the schedulers that require it.
*/
void setUp(void)
{
    numLoops = 0;

    Sche.tasks = 2;
    Sche.tick = 100;
    Sche.taskPtr = tasks;
    Sche.timers = 2;
    Sche.timerPtr = timers1;
    AppSched_initScheduler( &Sche );

    /*Init a scheduler with two tasks*/
    ScheWithTask.tasks = 4;
    ScheWithTask.tick = 100;
    ScheWithTask.taskPtr = taskSche;
    AppSched_initScheduler( &ScheWithTask );
    AppSched_registerTask( &ScheWithTask, Task02, Task01, 200 );
    AppSched_registerTask( &ScheWithTask, NULL, Task02, 100 );
    AppSched_registerTask( &ScheWithTask, NULL, Task02, 500 );
    AppSched_registerTask( &ScheWithTask, NULL, Task02, 1000 );
    AppSched_stopTask( &ScheWithTask, 2 );

    /*Init a scheduler with a timer*/
    ScheWithTimer.tasks = 1;
    ScheWithTimer.tick = 100;
    ScheWithTimer.taskPtr = taskSche2;
    ScheWithTimer.timers = 2;
    ScheWithTimer.timerPtr = timers2;
    AppSched_initScheduler( &ScheWithTimer );
    AppSched_registerTimer( &ScheWithTimer, 1000, Task01 );

    /*Init a scheduler with a timer started*/
    ScheWithTimerStart.tasks = 2;
    ScheWithTimerStart.tick = 100;
    ScheWithTimerStart.taskPtr = taskSche3;

    ScheWithTimerStart.timers = 7;
    ScheWithTimerStart.timerPtr = timersStart;

    AppSched_initScheduler( &ScheWithTimerStart );
    AppSched_registerTimer( &ScheWithTimerStart, 100, Task01 );
    AppSched_registerTimer( &ScheWithTimerStart, 2000, Task01 );
    AppSched_registerTimer( &ScheWithTimerStart, 100, NULL );
    AppSched_registerTimer( &ScheWithTimerStart, 9000, NULL );
    AppSched_registerTimer( &ScheWithTimerStart, 100, NULL );
    AppSched_registerTask( &ScheWithTimerStart, InitTask01, Task03, 500 );
    AppSched_registerTask( &ScheWithTimerStart, NULL, Task02, 500 );
    AppSched_stopTask( &ScheWithTimerStart, 1 );

    AppSched_startTimer( &ScheWithTimerStart, 1 );
    AppSched_startTimer( &ScheWithTimerStart, 2 );
    AppSched_startTimer( &ScheWithTimerStart, 4 );
    AppSched_startTimer( &ScheWithTimerStart, 5 );
}

/**
 * @brief   function that is executed after any unit test function.
*/
void tearDown(void)
{
    
}

/**
 * @brief   test AppSched_initScheduler initialize a scheduler without task and timers
 * 
 * In this test just the timers and tasks counters are evaluate with the assertion TEST_ASSERT_EQUAL
 * to check that after using the function there aren't registered tasks or timers.
*/
void test__AppSched_initScheduler__scheduler_without_registered_tasks_and_timers(void)
{
    AppSched_initScheduler( &Sche );

    TEST_ASSERT_EQUAL( 0, Sche.tasksCount );
    TEST_ASSERT_EQUAL( 0, Sche.timersCount );
}

/**
 * @brief   test AppSched_registerTask register first task in the scheduler.
 * 
 * The function returned the task ID and this value shall be 1 because is the first registered task
 * in the scheduler, this value is checked with TEST_ASSSERT_EQUAL assertion.
*/
void test__AppSched_registerTask__register_a_Task_check_taskID(void)
{
    uint8_t taskID;

    taskID = AppSched_registerTask( &Sche, InitTask01, Task01, 500 );   //TEST TaskID = 1 -> First task registered

    TEST_ASSERT_EQUAL( 1, taskID ); 
}

/**
 * @brief   test AppSched_registerTask check runTask flag.
 * 
 * After registering a task your runTask flag shall be True, using TEST_ASSERT_TRUE the flag
 * is tested, accesing it trought the taskPtr of the scheduler.
*/
void test__AppSched_registerTask__register_a_Task_check_startFlag(void)
{
    uint8_t taskID;

    taskID = AppSched_registerTask( &Sche, InitTask01, Task01, 500 );

    TEST_ASSERT_TRUE( Sche.taskPtr[ taskID - 1 ].runTask ); 
}

/**
 * @brief   test AppSched_registerTask register a task with not valid periodicity.
 * 
 * If the task periodicity isn't a multiple of scheduler tick value the function return False.
 * The Sche tick value is 100ms then in this test is trying to register a task with periodicity 
 * of 150ms. the returned value is storaged in varRet then this variable shall be False.
*/
void test__AppSched_registerTask__register_a_Task_period_noValid(void)
{
    uint8_t varRet;

    varRet = AppSched_registerTask( &Sche, NULL, Task02, 150 ); //Return 0, period no valid, is not a multiple of Sche.tick=100

    TEST_ASSERT_FALSE( varRet );      
}

/**
 * @brief   test AppSched_registerTask register two tasks and check taskCounter value.
 * 
 * After registering two tasks succesfully the taskCounter value shall be two, this value is checked
 * using TEST_ASSERT_EQUAL.
*/
void test__AppSched_registerTask__register_two_tasks_check_taskCounter(void)
{
    /*Register two tasks*/
    AppSched_registerTask( &Sche, InitTask01, Task01, 500 );
    AppSched_registerTask( &Sche, NULL, Task02, 1000 );

    TEST_ASSERT_EQUAL( 2, Sche.tasksCount );    
}

/**
 * @brief   test AppSched_stopTask Stop an existing task and check returned value (True).
 * 
 * The ScheWithTask has already initialized and has a registered task, then the function is called
 * with a taskID of 1, after this check the returned value that is true.
*/
void test__AppSched_stopTask__stop_a_existing_task_check_returned_value_True(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTask( &ScheWithTask, 1 );  //Function should return true, if task exists

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test AppSched_stopTask Stop an existing task and check runTask flag (False).
 * 
 * The ScheWithTask has already initialized and has a registered task, then the function is called
 * with a taskID of 1, after this check the runTask flag of the stopped task that is False.
*/
void test__AppSched_stopTask__stop_a_existing_task_check_runTask_flag_False(void)
{
    AppSched_stopTask( &ScheWithTask, 1 );  //Function should return true, if task exists

    TEST_ASSERT_FALSE( ScheWithTask.taskPtr[0].runTask );        //runTask flag should be false after stop the task
}

/**
 * @brief   test AppSched_stopTask stop a non-existing task and check returned value (False).
 * 
 * The Sche has no registered tasks, the function is called to stop a non-existing task then
 * the value returned is False.
*/
void test__AppSched_stopTask__stop_a_non_existing_task(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTask( &Sche, 1 );  //Function should return False, if task doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_stopTask stop a non-existing task and check returned value (False).
 * 
 * The Sche has no registered tasks, the function is called to stop a task with a not valid task ID
 * then the value returned is False.
*/
void test__AppSched_stopTask__stop_a_non_existing_task_0(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTask( &Sche, 0 );  //Function should return False, if task doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_startTask trying to start a task with no valid task ID.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask, in this test the function
 * is called with a taskID of 3, this is not valid then the value returned is False.
*/
void test__AppSched_starTask__start_a_stopped_task_noValid_taskID(void)
{
    uint8_t varRet;

    varRet = AppSched_startTask( &ScheWithTask, 10 );    //this scheduler just have 2 task, timerID=3 doesn't exist

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_startTask trying to start a task with no valid task ID.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask, in this test the function
 * is called with a taskID of 0, this is not valid then the value returned is False.
*/
void test__AppSched_starTask__start_a_stopped_task_taskID_0(void)
{
    uint8_t varRet;

    varRet = AppSched_startTask( &ScheWithTask, 0 );    //timerID = 0 its not valid.

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_startTask trying to start a task with valid task ID.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask, in this test the function
 * is called with a taskID of 2, then the value returned is True.
*/
void test__AppSched_starTask__start_a_stopped_task_valid_taskID(void)
{
    uint8_t varRet;

    varRet = AppSched_startTask( &ScheWithTask, 2 );    //the task 2 has already stopped, with this fcn start again

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test AppSched_periodTask trying to change periodicity of a not valid task ID.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask, in this test the function
 * is called with a taskID of 3, this is not valid then the value returned is False.
*/
void test__AppSched_periodTask__change_period_non_existing_task(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 10, 300 );

    TEST_ASSERT_FALSE(varRet);
}

/**
 * @brief   test AppSched_periodTask trying to change periodicity of a not valid task ID.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask, in this test the function
 * is called with a taskID of 0, this is not valid then the value returned is False.
*/
void test__AppSched_periodTask__change_period_non_existing_task_0(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 0, 300 );//Function should return false, if task doesn't exist

    TEST_ASSERT_FALSE(varRet);
}

/**
 * @brief   test AppSched_periodTask trying to change periodicity with a no multiple value.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask and the scheduler has a tick
 * value of 100, in this test the function is called with a taskID of 1 and a new periodicity of 130,
 * that isn't a multiple of the tick value, then the returned value is False.
*/
void test__AppSched_periodTask__change_period_no_multiple(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 1, 130 );  //Function should return false, if timeout isn't a multiple of period

    TEST_ASSERT_FALSE(varRet);
}

/**
 * @brief   test AppSched_periodTask change task periodicity.
 * 
 * In the function setUp was registered two tasks in the ScheWithTask and the scheduler has a tick
 * value of 100, in this test the function is called with a taskID of 1 and a new periodicity of 300,
 * that is a multiple of the tick value, then the returned value is True.
*/
void test__AppSched_periodTask__change_period_valid_arguments(void)
{
    uint8_t varRet;

    varRet = AppSched_periodTask( &ScheWithTask, 1, 300 );  //Function should return true, for valid task and period

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test AppSched_registerTimer registering a timer with a not valid timeout.
 * 
 * The Sche scheduler was initialized in the setUp funtion with a tick value of 100, the function to
 * test is called with a timeout parameter of 550, that it isn't a multiple of tick, then the value
 * returned is False.
*/
void test__AppSched_registerTimer__register_a_timer_no_valid_timeout(void)
{
    uint8_t varRet;

    varRet = AppSched_registerTimer( &Sche, 550, Task01 );   //return False the timeout isn't a multiple of tick

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_registerTimer registering a timer with a valid timeout.
 * 
 * The Sche scheduler was initialized in the setUp funtion with a tick value of 100, the function to
 * test is called with a timeout parameter of 1000, that it is a multiple of tick, then the value
 * returned is 1 that is the ID of the registered timer.
*/
void test__AppSched_registerTimer__register_a_timer_valid_timeout(void)
{
    uint8_t varRet;

    varRet = AppSched_registerTimer( &Sche, 1000, Task01 );  

    TEST_ASSERT_EQUAL( 1, varRet );
}

/**
 * @brief   test AppSched_getTimer get count from no valid timerID.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer, the function to test is called
 * with a timer ID of 3 this timer doesn't exist then the returned value is zero.
*/
void test__AppSched_getTimer__get_count_timer_noValid_timerID(void)
{
    unsigned long timeout;

    timeout = AppSched_getTimer( &ScheWithTimer, 3 );

    TEST_ASSERT_EQUAL( 0, timeout );      
}

/**
 * @brief   test AppSched_getTimer get count from no valid timerID.
 * 
 * In the function setUp was registered 1 timer in the ScheWithTimer, the function to test is called
 * with a timer ID of 0 this ID isn't valid then the returned value is zero.
*/
void test__AppSched_getTimer__get_count_timer_timerID_0(void)
{
    unsigned long timeout;

    timeout = AppSched_getTimer( &ScheWithTimer, 0 );     //return 0 for no valid timerID 

    TEST_ASSERT_EQUAL( 0, timeout );      
}

/**
 * @brief   test AppSched_getTimer get count from valid timerID.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer, the function to test is called
 * with a timer ID of 1 this ID, this timer was registered with a timeout of 1000, then the returned 
 * value shall be the same because the scheduler has not been initialized.
*/
void test__AppSched_getTimer__get_count_timer_valid_timerID(void)
{
    unsigned long timeout;

    timeout = AppSched_getTimer( &ScheWithTimer, 1 );

    TEST_ASSERT_EQUAL( 1000, timeout );   //the actual count is the timeout
}

/**
 * @brief   test AppSched_reloadTimer trying with a not valid timer ID.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 2 this timer doesn't exist, the returned value is False.
*/
void test__AppSched_reloadTimer__reload_timer_noValid_ID(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 2, 1000 ); //no valid timerID, return false

    TEST_ASSERT_FALSE( varRet );     
}

/**
 * @brief   test AppSched_reloadTimer trying with a not valid timeout.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer and the scheduler has
 * a tick value of 100, the function to test is called with a timer ID of 1 and a timeout of
 * 1250 that isn't tick multiple the returned value is False.
*/
void test__AppSched_reloadTimer__reload_timer_noValid_timeout(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 1, 1250 );    //no valid new timeout, return false

    TEST_ASSERT_FALSE( varRet );  
}

/**
 * @brief   test AppSched_reloadTimer trying with a not valid timer ID.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 0 this ID isn't valid, the returned value is False.
*/
void test__AppSched_reloadTimer__reload_timer_zeroID(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 0, 500 );    //no valid new timeout, return false

    TEST_ASSERT_FALSE( varRet );  
}

/**
 * @brief   test AppSched_reloadTimer reaload a existing timer.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 2 this timer doesn't exist, the returned value is False.
*/
void test__AppSched_reloadTimer__reload_timer_valid_arguments_check_new_timeout(void)
{
    AppSched_reloadTimer( &ScheWithTimer, 1, 500 );    //valid parameters return true
 
    TEST_ASSERT_EQUAL( 500, ScheWithTimer.timerPtr[0].timeout ); //New timeout == 500
}

/**
 * @brief   test AppSched_reloadTimer reaload a existing timer check returned value.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 1 and a timeout of 500, this parameters are valid, and the returned value is True.
*/
void test__AppSched_reloadTimer__reload_timer_valid_arguments_check_returned_value_True(void)
{
    uint8_t varRet;

    varRet = AppSched_reloadTimer( &ScheWithTimer, 1, 500 );    //valid parameters return true

    TEST_ASSERT_TRUE( varRet );       
}

/**
 * @brief   test AppSched_reloadTimer reaload a existing timer check new timeout.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 1 and a timeout of 500, this parameters are valid, and the new timeout is tested
 * accesing to it directly in the timerPtr and this value shall be 500.
*/
void test__AppSched_reloadTimer__reload_timer_valid_arguments_check_new_timeout_return_500(void)
{
    AppSched_reloadTimer( &ScheWithTimer, 1, 500 );   
  
    TEST_ASSERT_EQUAL( 500, ScheWithTimer.timerPtr[0].timeout ); //New timeout == 500
}

/**
 * @brief   test AppSched_startTimer start a not valid timer ID.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 2 this timer doesn't exist, the returned value is False.
*/
void test__AppSched_startTimer__start_Timer_noValid_timerID(void)
{
    uint8_t retVar;

    retVar = AppSched_startTimer( &ScheWithTimer, 2 );   //no valid timerID, return false

    TEST_ASSERT_FALSE( retVar );     
}

/**
 * @brief   test AppSched_startTimer start a not valid timer ID.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 0 this timer doesn't exist, the returned value is False.
*/
void test__AppSched_startTimer__start_Timer_timerID_0(void)
{
    uint8_t retVar;

    retVar = AppSched_startTimer( &ScheWithTimer, 0 );   //no valid timerID, return false

    TEST_ASSERT_FALSE( retVar );     
}

/**
 * @brief   test AppSched_startTimer start a existing timer, check returned value.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 1, this parameters are valid, and the returned value is True.
*/
void test__AppSched_startTimer__start_Timer_valid_timerID_check_returned_value_True(void)
{
    uint8_t retVar;

    retVar = AppSched_startTimer( &ScheWithTimer, 1 );   //valid ID return true

    TEST_ASSERT_TRUE( retVar );
}

/**
 * @brief   test AppSched_startTimer start a existing timer, check start flag.
 * 
 * In the function setUp was registered one timer in the ScheWithTimer the function to test is called
 * with a timer ID of 1, this parameters are valid, and the startFlag value is True.
*/
void test__AppSched_startTimer__start_Timer_valid_timerID_check_startFlag_True(void)
{
    AppSched_startTimer( &ScheWithTimer, 1 );   //valid ID return true

    TEST_ASSERT_TRUE( ScheWithTimer.timerPtr[0].startFlag );    //check start flag after starTimer function
}

/**
 * @brief   test AppSched_stopTimer stop a non-existing timer, check returned value.
 * 
 * In the function setUp was registered one timer in the ScheWithTimerStart the function to test is
 * called with a timer ID of 5 this timer doesn't exist, the returned value is False.
*/
void test__AppSched_stopTimer__stop_Timer_noValid_timerID(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTimer( &ScheWithTimerStart, 10 );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_stopTimer stop a non-existing timer, check returned value.
 * 
 * In the function setUp was registered one timer in the ScheWithTimerStart the function to test is
 * called with a timer ID of 0 this timer doesn't exist, the returned value is False.
*/
void test__AppSched_stopTimer__stop_Timer_timerID_0(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTimer( &ScheWithTimerStart, 0 );    //no valid timerID, return false

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppSched_stopTimer stop a registered timer and check the returned value.
 * 
 * In the function setUp was registered and started one timer in the ScheWithTimerStart the function
 * to test is called with a timer ID of 1 this timer exists, the returned value is True.
*/
void test__AppSched_stopTimer__stop_Timer_valid_arguments_check_returned_value_True(void)
{
    uint8_t varRet;

    varRet = AppSched_stopTimer( &ScheWithTimerStart, 1 );    //valid ID return true

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test AppSched_stopTimer stop a registered timer and check the startFlag.
 * 
 * In the function setUp was registered and started one timer in the ScheWithTimerStart the function
 * to test is called with a timer ID of 1 this timer exists, the value of the startFlag is false.
*/
void test__AppSched_stopTimer__stop_Timer_valid_arguments_check_startFlag_False(void)
{
    AppSched_stopTimer( &ScheWithTimerStart, 1 );   

    TEST_ASSERT_FALSE( ScheWithTimerStart.timerPtr[0].startFlag  );  //check start flag after stopTimer function
}

/**
 * @brief   test AppSched_startScheduler start ScheWithTimerStart
 * 
*/
void test__AppSched_startScheduler__start_scheduler_and_reach_timeout_timer( void )
{
    HAL_GetTick_IgnoreAndReturn( 100 );
    HAL_GetTick_IgnoreAndReturn( 200 );

    numLoops = 8;

    AppSched_startScheduler( &ScheWithTimerStart );

    TEST_ASSERT_FALSE( ScheWithTimerStart.timerPtr[2].startFlag ); 
}

/**
 * @brief   test AppSched_startScheduler start ScheWithTimerStart
 * 
*/
void test__AppSched_startScheduler__start_scheduler_and_reach_timeout_task( void )
{
    HAL_GetTick_IgnoreAndReturn( 100 );
    HAL_GetTick_IgnoreAndReturn( 200 );
    HAL_GetTick_IgnoreAndReturn( 300 );
    HAL_GetTick_IgnoreAndReturn( 400 );
    HAL_GetTick_IgnoreAndReturn( 500 );
    HAL_GetTick_IgnoreAndReturn( 600 );

    numLoops = 8;

    AppSched_startScheduler( &ScheWithTask );
}