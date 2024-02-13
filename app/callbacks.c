/**
 * @file callbacks.c
 * 
 * @brief File where are the error callbacks of the used modules and the EWC of the WWDG.
*/
#include "bsp.h"

/**
 * @brief WWDG EWI callback.
 * 
 * When the watchdog wasn't refreshed this callback send to the safe state.
 * 
 * @param hwwdg pointer to the WWDG handle struct.
*/
void HAL_WWDG_EarlyWakeupCallback( WWDG_HandleTypeDef *hwwdg )
{
    HAL_WWDG_Refresh( hwwdg );
    
    assert_error( 0u, WWDG_RESET_ERROR );   /* Send to the safe state */
}

/**
 * @brief   FDCAN Error Callback.
 * 
 * This callback send the microcontroller to the safe state.
 * 
 * @param hfdcan pointer to the FDCAN handle struct.
*/
void HAL_FDCAN_ErrorCallback( FDCAN_HandleTypeDef *hfdcan )
{
    (void) hfdcan;
    assert_error( 0u, CAN_FUNC_ERROR );
}

/**
 * @brief   SPI Error Callback.
 * 
 * This callback send the microcontroller to the safe state.
 * 
 * @param hspi pointer to the SPI handle struct.
*/
void HAL_SPI_ErrorCallback( SPI_HandleTypeDef *hspi )
{
    (void) hspi;
    assert_error( 0u, SPI_FUNC_ERROR );
}

/**
 * @brief   TIM Error Callback.
 * 
 * This callback send the microcontroller to the safe state.
 * 
 * @param htim pointer to the TIM handle struct.
*/
void HAL_TIM_ErrorCallback( TIM_HandleTypeDef * htim )
{
    (void) htim;
    assert_error( 0u, TIM_FUNC_ERROR );
}