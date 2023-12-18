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

void setUp( void )
{

}

void tearDown( void )
{

}

void test__Clock_InitTask__( void )
{
    AppQueue_initQueue_Ignore( );
    HAL_RTC_Init_IgnoreAndReturn( HAL_OK );
    HAL_RTC_SetTime_IgnoreAndReturn( HAL_OK );
    HAL_RTC_SetDate_IgnoreAndReturn( HAL_OK );

    Clock_InitTask( );
}

void test__ClockUpdate_Callback( void )
{
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    AppSched_startTimer_ExpectAnyArgsAndReturn( TRUE );
    ClockUpdate_Callback( );
}

STATIC ClkState Evaluate_Msg( const APP_MsgTypeDef *PtrMsgClk );

void test__Evaluate_Msg__SERIAL_MSG_TIME_return_time_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    msgReceived.msg = SERIAL_MSG_TIME;

    retState = Evaluate_Msg( &msgReceived );

    TEST_ASSERT_EQUAL( retState, TIME );
}

void test__Evaluate_Msg__SERIAL_MSG_DATE_return_date_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    msgReceived.msg = SERIAL_MSG_DATE;

    retState = Evaluate_Msg( &msgReceived );

    TEST_ASSERT_EQUAL( retState, DATE );
}

void test__Evaluate_Msg__SERIAL_MSG_ALARM_return_alarm_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    msgReceived.msg = SERIAL_MSG_ALARM;

    retState = Evaluate_Msg( &msgReceived );

    TEST_ASSERT_EQUAL( retState, ALARM );
}

void test__Evaluate_Msg__SERIAL_MSG_DISPLAY_return_display_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    msgReceived.msg = SERIAL_MSG_DISPLAY;

    retState = Evaluate_Msg( &msgReceived );

    TEST_ASSERT_EQUAL( retState, DISPLAY );
}

void test__Evaluate_Msg__default_case_return_idle_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    msgReceived.msg = 0xFF;     /*default value*/

    retState = Evaluate_Msg( &msgReceived );

    TEST_ASSERT_EQUAL( retState, IDLE );
}

STATIC ClkState Update_Time( APP_MsgTypeDef *PtrMsgClk );

void test__Update_Time__return_IDLE_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    HAL_RTC_SetTime_ExpectAnyArgsAndReturn( HAL_OK );

    retState = Update_Time( &msgReceived );

    TEST_ASSERT_EQUAL( retState, IDLE );
}

STATIC ClkState Update_Date( APP_MsgTypeDef *PtrMsgClk );

void test__Update_Date__return_IDLE_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    HAL_RTC_SetDate_ExpectAnyArgsAndReturn( HAL_OK );

    retState = Update_Date( &msgReceived );

    TEST_ASSERT_EQUAL( retState, IDLE );
}

STATIC ClkState Set_Alarm( APP_MsgTypeDef *PtrMsgClk );

void test__Set_Alarm__return_IDLE_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    HAL_RTC_SetAlarm_ExpectAnyArgsAndReturn( HAL_OK );

    retState = Set_Alarm( &msgReceived );

    TEST_ASSERT_EQUAL( retState, IDLE );    
}

STATIC ClkState Update_Display( APP_MsgTypeDef *PtrMsgClk );

void test__Update_Display__return_IDLE_state( void )
{
    APP_MsgTypeDef msgReceived = {0};
    ClkState retState;

    HAL_RTC_GetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetAlarm_ExpectAnyArgsAndReturn( HAL_OK );

    retState = Update_Display( &msgReceived );

    TEST_ASSERT_EQUAL( retState, IDLE );   
}