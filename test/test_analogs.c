/**
 * @file    test_analogs.c
 * 
 * @brief   Unit tests for the analogs functions.
*/
#include "unity.h"
#include "bsp.h"
#include "analogs.h"
#include <stdint.h>

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

void test__Analogs_Get_Temperature( void )
{

}

void test__Analogs_Get_Contrast__AdcValue_250_return_contrast_14( void )
{
    uint8_t contrast;

    AdcData[CONTRAST_INDEX] = 250u;

    contrast = Analogs_Get_Contrast( );

    TEST_ASSERT_EQUAL_UINT8( contrast, 14u );
}

void test__Analogs_Get_Intensity( void )
{
    uint8_t intensity;

    AdcData[INTENSITY_INDEX] = 250u;

    intensity = Analogs_Get_Intensity( );

    TEST_ASSERT_EQUAL( intensity, 10u );
}