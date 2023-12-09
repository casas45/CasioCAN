/**
 * @file serial.c
 * @brief Implementation of the message processing.
 * 
 * Here it is implemented the message processing using a state machine, which has 7 states IDLE
 * where wait for a message, MESSAGE where the msg is evaluated, ALARM, DATE and TIME to 
 * evaluate the parameter of respective msg type, finally OK and ERROR states to send a msg.
 * Messagges are sent and received through FDCAN module. 
*/
#include "serial.h"
#include "bsp.h" 

#define BCD_TO_BIN( x ) ( ( ( (x) >> 4u ) * 10u ) + ( (x) & 0x0Fu ) ) /*!< Macro to conver BCD data to an integer */

/*structure fort CAN initialization*/
FDCAN_HandleTypeDef CANHandler;

/*CAN header structure*/
static FDCAN_TxHeaderTypeDef CANTxHeader;

static APP_MsgTypeDef ClkMsg;

static AppQue_Queue queue;


/*Functions prototypes*/
STATIC void Serial_SingleFrameTx( uint8_t *data, uint8_t size );

STATIC uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size);

STATIC uint8_t Validate_LeapYear( uint16_t year );

STATIC uint8_t Validate_Date( uint8_t days, uint8_t month, uint16_t year );

STATIC uint8_t WeekDay( uint8_t days, uint8_t month, uint16_t year );

STATIC uint8_t Validate_Time ( uint8_t hour, uint8_t minutes, uint8_t seconds);

STATIC APP_Messages Evaluate_Time_Parameters( APP_CanTypeDef *SerialMsgPtr );

STATIC APP_Messages Evaluate_Date_Parameters( APP_CanTypeDef *SerialMsgPtr );

STATIC APP_Messages Evaluate_Alarm_Parameters( APP_CanTypeDef *SerialMsgPtr );

STATIC APP_Messages Send_Ok_Message( APP_CanTypeDef *SerialMsgPtr );

STATIC APP_Messages Send_Error_Message( APP_CanTypeDef *SerialMsgPtr );


/**
 * @brief Interface to initialize all required about message processing.
 * 
 * FDCAN module is initialize to work with a baudrate of 100kps to transmit and receive
 * standard messages, and also is configured 2 filters, one of them its a mask type and other
 * is Dual type, with 3 differents ID's.
 * Here is also configured the queue in charge of pass the messages from the CAN interrupt
 * to the state machine.
 * 
*/
void Serial_InitTask( void )
{
    static APP_CanTypeDef messages[ MESSAGES_N ];   /*queue buffer*/
    /*structure to config CAN filters*/
    FDCAN_FilterTypeDef CANFilter;

    /*FDCAN Configuration (100kps)*/
    CANHandler.Instance                     = FDCAN1;
    CANHandler.Init.Mode                    = FDCAN_MODE_NORMAL;
    CANHandler.Init.FrameFormat             = FDCAN_FRAME_CLASSIC;
    CANHandler.Init.ClockDivider            = FDCAN_CLOCK_DIV1;
    CANHandler.Init.TxFifoQueueMode         = FDCAN_TX_FIFO_OPERATION;
    CANHandler.Init.NominalPrescaler        = 10;
    CANHandler.Init.NominalSyncJumpWidth    = 1;
    CANHandler.Init.NominalTimeSeg1         = 11;
    CANHandler.Init.NominalTimeSeg2         = 4;
    CANHandler.Init.StdFiltersNbr           = FILTERS_N;
    HAL_FDCAN_Init( &CANHandler );

    HAL_FDCAN_ConfigGlobalFilter( &CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE );

    /*Config filter to ID TIME*/
    CANFilter.IdType        = FDCAN_STANDARD_ID;
    CANFilter.FilterIndex   = 0;
    CANFilter.FilterType    = FDCAN_FILTER_MASK;
    CANFilter.FilterConfig  = FDCAN_FILTER_TO_RXFIFO0;
    CANFilter.FilterID1     = ID_TIME_MSG;
    CANFilter.FilterID2     = FILTER_MASK;
    HAL_FDCAN_ConfigFilter( &CANHandler, &CANFilter );
    /*Config filter to ID DATE and ID ALARM*/
    CANFilter.FilterIndex   = 1;
    CANFilter.FilterType    = FDCAN_FILTER_DUAL;
    CANFilter.FilterID1     = ID_DATE_MSG;
    CANFilter.FilterID2     = ID_ALARM_MSG;
    HAL_FDCAN_ConfigFilter ( &CANHandler, &CANFilter );

    /*FDCAN to normal mode*/
    HAL_FDCAN_Start( &CANHandler );

    /*CANTxHeader configuration*/
    CANTxHeader.IdType      = FDCAN_STANDARD_ID;
    CANTxHeader.FDFormat    = FDCAN_CLASSIC_CAN;
    CANTxHeader.TxFrameType = FDCAN_DATA_FRAME;
    CANTxHeader.Identifier  = RESPONSE_ID;          
    CANTxHeader.DataLength  = FDCAN_DLC_BYTES_8;

    HAL_FDCAN_ActivateNotification( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0 );

    /*Queue configuration*/
    queue.Buffer    = messages;
    queue.Elements  = MESSAGES_N;
    queue.Size      = sizeof( APP_CanTypeDef );
    AppQueue_initQueue( &queue );
}

/**
 * @brief Interface to implement state machine.
 * 
 * The event machine implementation is made using a pointer to functions, in each case a function
 * is called depending on the type of msg read from the queue.
*/
void Serial_PeriodicTask( void )
{
    APP_Messages (*SerialEventMachine[ 5 ]) ( APP_CanTypeDef *SerialMsgPtr ) = 
    {
        Evaluate_Time_Parameters,
        Evaluate_Date_Parameters,
        Evaluate_Alarm_Parameters,
        Send_Ok_Message,
        Send_Error_Message
    };

    APP_CanTypeDef SerialMsg;

    while( HIL_QUEUE_isQueueEmptyISR( &queue ) == FALSE )
    {
        (void) HIL_QUEUE_readDataISR( &queue, &SerialMsg );
        (void) SerialEventMachine[ SerialMsg.bytes[ MSG ] ]( &SerialMsg );
    }
    
}

/**
 * @brief Callback function called by FDCAN interrupt.
 * 
 * In this function is the code to read the message from the FIFO0 and check if its a valid CAN_TP
 * single frame to copy the bytes of the message and the identifier in the queue buffer
 * 
 * @param   hfdcan [in] is the FDCAN init structure.
 * @param   TxEventFifoITs [in] is the interrupt by which the function is called.
 * 
 * @retval None
*/

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is in HAL library*/
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs )
{
    if ( TxEventFifoITs == FDCAN_IT_RX_FIFO0_NEW_MESSAGE )
    {
        APP_CanTypeDef MsgCAN;
        /*structure CAN Rx Header*/
        FDCAN_RxHeaderTypeDef CANRxHeader;

        /*get the msg from fifo0*/
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, MsgCAN.bytes );

        /*evaluate if its a valid CAN-TP single frame*/
        if ( Serial_SingleFrameRx( MsgCAN.bytes, &MsgCAN.lenght ) == TRUE )     
        {
            MsgCAN.id = CANRxHeader.Identifier;                   /*get msg ID*/

            switch( MsgCAN.id )
            {
                case ID_TIME_MSG:
                    MsgCAN.bytes[MSG] = SERIAL_MSG_TIME;
                    break;
                
                case ID_DATE_MSG:
                    MsgCAN.bytes[MSG] = SERIAL_MSG_DATE;
                    break;
                
                case ID_ALARM_MSG:
                    MsgCAN.bytes[MSG] = SERIAL_MSG_ALARM;
                    break;
                
                default:
                    break;
            }

            (void) HIL_QUEUE_writeDataISR( &queue, &MsgCAN );     /*add msg to queue*/
        }
    }
}

/**
 * @brief Function to pack a msg in the CAN-TP single frame format.
 * 
 * The function  will receive an array of data size specified by size, larger than 0 bytes
 * but no more than 7, the function shall append a first byte where the most significant 
 * nibble will be always zero and the less significant nibble will indicate the number of 
 * bytes after that it will return through a pointer 8 bytes with the message packet in CAN-TP
 * format.
 * 
 * @param   data [out] the result of append the firs byte of CAN-TP format
 * @param   size [in] size that will be append in less significant nibble of the first byte.
*/
STATIC void Serial_SingleFrameTx( uint8_t *data, uint8_t size )
{
    uint8_t size_aux = size;

    if(size > 7u)
    {
        size_aux = 7u;
    }

    for ( uint8_t i = size_aux; i > 0u; i--)
    {
        data[ i ] = data[ i - 1u ];
    }

    data[ 0 ] = size_aux;
}

/**
 * @brief Function to unpack a msg in the CAN-TP single frame format.
 * 
 * The function will receive an array of 8 bytes of data with CAN-TP format where the first
 * byte indicate if it was a single frame and the number of valid bytes received. Function
 * must validate if it was a valid single frame and after that remove the first byte and
 * return TRUE.
 * 
 * @param   data [out] the bytes that contain the msg.
 * @param   size [out] the number of payload bytes.
 * 
 * @retval return TRUE if the msg has the CAN-TP single frame format.
 * 
*/
STATIC uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size)
{
    uint8_t varRet = FALSE;
    *size = data[0] & LS_NIBBLE_MASK;

    if ( ( (data[0] & MS_NIBBLE_MASK) == 0u ) && (*size > 0u) && (*size < N_BYTES_CAN_MSG) )  /*check if its a valid CAN-TP single frame*/
    {
        varRet = TRUE;                           /*if it is, return TRUE*/
        for (uint8_t i = 0u; i < *size; i++)     /*and remove the first byte of data*/
        {
            data[ i ] = data[ i + 1u ];
        }
        
    }
    
    return varRet;
}

/**
 * @brief   Function to evaluate the time parameters of a message.
 * 
 * This function receive the memmory address of a read message and uses the MACRO BCD_TO_BIN
 * to convert the message parameter and then evaluate if are valid, in true case save the time
 * in the tm_msg struct and change the type of msg to SERIAL_MSG_OK. 
 * 
 * @param   SerialMsgPtr [in] is the message with time parameters.
 * 
 * 
*/
STATIC APP_Messages Evaluate_Time_Parameters( APP_CanTypeDef *SerialMsgPtr )
{   
    APP_Messages eventRet = SERIAL_MSG_ERROR;

    SerialMsgPtr->bytes[MSG] = SERIAL_MSG_ERROR;

    uint8_t hour    = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_1 ] );     /*time parameter 1*/
    uint8_t minutes = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_2 ] );     /*time parameter 2*/
    uint8_t seconds = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_3 ] );     /*time parameter 3*/
    
    if ( Validate_Time( hour, minutes, seconds ) == TRUE )
    {
        eventRet = SERIAL_MSG_OK;
        SerialMsgPtr->bytes[MSG] = SERIAL_MSG_OK;
        ClkMsg.msg        = SERIAL_MSG_TIME;
        ClkMsg.tm.tm_hour = hour;
        ClkMsg.tm.tm_min  = minutes;
        ClkMsg.tm.tm_sec  = seconds;
    }

    (void) HIL_QUEUE_writeDataISR( &queue, SerialMsgPtr );

    return eventRet;
}

/**
 * @brief   Function to evaluate the date parameters of a message.
 * 
 * This function receive the memmory address of a read message and uses the MACRO BCD_TO_BIN
 * to convert the message parameter and then evaluate if are valid, in true case save the time
 * in the ClkMsg struct and change the type of msg to SERIAL_MSG_OK.  
 * 
 * @param   SerialMsgPtr [in] is the message with date parameters.
 * 
 * 
*/
STATIC APP_Messages Evaluate_Date_Parameters( APP_CanTypeDef *SerialMsgPtr )
{
    APP_Messages eventRet = SERIAL_MSG_ERROR;

    SerialMsgPtr->bytes[MSG] = SERIAL_MSG_ERROR;

    uint8_t day   = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_1 ] );           /*date parameter 1*/
    uint8_t month = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_2 ] );           /*date parameter 2*/
    uint16_t year = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_3 ] ) * 100u;     /*param 3 * 100 to get two most significant figures of the year */
    year += BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_4 ] );                   /*add param 4 */
    
    if( Validate_Date( day, month, year ) == TRUE )
    {
        eventRet = SERIAL_MSG_OK;
        SerialMsgPtr->bytes[MSG] = SERIAL_MSG_OK;
        ClkMsg.msg        = SERIAL_MSG_DATE;
        ClkMsg.tm.tm_mday = day;
        ClkMsg.tm.tm_mon  = month;
        ClkMsg.tm.tm_year = year;
        ClkMsg.tm.tm_wday = WeekDay( day, month, year );
    }

    (void) HIL_QUEUE_writeDataISR( &queue, SerialMsgPtr );

    return eventRet;
}

/**
 * @brief   Function to evaluate the alarm parameters of a message.
 * 
 * This function receive the memmory address of a read message and uses the MACRO BCD_TO_BIN
 * to convert the message parameters and then evaluate if are valid, in true case save the time
 * in the ClkMsg struct and change the type of msg to SERIAL_MSG_OK.  
 * 
 * @param   SerialMsgPtr [in] is the message with alarm parameters.
*/
STATIC APP_Messages Evaluate_Alarm_Parameters( APP_CanTypeDef *SerialMsgPtr )
{
    APP_Messages eventRet = SERIAL_MSG_ERROR;

    SerialMsgPtr->bytes[MSG] = SERIAL_MSG_ERROR;

    uint8_t hour    = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_1 ] );     /*Alarm parameter 1*/
    uint8_t minutes = BCD_TO_BIN( SerialMsgPtr->bytes[ PARAMETER_2 ] );     /*Alarm parameter 2*/

    if ( Validate_Time( hour, minutes, VALID_SECONDS_PARAM ) == TRUE )
    {
        eventRet = SERIAL_MSG_OK;
        SerialMsgPtr->bytes[MSG] = SERIAL_MSG_OK;
        ClkMsg.msg = SERIAL_MSG_ALARM;
    }

    (void) HIL_QUEUE_writeDataISR( &queue, SerialMsgPtr );

    return eventRet;
}

/**
 * @brief   Function to send a "OK" message.
 * 
 * This function define an array and append to it in the parameter 1 the OK message that is a value
 * of 0x55 and uses the Serial_SingleFrameTx to pack it in the CAN-TP format, this msg is sent with
 * the RESPONSE_ID (0x122).  
 * 
 * 
*/
STATIC APP_Messages Send_Ok_Message( APP_CanTypeDef *SerialMsgPtr )
{
    APP_Messages eventRet = SERIAL_MSG_NONE;
    
    (void) SerialMsgPtr;
    uint8_t data[ N_BYTES_CAN_MSG ] = {0};
    data[ PARAMETER_1 ] = OK_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );

    return eventRet;
}

/**
 * @brief   Function to send an "ERROR" message.
 * 
 * This function define an array and append to it in the parameter 1 the ERROR message that is a value
 * of 0xAA and uses the Serial_SingleFrameTx to pack it in the CAN-TP format, this msg is sent with
 * the RESPONSE_ID (0x122).  
*/
STATIC APP_Messages Send_Error_Message( APP_CanTypeDef *SerialMsgPtr )
{ 
    APP_Messages eventRet = SERIAL_MSG_NONE;

    (void) SerialMsgPtr;
    uint8_t data[ N_BYTES_CAN_MSG ] = {0};
    data[ PARAMETER_1 ] = ERROR_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );

    return eventRet;
}


/**
 * @brief   Function to check if the year is leap or not.
 * 
 * @param   year[in] year value to be check.
 * 
 * @retval  TRUE if its a leap year and FALSE if not. 
*/
STATIC uint8_t Validate_LeapYear( uint16_t year )
{
    return ( (year % 400u) == 0u ) || ( ( (year % 4u) == 0u ) && ( (year % 100u) != 0u ) );
}

/**
 * @brief   Function to validate a date, considering the leap years.
 * 
 * This function define an array with the maximum days on each month, check if the argument year it's 
 * a leap year and then set February mdays to 29, then evalute using AND operators to check if the 
 * days, month and year are in the correct intervalue.
 * 
 * @param   days [in] day value to be check.
 * @param   month [in] month value to be check.
 * @param   year [in] year value to be check.
 * 
 * 
 * @retval  Returns TRUE when its a valid date and FALSE when it is not;
*/
STATIC uint8_t Validate_Date( uint8_t days, uint8_t month, uint16_t year )
{
    uint8_t m_days [ MONTHS ] = {MONTH_31_D, FEB_28, MONTH_31_D, MONTH_30_D, MONTH_31_D, MONTH_30_D, MONTH_31_D, MONTH_31_D, MONTH_30_D, MONTH_31_D, MONTH_30_D, MONTH_31_D};
    uint8_t varRet = FALSE;

    if ( Validate_LeapYear( year ) == TRUE )  //if its a leap year February has 29 days
    {
        m_days[ FEB ] = FEB_29;
    }

    if ( (month > 0u) && ( month <= MONTHS ) )   /*check first if month is correct to guarantee a correct m_days index*/
    {
        if ( ( days > 0u ) && ( days <= m_days[ month - 1u ] ) && ( year >= YEAR_MIN ) && ( year <= YEAR_MAX ) )
        {
            varRet = TRUE;
        }     
    }

    return varRet;
}

/**
 * @brief   Function to know the week day with a given date.
 * 
 * Using an algorithm to calcute the day of the week with a date.
 * 
 * @param   days[in] day value.
 * @param   month[in] month.
 * @param   year[in] year value.
 * 
 * @retval  Return the week day (0 to 6 -> Sunday to Monday).
*/
STATIC uint8_t WeekDay( uint8_t days, uint8_t month, uint16_t year ){
    unsigned long part1;
    unsigned long part2;
    unsigned long part3;
    unsigned long part4;
    unsigned long part5;

    unsigned long day = (unsigned short) days;
    unsigned long months = (unsigned short) month;
    unsigned long years = (unsigned short) year;

    unsigned char result;

    part1 = day + ( ( 153u * ( months + ( 12u * ( ( 14u - months ) / 12u ) ) - 3u ) + 2u ) / 5u );

    part2 = 365u * ( years + 4800u - ( ( 14u - months ) / 12u ) );

    part3 = ( years + 4800u - ( ( 14u - months ) / 12u ) ) / 4u;

    part4 = ( ( years + 4800u - ( ( 14u - months ) / 12u ) ) / 100u );

    part5 = ( ( years + 4800u - ( ( 14u - months) / 12u ) ) / 400u ) - 32044u;

    result = (unsigned char) ( ( part1 + part2 + part3 - part4 + part5 ) % 7u );

    return result;
}

/**
 * @brief   Function to validate a time.
 * 
 * This function returns the result of verifying that the hour is less than 24, the minutes less than 60
 * and the seconds less than 60.
 * 
 * @param   hour[in] hour value to be check.
 * @param   minutes[in] minutes value to be check.
 * @param   seconds[in] seconds value to be check.
 * 
 * 
 * @retval  Returns TRUE when its a valid time and FALSE if it is not;
*/
STATIC uint8_t Validate_Time (uint8_t hour, uint8_t minutes, uint8_t seconds)
{
    return ( hour < 24u ) && ( minutes < 60u ) && ( seconds < 60u );
}