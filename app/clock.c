/**
 * @file    clock.c
 * 
 * @brief   In this file the clock state machine is implemented.
*/
#include "clock.h"
#include "bsp.h"

#define N_MESSAGES_CLKQUEUE     20u      /*!< Number of messages in ClkQueue (20)*/
#define CENTENARY               100u     /*!< Value of a centenary */

/**
 * @brief Queue to communicate serial and clock tasks.
*/
AppQue_Queue ClockQueue;

/**
 * @brief   RTC structure
*/
RTC_HandleTypeDef hrtc;

/** @brief Struct used to set and get time in RTC */
static RTC_TimeTypeDef sTime = {0};
/** @brief Struct used to set and get date in RTC */
static RTC_DateTypeDef sDate = {0};
/** @brief Struct used to set and get alarm in RTC */
static RTC_AlarmTypeDef sAlarm = {0};


STATIC void Update_Time( APP_MsgTypeDef *PtrMsgClk );

STATIC void Update_Date( APP_MsgTypeDef *PtrMsgClk );

STATIC void Set_Alarm( APP_MsgTypeDef *PtrMsgClk );

STATIC void Send_Display_Msg( APP_MsgTypeDef *PtrMsgClk );

/**
 * @brief   Function to initialize RTC module and ClkQueue.
 * 
 * In this function is configured the ClkQueue that is in charge of communicate serial and clock
 * tasks, and also is initialized the RTC module with 24hour format, the values for PREDIV_A and 
 * PREDIV_S are 127 and 255, respectively, to deliver a frequency of 1 Hz clock to the calendar
 * unit, taking into account the RTC is working with the LSE clock.
 * The alarm A is configure to consider just the hour and minutes.
*/
void Clock_InitTask( void )
{
    HAL_StatusTypeDef Status = HAL_ERROR;
    
    /*Clock Queue config*/
    static APP_MsgTypeDef messagesClock[ N_MESSAGES_CLKQUEUE ];

    ClockQueue.Buffer   = messagesClock;
    ClockQueue.Elements = N_MESSAGES_CLKQUEUE;
    ClockQueue.Size     = sizeof( APP_MsgTypeDef );
    AppQueue_initQueue( &ClockQueue );

    /*RTC configuration*/
    hrtc.Instance            = RTC;
    hrtc.Init.AsynchPrediv   = 127;
    hrtc.Init.SynchPrediv    = 255;
    hrtc.Init.HourFormat     = RTC_HOURFORMAT_24;
    hrtc.Init.OutPut         = RTC_OUTPUT_DISABLE;

    Status = HAL_RTC_Init( &hrtc );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*set Time */
    sTime.Hours     = 0x23;
    sTime.Minutes   = 0x59;
    sTime.Seconds   = 0x00;

    Status = HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*set Date */
    sDate.WeekDay   = RTC_WEEKDAY_TUESDAY;
    sDate.Date      = 0x16;
    sDate.Month     = RTC_MONTH_JANUARY;
    sDate.Year      = 0x23;
    
    Status = HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    /*configure Alarm*/
    sAlarm.AlarmMask            = RTC_ALARMMASK_HOURS | RTC_ALARMMASK_MINUTES;
    sAlarm.AlarmSubSecondMask   = RTC_ALARMSUBSECONDMASK_NONE;
    sAlarm.AlarmDateWeekDaySel  = RTC_ALARMDATEWEEKDAYSEL_WEEKDAY;
    sAlarm.AlarmDateWeekDay     = RTC_WEEKDAY_SATURDAY;
    sAlarm.Alarm                = RTC_ALARM_A;
}

/**
 * @brief   Callback function to be run every second to write an update msg in ClkQueue.
 * 
 * This function write a SERIAL_MSG_DISPLAY in the ClockQueue, this is to indicate that it's
 * time to update the display, and also the timer is started again to continue using it.
*/
void ClockUpdate_Callback( void )
{
    APP_MsgTypeDef msgCallback = {0};

    msgCallback.msg = CLOCK_MSG_DISPLAY;

    (void) HIL_QUEUE_writeDataISR( &ClockQueue, &msgCallback );
    (void) AppSched_startTimer( &Scheduler, UpdateTimerID );    /*Restart the timer */
}

/**
 * @brief   Function where the event machine is implemented.
 * 
 * The state machine implementation is made througha a switch sentence where is evaluated
 * a ClkState variable that is in charge to save the next state to run.
*/
void Clock_PeriodicTask( void )
{
    APP_MsgTypeDef MsgClkRead = {0};

    void (*ClockStateMachine[ N_CLK_STATES ]) (APP_MsgTypeDef *PtrMsgClk) = 
    {
        Update_Time,
        Update_Date,
        Set_Alarm,
        Send_Display_Msg
    };

    while( ( HIL_QUEUE_isQueueEmptyISR( &ClockQueue ) == FALSE ) )
    {
        (void) HIL_QUEUE_readDataISR( &ClockQueue, &MsgClkRead );

        if ( MsgClkRead.msg < (uint8_t) N_CLK_STATES )
        {
            ClockStateMachine[ MsgClkRead.msg ]( &MsgClkRead );
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
*/
STATIC void Update_Time( APP_MsgTypeDef *PtrMsgClk )
{
    HAL_StatusTypeDef Status = HAL_ERROR; 

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;

    sTime.Hours     = PtrMsgClk->tm.tm_hour;
    sTime.Minutes   = PtrMsgClk->tm.tm_min;
    sTime.Seconds   = PtrMsgClk->tm.tm_sec;

    Status = HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    (void) HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );
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
 * @note    Only the last two digits of the year are used because that's how the RTC works.
*/
STATIC void Update_Date( APP_MsgTypeDef *PtrMsgClk )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;

    sDate.WeekDay   = PtrMsgClk->tm.tm_wday;
    sDate.Date      = PtrMsgClk->tm.tm_mday;
    sDate.Month     = PtrMsgClk->tm.tm_mon;
    sDate.Year      = (uint8_t) ( PtrMsgClk->tm.tm_year % CENTENARY );  /*Get last two digits of the year*/

    Status = HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    (void) HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );
}

/**
 * @brief   Function to set the RTC Alarm.
 * 
 * This function is called when a alarm msg arrive from serial task, using the pointer to read
 * message the values are storage in the respective elements of sAlarm, then this strucutre is
 * used to set the alarm in the RTC.
 * 
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
*/
STATIC void Set_Alarm( APP_MsgTypeDef *PtrMsgClk )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;

    sAlarm.AlarmTime.Hours      = PtrMsgClk->tm.tm_hour;
    sAlarm.AlarmTime.Minutes    = PtrMsgClk->tm.tm_min;

    Status = HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );

    (void) HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );
}

/**
 * @brief   Function to write an updated message in DisplayQueue.
 * 
 * This funtion get the date, time and alarm values using the structures sTime, sDate and sAlarm,
 * and the respective functions from HAL library.
 * And then that information is writed in the DisplayQueue.
 * 
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
*/
STATIC void Send_Display_Msg( APP_MsgTypeDef *PtrMsgClk )
{
    (void) PtrMsgClk;

    HAL_StatusTypeDef Status = HAL_ERROR;

    APP_MsgTypeDef updateMsg;

    updateMsg.msg = DISPLAY_MSG_UPDATE;

    Status = HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
    
    Status = HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
    
    Status = HAL_RTC_GetAlarm( &hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );
    assert_error( Status == HAL_OK, RTC_RET_ERROR );
    
    updateMsg.tm.tm_hour    = sTime.Hours;
    updateMsg.tm.tm_min     = sTime.Minutes;
    updateMsg.tm.tm_sec     = sTime.Seconds;

    updateMsg.tm.tm_mday    = sDate.Date;
    updateMsg.tm.tm_mon     = sDate.Month;
    updateMsg.tm.tm_year    = sDate.Year;
    updateMsg.tm.tm_wday    = sDate.WeekDay;

    /*Write to the display queue*/
    (void) HIL_QUEUE_writeDataISR( &DisplayQueue, &updateMsg );
}