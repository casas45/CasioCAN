#include "analogs.h"
#include "bsp.h"

#define TIM2_PRESCALER          10000u
#define TIM2_PERIOD             320u
#define N_ADC_CHANNELS_USED     4u
#define ADC_OVERSAMPLING_RATIO  4u
#define VREF_INDEX              3u
#define MAX_ADC_VALUE           255u
#define LCD_INTENSITY_INTERVALS 10u
#define ADC_CONTRAST_DIV        17u 

/**
 * @brief   Array where DMA puts the ADC reads.
*/
/* cppcheck-suppress misra-c2012-8.4 ; false warning, the macro STATIC makes the variable static */
STATIC uint32_t AdcData[ N_ADC_CHANNELS_USED ];

/**
 * @brief   Function to initialize the ADC, DMA and TIMER 2.
 * 
 * The timer 2 need to have a periodicity of 50 ms and its clokc its working at 64MHz, to get the
 * required frequency is used the next calculation ( TIM2_Freq / TIM2_PRESCALER ) / TIM2_PERIOD =
 * ( 64 Mhz / 10000 ) / 320 = 20 Hz = 50 ms. The aim of this timer is to trigger the ADC conversion
 * and to do this the update event (counter overflow) is used as trigger output.
 * 
 * The DMA channel 1 is configured to requested by the ADC, direction Peripheral to Memory and mode
 * circular to use it continuosly.
 * 
 * The ADC is configured with a resolution of 8 bits and to be triggered by the trigger output of the
 * TIM2 on the rising edge, the scan sequence is set to fully configurable to get the measurements in
 * the desired order, first the temperature sensor, second the pot0 (intensity) and finally the pot1 
 * (constrast). The sampling time common 1 si configured to 1.5 ADC cycles for the channels 0 and 1,
 * for the temperature sensor is used the sampling time common 2 and is set to 160.5 ADC cycles, this
 * is to get a time conversion greater than the ts_temp specified in the datasheet.
 * The ADC divider is set to 2.
 * Time conversion potentiometers.
 * t_conv = t_smpl + t_sar = ( 1.5 + 8.5 ) * t_adc_clk = 625 ns. 
 * Time conversion temperature sensor.
 * t_conv = t_smpl + t_sar = ( 160.5 + 8.5 ) * t_adc_clk = 10.56 ms. 
 * The oversampling configuration is set with a ratio of 4, and the division coefficient is set to 4.
*/
void Analogs_Init( void )
{
    TIM_HandleTypeDef TIM2_Handler;
    TIM_MasterConfigTypeDef TIM2_MasterConfig;

    ADC_HandleTypeDef ADC_Handler;
    ADC_ChannelConfTypeDef sChanConfig;

    DMA_HandleTypeDef DMA_Handler;

    HAL_StatusTypeDef Status = HAL_ERROR;

    /* Timer 2 configuration */
    TIM2_Handler.Instance           = TIM2;
    TIM2_Handler.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    TIM2_Handler.Init.Prescaler     = TIM2_PRESCALER; 
    TIM2_Handler.Init.Period        = TIM2_PERIOD;
    TIM2_Handler.Init.CounterMode   = TIM_COUNTERMODE_UP;

    Status = HAL_TIM_Base_Init( &TIM2_Handler );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );

    TIM2_MasterConfig.MasterOutputTrigger   = TIM_TRGO_UPDATE;
    TIM2_MasterConfig.MasterSlaveMode       = TIM_MASTERSLAVEMODE_DISABLE;

    Status = HAL_TIMEx_MasterConfigSynchronization( &TIM2_Handler, &TIM2_MasterConfig );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );

    /* DMA configuration */
    DMA_Handler.Instance                    = DMA1_Channel1;
    DMA_Handler.Init.Request                = DMA_REQUEST_ADC1;
    DMA_Handler.Init.Direction              = DMA_PERIPH_TO_MEMORY;
    DMA_Handler.Init.MemInc                 = DMA_MINC_ENABLE;
    DMA_Handler.Init.PeriphInc              = DMA_PINC_ENABLE;
    DMA_Handler.Init.MemDataAlignment       = DMA_MDATAALIGN_WORD;
    DMA_Handler.Init.PeriphDataAlignment    = DMA_PDATAALIGN_WORD;
    DMA_Handler.Init.Mode                   = DMA_CIRCULAR;
    DMA_Handler.Init.Priority               = DMA_PRIORITY_HIGH;

    Status = HAL_DMA_Init( &DMA_Handler );
    assert_error( Status == HAL_OK, DMA_RET_ERROR );

    /* Link the ADC to the DMA */
    ADC_Handler.DMA_Handle  = &DMA_Handler;
    DMA_Handler.Parent      = &ADC_Handler;

    /* ADC configuration */
    ADC_Handler.Instance                        = ADC1;
    ADC_Handler.Init.ClockPrescaler             = ADC_CLOCK_SYNC_PCLK_DIV2;
    ADC_Handler.Init.SamplingTimeCommon1        = ADC_SAMPLETIME_1CYCLE_5;
    ADC_Handler.Init.SamplingTimeCommon2        = ADC_SAMPLETIME_160CYCLES_5;
    ADC_Handler.Init.ScanConvMode               = ADC_SCAN_ENABLE;
    ADC_Handler.Init.NbrOfConversion            = N_ADC_CHANNELS_USED;
    ADC_Handler.Init.DataAlign                  = ADC_DATAALIGN_RIGHT;
    ADC_Handler.Init.Resolution                 = ADC_RESOLUTION8b;
    ADC_Handler.Init.ExternalTrigConv           = ADC_EXTERNALTRIG_T2_TRGO;
    ADC_Handler.Init.ExternalTrigConvEdge       = ADC_EXTERNALTRIG_EDGE_RISING;
    ADC_Handler.Init.EOCSelection               = ADC_EOC_SEQ_CONV;
    ADC_Handler.Init.DMAContinuousRequests      = ENABLE;
    ADC_Handler.Init.Oversampling.Ratio         = ADC_OVERSAMPLING_RATIO;
    ADC_Handler.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_2;
    ADC_Handler.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER; 

    Status = HAL_ADC_Init( &ADC_Handler );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration pot0 - intensity */
    sChanConfig.Channel         = ADC_CHANNEL_0;
    sChanConfig.Rank            = ADC_REGULAR_RANK_2;
    sChanConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_1;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration pot1 - contrast */
    sChanConfig.Channel         = ADC_CHANNEL_1;
    sChanConfig.Rank            = ADC_REGULAR_RANK_3;
    sChanConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_1;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration temperature sensor */
    sChanConfig.Channel         = ADC_CHANNEL_TEMPSENSOR;
    sChanConfig.Rank            = ADC_REGULAR_RANK_1;
    sChanConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );

    /* ADC channel configuration Vref */
    sChanConfig.Channel         = ADC_CHANNEL_VREFINT;
    sChanConfig.Rank            = ADC_REGULAR_RANK_4;
    sChanConfig.SamplingTime    = ADC_SAMPLINGTIME_COMMON_2;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );

    Status = HAL_ADCEx_Calibration_Start( &ADC_Handler );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    Status = HAL_ADC_Start_DMA( &ADC_Handler, AdcData, N_ADC_CHANNELS_USED );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );
    
    Status = HAL_TIM_Base_Start( &TIM2_Handler );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );
}

int8_t Analogs_GetTemperature( void )
{
    int8_t temperature;
    uint16_t Vref;

    Vref = ( (uint16_t)(*VREFINT_CAL_ADDR) * VREFINT_CAL_VREF ) / ( AdcData[VREF_INDEX] << 4u );

}

/**
 * @brief   Function to determine the level of LCD contrast based on potentiometer pot0.
*/
uint8_t Analogs_GetContrast( void )
{
    uint8_t contrast;

    contrast = AdcData[ CONTRAST_INDEX ] / ADC_CONTRAST_DIV;
    
    return contrast;
}

/**
 * @brief   Function to determine the level of LCD intensity based on potentiometer pot1.
*/
uint8_t Analogs_GetIntensity( void )
{
    uint8_t intensity;
    
    intensity = ( ( AdcData[ INTENSITY_INDEX ] * LCD_INTENSITY_INTERVALS ) / MAX_ADC_VALUE) * LCD_INTENSITY_INTERVALS;

    return intensity;
}