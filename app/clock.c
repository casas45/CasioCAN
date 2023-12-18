/**
 * @file    clock.c
 * 
 * @brief   In this file the clock state machine is implemented.
*/
#include "clock.h"
#include "bsp.h"

#define N_MESSAGES_CLKQUEUE   0x0Au      /*!< Number of messages in ClkQueue */

/**
 * @brief Queue to communicate serial and clock tasks.
*/
AppQue_Queue ClockQueue;

/**
 * @brief   RTC structure
*/
static RTC_HandleTypeDef hrtc;

/** @brief Struct used to set and get time in RTC */
static RTC_TimeTypeDef sTime = {0};
/** @brief Struct used to set and get date in RTC */
static RTC_DateTypeDef sDate = {0};
/** @brief Struct used to set and get alarm in RTC */
static RTC_AlarmTypeDef sAlarm = {0};


STATIC ClkState Evaluate_Msg( const APP_MsgTypeDef *PtrMsgClk );

STATIC ClkState Update_Time( APP_MsgTypeDef *PtrMsgClk );

STATIC ClkState Update_Date( APP_MsgTypeDef *PtrMsgClk );

STATIC ClkState Set_Alarm( APP_MsgTypeDef *PtrMsgClk );

STATIC ClkState Update_Display( APP_MsgTypeDef *PtrMsgClk );

/**
 * @brief   Function to initialize RTC module and ClkQueue.
 * 
 * In this function is configured the ClkQueue that is in charge of communicate serial and clock
 * tasks, and also is initialized the RTC module  with 24hour format, and the time and date is set
 * in BCD format.
*/
void Clock_InitTask( void )
{
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
    HAL_RTC_Init( &hrtc );

    /*set Time */
    sTime.Hours     = 0x12;
    sTime.Minutes   = 0x10;
    sTime.Seconds   = 0x45;
    HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BCD );

    /*set Date */
    sDate.WeekDay   = RTC_WEEKDAY_FRIDAY;
    sDate.Date      = 0x14;
    sDate.Month     = RTC_MONTH_DECEMBER;
    sDate.Year      = 0x23;
    HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BCD );

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

    msgCallback.msg = SERIAL_MSG_DISPLAY;

    (void) HIL_QUEUE_writeDataISR( &ClockQueue, &msgCallback );
    (void) AppSched_startTimer( &Scheduler, UpdateTimerID );    /*Restart the timer */
}

/**
 * @brief   Function where the state machine is implemented.
 * 
 * The state machine implementation is made througha a switch sentence where is evaluated
 * a ClkState variable that is in charge to save the next state to run.
*/
void Clock_PeriodicTask( void )
{
    static ClkState state = IDLE;
    static APP_MsgTypeDef MsgClkRead = {0};

    ClkState (*ClockStateMachine[ N_CLK_STATES ]) ( APP_MsgTypeDef *PtrMsgClk ) =
    {
        Evaluate_Msg,
        Update_Time,
        Update_Date,
        Set_Alarm,
        Update_Display
    };
    
    while( ( HIL_QUEUE_isQueueEmptyISR( &ClockQueue ) == FALSE ) || ( state != IDLE ) )
    {
        switch ( state )
        {
            case IDLE:
                    state = EVALUATE_MSG;
                break;

            case EVALUATE_MSG:
                    (void) HIL_QUEUE_readDataISR( &ClockQueue, &MsgClkRead );
                    state = Evaluate_Msg( &MsgClkRead );
                break;

            case TIME:
                    state = Update_Time( &MsgClkRead );
                break;

            case DATE:
                    state = Update_Date( &MsgClkRead );
                break;
            
            case ALARM:
                    state = Set_Alarm( &MsgClkRead );
                break;

            case DISPLAY:
                    state = Update_Display( &MsgClkRead );
                break;

            default:
                break;
        }
    }
        
}

/**
 * @brief   Function to evaluate the msg from ClkQueue.
 * 
 * The msg element of the PtrMsgClk is evaluated to know wich state is next to run, this is made
 * using a switch sentence.
 * 
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
 * 
 * @retval  The next state determined based on the message type. Possible values are: IDLE, TIME, DATE, ALARM, DISPLAY
*/
STATIC ClkState Evaluate_Msg( const APP_MsgTypeDef *PtrMsgClk )
{
    ClkState retState = IDLE;
    
    switch ( PtrMsgClk->msg )
    {
        case SERIAL_MSG_TIME:
            retState = TIME;
            break;

        case SERIAL_MSG_DATE:
            retState = DATE;
            break;

        case SERIAL_MSG_ALARM:
            retState = ALARM;
            break;
        
        case SERIAL_MSG_DISPLAY:
            retState = DISPLAY;
            break;

        default:
            break;
    }

    return retState;
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
 * @retval  Next state, always is IDLE.
*/
STATIC ClkState Update_Time( APP_MsgTypeDef *PtrMsgClk )
{
    sTime.Hours     = PtrMsgClk->tm.tm_hour;
    sTime.Minutes   = PtrMsgClk->tm.tm_min;
    sTime.Seconds   = PtrMsgClk->tm.tm_sec;
    (void) HAL_RTC_SetTime( &hrtc, &sTime, RTC_FORMAT_BIN );

    return IDLE;
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
 * @retval  Next state, always is IDLE.
 * 
 * @note    Only the last two digits of the year are used because that's how the RTC works.
*/
STATIC ClkState Update_Date( APP_MsgTypeDef *PtrMsgClk )
{
    sDate.WeekDay   = RTC_WEEKDAY_FRIDAY;
    sDate.Date      = PtrMsgClk->tm.tm_mday;
    sDate.Month     = PtrMsgClk->tm.tm_mon;
    sDate.Year      = (uint8_t) ( PtrMsgClk->tm.tm_year % 100u );

    (void) HAL_RTC_SetDate( &hrtc, &sDate, RTC_FORMAT_BIN );

    return IDLE;
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
 * @retval  Next state, always is IDLE.
*/
STATIC ClkState Set_Alarm( APP_MsgTypeDef *PtrMsgClk )
{
    
    sAlarm.AlarmTime.Hours      = PtrMsgClk->tm.tm_hour;
    sAlarm.AlarmTime.Minutes    = PtrMsgClk->tm.tm_min;

    (void) HAL_RTC_SetAlarm( &hrtc, &sAlarm, RTC_FORMAT_BIN );

    return IDLE;
}

/**
 * @brief   Function to update the time and date in the display.
 * 
 * This funtion get the date, time and alarm values using the structures sTime, sDate and sAlarm,
 * and the respective functions from HAL library.
 * This information is showed through the printf function using semihosting.
 * 
 * @param   PtrMsgClk [in] Pointer to the clock message read from ClkQueue.
 * 
 * @retval  Next state, always is IDLE.
*/
STATIC ClkState Update_Display( APP_MsgTypeDef *PtrMsgClk )
{
    (void) PtrMsgClk;

    HAL_RTC_GetTime( &hrtc, &sTime, RTC_FORMAT_BIN );
    HAL_RTC_GetDate( &hrtc, &sDate, RTC_FORMAT_BIN );
    HAL_RTC_GetAlarm( &hrtc, &sAlarm, RTC_ALARM_A, RTC_FORMAT_BIN );

    (void) printf( "Time: %d:%d:%d\n\r", sTime.Hours, sTime.Minutes, sTime.Seconds );
    (void) printf( "Date: %d/%d/%d\n\r", sDate.Date, sDate.Month, sDate.Year );
    (void) printf( "Alarm: %d:%d\n\r", sAlarm.AlarmTime.Hours, sAlarm.AlarmTime.Minutes );

    return IDLE;   
}