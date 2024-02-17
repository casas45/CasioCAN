/**
 * @file    analogs.h
 * 
 * @brief   Header file of the analogs driver.
*/
#ifndef ANALOGS_H
#define ANALOGS_H

#ifndef UTEST
#define STATIC static       /*!< Macro to remove static keyword only for unit tests */
#else
#define STATIC
#endif

#define TEMP_SENSOR_INDEX       0u
#define CONTRAST_INDEX          1u
#define INTENSITY_INDEX         2u

void Analogs_Init( void );

int8_t Analogs_GetTemperature( void );

uint8_t Analogs_GetContrast( void );

uint8_t Analogs_GetIntensity( void );


#endif