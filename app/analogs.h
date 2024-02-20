/**
 * @file    analogs.h
 * 
 * @brief   Header file of the analogs driver.
*/
#include <stdint.h>

#ifndef ANALOGS_H
#define ANALOGS_H

#ifndef UTEST
#define STATIC static       /*!< Macro to remove static keyword only for unit tests */
#else
#define STATIC
#endif

#define N_ADC_CHANNELS_USED     4u              /*!< Number of ADC channels used */
#define TEMP_INDEX              0u      /*!< Temperature value index in AdcData array */
#define CONTRAST_INDEX          1u      /*!< Contrast value index in AdcData array */
#define INTENSITY_INDEX         2u      /*!< Intensity value index in AdcData array */
#define VREF_INDEX              3u      /*!< Vref value index in AdcData array */

void Analogs_Init( void );

int8_t Analogs_GetTemperature( void );

uint8_t Analogs_GetContrast( void );

uint8_t Analogs_GetIntensity( void );


#endif