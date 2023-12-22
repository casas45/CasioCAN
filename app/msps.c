/*Archivo con la funciones de las incilaizaciones auxiliares de la libreria*/
#include "bsp.h"

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void HAL_MspInit( void )
{
    RCC_OscInitTypeDef        RCC_OscInitStruct = {0};
    RCC_PeriphCLKInitTypeDef  PeriphClkInitStruct = {0};

    RCC_ClkInitTypeDef  RCC_ClkInitStruct       = {0};

    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    
    /*Enable backup domain*/
    HAL_PWREx_ControlVoltageScaling( PWR_REGULATOR_VOLTAGE_SCALE1 );
    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_LSEDRIVE_CONFIG( RCC_LSEDRIVE_LOW );

    /*reset previous RTC source clock*/
    PeriphClkInitStruct.PeriphClockSelection    = RCC_PERIPHCLK_RTC;
    PeriphClkInitStruct.RTCClockSelection       = RCC_RTCCLKSOURCE_NONE;
    HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );
    
    /** 
     * Init the LSE oscillator and PLL
     * The PLL is configured with the HSI, M = 1 and N = 8 
     * fVCO = fPLLIN x ( N / M ) = 16MHz x (8 / 1) = 128MHz
     * fPLLP = fVCO / P = 128MHz / 2 = 64MHz
     * fPLLQ = fVCO / Q = 128MHz / 2 = 64MHz
     * fPLLR = fVCO / R = 128MHz / 2 = 64MHz
    */
    RCC_OscInitStruct.OscillatorType        = RCC_OSCILLATORTYPE_LSI | RCC_OSCILLATORTYPE_LSE | RCC_OSCILLATORTYPE_HSI;
    RCC_OscInitStruct.HSIState              = RCC_HSI_ON;
    RCC_OscInitStruct.HSIDiv                = RCC_HSI_DIV1;
    RCC_OscInitStruct.HSICalibrationValue   = RCC_HSICALIBRATION_DEFAULT;
    RCC_OscInitStruct.PLL.PLLState          = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource         = RCC_PLLSOURCE_HSI;
    RCC_OscInitStruct.PLL.PLLM              = RCC_PLLM_DIV1;
    RCC_OscInitStruct.PLL.PLLN              = 8;
    RCC_OscInitStruct.PLL.PLLP              = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ              = RCC_PLLQ_DIV2;
    RCC_OscInitStruct.PLL.PLLR              = RCC_PLLR_DIV2;
    RCC_OscInitStruct.LSEState              = RCC_LSE_ON;    /*enable LSE*/
    RCC_OscInitStruct.LSIState              = RCC_LSI_OFF;   /*disable LSI*/
    HAL_RCC_OscConfig( &RCC_OscInitStruct );

    /*Set LSE as source clock for the RTC*/
    PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    HAL_RCCEx_PeriphCLKConfig( &PeriphClkInitStruct );
    
    __HAL_RCC_RTC_ENABLE();         /*Enable the clock for the RTC */
    __HAL_RCC_RTCAPB_CLK_ENABLE();

    /* Initializes the CPU, AHB and APB buses clocks */
    RCC_ClkInitStruct.ClockType         = RCC_CLOCKTYPE_ALL;        /* HCLK, SYSCLK and PCLK1*/
    RCC_ClkInitStruct.SYSCLKSource      = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider     = RCC_SYSCLK_DIV1;          /* AHB - 64 MHz */
    RCC_ClkInitStruct.APB1CLKDivider    = RCC_HCLK_DIV2;            /* APB - 32 MHz */
    HAL_RCC_ClockConfig( &RCC_ClkInitStruct, FLASH_LATENCY_2 );
}


/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void HAL_FDCAN_MspInit(FDCAN_HandleTypeDef *hfdcan)
{
    (void) hfdcan;
    GPIO_InitTypeDef GpioCanStruct;

    __HAL_RCC_FDCAN_CLK_ENABLE();
    __HAL_RCC_GPIOD_CLK_ENABLE();
    
    GpioCanStruct.Mode      = GPIO_MODE_AF_PP;
    GpioCanStruct.Alternate = GPIO_AF3_FDCAN1;
    GpioCanStruct.Pin       = GPIO_PIN_0 | GPIO_PIN_1;
    GpioCanStruct.Pull      = GPIO_NOPULL;
    GpioCanStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init( GPIOD, &GpioCanStruct );

    HAL_NVIC_SetPriority(TIM16_FDCAN_IT0_IRQn, 2, 0);
    HAL_NVIC_EnableIRQ(TIM16_FDCAN_IT0_IRQn);
}