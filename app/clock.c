/**
 * @file    clock.c
 *
 * @brief   In this file the clock state machine is implemented.
 */
#include "clock.h"
#include "bsp.h"

#define N_MESSAGES_CLKQUEUE 20u     /*!< Number of messages in ClkQueue (20)*/
#define CENTENARY           100u    /*!< Value of a centenary */
#define TIM14_PRESCALER     40U     /*!< Value of the TIM14 prescaler */
#define TIM14_PERIOD        1600u   /*!< Value of the TIM14 period */
#define BUZZER_DUTY_CYCLE   (TIM14_Handler.Init.Period / 2) /*!< 50% of duty cycle, TIM14 channel */

/**
 * @brief Queue to communicate serial and clock tasks.
 */
AppQue_Queue ClockQueue;

/**
 * @brief   RTC structure
 */
RTC_HandleTypeDef hrtc;

/**
 * @brief   TIM14 Handle struct.
*/
static TIM_HandleTypeDef TIM14_Handler;

/**
 * @brief   Alarm Activated Flag.
*/
/* cppcheck-suppress misra-c2012-8.4 ; false warning, the macro STATIC makes the variable static */
STATIC uint8_t AlarmActivated_flg;


STATIC APP_MsgTypeDef Clock_Set_Time( APP_MsgTypeDef *PtrMsgClk );

STATIC APP_MsgTypeDef Clock_Set_Date( APP_MsgTypeDef *PtrMsgClk );

STATIC APP_MsgTypeDef Clock_Set_Alarm( APP_MsgTypeDef *PtrMsgClk );

STATIC APP_MsgTypeDef Clock_Send_Display_Msg( APP_MsgTypeDef *PtrMsgClk );

STATIC APP_MsgTypeDef Clock_Alarm_Activated( APP_MsgTypeDef *PtrMsgClk );

STATIC APP_MsgTypeDef Clock_Deactivate_Alarm( APP_MsgTypeDef *PtrMsgClk );

/**
 * @brief   Function to initialize RTC module and ClkQueue.
 *
 * In this function is configured the ClkQueue that is in charge of communicate serial and clock
 * tasks, and also is initialized the RTC module with 24hour format, the values for PREDIV_A and
 * PREDIV_S are 127 and 255, respectively, to deliver a frequency of 1 Hz clock to the calendar
 * unit, taking into account the RTC is working with the LSE clock.
 * The alarm A interrupt is enabled.
 * The channel 1 of the TIM 14 is configure as PWM channel, with a frequency of 1 kHz and 50% of
 * duty cycle, the APB frequency is 32MHz, but its used with a divider then the TIMPLCK is 2*PCLK,
 * then is used a prescaler of 40 and a period of 1600 to get the indicated frequency.
 */
void Clock_InitTask( void )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    RTC_TimeTypeDef sTime = { 0 };
    RTC_DateTypeDef sDate = { 0 };

    TIM_OC_InitTypeDef PWM_ch;
    GPIO_InitTypeDef GPIO_InitStruct;

    /* Configuration PWM chanel C12 (buzzer) */
    GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull      = GPIO_PULLUP;
    GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Pin       = GPIO_PIN_12;
    GPIO_InitStruct.Alternate = GPIO_AF2_TIM14;

    HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

    TIM14_Handler.Instance         = TIM14;
    TIM14_Handler.Init.Prescaler   = TIM14_PRESCALER;
    TIM14_Handler.Init.Period      = TIM14_PERIOD;
    TIM14_Handler.Init.CounterMode = TIM_COUNTERMODE_UP;

    Status = HAL_TIM_PWM_Init( &TIM14_Handler );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );

    PWM_ch.OCMode     = TIM_OCMODE_PWM1;
    PWM_ch.OCPolarity = TIM_OCPOLARITY_HIGH;
    PWM_ch.OCFastMode = TIM_OCFAST_DISABLE;
    PWM_ch.Pulse      = BUZZER_DUTY_CYCLE;

    Status = HAL_TIM_PWM_ConfigChannel( &TIM14_Handler, &PWM_ch, TIM_CHANNEL_1 );
    assert_error( Status == HAL_OK, TIM_RET_ERROR );

    /*Clock Queue config*/
    static APP_MsgTypeDef messagesClock[ N_MESSAGES_CLKQUEUE ];

    ClockQueue.Buffer   = messagesClock;
    ClockQueue.Elements = N_MESSAGES_CLKQUEUE;
    ClockQueue.Size     = sizeof( APP_MsgTypeDef );
    AppQueue_initQueue( &ClockQueue );

    /*RTC configuration*/
    hrtc.Instance          = RTC;
    hrtc.Init.AsynchPrediv = 127;
    hrtc.Init.SynchPrediv  = 255;
    hrtc.Init.HourFormat   = RTC_HOURFORMAT_24;
    hrtc.Init.OutPut       = RTC_OUTPUT_DISABLE;

    Status = HAL_RTC_Init( &hrtc );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*set Time */
    sTime.Hours   = 0x23;
    sTime.Minutes = 0x59;
    sTime.Seconds = 0x00;

    Status = HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*set Date */
    sDate.WeekDay = RTC_WEEKDAY_TUESDAY;
    sDate.Date    = 0x16;
    sDate.Month   = RTC_MONTH_JANUARY;
    sDate.Year    = 0x23;

    Status = HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*enable RTC alarm interrupt*/
    HAL_NVIC_SetPriority( RTC_TAMP_IRQn, 2, 0 );
    HAL_NVIC_EnableIRQ( RTC_TAMP_IRQn );
}

/**
 * @brief   Callback function to be run every second to write an update msg in ClkQueue.
 *
 * This function write a SERIAL_MSG_DISPLAY in the ClockQueue, this is to indicate that it's
 * time to update the display, and also the timer is started again to continue using it.
 */
void ClockUpdate_Callback( void )
{
    uint8_t Status             = FALSE;
    APP_MsgTypeDef msgCallback = { 0 };

    msgCallback.msg = CLOCK_MSG_DISPLAY;

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &msgCallback );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    Status = AppSched_startTimer( &Scheduler, UpdateTimerID ); /*Restart the timer */
    assert_error( Status == TRUE, SCHE_RET_ERROR );
}

/**
 * @brief   Callback function for TimerAlarmActiveOneSecond.
 * 
 * This callback function is used to toggle the buzzer and the LCD backlight in lapses of
 * one second.
 */
void TimerAlarmOneSecond_Callback( void )
{
    static uint8_t buzzer_flg = FALSE;
    uint8_t Status = FALSE;

    if ( buzzer_flg == TRUE )
    {
        Status = HAL_TIM_PWM_Stop( &TIM14_Handler, TIM_CHANNEL_1 ); /* Turn off the buzzer */
        assert_error( Status == HAL_OK, TIM_RET_ERROR );

        buzzer_flg = FALSE;
    }else{
        Status = HAL_TIM_PWM_Start( &TIM14_Handler, TIM_CHANNEL_1 ); /* Turn on the buzzer */
        assert_error( Status == HAL_OK, TIM_RET_ERROR );

        buzzer_flg = TRUE;
    }
    
    HEL_LCD_Backlight( &LCD_Handler, LCD_TOGGLE );

    Status = AppSched_startTimer( &Scheduler, TimerAlarmActiveOneSecond_ID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );
}

/**
 * @brief   Callback function for TimerAlarmActiveOneMinute.
 *  
 * This callback is called when the period of 1 second has passed and write in the clock queue
 * the correspond event to deactivate the alarm. 
 */
void TimerDeactivateAlarm_Callback( void )
{
    uint8_t Status = FALSE;

    APP_MsgTypeDef alarmMsg;
    alarmMsg.msg = CLOCK_MSG_DEACTIVATE_ALARM;

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &alarmMsg );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );
}

/**
 * @brief   Function where the event machine is implemented.
 *
 * The state machine implementation is made througha a switch sentence where is evaluated
 * a ClkState variable that is in charge to save the next state to run.
 */
void Clock_PeriodicTask( void )
{
    APP_MsgTypeDef MsgClkRead = { 0 };

    APP_MsgTypeDef ( *ClockEventsMachine[ N_CLK_EVENTS ] )( APP_MsgTypeDef *PtrMsgClk ) =
    {
    Clock_Set_Time,
    Clock_Set_Date,
    Clock_Set_Alarm,
    Clock_Send_Display_Msg,
    Clock_Alarm_Activated,
    Clock_Deactivate_Alarm
    };

    while( ( HIL_QUEUE_isQueueEmptyISR( &ClockQueue ) == FALSE ) )
    {
        uint8_t Status = FALSE;

        Status = HIL_QUEUE_readDataISR( &ClockQueue, &MsgClkRead );
        assert_error( Status == TRUE, QUEUE_RET_ERROR );

        if( MsgClkRead.msg < (uint8_t) N_CLK_EVENTS )
        {
            (void) ClockEventsMachine[ MsgClkRead.msg ]( &MsgClkRead );
        }
    }
}

/**
 * @brief   Function to update RTC time.
 *
 * This function is called when a time msg arrive from serial task, the time is set using the
 * HAL_RTC_SetTime function with the structure sTime that previously storage the parameters
 * corresponding to time from the pointer to the read msg.
 *
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
 * 
 * @return The next clock event.
 */
STATIC APP_MsgTypeDef Clock_Set_Time( APP_MsgTypeDef *PtrMsgClk )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    RTC_TimeTypeDef sTime = { 0 };

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;

    sTime.Hours   = PtrMsgClk->tm.tm_hour;
    sTime.Minutes = PtrMsgClk->tm.tm_min;
    sTime.Seconds = PtrMsgClk->tm.tm_sec;

    Status = HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    APP_MsgTypeDef alarmMsg = {0};
    alarmMsg.msg = CLK_MSG_NONE;

    if( AlarmActivated_flg == TRUE )
    {
        alarmMsg.msg = CLOCK_MSG_DEACTIVATE_ALARM;

        Status = HIL_QUEUE_writeDataISR( &ClockQueue, &alarmMsg );
        assert_error( Status == TRUE, QUEUE_RET_ERROR );
    }

    return alarmMsg;
}

/**
 * @brief   Function to update RTC DATE.
 *
 * This function is called when a date message arrives from serial task, the values of the date
 * are extracted from the pointer to the received message and are then stored. Following this, the
 * HAL_RTC_SetDate function is utilized to update the date values in the in the RTC.
 *
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
 *
 * @return The next clock event.
 * 
 * @note    Only the last two digits of the year are used because that's how the RTC works.
 */
STATIC APP_MsgTypeDef Clock_Set_Date( APP_MsgTypeDef *PtrMsgClk )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    RTC_DateTypeDef sDate = { 0 };

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;

    sDate.WeekDay = PtrMsgClk->tm.tm_wday;
    sDate.Date    = PtrMsgClk->tm.tm_mday;
    sDate.Month   = PtrMsgClk->tm.tm_mon;
    sDate.Year    = (uint8_t)( PtrMsgClk->tm.tm_year % CENTENARY ); /*Get last two digits of the year*/

    Status = HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    APP_MsgTypeDef alarmMsg = {0};
    alarmMsg.msg = CLK_MSG_NONE;

    if( AlarmActivated_flg == TRUE )
    {
        alarmMsg.msg = CLOCK_MSG_DEACTIVATE_ALARM;

        Status = HIL_QUEUE_writeDataISR( &ClockQueue, &alarmMsg );
        assert_error( Status == TRUE, QUEUE_RET_ERROR );
    }

    return alarmMsg;
}

/**
 * @brief   Function to set the RTC Alarm.
 *
 * This function is called when a alarm msg arrive from serial task, using the pointer to read
 * message the values are storage in the respective elements of sAlarm, then this strucutre is
 * used to set the alarm in the RTC.
 *
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
 * 
 * @return The next clock event.
 */
STATIC APP_MsgTypeDef Clock_Set_Alarm( APP_MsgTypeDef *PtrMsgClk )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    RTC_AlarmTypeDef sAlarm = { 0 };

    APP_MsgTypeDef nextEventDisplay;
    nextEventDisplay.msg = DISPLAY_MSG_ALARM_SET;

    APP_MsgTypeDef alarmMsg  = {0};
    alarmMsg.msg = CLK_MSG_NONE;

    if( AlarmActivated_flg == TRUE )
    {
        alarmMsg.msg = CLOCK_MSG_DEACTIVATE_ALARM;

        Status = HIL_QUEUE_writeDataISR( &ClockQueue, &alarmMsg );
        assert_error( Status == TRUE, QUEUE_RET_ERROR );
    }

    sAlarm.AlarmMask         = RTC_ALARMMASK_DATEWEEKDAY | RTC_ALARMMASK_SECONDS; /* Ignore date and seconds */
    sAlarm.Alarm             = RTC_ALARM_A;
    sAlarm.AlarmTime.Hours   = PtrMsgClk->tm.tm_hour;
    sAlarm.AlarmTime.Minutes = PtrMsgClk->tm.tm_min;

    Status = HAL_RTC_SetAlarm_IT( &hrtc, &sAlarm, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    Status = HIL_QUEUE_writeDataISR( &DisplayQueue, &nextEventDisplay );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    return alarmMsg;
}

/**
 * @brief   Function to write an updated message in DisplayQueue.
 *
 * This funtion get the date, time and alarm values using the structures sTime, sDate and sAlarm,
 * and the respective functions from HAL library.
 * And then that information is writed in the DisplayQueue.
 *
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
 * 
 * @return  The next display event with the date and time parameters. 
 */
STATIC APP_MsgTypeDef Clock_Send_Display_Msg( APP_MsgTypeDef *PtrMsgClk )
{
    (void)PtrMsgClk;

    HAL_StatusTypeDef Status = HAL_ERROR;

    RTC_TimeTypeDef sTime = { 0 };
    RTC_DateTypeDef sDate = { 0 };

    APP_MsgTypeDef updateMsg;
    updateMsg.msg = DISPLAY_MSG_UPDATE;

    Status = HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    Status = HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    updateMsg.tm.tm_hour = sTime.Hours;
    updateMsg.tm.tm_min  = sTime.Minutes;
    updateMsg.tm.tm_sec  = sTime.Seconds;

    updateMsg.tm.tm_mday = sDate.Date;
    updateMsg.tm.tm_mon  = sDate.Month;
    updateMsg.tm.tm_year = sDate.Year;
    updateMsg.tm.tm_wday = sDate.WeekDay;

    /*Write to the display queue*/
    Status = HIL_QUEUE_writeDataISR( &DisplayQueue, &updateMsg );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    return updateMsg;
}

/**
 * @brief   Event where the processes to activate the alarm are initiated.
 *
 * This event is in charge of start the processes to indicate that the alarm has been activated
 * first set the AlarmActivated_flg to TRUE, stop the update timer, start the Alarm Timers and
 * write in the DisplayQueue to show the message "ALARM!!!" in the LCD.
 *
 * @param   PtrMsgClk Pointer to message clock read.
 * 
 * @return The next display event.
 */
STATIC APP_MsgTypeDef Clock_Alarm_Activated( APP_MsgTypeDef *PtrMsgClk )
{
    (void) PtrMsgClk;

    uint8_t Status = FALSE;

    APP_MsgTypeDef displayMsg = {0};
    displayMsg.msg = DISPLAY_MSG_ALARM_ACTIVE;

    AlarmActivated_flg = TRUE;  /* Set flag */

    Status = AppSched_stopTimer( &Scheduler, UpdateTimerID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    Status = HIL_QUEUE_writeDataISR( &DisplayQueue, &displayMsg );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    Status = AppSched_startTimer( &Scheduler, TimerDeactivateAlarm_ID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    Status = AppSched_startTimer( &Scheduler, TimerAlarmActiveOneSecond_ID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    return displayMsg;
}

/**
 * @brief   Event to deactivate the alarm.
 * 
 * This event is in charge of stop the alarm timers, turn off the buzzer, turn on the LCD backlight
 * set the AlarmActivated_flg to false, restar the update timer and write in the clock queue the
 * next event that is to update the display.
 * 
 * @param   PtrMsgClk   Pointer to the clock message read.
 * 
 * @return The next clock event.
*/
STATIC APP_MsgTypeDef Clock_Deactivate_Alarm( APP_MsgTypeDef *PtrMsgClk )
{
    (void) PtrMsgClk;

    APP_MsgTypeDef updateMsg  = {0};
    updateMsg.msg = CLOCK_MSG_DISPLAY;

    uint8_t Status = FALSE;

    AlarmActivated_flg = FALSE;

    Status = HAL_TIM_PWM_Stop( &TIM14_Handler, TIM_CHANNEL_1 ); /* Turn off the buzzer */
    assert_error( Status == HAL_OK, TIM_RET_ERROR );

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &updateMsg ); /* Write the update display event */
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    Status = AppSched_stopTimer( &Scheduler, TimerAlarmActiveOneSecond_ID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    Status = AppSched_stopTimer( &Scheduler, TimerDeactivateAlarm_ID );
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    Status = AppSched_startTimer( &Scheduler, UpdateTimerID ); /* restart the update timer */
    assert_error( Status == TRUE, SCHE_RET_ERROR );

    HEL_LCD_Backlight( &LCD_Handler, LCD_ON );

    return updateMsg;
}

/**
 * @brief   Alarm A callback.
 *
 * This callback is used to initiate the processes that need to be executed when the
 * alarm is activated.
 *
 * @param   hrtc pointer to the RTC handle struct.
 */
/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is declared at HAL library */
void HAL_RTC_AlarmAEventCallback( RTC_HandleTypeDef *hrtc )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    APP_MsgTypeDef alarmMsg;
    alarmMsg.msg = CLOCK_MSG_ALARM_ACTIVATED;

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &alarmMsg );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    Status = HAL_RTC_DeactivateAlarm( hrtc, RTC_ALARM_A );     /* Disable the alarm A*/
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
}