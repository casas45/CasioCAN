/**
 * @file    test_analogs.c
 * 
 * @brief   Unit tests for the analogs functions.
*/
#include "unity.h"
#include "bsp.h"
#include "analogs.h"
#include <stdint.h>

#include "mock_stm32g0xx_hal_tim.h"
#include "mock_stm32g0xx_hal_tim_ex.h"
#include "mock_stm32g0xx_hal_adc.h"
#include "mock_stm32g0xx_hal_adc_ex.h"
#include "mock_stm32g0xx_hal_dma.h"

/**
 * @brief   AdcData array reference.
*/
extern uint32_t AdcData[ N_ADC_CHANNELS_USED ];

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
 * @brief   Test Analogs_Init function.
*/
void test__Analogs_Init( void )
{
    HAL_TIM_Base_Init_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_TIMEx_MasterConfigSynchronization_IgnoreAndReturn( HAL_OK );
    HAL_DMA_Init_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Init_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_ConfigChannel_IgnoreAndReturn( HAL_OK );
    HAL_ADCEx_Calibration_Start_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_TIM_Base_Start_ExpectAnyArgsAndReturn( HAL_OK );

    Analogs_Init( );
}


/**
 * @brief   Test Analogs_Get_Temperature AdcValue higher than maximum value.
 * 
 * The function when read a value from the AdcData array and its higher than the maximum ADC value
 * possible (12-bit resolution) returns 0 
*/
void test__Analogs_GetTemperature__Vref_4100_return_temperature_0( void )
{
    int8_t temperature;
    int8_t expect_value = 0;

    AdcData[TEMP_INDEX] = 4095u;
    AdcData[VREF_INDEX] = 4100u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    temperature = Analogs_GetTemperature( );

    TEST_ASSERT_EQUAL_INT8( expect_value, temperature );
}

/**
 * @brief   Test Analogs_Get_Temperature AdcValue higher than maximum value.
 * 
 * The function when read a value from the AdcData array and its higher than the maximum ADC value
 * possible (12-bit resolution) returns 0 
*/
void test__Analogs_GetTemperature__AdcTemp_4100_return_temperature_0( void )
{
    int8_t temperature;
    int8_t expect_value = 0;

    AdcData[TEMP_INDEX] = 4100u;
    AdcData[VREF_INDEX] = 4095u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    temperature = Analogs_GetTemperature( );

    TEST_ASSERT_EQUAL_INT8( expect_value, temperature );
}

/**
 * @brief   Test Analogs_Get_Temperature AdcData[TEMP_INDEX] 950.
 * 
 * The value of Vref is set to 1507 (value read using the ADC), and the temperature value read from the ADC is set to
 * 950, the expected value is 33.
*/
void test__Analogs_GetTemperature__AdcTemp_950_return_temperature_33_degrees( void )
{
    int8_t temperature;
    int8_t expect_value = 33;

    AdcData[TEMP_INDEX] = 950u;
    AdcData[VREF_INDEX] = 1507u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    temperature = Analogs_GetTemperature( );

    TEST_ASSERT_EQUAL_INT8( expect_value, temperature );
}

/**
 * @brief   Test Analogs_Get_Temperature AdcData[TEMP_INDEX] 800.
 * 
 * The value of Vref is set to 1507 (value read using the ADC), and the temperature value read from the ADC is set to
 * 800, the expected value is -13.
*/
void test__Analogs_GetTemperature__AdcTemp_800_return_temperature_minus_13_degrees( void )
{
    int8_t temperature;
    int8_t expect_value = -13;

    AdcData[TEMP_INDEX] = 800u;
    AdcData[VREF_INDEX] = 1507u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    temperature = Analogs_GetTemperature( );

    TEST_ASSERT_EQUAL_INT8( expect_value, temperature );
}


/**
 * @brief Test Analog_Get_Contrast with a AdcValue of 4095 return contrast of 15.
 * 
 * Set a value of 4095 in the AdcData array at the contrast index, then the function is
 * used and the expected value is 15.
*/
void test__Analogs_GetContrast__AdcValue_4095_return_contrast_15( void )
{
    uint8_t contrast;

    AdcData[CONTRAST_INDEX] = 4095u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    contrast = Analogs_GetContrast( );

    TEST_ASSERT_EQUAL( contrast, 15u );
}

/**
 * @brief Test Analog_Get_Contrast with an AdcValue of 4200 return a contrast of 0.
 * 
 * Set a value of 4200 in the AdcData array at the contrast index, then the function is
 * called and the expected value is 0, this AdcValue is higher than the maximum value
 * possible obtained with a 12-bit resolution.
*/
void test__Analogs_GetContrast__AdcValue_4200_return_contrast_0( void )
{
    uint8_t contrast;

    AdcData[CONTRAST_INDEX] = 4200u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    contrast = Analogs_GetContrast( );

    TEST_ASSERT_EQUAL( contrast, 0u );
}

/**
 * @brief Test Analog_Get_Contrast with an AdcValue of 100 return a contrast of 5.
 * 
 * Set a value of 100 in the AdcData array at the contrast index, then the function is
 * called and the expected value is 5.
*/
void test__Analogs_GetContrast__AdcValue_1200_return_contrast_4( void )
{
    uint8_t contrast;

    AdcData[CONTRAST_INDEX] = 1200u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    contrast = Analogs_GetContrast( );

    TEST_ASSERT_EQUAL( contrast, 4u );
}

/**
 * @brief Test Analog_Get_Contrast with an AdcValue of 0 return a contrast of 0.
 * 
 * Set a value of 0 in the AdcData array at the contrast index, then the function is
 * called and the expected value is 0.
*/
void test__Analogs_GetContrast__AdcValue_0_return_contrast_0( void )
{
    uint8_t contrast;

    AdcData[CONTRAST_INDEX] = 0u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    contrast = Analogs_GetContrast( );

    TEST_ASSERT_EQUAL( contrast, 0 );
}



/**
 * @brief   Test Analogs_Get_Intensity with an AdcValue of 4095 to return an intensity of 100.
 * 
 * In this test, is used the maximum AdcValue possible and the expected intensity level is 100.
*/
void test__Analogs_GetIntensity__AdcValue_4095_return_intensity_100( void )
{
    uint8_t intensity;

    AdcData[INTENSITY_INDEX] = 4095u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    intensity = Analogs_GetIntensity( );

    TEST_ASSERT_EQUAL( intensity, 100u );
}

/**
 * @brief   Test Analogs_Get_Intensity with an AdcValue of 4200 to return an intensity of 0.
 * 
 * In this test, a value higher than the maximum AdcValue is used, and the expected intensity 
 * level is 0.
*/
void test__Analogs_GetIntensity__AdcValue_4200_return_intensity_0( void )
{
    uint8_t intensity;

    AdcData[INTENSITY_INDEX] = 4200u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    intensity = Analogs_GetIntensity( );

    TEST_ASSERT_EQUAL( intensity, 0u );
}

/**
 * @brief   Test Analogs_Get_Intensity with an AdcValue of 0 to return an intensity of 0.
 * 
 * In this test, an AdcValue of 0 is used, and the expected intensity level is 0.
*/
void test__Analogs_GetIntensity__AdcValue_0_return_intensity_0( void )
{
    uint8_t intensity;

    AdcData[INTENSITY_INDEX] = 0u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    intensity = Analogs_GetIntensity( );

    TEST_ASSERT_EQUAL( intensity, 0u );
}

/**
 * @brief   Test Analogs_Get_Intensity with an AdcValue of 1200 to return an intensity of 20.
 * 
 * In this test, an AdcValue of 100 is used, and the expected intensity level is 20.
*/
void test__Analogs_GetIntensity__AdcValue_1200_return_intensity_20( void )
{
    uint8_t intensity;

    AdcData[INTENSITY_INDEX] = 1200u;

    HAL_ADC_Stop_DMA_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_ADC_Start_DMA_ExpectAnyArgsAndReturn( HAL_OK );

    intensity = Analogs_GetIntensity( );

    TEST_ASSERT_EQUAL( intensity, 20u );
}
