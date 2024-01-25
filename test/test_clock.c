/**
 * @file    test_clock.c
 * 
 * @brief   Unit tests for clock event machine.
*/
#include "unity.h"
#include "bsp.h"
#include "clock.h"

#include "mock_queue.h"
#include "mock_scheduler.h"
#include "mock_stm32g0xx_hal_rtc.h"

/**
 * @brief   reference to the Scheduler.
*/
AppSched_Scheduler Scheduler;
/**
 * @brief   reference to the timer ID.
*/
uint8_t UpdateTimerID;

/**
 * @brief   reference to the DisplayQueue.
*/
AppQue_Queue DisplayQueue;

/**
 * @brief   Function that runs before any unit test.
*/
void setUp( void )
{

}

/**
 * @brief   Function that runs after any unit test.
*/
void tearDown( void )
{

}

/** @brief Reference for the private function Update_Time. */
void Update_Time( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Update_Date.
*/
void Update_Date( APP_MsgTypeDef * );

/** @brief Reference for the private function Set_Alarm.
*/
void Set_Alarm( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Update_Display 
*/
void Send_Display_Msg( APP_MsgTypeDef * );

/**
 * @brief   Function to test the Clock_InitTask function.
 * 
 * This function ignores the initialization of the queue and mocks the HAL_RTC_Init, HAL_RTC_SetTime,
 * and HAL_RTC_SetDate functions to return HAL_OK.
 * The aim of this test is to know if all lines are executed in this function.
*/
void test__Clock_InitTask__( void )
{
    AppQueue_initQueue_Ignore( );
    HAL_RTC_Init_IgnoreAndReturn( HAL_OK );
    HAL_RTC_SetTime_IgnoreAndReturn( HAL_OK );
    HAL_RTC_SetDate_IgnoreAndReturn( HAL_OK );

    Clock_InitTask( );
}

/**
 * @brief   Test the ClockUpdate_Callback function.
 * 
 * The objective of this test is to check if all lines in this function are executed when it's called.
*/
void test__ClockUpdate_Callback( void )
{
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    AppSched_startTimer_ExpectAnyArgsAndReturn( TRUE );
    ClockUpdate_Callback( );
}

/**
 * @brief   test Clock_PeriodTask, queue with a time message.
 * 
 * It's defined message of type SERIAL_MSG_TIME.
 * Mock the function HIL_QUEUE_isQueueEmptyISR to return a FALSE value first, then a TRUE value.
 * Also mock the function HIL_QUEUE_readDataISR to return the defined message.
 * And finally call the function to ensure that the correct function is run. 
*/
void test__Clock_PeriodicTask__time_msg_case_TIME( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg = CLOCK_MSG_TIME;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_Messages ) );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );
    HAL_RTC_SetTime_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Clock_PeriodicTask( );
}

/**
 * @brief   test Clock_PeriodTask, queue with a date message.
 * 
 * It's defined message of type SERIAL_MSG_DATE.
 * Mock the function HIL_QUEUE_isQueueEmptyISR to return a FALSE value first, then a TRUE value.
 * Also mock the function HIL_QUEUE_readDataISR to return the defined message.
 * And finally call the function to ensure that the correct function is run. 
*/
void test__Clock_PeriodicTask__date_msg_case_DATE( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg = CLOCK_MSG_DATE;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_Messages ) );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );
    HAL_RTC_SetDate_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Clock_PeriodicTask( );
}

/**
 * @brief   test Clock_PeriodTask queue with a ALARM message.
 * 
 * It's defined message of type SERIAL_MSG_ALARM.
 * Mock the function HIL_QUEUE_isQueueEmptyISR to return a FALSE value first, then a TRUE value.
 * Also mock the function HIL_QUEUE_readDataISR to return the defined message.
 * And finally call the function to ensure that the correct function is run. 
*/
void test__Clock_PeriodicTask__alarm_msg_case_alarm( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg = CLOCK_MSG_ALARM;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_Messages ) );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );
    HAL_RTC_SetAlarm_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Clock_PeriodicTask( );
}

/**
 * @brief   test Clock_PeriodTask queue with a DISPLAY message.
 * 
 * It's defined message of type SERIAL_MSG_DISPLAY.
 * Mock the function HIL_QUEUE_isQueueEmptyISR to return a FALSE value first, then a TRUE value.
 * Also mock the function HIL_QUEUE_readDataISR to return the defined message.
 * And finally call the function to ensure that the correct function is run. 
*/
void test__Clock_PeriodicTask__display_msg_case_DISPLAY( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg = CLOCK_MSG_DISPLAY;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_Messages ) );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );
    HAL_RTC_GetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetAlarm_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Clock_PeriodicTask( );
}

/**
 * @brief   test Clock_PeriodTask queue with a unkown message.
 * 
 * It's defined message of type unkown.
 * Mock the function HIL_QUEUE_isQueueEmptyISR to return a FALSE value first, then a TRUE value.
 * Also mock the function HIL_QUEUE_readDataISR to return the defined message.
 * And finally call the function to ensure that any function is run. 
*/
void test__Clock_PeriodicTask__uknown_msg_dont_run_any_function( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg = 0xFF;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_Messages ) );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );

    Clock_PeriodicTask( );
}



/**
 * @brief   test Update_Time function.
*/
void test__Update_Time__return_IDLE_state( void )
{
    APP_MsgTypeDef msgReceived = {0};

    HAL_RTC_SetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Update_Time( &msgReceived );
}

/**
 * @brief   test Update_Date function.
 *  
*/
void test__Update_Date( void )
{
    APP_MsgTypeDef msgReceived = {0};

    HAL_RTC_SetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Update_Date( &msgReceived );
}

/**
 * @brief   test Set_Alarm function.
*/
void test__Set_Alarm( void )
{
    APP_MsgTypeDef msgReceived = {0};

    HAL_RTC_SetAlarm_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Set_Alarm( &msgReceived );
}

/**
 * @brief   test Send_Display_Msg function.
*/
void test__Send_Display_Msg( void )
{
    APP_MsgTypeDef msgReceived = {0};

    HAL_RTC_GetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetAlarm_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    Send_Display_Msg( &msgReceived );
}