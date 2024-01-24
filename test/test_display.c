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

/** @brief Reference for the private function UpdateDisplay. */
void UpdateDisplay( APP_MsgTypeDef * );

/** @brief Reference for the private function TimeString. */
void TimeString( char *, uint8_t, uint8_t, uint8_t );

/** @brief Reference for the private function DateString. */
void DateString( char *, uint8_t, uint8_t, uint16_t, uint8_t );


/**
 * @brief Test Display_InitTask function.
*/
void test__Display_InitTask( void )
{
    AppQueue_initQueue_Ignore( );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    HAL_SPI_Init_IgnoreAndReturn( HAL_OK );
    HEL_LCD_MspInit_Ignore( );
    HEL_LCD_Init_ExpectAnyArgsAndReturn( TRUE );
    HEL_LCD_Backlight_Ignore( );

    Display_InitTask( );
}

/**
 * @brief Test Display_PeriodicTask with a DISPLAY_MSG_UPDATE.
*/
void test__Display_PeriodicTask__Update_Display_message( void )
{
    APP_MsgTypeDef receivedMSG = {0};
    receivedMSG.msg         = DISPLAY_MSG_UPDATE;
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
 * @brief Test UpdateDisplay.
*/
void test__UpdateDisplay( void )
{
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

    UpdateDisplay( &receivedMSG );
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