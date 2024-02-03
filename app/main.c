/**
 * @file    main.c
 * @brief   main file where the scheduler is configured and run.
 *
 * In this file is the configuration of the scheduler to manage the tasks that handle message
 * processing and clock updates.  
 *
 */
#include "bsp.h"
#include "serial.h"
#include "clock.h"
#include "display.h"

#define TICK_VAL                5u          /*!< Tick value to scheduler */
#define ONE_SECOND              1000u       /*!< Value of 1000 ms */
#define WINDOW_VALUE_WWDG       100u        /*!< Watchdog window value */
#define COUNTER_VALUE_WWDG      127u        /*!< Watchdog counter value */
#define PERIOD_DISPLAY_TASK     100u        /*!< Display task periodicity */
#define LEDS                    0xFFu       /*!< define to initialize the 8 LEDs */
#define SAFE_STATE              1u          /*!< safe state value */      

static void Heartbeat_InitTask( void );
static void Heartbeat_PeriodicTask( void );
static void Watchdog_InitTask( void );
static void Watchdog_PeriodicTask( void );
void safe_state( const char *file, uint32_t line, uint8_t error );

AppSched_Scheduler Scheduler;

/** @brief  struct to handle the WWDG*/
WWDG_HandleTypeDef h_watchdog;

/** @brief  Variable to save the update timer ID */
uint8_t UpdateTimerID;

/** @brief  TIM6 Handler struct */
TIM_HandleTypeDef TIM6_Handler;

/**
 * @brief   **Application entry point**
 *
 * This function is the application entry point. It initializes the HAL library and configure the
 * scheduler, the tasks in charge of message processing and clock are registered with their periodicity
 * and starts the same scheduler.
 *
 * @retval  None
 */
int main( void )
{
    HAL_StatusTypeDef Status = FALSE;

    HAL_Init( );

    /*Scheduler config*/
    static AppSched_Task tasks[ TASKS_N ];
    static AppSched_Timer timers[ TIMERS_N ];

    Scheduler.tick      = TICK_VAL;
    Scheduler.tasks     = TASKS_N;
    Scheduler.taskPtr   = tasks;
    Scheduler.timers    = TIMERS_N;
    Scheduler.timerPtr  = timers;

    AppSched_initScheduler( &Scheduler );
    /*Register serial task*/
    Status = AppSched_registerTask( &Scheduler, Serial_InitTask, Serial_PeriodicTask, PERIOD_SERIAL_TASK );
    assert_error( Status != FALSE, SCHE_RET_ERROR );

    Status = AppSched_registerTask( &Scheduler, Clock_InitTask, Clock_PeriodicTask, PERIOD_CLOCK_TASK );
    assert_error( Status != FALSE, SCHE_RET_ERROR );

    Status = AppSched_registerTask( &Scheduler, Heartbeat_InitTask, Heartbeat_PeriodicTask, PERIOD_HEARTBEAT_TASK );    
    assert_error( Status != FALSE, SCHE_RET_ERROR );

    Status = AppSched_registerTask( &Scheduler, Display_InitTask, Display_PeriodicTask, PERIOD_DISPLAY_TASK );
    assert_error( Status != FALSE, SCHE_RET_ERROR );

    Status = AppSched_registerTask( &Scheduler, Watchdog_InitTask, Watchdog_PeriodicTask, PERIOD_WATCHDOG_TASK );
    assert_error( Status != FALSE, SCHE_RET_ERROR );

    /*Software timer register to update time and date in display*/
    UpdateTimerID = AppSched_registerTimer( &Scheduler, ONE_SECOND, ClockUpdate_Callback );
    
    Status = AppSched_startTimer( &Scheduler, UpdateTimerID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    AppSched_startScheduler( &Scheduler );

    return 0u;
}

/**
 * @brief   Function to initialize the onboard LED (PORTA-P5).
 * 
 * This function initialize the on-board LED that is in PORTA PIN5, the selected mode is
 * output push-pull, without pull resistor. 
*/
static void Heartbeat_InitTask( void )
{
    __HAL_RCC_GPIOA_CLK_ENABLE( );
    GPIO_InitTypeDef GPIO_Init_Struct;

    GPIO_Init_Struct.Mode   = GPIO_MODE_OUTPUT_PP;
    GPIO_Init_Struct.Pull   = GPIO_NOPULL;
    GPIO_Init_Struct.Speed  = GPIO_SPEED_FREQ_LOW;
    GPIO_Init_Struct.Pin    = GPIO_PIN_5;

    HAL_GPIO_Init( GPIOA, &GPIO_Init_Struct );
}

/**
 * @brief   Function that controls the blinking of the heartbeat LED.
*/
static void Heartbeat_PeriodicTask( void )
{
    HAL_GPIO_TogglePin( GPIOA, GPIO_PIN_5 );
}

/**
 * @brief   Funtion to initialize the Window Watchdog.
 * 
 * The WWDG is an APB peripheral, and the bus is working with a frequency of 32 MHz.
 * 
 * Counter value    = (1/32MHz) * 4096 * 32 * 127 = 520.19 ms
 * Max window value = (1/32MHz) * 4096 * 32 * 100 = 409.60 ms
 * Min window value = (1/32MHz) * 4096 * 32 * 63  = 258.05 ms
 * 
 * The WWDG must be refreshed after passing 110 ms and before passing 262 ms to prevent 
 * a system reset.
*/
static void Watchdog_InitTask( void )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    __HAL_RCC_WWDG_CLK_ENABLE( );

    h_watchdog.Instance         = WWDG;
    h_watchdog.Init.Prescaler   = WWDG_PRESCALER_32;
    h_watchdog.Init.Counter     = COUNTER_VALUE_WWDG;    
    h_watchdog.Init.Window      = WINDOW_VALUE_WWDG;
    h_watchdog.Init.EWIMode     = WWDG_EWI_ENABLE;

    HAL_NVIC_SetPriority( WWDG_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( WWDG_IRQn );

    Status = HAL_WWDG_Init( &h_watchdog );

    assert_error( Status == HAL_OK, WWDG_RET_ERROR );
}

/**
 * @brief   Function where the WWDG is refreshed.
*/
static void Watchdog_PeriodicTask( void )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    Status = HAL_WWDG_Refresh( &h_watchdog );

    assert_error( Status == HAL_OK, WWDG_RET_ERROR );
}

/**
 * @brief   Safe state function.
 * 
 * Function where the used modules are disabled, output the corresponding error code using,
 * the PORTC LEDs and enter on an infinite loop.
 * 
 * @param   file String to indicate in which file is the error.
 * @param   line Line where the error its detected.
 * @param   error Error code. 
*/
void safe_state( const char *file, uint32_t line, uint8_t error )
{
    __disable_irq();        /* Disable interrupts */

    (void) file;
    (void) line;

    GPIO_InitTypeDef GPIO_Init;

    __HAL_RCC_GPIOC_CLK_ENABLE( );

    GPIO_Init.Mode      = GPIO_MODE_OUTPUT_PP;
    GPIO_Init.Pull      = GPIO_NOPULL;
    GPIO_Init.Speed     = GPIO_SPEED_FREQ_LOW;
    GPIO_Init.Pin       = LEDS;

    HAL_GPIO_Init( GPIOC, &GPIO_Init );

    HAL_GPIO_WritePin( GPIOA, GPIO_PIN_5, RESET );      /* Heartbeat LED turn off */

    HEL_LCD_Backlight( &LCD_Handler, LCD_OFF );             

    HAL_RTC_DeInit( &hrtc );                               

    HAL_FDCAN_DeInit( &CANHandler );                       

    HAL_SPI_DeInit( &SPI_Handler );

    HAL_GPIO_WritePin( GPIOC, error, SET );             /* output error code using LEDs on PORTC */

    while ( SAFE_STATE == TRUE )
    { 
        HAL_Delay( PERIOD_WATCHDOG_TASK );      /* delay to refreshed the WWDG in the correct period */
        HAL_WWDG_Refresh( &h_watchdog );
    }
    
}


