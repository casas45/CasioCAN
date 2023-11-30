/**
 * @file    main.c
 * @brief   **Template Application entry point**
 *
 * The main file is the entry point of the application or any user code, please provide the
 * proper description of this file according to your own implementation
 * This Demo app only blinks an LED connected to PortA Pin 5
 *
 * @note    Only the files inside folder app will be take them into account when the
 *          doxygen runs by typing "make docs", index page is generated in
 *          Build/doxigen/html/index.html
 */
#include "bsp.h"

/**
 * @brief   **Application entry point**
 *
 * Ptovide the proper description for function main according to your own
 * implementation
 *
 * @retval  None
 */
int main( void )
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_Init( );

    __HAL_RCC_GPIOA_CLK_ENABLE( );

    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin   = GPIO_PIN_5;
    HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

    for( ;; )
    {
        HAL_GPIO_TogglePin( GPIOA, GPIO_PIN_5 );
        HAL_Delay( 1000u );
    }

    return 0u;
}
