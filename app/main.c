/**
 * @file    main.c
 * @brief   main file where the scheduler is configured and run.
 *
 * In this file is the configuration of the scheduler to manage the tasks that handle message
 * processing and clock updates.  
 *
 */
#include "bsp.h"

#define TASKS_N                 4u          /*!< Number of tasks registered in the scheduler */
#define TIMERS_N                1u          /*!< Number of timers registered in the scheduler */
#define TICK_VAL                5u          /*!< Tick value to scheduler */
#define PERIOD_SERIAL_TASK      10u         /*!< Serial task periodicity */
#define PERIOD_CLOCK_TASK       50u         /*!< Clock task periodicity */
#define ONE_SECOND              1000u       /*!< Value of 1000 ms*/
#define PERIOD_HEARTBEAT_TASK   300u        /*!< Heartbeat task periodicity */
#define PERIOD_WATHCDOG_TASK    60u         /*!< Watchdog task periodicity */

/**
 * @brief   Reference to semihosting function.
*/
extern void initialise_monitor_handles(void);

void Heartbeat_InitTask( void );
void Heartbeat_PeriodicTask( void );
void Watchdog_InitTask( void );
void Watchdog_PeriodicTask( void );

AppSched_Scheduler Scheduler;

static WWDG_HandleTypeDef h_watchdog;

/** @brief  Variable to save the update timer ID */
uint8_t UpdateTimerID;

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
    initialise_monitor_handles();
    
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
    (void) AppSched_registerTask( &Scheduler, Serial_InitTask, Serial_PeriodicTask, PERIOD_SERIAL_TASK );
    (void) AppSched_registerTask( &Scheduler, Clock_InitTask, Clock_PeriodicTask, PERIOD_CLOCK_TASK );
    (void) AppSched_registerTask( &Scheduler, Heartbeat_InitTask, Heartbeat_PeriodicTask, PERIOD_HEARTBEAT_TASK );
    (void) AppSched_registerTask( &Scheduler, Watchdog_InitTask, Watchdog_PeriodicTask, PERIOD_WATHCDOG_TASK );

    /*Software timer register to update time and date in display*/
    UpdateTimerID = AppSched_registerTimer( &Scheduler, ONE_SECOND, ClockUpdate_Callback );
    (void) AppSched_startTimer( &Scheduler, UpdateTimerID );

    AppSched_startScheduler( &Scheduler );

    return 0u;
}

/**
 * @brief   Function to initialize the onboard LED (PORTA-P5).
 * 
 * This function initialize the on-board LED that is in PORTA PIN5, the selected mode is
 * output push-pull, without pull resistor. 
*/
void Heartbeat_InitTask( void )
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
void Heartbeat_PeriodicTask( void )
{
    HAL_GPIO_TogglePin( GPIOA, GPIO_PIN_5 );
}

/**
 * @brief   Funtion to initialize the Window Watchdog.
 * 
 * The WWDG is an APB peripheral, and the bus is working with a frequency of 32 MHz.
 * The WWDG timeout is calculated with this formula
 * twwdg = tpclk * 4096 * ( 2 ^ WWDG prescaler) * Window Value
 * twwdg = ( 1 / 32 MHz ) * 4096 * ( 65536 ) * 9 = 75.49 mS
 * 
*/
void Watchdog_InitTask( void )
{
    h_watchdog.Instance         = WWDG;
    h_watchdog.Init.Prescaler   = WWDG_PRESCALER_16;
    h_watchdog.Init.Counter     = 0x49;             /* Min Value (0x40) + 9*/
    h_watchdog.Init.Window      = 0x49;
    h_watchdog.Init.EWIMode     = WWDG_EWI_DISABLE;

    HAL_WWDG_Init( &h_watchdog );
}

/**
 * @brief   Function where the WWDG is refreshed.
*/
void Watchdog_PeriodicTask( void )
{
    HAL_WWDG_Refresh( &h_watchdog );
}