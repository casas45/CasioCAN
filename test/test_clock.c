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
#include "mock_stm32g0xx_hal_tim.h"
#include "mock_stm32g0xx_hal_gpio.h"
#include "mock_stm32g0xx_hal_cortex.h"
#include "mock_hel_lcd.h"

/**
 * @brief   reference to the Scheduler.
*/
AppSched_Scheduler Scheduler;

/**
 * @brief   reference to the LCD Handler.
*/
LCD_HandleTypeDef LCD_Handler;

/**
 * @brief   reference to the timer ID.
*/
uint8_t UpdateTimerID;

/** @brief  reference to the TimerAlarmActiveOneSecond_ID */
uint8_t TimerAlarmActiveOneSecond_ID;

/** @brief  reference to the TimerAlarmActiveOneMinute_ID */
uint8_t TimerDeactivateAlarm_ID;

/**
 * @brief   Alarm Activated Flag reference.
*/
extern uint8_t AlarmActivated_flg;

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

/** 
 * @brief   Reference for the private function Clock_Set_Time. 
 * @return  Message with the next event.
 * */
APP_MsgTypeDef Clock_Set_Time( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Clock_Set_Date.
 * @return  Message with the next event.
*/
APP_MsgTypeDef Clock_Set_Date( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Clock_Set_Alarm.
 * @return  Message with the next event.
*/
APP_MsgTypeDef Clock_Set_Alarm( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Update_Display 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Clock_Send_Display_Msg( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Clock_Alarm_Activated.
 * @return  Message with the next event.
*/
APP_MsgTypeDef Clock_Alarm_Activated( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Clock_Deactivate_Alarm. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Clock_Deactivate_Alarm( APP_MsgTypeDef * );

/**
 * @brief   Function to test the Clock_InitTask function.
 * 
 * This function ignores the initialization of the queue and mocks the HAL_RTC_Init, HAL_RTC_SetTime,
 * and HAL_RTC_SetDate functions to return HAL_OK.
 * The aim of this test is to know if all lines are executed in this function.
*/
void test__Clock_InitTask__( void )
{
    HAL_GPIO_Init_Ignore( );
    HAL_TIM_PWM_Init_IgnoreAndReturn( HAL_OK );
    HAL_TIM_PWM_ConfigChannel_IgnoreAndReturn( HAL_OK );
    AppQueue_initQueue_Ignore( );
    HAL_RTC_Init_IgnoreAndReturn( HAL_OK );
    HAL_RTC_SetTime_IgnoreAndReturn( HAL_OK );
    HAL_RTC_SetDate_IgnoreAndReturn( HAL_OK );
    HAL_NVIC_SetPriority_Ignore();
    HAL_NVIC_EnableIRQ_Ignore();

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
 * @brief   Test the TimerAlarmOneSecond_Callback function, buzzer flag FALSE.
 * 
 * The objective of this test is to check if all lines in this function are executed when it's called.
*/
void test__TimerAlarmOneSecond_Callback__buzzer_flg_FALSE( void )
{
    HAL_TIM_PWM_Start_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_Backlight_Ignore( );
    AppSched_startTimer_ExpectAnyArgsAndReturn( TRUE );
    TimerAlarmOneSecond_Callback( );
}

/**
 * @brief   Test the TimerAlarmOneSecond_Callback function, buzzer flag TRUE.
 * 
 * The objective of this test is to check if all lines in this function are executed when it's called.
*/
void test__TimerAlarmOneSecond_Callback__buzzer_flg_TRUE( void )
{
    HAL_TIM_PWM_Start_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_Backlight_Ignore( );
    AppSched_startTimer_IgnoreAndReturn( TRUE );
    HAL_TIM_PWM_Stop_IgnoreAndReturn( HAL_OK );

    TimerAlarmOneSecond_Callback( );
    TimerAlarmOneSecond_Callback( );
}

/**
 * @brief   Test the TimerDeactivateAlarm_Callback function.
 * 
 * The objective of this test is to check if all lines in this function are executed when it's called.
*/
void test__TimerDeactivateAlarm_Callback( void )
{
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    TimerDeactivateAlarm_Callback( );
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
    HAL_RTC_SetAlarm_IT_ExpectAnyArgsAndReturn( TRUE );
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
 * @brief   test Clock_Set_Time function.
*/
void test__Clock_Set_Time__AlarmActivated_flg_FALSE( void )
{
    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    AlarmActivated_flg = FALSE;

    HAL_RTC_SetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    nextEvent = Clock_Set_Time( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, CLK_MSG_NONE );
}

/**
 * @brief   test Clock_Set_Time function, AlarmActivated_flg TRUE.
*/
void test__Clock_Set_Time__AlarmActivated_flg_TRUE( void )
{
    AlarmActivated_flg = TRUE;

    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HAL_RTC_SetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );

    nextEvent = Clock_Set_Time( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, CLOCK_MSG_DEACTIVATE_ALARM );
}

/**
 * @brief   test Clock_Set_Date function, AlarmActivated_flg TRUE.
 *  
*/
void test__Clock_Set_Date__AlarmActivated_flg_TRUE( void )
{
    AlarmActivated_flg = TRUE;

    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HAL_RTC_SetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );

    nextEvent = Clock_Set_Date( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, CLOCK_MSG_DEACTIVATE_ALARM );
}

/**
 * @brief   test Clock_Set_Date function, AlarmActivated_flg FALSE.
 *  
*/
void test__Clock_Set_Date__AlarmActivated_flg_FALSE( void )
{
    AlarmActivated_flg = FALSE;

    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HAL_RTC_SetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );

    nextEvent = Clock_Set_Date( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, CLK_MSG_NONE );
}

/**
 * @brief   test Clock_Set_Alarm function.
*/
void test__Clock_Set_Alarm__AlarmActivated_flg_FALSE( void )
{
    AlarmActivated_flg = FALSE;

    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );
    HAL_RTC_SetAlarm_IT_ExpectAnyArgsAndReturn( HAL_OK );

    nextEvent = Clock_Set_Alarm( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, CLK_MSG_NONE );
}

/**
 * @brief   test Clock_Set_Alarm function, AlarmActivated_flg TRUE.
*/
void test__Clock_Set_Alarm__AlarmActivated_flg_TRUE( void )
{
    AlarmActivated_flg = TRUE;

    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );
    HAL_RTC_SetAlarm_IT_ExpectAnyArgsAndReturn( HAL_OK );

    nextEvent = Clock_Set_Alarm( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, CLOCK_MSG_DEACTIVATE_ALARM );
}

/**
 * @brief   test Clock_Send_Display_Msg function.
*/
void test__Clock_Send_Display_Msg( void )
{
    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HAL_RTC_GetTime_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_RTC_GetDate_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    nextEvent = Clock_Send_Display_Msg( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, DISPLAY_MSG_UPDATE );
}

/**
 * @brief   test Clock_Alarm_Activated function.
*/
void test__Clock_Alarm_Activated( void )
{
    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    AppSched_stopTimer_IgnoreAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );
    AppSched_startTimer_IgnoreAndReturn( TRUE );
    
    nextEvent = Clock_Alarm_Activated( &msgReceived );

    TEST_ASSERT_EQUAL( nextEvent.msg, DISPLAY_MSG_ALARM_ACTIVE );
}

/**
 * @brief   test Clock_Deactivate_Alarm function.
*/
void test__Clock_Deactivate_Alarm( void )
{
    APP_MsgTypeDef msgReceived = {0};
    APP_MsgTypeDef nextEvent = {0};

    HAL_TIM_PWM_Stop_IgnoreAndReturn( HAL_OK );
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( HAL_OK );
    AppSched_stopTimer_IgnoreAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );
    AppSched_startTimer_IgnoreAndReturn( TRUE );
    HEL_LCD_Backlight_Ignore();
    
    nextEvent = Clock_Deactivate_Alarm( &msgReceived );
    
    TEST_ASSERT_EQUAL( nextEvent.msg, CLOCK_MSG_DISPLAY );
}

/**
 * @brief   test HAL_RTC_AlarmAEventCallback,
*/
void test__HAL_RTC_AlarmAEventCallback( void )
{
    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );
    HAL_RTC_DeactivateAlarm_IgnoreAndReturn( HAL_OK );

    HAL_RTC_AlarmAEventCallback( &hrtc );
}