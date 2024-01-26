/*Archivo con la funciones de interrupcion del micrcontroladores, revisar archivo startup_stm32g0b1.S */
#include "bsp.h"


/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void NMI_Handler( void )
{
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