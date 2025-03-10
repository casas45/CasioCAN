/*Archivo con la funciones de interrupcion del micrcontroladores, revisar archivo startup_stm32g0b1.S */
#include "bsp.h"


/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void NMI_Handler( void )
{
    assert_error( 0u, ECC_TWO_ERROR );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void HardFault_Handler( void )
{
    assert_error( 0u, HARD_FAULT_ERROR );       /* Send to the safe state */
    assert_param( 0u ); 
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void SVC_Handler( void )
{
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void PendSV_Handler( void )
{
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void SysTick_Handler( void )
{
    HAL_IncTick( );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void TIM16_FDCAN_IT0_IRQHandler( void )
{
    HAL_FDCAN_IRQHandler( &CANHandler );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void WWDG_IRQHandler( void )
{
    HAL_WWDG_IRQHandler( &h_watchdog );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void TIM6_DAC_LPTIM1_IRQHandler( void )
{
    HAL_TIM_IRQHandler( &TIM6_Handler );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void FLASH_IRQHandler( void )
{
    assert_error( 0u, ECC_ONE_ERROR );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void RTC_TAMP_IRQHandler( void )
{
    HAL_RTC_AlarmIRQHandler( &h_rtc );
}

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void EXTI4_15_IRQHandler( void )
{
    HAL_GPIO_EXTI_IRQHandler( GPIO_PIN_15 );
}
