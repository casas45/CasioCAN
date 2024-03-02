/**
 * @file    analogs.c
 * 
 * @brief   File where are the functions related with the ADC.
*/
#include "analogs.h"
#include "bsp.h"

#define TIM2_PRESCALER          10000u          /*!< Prescaler used for the TIM2 */
#define TIM2_PERIOD             320u            /*!< TIM2 period to get a periodicity of 50 ms */
#define ADC_OVERSAMPLING_RATIO  4u              /*!< Number of times each channel will be sampled */
#define MAX_ADC_VALUE           0xFFFu          /*!< Maximum ADC value with a 12-bit resolution */
#define LCD_INTENSITY_INTERVALS 10u             /*!< Number of intervals of the intensity LCD levels */
#define ADC_CONTRAST_DIV        273u            /*!< Value of each contrast interval to get 15 levels */
#define VREF_CHARAC             1662u           /*!< Value of VREF+ voltage, retrieved from VREFINT_CAL_ADDR */
#define TS_CAL1                 (int32_t)1034        /*!< TS_CAL1 value retrieved from TEMPSENSOR_CAL1_ADDR */
#define TS_CAL2                 (int32_t)1381        /*!< TS_CAL2 value retrieved from TEMPSENSOR_CAL2_ADDR */
#define TS_CAL1_TEMP            TEMPSENSOR_CAL1_TEMP    /*!< TS_CAL1 value */
#define TS_CAL2_TEMP            TEMPSENSOR_CAL2_TEMP    /*!< TS_CAL2 value */
#define CONTRAST_10_PERCENT     27              /*!< 10% of a contrast step (12-bit resolution) */
#define INTENSITY_10_PERCENT    40              /*!< 10% of a intensity step (12-bit resolution) */

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
 * The ADC is configured with a resolution of 12 bits and to be triggered by the trigger output of the
 * TIM2 on the rising edge, the scan sequence is set to fully configurable to get the measurements in
 * the desired order, first the temperature sensor, second the pot0 (intensity) and finally the pot1
 * (constrast). The sampling time common 1 si configured to 79.5 ADC cycles, this is to get a time 
 * conversion greater than the ts_temp specified in the datasheet.
 * The ADC divider is set to 2.
 * Time conversion.
 * t_conv = t_smpl + t_sar = ( 79.5 + 12.5 ) * t_adc_clk = 5.75 us
 * The oversampling configuration is set with a ratio of 4, and the division coefficient is set to 4.
 */
void Analogs_Init( void )
{
    ADC_HandleTypeDef ADC_Handler = {0};

    TIM_MasterConfigTypeDef TIM2_MasterConfig = {0};

    ADC_ChannelConfTypeDef sChanConfig = {0};

    static DMA_HandleTypeDef DMA_Handler = {0};

    TIM_HandleTypeDef TIM2_Handler = {0};

    HAL_StatusTypeDef Status = HAL_ERROR;

    /* Timer 2 configuration */
    TIM2_Handler.Instance               = TIM2;
    TIM2_Handler.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    TIM2_Handler.Init.Prescaler         = TIM2_PRESCALER;
    TIM2_Handler.Init.Period            = TIM2_PERIOD;
    TIM2_Handler.Init.CounterMode       = TIM_COUNTERMODE_UP;
    TIM2_Handler.Init.RepetitionCounter = 0;

    Status = HAL_TIM_Base_Init( &TIM2_Handler );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );

    TIM2_MasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    TIM2_MasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_ENABLE;

    Status = HAL_TIMEx_MasterConfigSynchronization( &TIM2_Handler, &TIM2_MasterConfig );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );
    
    /* DMA configuration */
    DMA_Handler.Instance                 = DMA1_Channel1;
    DMA_Handler.Init.Request             = DMA_REQUEST_ADC1;
    DMA_Handler.Init.Direction           = DMA_PERIPH_TO_MEMORY;
    DMA_Handler.Init.MemInc              = DMA_MINC_ENABLE;
    DMA_Handler.Init.PeriphInc           = DMA_PINC_DISABLE;
    DMA_Handler.Init.MemDataAlignment    = DMA_MDATAALIGN_WORD;
    DMA_Handler.Init.PeriphDataAlignment = DMA_PDATAALIGN_WORD;
    DMA_Handler.Init.Mode                = DMA_CIRCULAR;
    DMA_Handler.Init.Priority            = DMA_PRIORITY_HIGH;

    Status = HAL_DMA_Init( &DMA_Handler );
    assert_error( Status == HAL_OK, DMA_RET_ERROR );

    /* Link the ADC to the DMA */
    ADC_Handler.DMA_Handle = &DMA_Handler;
    DMA_Handler.Parent     = &ADC_Handler;

    /* ADC configuration */
    ADC_Handler.Instance                        = ADC1;
    ADC_Handler.Init.ClockPrescaler             = ADC_CLOCK_SYNC_PCLK_DIV2;
    ADC_Handler.Init.Resolution                 = ADC_RESOLUTION12b;
    ADC_Handler.Init.ScanConvMode               = ADC_SCAN_ENABLE;
    ADC_Handler.Init.DataAlign                  = ADC_DATAALIGN_RIGHT;
    ADC_Handler.Init.SamplingTimeCommon1        = ADC_SAMPLETIME_79CYCLES_5;
    ADC_Handler.Init.ExternalTrigConv           = ADC_EXTERNALTRIG_T2_TRGO;
    ADC_Handler.Init.ExternalTrigConvEdge       = ADC_EXTERNALTRIG_EDGE_RISING;
    ADC_Handler.Init.NbrOfConversion            = N_ADC_CHANNELS_USED;
    ADC_Handler.Init.EOCSelection               = ADC_EOC_SEQ_CONV;
    ADC_Handler.Init.Overrun                    = ADC_OVR_DATA_OVERWRITTEN;
    ADC_Handler.Init.DMAContinuousRequests      = ENABLE;

    ADC_Handler.Init.OversamplingMode           = ENABLE;
    ADC_Handler.Init.Oversampling.Ratio         = ADC_OVERSAMPLING_RATIO;
    ADC_Handler.Init.Oversampling.RightBitShift = ADC_RIGHTBITSHIFT_2;
    ADC_Handler.Init.Oversampling.TriggeredMode = ADC_TRIGGEREDMODE_SINGLE_TRIGGER;

    Status = HAL_ADC_Init( &ADC_Handler );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration pot0 - intensity */
    sChanConfig.Channel      = ADC_CHANNEL_0;
    sChanConfig.Rank         = ADC_REGULAR_RANK_2;
    sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration pot1 - contrast */
    sChanConfig.Channel      = ADC_CHANNEL_1;
    sChanConfig.Rank         = ADC_REGULAR_RANK_3;
    sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration temperature sensor */
    sChanConfig.Channel      = ADC_CHANNEL_TEMPSENSOR;
    sChanConfig.Rank         = ADC_REGULAR_RANK_1;
    sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel configuration internal Vref */
    sChanConfig.Channel      = ADC_CHANNEL_VREFINT;
    sChanConfig.Rank         = ADC_REGULAR_RANK_6;
    sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    Status = HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel 5 configuration  */
    sChanConfig.Channel      = ADC_CHANNEL_6;
    sChanConfig.Rank         = ADC_REGULAR_RANK_4;
    sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    /* ADC channel 6 configuration */
    sChanConfig.Channel      = ADC_CHANNEL_7;
    sChanConfig.Rank         = ADC_REGULAR_RANK_5;
    sChanConfig.SamplingTime = ADC_SAMPLINGTIME_COMMON_1;
    HAL_ADC_ConfigChannel( &ADC_Handler, &sChanConfig );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    Status = HAL_ADCEx_Calibration_Start( &ADC_Handler );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    Status = HAL_ADC_Start_DMA( &ADC_Handler, &AdcData[0], N_ADC_CHANNELS_USED );
    assert_error( Status == HAL_OK, ADC_RET_ERROR );

    Status = HAL_TIM_Base_Start( &TIM2_Handler );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );
}

/**
 * @brief   Calculates the internal temperature value.
 * 
 * This function retrieve the ADC values of the Vref and the internal temperature
 * sensor, with this values calculate the temperature in celsius degrees using the
 * formula from the reference manual, the calibration values stored in memory were 
 * retrieved and used as macros.
 * 
 * @retval  Temperature (degree Celsius), falls within the range of -40 to 125. 
*/
int8_t Analogs_GetTemperature( void )
{
    int8_t temp = 0;
    uint32_t temperature;
    uint32_t voltage_ref;

    __disable_irq( );
    
    temperature = AdcData[ TEMP_INDEX ];                
    voltage_ref = AdcData[ VREF_INDEX ];

    __enable_irq( );

    if ( ( temperature <= MAX_ADC_VALUE ) && ( voltage_ref <= MAX_ADC_VALUE ) )
    {
        uint16_t Vref;
        
        Vref =  ( VREF_CHARAC * VREFINT_CAL_VREF ) / (voltage_ref);
        
        temp = ((( ((int32_t)((temperature * Vref) / TEMPSENSOR_CAL_VREFANALOG) - TS_CAL1) ) * (TS_CAL2_TEMP - TS_CAL1_TEMP) ) / (TS_CAL2 - TS_CAL1) ) + TS_CAL1_TEMP; 
    }

    return temp;
}

/**
 * @brief   Function to get the LCD contrast level based on potentiometer pot0.
 * 
 * This function retrieves the contrast level from the ADC data array, scales it
 * using a division factor specified by ADC_CONTRAST_DIV (273), and returns a value
 * within the range of 0 to 15.
 *
 * @retval  The contrast level, ranging from 0 to 15.
 */
uint8_t Analogs_GetContrast( void )
{
    uint8_t contrast = 0;
    bool FuncSafety_flg;
    uint32_t contrastADC;
    uint32_t contrastADC_sec_lecture;

    __disable_irq( );
    
    contrastADC = AdcData[ CONTRAST_INDEX ];                 
    contrastADC_sec_lecture = AdcData[ POT1_ADC7_INDEX ];

    __enable_irq( );

    if ( contrastADC <= MAX_ADC_VALUE )
    {
        contrast = (uint8_t)( contrastADC / ADC_CONTRAST_DIV );
    }

    FuncSafety_flg = ( (int32_t)contrastADC ) <= (( (int32_t) contrastADC_sec_lecture ) + CONTRAST_10_PERCENT);
    assert_error( FuncSafety_flg, POT1_H_READING_ERROR );

    FuncSafety_flg = ( (int32_t)contrastADC ) >= (( (int32_t) contrastADC_sec_lecture ) - CONTRAST_10_PERCENT);
    assert_error( FuncSafety_flg, POT1_L_READING_ERROR );   

    return contrast;
}

/**
 * @brief   Function to get the LCD intensity based on potentiometer pot1.
 * 
 * This function calculates the intensity level of an LCD display based on the value read 
 * from a potentiometer. The intensity level is scaled to a range from 0 to 100 with a 
 * resolution of 10 values.
 * 
 * @retval  The intesity level calculated with the ADC value, ranging from 0 to 100, with a 
 * resolution of 10 values 
 */
uint8_t Analogs_GetIntensity( void )
{
    uint8_t intensity = 0;
    bool FuncSafety_flg;
    uint32_t intensityADC;
    uint32_t intensityADC_sec_lecture;

    __disable_irq( );
    
    intensityADC = AdcData[ INTENSITY_INDEX ];               /* back up AdcData into local variables */
    intensityADC_sec_lecture = AdcData[ POT0_ADC6_INDEX ];

    __enable_irq( );
  
    if ( AdcData[ INTENSITY_INDEX ] <= MAX_ADC_VALUE )
    {
        intensity = (uint8_t)( ( ( intensityADC * LCD_INTENSITY_INTERVALS ) / MAX_ADC_VALUE ) * LCD_INTENSITY_INTERVALS );
    }

    FuncSafety_flg = ( (int32_t)intensityADC ) <= (( (int32_t) intensityADC_sec_lecture ) + INTENSITY_10_PERCENT);
    assert_error( FuncSafety_flg, POT0_H_READING_ERROR ); 

    FuncSafety_flg = ( ( (int32_t)intensityADC ) >= (( (int32_t) intensityADC_sec_lecture ) - INTENSITY_10_PERCENT));
    assert_error( FuncSafety_flg, POT0_L_READING_ERROR );    

    return intensity;
}