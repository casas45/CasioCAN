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

#define TASKS_N                 5u          /*!< Number of tasks registered in the scheduler */
#define TIMERS_N                1u          /*!< Number of timers registered in the scheduler */
#define TICK_VAL                5u          /*!< Tick value to scheduler */
#define PERIOD_SERIAL_TASK      10u         /*!< Serial task periodicity */
#define PERIOD_CLOCK_TASK       50u         /*!< Clock task periodicity */
#define ONE_SECOND              1000u       /*!< Value of 1000 ms */
#define PERIOD_HEARTBEAT_TASK   300u        /*!< Heartbeat task periodicity */
#define PERIOD_WATHCDOG_TASK    100u        /*!< Watchdog task periodicity */
#define WINDOW_VALUE_WWDG       127u        /*!< Watchdog window value */
#define PERIOD_DISPLAY_TASK     100u        /*!< Display task periodicity */

static void Heartbeat_InitTask( void );
static void Heartbeat_PeriodicTask( void );
static void Watchdog_InitTask( void );
static void Watchdog_PeriodicTask( void );

AppSched_Scheduler Scheduler;

/** @brief  struct to handle the WWDG*/
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
    (void) AppSched_registerTask( &Scheduler, Display_InitTask, Display_PeriodicTask, PERIOD_DISPLAY_TASK );
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
 * Max window value = (1/32MHz) * 4096 * 32 * 127 = 520.19 ms
 * Min window value = (1/32MHz) * 4096 * 32 * 63  = 258.05 ms
 * 
 * The WWDG must be refreshed before 262 mS to prevent a system reset.
*/
static void Watchdog_InitTask( void )
{
    __HAL_RCC_WWDG_CLK_ENABLE( ); 

    h_watchdog.Instance         = WWDG;
    h_watchdog.Init.Prescaler   = WWDG_PRESCALER_32;
    h_watchdog.Init.Counter     = WINDOW_VALUE_WWDG;    
    h_watchdog.Init.Window      = WINDOW_VALUE_WWDG;
    h_watchdog.Init.EWIMode     = WWDG_EWI_DISABLE;

    HAL_WWDG_Init( &h_watchdog );
}

/**
 * @brief   Function where the WWDG is refreshed.
*/
static void Watchdog_PeriodicTask( void )
{
    HAL_WWDG_Refresh( &h_watchdog );
}