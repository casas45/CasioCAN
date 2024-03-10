/**
 * @file    analogs.h
 * 
 * @brief   Header file of the analogs driver.
*/
#include <stdint.h>

#ifndef ANALOGS_H
#define ANALOGS_H

/**
 * @brief   enum to organize the ADC channels
*/
typedef enum
{

    TEMP_INDEX = 0u,        /*!< Temperature value index in AdcData array */
    INTENSITY_INDEX,        /*!< Intensity value index in AdcData array */
    CONTRAST_INDEX,         /*!< Contrast value index in AdcData array */
    POT0_ADC6_INDEX,             /*!< Pot0 second lecture value index in AdcData array */
    POT1_ADC7_INDEX,             /*!< Pot1 second lecture value index in AdcData array */
    VREF_INDEX,             /*!< Vref value index in AdcData array */
    N_ADC_CHANNELS_USED     /*!< Number of ADC channels used */

} AdcChannels;

void Analogs_Init( void );

int8_t Analogs_GetTemperature( void );

uint8_t Analogs_GetContrast( void );

uint8_t Analogs_GetIntensity( void );


#endif