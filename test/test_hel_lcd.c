/**
 * @file    test_hel_lcd.c
 * 
 * @brief   This file contains unit tests for the LCD driver functions
*/
#include "unity.h"
#include "hel_lcd.h"
#include <stdint.h>

#include "mock_stm32g0xx_hal.h"
#include "mock_stm32g0xx_hal_spi.h"
#include "mock_stm32g0xx_hal_gpio.h"

/**
 * @brief   struct to handle the LCD.
*/
LCD_HandleTypeDef LCD_Handler;

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
 * @brief   Test case for HEL_LCD_Init function returning HAL_OK.
 * 
 * This unit test utilizes mock functions to verify the execution of all lines within the
 * tested function.
*/
void test__HEL_LCD_Init__return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;

    retValue = HEL_LCD_Init( &LCD_Handler );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_Command function returning HAL_OK.
 * 
 * This unit test utilizes mock functions to verify the execution of all lines within the
 * tested function.
*/
void test__HEL_LCD_Command__return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;

    retValue = HEL_LCD_Command( &LCD_Handler, CMD_WAKEUP );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_Data function returning HAL_OK.
 * 
 * This unit test utilizes mock functions to verify the execution of all lines within the
 * tested function.
*/
void test__HEL_LCD_Data__return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;

    retValue = HEL_LCD_Data( &LCD_Handler, 'X' );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_String function returning HAL_OK.
 * 
 * This unit test is designed to check that the while loop ends after sending only 16 characters
 * from a string with over 16 characters. This verification cannot be achieved with an ASSERT 
 * statement but requires code coverage analysis to ensure that the correct branch is taken.
*/
void test__HEL_LCD_String__string_over_16_characters_return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;
    char *str = "STRING WITH OVER 16 CHARACTERS.";

    retValue = HEL_LCD_String( &LCD_Handler, str );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_String function returning HAL_OK.
 * 
 * This unit test is desigmed to check that the while loop ends after recognize the null character
 * from a string with 13 characters. This verification cannot be achieved with an ASSERT 
 * statement but requires code coverage analysis to ensure that the correct branch is taken. 
*/
void test__HEL_LCD_String__string_under_16_characters_return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;
    char *str = "13 CHARACTERS";

    retValue = HEL_LCD_String( &LCD_Handler, str );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_SetCursor function returning HAL_OK.
 * 
 * Set the cursor in the position (0,0) and check the return value.
*/
void test__HEL_LCD_SetCursor__col_0_row_0_return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;

    retValue = HEL_LCD_SetCursor( &LCD_Handler, 0, 0 );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_SetCursor function returning HAL_OK.
 * 
 * Set the cursor in the position (3,17), this value aren't accepted by the LCD, then the
 * function take the other if branch to set correct parameters (1, 0).
*/
void test__HEL_LCD_SetCursor__exceed_limits_goto_col_0_row_1_return_HAL_OK( void )
{
    HAL_Delay_Ignore( );
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;

    retValue = HEL_LCD_SetCursor( &LCD_Handler, 3, 17 );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}

/**
 * @brief   Test case for HEL_LCD_Backlight setting ON.
 * 
 * This unit test mocks the HAL_GPIO_WritePin function, and it only checks that the case of
 * LCD_ON is taken.
*/
void test__HEL_LCD_Backlight__set_ON( void )
{
    HAL_GPIO_WritePin_Ignore( );

    HEL_LCD_Backlight( &LCD_Handler, LCD_ON );
}

/**
 * @brief   Test case for HEL_LCD_Backlight setting OFF.
 * 
 * This unit test mocks the HAL_GPIO_WritePin function, and it only checks that the case of
 * LCD_OFF is taken.
*/
void test__HEL_LCD_Backlight__set_OFF( void )
{
    HAL_GPIO_WritePin_Ignore( );

    HEL_LCD_Backlight( &LCD_Handler, LCD_OFF );
}

/**
 * @brief   Test case for HEL_LCD_Backlight toggle it.
 * 
 * This unit test mocks the HAL_GPIO_WritePin function, and it only checks that the case of
 * LCD_TOGGLE is taken.
*/
void test__HEL_LCD_Backlight__Toggle( void )
{
    HAL_GPIO_TogglePin_Ignore( );

    HEL_LCD_Backlight( &LCD_Handler, LCD_TOGGLE );
}

/**
 * @brief   Test case for HEL_LCD_Backlight.
 * 
 * This unit test mocks the HAL_GPIO_WritePin function, and it only checks that the default 
 * case is taken.
*/
void test__HEL_LCD_Backlight__unknown_value_default_case( void )
{
    HAL_GPIO_WritePin_Ignore( );

    HEL_LCD_Backlight( &LCD_Handler, 5u );
}

/**
 * @brief   Test case for HEL_LCD_Contrast returning HAL_ERROR.
 * 
 * This unit test utilizes the tested function trying to attempt to set a contrast below the 
 * LCD_CONTRAST_1 value, expecting the returned value to be HAL_ERROR.
*/
void test__HEL_LCD_Contrast__value_under_LCD_CONTRAST_1_return_HAL_ERROR( void )
{
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_ERROR );

    uint8_t retValue = HAL_OK;

    retValue = HEL_LCD_Contrast( &LCD_Handler, 0u );

    TEST_ASSERT_EQUAL( retValue, HAL_ERROR );
}

/**
 * @brief   Test case for HEL_LCD_Contrast returning HAL_ERROR.
 * 
 * This unit test utilizes the tested function trying to attempt to set a contrast above the 
 * LCD_CONTRAST_16 value, expecting the returned value to be HAL_ERROR.
*/
void test__HEL_LCD_Contrast__value_over_LCD_CONTRAST_16_return_HAL_ERROR( void )
{
    HAL_GPIO_WritePin_Ignore( );

    uint8_t retValue = HAL_OK;

    retValue = HEL_LCD_Contrast( &LCD_Handler, 0xFFu );

    TEST_ASSERT_EQUAL( retValue, HAL_ERROR );
}

/**
 * @brief   Test case for HEL_LCD_Contrast returning HAL_OK.
 * 
 * This unit test utilizes the tested function trying to attempt to set a contrast of
 * LCD_CONTRAST_1 value, expecting the returned value to be HAL_OK.
*/
void test__HEL_LCD_Contrast__valid_value_LCD_CONTRAST_1_return_HAL_OK( void )
{
    HAL_GPIO_WritePin_Ignore( );
    HAL_SPI_Transmit_IgnoreAndReturn( HAL_OK );

    uint8_t retValue = HAL_ERROR;

    retValue = HEL_LCD_Contrast( &LCD_Handler, LCD_CONTRAST_1 );

    TEST_ASSERT_EQUAL( retValue, HAL_OK );
}