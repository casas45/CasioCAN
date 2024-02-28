/**
 * @file test_display.c
 * 
 * @brief Unit tests for display event machine.
*/
#include "unity.h"
#include "display.h"
#include <stdint.h>
#include "bsp.h"

#include "mock_hel_lcd.h"
#include "mock_queue.h"
#include "mock_stm32g0xx_hal_spi.h"
#include "mock_stm32g0xx_hal_tim.h"
#include "mock_analogs.h"

/**
 * @brief   reference to the ClockQueue.
*/
AppQue_Queue ClockQueue;

/**
 * @brief   function that is executed before any unit test function.
*/
void setUp( void )
{
}

/**
 * @brief   function that is executed after any unit test function.
*/
void tearDown( void )
{
}

/** 
 * @brief Reference for the private function Display_Update. 
 * @return  Message with the next event.
 * */
APP_MsgTypeDef Display_Update( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_AlarmSet. 
 * 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_AlarmSet( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_AlarmActive. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_AlarmActive( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_ChangeBacklightState. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_ChangeBacklightState( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_AlarmValues. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_AlarmValues( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_AlarmNoConfig. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_AlarmNoConfig( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_ClearSecondLine. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_ClearSecondLine( APP_MsgTypeDef * );

/** 
 * @brief Reference for the private function Display_Temperature. 
 * @return  Message with the next event.
*/
APP_MsgTypeDef Display_Temperature( APP_MsgTypeDef * );

/** @brief Reference for the private function TimeString. */
void TimeString( char *, uint8_t, uint8_t, uint8_t );

/** @brief Reference for the private function DateString. */
void DateString( char *, uint8_t, uint8_t, uint16_t, uint8_t );

/** @brief Reference for the private function AlarmString. */
void AlarmString( char *, uint8_t, uint8_t );

/** @brief Reference for the private function TemperatureString. */
void TemperatureString( char *, int8_t );

/**
 * @brief   test AlarmString function case "ALARM=00:00".
 * 
 * This test compares a string that has the expected result after using the function with
 * the result of the function.
*/
void test__AlarmString__expected_result_ALARM_00_00( void )
{
    char buffer[11];        /* Buffer to hold the formatted string */

    AlarmString( buffer, 0u, 0u );  /* Alarm parameters 00:00 */

    TEST_ASSERT_EQUAL_STRING_LEN( "ALARM=00:00\0", buffer, 11 );
}

/**
 * @brief   test AlarmString function case "ALARM=08:00".
 * 
 * This test compares a string that has the expected result after using the function with
 * the result of the function.
*/
void test__AlarmString__expected_result_ALARM_06_50( void )
{
    char buffer[11];        /* Buffer to hold the formatted string */

    AlarmString( buffer, 6u, 50u );  /* Alarm parameters 06:50 */

    TEST_ASSERT_EQUAL_STRING_LEN( "ALARM=06:50\0", buffer, 11 );
}

/**
 * @brief   test AlarmString function case "ALARM=06:50".
 * 
 * This test compares a string that has the expected result after using the function with
 * the result of the function.
*/
void test__AlarmString__expected_result_ALARM_08_00( void )
{
    char buffer[11];        /* Buffer to hold the formatted string */

    AlarmString( buffer, 8u, 0u );  /* Alarm parameters 08:00 */

    TEST_ASSERT_EQUAL_STRING_LEN( "ALARM=08:00\0", buffer, 11 );
}


/**
 * @brief Test Display_InitTask function.
*/
void test__Display_InitTask( void )
{
    AppQueue_initQueue_Ignore( );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    HAL_SPI_Init_IgnoreAndReturn( HAL_OK );
    HAL_TIM_PWM_Init_IgnoreAndReturn( HAL_OK );
    HAL_TIM_PWM_ConfigChannel_IgnoreAndReturn( HAL_OK );
    HAL_TIM_PWM_Start_IgnoreAndReturn( HAL_OK );
    HEL_LCD_MspInit_Ignore( );
    HEL_LCD_Init_ExpectAnyArgsAndReturn( TRUE );
    HEL_LCD_Backlight_ExpectAnyArgsAndReturn( HAL_OK );

    Display_InitTask( );
}

/**
 * @brief Test Display_PeriodicTask with a DISPLAY_MSG_UPDATE.
*/
void test__Display_PeriodicTask__Update_Display_message( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg         = DISPLAY_MSG_UPDATE;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_MsgTypeDef ) );
    HEL_LCD_SetCursor_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_String_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_SetCursor_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_String_ExpectAnyArgsAndReturn( HAL_OK );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );

    Display_PeriodicTask( );
}

/**
 * @brief Test Display_PeriodicTask with a not valid type message.
*/
void test__Display_PeriodicTask__Update_Display_no_valid_message( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg         = N_DISPLAY_EVENTS;
    receivedMSG.tm.tm_hour  = 0x23;
    receivedMSG.tm.tm_min   = 0x23;
    receivedMSG.tm.tm_sec   = 0x23;

    receivedMSG.tm.tm_mday  = 0x23;
    receivedMSG.tm.tm_mon   = RTC_MONTH_JANUARY;
    receivedMSG.tm.tm_year  = 0x23;
    receivedMSG.tm.tm_wday  = RTC_WEEKDAY_TUESDAY;

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( FALSE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &receivedMSG, sizeof( APP_MsgTypeDef ) );
    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );

    Display_PeriodicTask( );
}

/**
 * @brief Test Display_Update.
*/
void test__UpdateDisplay( void )
{
    APP_MsgTypeDef nextEvent = {0};
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg         = DISPLAY_MSG_UPDATE;
    receivedMSG.tm.tm_hour  = 0x23;
    receivedMSG.tm.tm_min   = 0x23;
    receivedMSG.tm.tm_sec   = 0x23;

    receivedMSG.tm.tm_mday  = 0x23;
    receivedMSG.tm.tm_mon   = RTC_MONTH_JANUARY;
    receivedMSG.tm.tm_year  = 0x23;
    receivedMSG.tm.tm_wday  = RTC_WEEKDAY_TUESDAY;

    HEL_LCD_SetCursor_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_String_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_SetCursor_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_String_ExpectAnyArgsAndReturn( HAL_OK );

    HIL_QUEUE_writeDataISR_IgnoreAndReturn( TRUE );

    nextEvent = Display_Update( &receivedMSG );

    TEST_ASSERT_EQUAL( nextEvent.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   test Display_AlarmSet function.
*/
void test__DisplayAlarmSet(void)
{
    APP_MsgTypeDef receivedMSG = {0};
    APP_MsgTypeDef nextEvent = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( TRUE );
    HEL_LCD_Data_IgnoreAndReturn( TRUE );

    nextEvent = Display_AlarmSet( &receivedMSG );

    TEST_ASSERT_EQUAL( nextEvent.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   test Display_AlarmSet function.
*/
void test__DisplayAlarmActive(void)
{
    APP_MsgTypeDef receivedMSG = {0};
    APP_MsgTypeDef nextEvent = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( TRUE );
    HEL_LCD_String_IgnoreAndReturn( TRUE );

    nextEvent = Display_AlarmActive( &receivedMSG );

    TEST_ASSERT_EQUAL( nextEvent.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test of the function TimeString.
 * 
 * Using the TimeString function, an array it's setted with the time values, then this string
 * is compared with other string with the expected values and format, considering only the first
 * 9 characters.
*/
void test__TimeString__check_char_array( void )
{
    char str[16];

    TimeString( str, 22, 58, 59 );

    TEST_ASSERT_EQUAL_STRING_LEN( "22:58:59\0", str, 9);
}

/**
 * @brief Unit test for the function DateString with correct values.
 * 
 * Using the DateString function, an array it's setted with the date values, then this string
 * is compared with other string with the expected values and format, considering only the first
 * 15 characters.
*/
void test__DateString__correct_date_values( void )
{
    char str[16];

    DateString( str, 1, 17, 2023, RTC_WEEKDAY_WEDNESDAY );

    TEST_ASSERT_EQUAL_STRING_LEN( "ENE,17 2023 Mi\0", str, 15);
}

/**
 * @brief Unit test for the function DateString with month and weekday values set to 0.
 * 
 * Using the DateString function, an array it's setted with the date values, then this string
 * is compared with other string with the expected values and format, considering only the first
 * 15 characters, the expected values for month and weekday are "ENE" and "Lu". The function does 
 * this to prevent a bad memory access problem..
*/
void test__DateString__month_0_and_weekday_0( void )
{
    char str[16];

    DateString( str, 0, 17, 2023, 0 );

    TEST_ASSERT_EQUAL_STRING_LEN( "ENE,17 2023 Lu\0", str, 15);
}

/**
 * @brief   test Display_ChangeBacklightState event.
 * 
 * This function change the backlight state and return a message with the next event, that its a idle
 * state called DISPLAY_MSG_NONE.
*/
void test__DisplayChangeBacklightState( void )
{
    APP_MsgTypeDef readMessage = {0};
    APP_MsgTypeDef nextEvent = {0};
    readMessage.displayBkl = LCD_ON;

    HEL_LCD_Backlight_ExpectAnyArgsAndReturn( HAL_OK );

    nextEvent = Display_ChangeBacklightState( &readMessage );

    TEST_ASSERT_EQUAL( nextEvent.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test for the event Display_AlarmValues.
*/
void test__Display_AlarmValues( void )
{
    APP_MsgTypeDef pDisplayMsg;
    APP_MsgTypeDef nextEventMsg;

    pDisplayMsg.tm.tm_hour  = 6u;
    pDisplayMsg.tm.tm_min   = 50u;

    HEL_LCD_SetCursor_IgnoreAndReturn( HAL_OK );
    HEL_LCD_String_IgnoreAndReturn( HAL_OK );

    nextEventMsg = Display_AlarmValues( &pDisplayMsg );

    TEST_ASSERT_EQUAL( nextEventMsg.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test for the event Display_AlarmNoConfig.
*/
void test__Display_AlarmNoConfig( void )
{
    APP_MsgTypeDef pDisplayMsg = {0};
    APP_MsgTypeDef nextEventMsg = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( HAL_OK );
    HEL_LCD_String_IgnoreAndReturn( HAL_OK );

    nextEventMsg = Display_AlarmNoConfig( &pDisplayMsg );

    TEST_ASSERT_EQUAL( nextEventMsg.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test for the event Display_ClearSecondLine.
*/
void test__Display_ClearSecondLine( void )
{
    APP_MsgTypeDef pDisplayMsg = {0};
    APP_MsgTypeDef nextEventMsg = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( HAL_OK );
    HEL_LCD_String_IgnoreAndReturn( HAL_OK );

    nextEventMsg = Display_ClearSecondLine( &pDisplayMsg );

    TEST_ASSERT_EQUAL( nextEventMsg.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test for the Display_AlarmValues function.
 * 
 * This event return the a message of type DISPLAY_MSG_NONE, that is interpretaded as a
 * an idle state. 
*/
void test__Display_AlarmValues__return_msg_with_DISPLAY_MSG_NONE(void)
{
    APP_MsgTypeDef pDisplayMsg = {0};
    APP_MsgTypeDef nextEventMsg = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( HAL_OK );
    HEL_LCD_String_IgnoreAndReturn( HAL_OK );

    nextEventMsg = Display_AlarmValues( &pDisplayMsg );

    TEST_ASSERT_EQUAL( nextEventMsg.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test for the Display_ClearSecondLine function.
 * 
 * his event return the a message of type DISPLAY_MSG_NONE, that is interpretaded as a
 * an idle state. 
*/
void test__Display_ClearSecondLine__return_msg_with_DISPLAY_MSG_NONE(void)
{
    APP_MsgTypeDef pDisplayMsg = {0};
    APP_MsgTypeDef nextEventMsg = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( HAL_OK );
    HEL_LCD_String_IgnoreAndReturn( HAL_OK );

    nextEventMsg = Display_ClearSecondLine( &pDisplayMsg );

    TEST_ASSERT_EQUAL( nextEventMsg.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Unit test for the Display_AlarmActive function.
 * 
 * his event return the a message of type DISPLAY_MSG_NONE, that is interpretaded as a
 * an idle state. 
*/
void test__Display_AlarmActive__return_msg_with_DISPLAY_MSG_NONE(void)
{
    APP_MsgTypeDef pDisplayMsg = {0};
    APP_MsgTypeDef nextEventMsg = {0};

    HEL_LCD_SetCursor_IgnoreAndReturn( HAL_OK );
    HEL_LCD_String_IgnoreAndReturn( HAL_OK );

    nextEventMsg = Display_AlarmActive( &pDisplayMsg );

    TEST_ASSERT_EQUAL( nextEventMsg.msg, DISPLAY_MSG_NONE );
}

/**
 * @brief   Display_LcdTask unit test.
 * 
 * The initial value for the intensity is 100 and for the contrast is 0, using mock for the
 * functions Analogs_GetContrast and Analogs_GetIntensity return values of 0's and only
 * the intensity need to be changed.
*/
void test__Display_LcdTask__same_contrast_different_intensity( void )
{
    uint8_t contrast    = 0u;
    uint8_t intensity   = 100u;

    Analogs_GetContrast_IgnoreAndReturn( contrast );
    Analogs_GetIntensity_IgnoreAndReturn( intensity );

    HEL_LCD_Intensity_IgnoreAndReturn( HAL_OK );

    Display_LcdTask( );
}

/**
 * @brief   Display_LcdTask unit test.
 * 
 * The initial value for the intensity is 100 and for the contrast is 0, using mock for the
 * functions Analogs_GetContrast and Analogs_GetIntensity return values of 10 and 100, respectively
 * and only the contrast need to be changed.
*/
void test__Display_LcdTask__same_intensity_different_contrast( void )
{
    uint8_t contrast    = 10u;
    uint8_t intensity   = 0u;

    Analogs_GetContrast_IgnoreAndReturn( contrast );
    Analogs_GetIntensity_IgnoreAndReturn( intensity );

    HEL_LCD_Contrast_IgnoreAndReturn( HAL_OK );

    Display_LcdTask( );
}

/**
 * @brief   Display_Temperature unit test. 
*/
void test__Display_Temperature( void )
{
    APP_MsgTypeDef nextEvent = {0};
    APP_MsgTypeDef readMsg = {0};
    int8_t temp = 25;

    Analogs_GetTemperature_IgnoreAndReturn( temp );

    HEL_LCD_SetCursor_ExpectAnyArgsAndReturn( HAL_OK );
    HEL_LCD_String_ExpectAnyArgsAndReturn( HAL_OK );

    nextEvent = Display_Temperature( &readMsg );

    TEST_ASSERT_EQUAL( DISPLAY_MSG_NONE, nextEvent.msg ); 
}

/**
 * @brief   TemperatureString unit test 1.
 * 
*/
void test__TemperatureString__temperature_110_degrees( void )
{
    int8_t temp = 110;
    const char *expectedString = "110C";
    char tempString[5]; 

    TemperatureString( tempString, temp );

    TEST_ASSERT_EQUAL_STRING_LEN( expectedString, tempString, 5);
}

/**
 * @brief   TemperatureString unit test 2.
 * 
*/
void test__TemperatureString__temperature_25_degrees( void )
{
    int8_t temp = 25;
    const char *expectedString = " 25C";
    char tempString[5]; 

    TemperatureString( tempString, temp );

    TEST_ASSERT_EQUAL_STRING_LEN( expectedString, tempString, 5);
}

/**
 * @brief   TemperatureString unit test 3.
 * 
*/
void test__TemperatureString__temperature_minus_40_degrees( void )
{
    int8_t temp = -40;
    const char *expectedString = "-40C";
    char tempString[5]; 

    TemperatureString( tempString, temp );

    TEST_ASSERT_EQUAL_STRING_LEN( expectedString, tempString, 5);
}