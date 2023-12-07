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

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is defined in HAL library */
/*structure fort CAN initialization*/
FDCAN_HandleTypeDef CANHandler;

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is defined in HAL library */
/*CAN header structure*/
STATIC FDCAN_TxHeaderTypeDef CANTxHeader;

static APP_MsgTypeDef tm_msg;

/* cppcheck-suppress misra-c2012-8.4 ; its extern linkage is defined in queue.h */
STATIC AppQue_Queue queue;

static APP_CanTypeDef readMsg;

/*Functions prototypes*/
STATIC void Serial_SingleFrameTx( uint8_t *data, uint8_t size );

STATIC uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size);

STATIC unsigned char Validate_LeapYear( unsigned short year );

STATIC unsigned char Validate_Date( unsigned char days, unsigned char month, unsigned short year );

STATIC unsigned char WeekDay( unsigned char days, unsigned char month, unsigned short year );

STATIC unsigned char Validate_Time (unsigned char hour, unsigned char minutes, unsigned char seconds);

STATIC APP_state Evaluate_Msg( AppQue_Queue *hqueue );

STATIC APP_state Evaluate_Time_Parameters( APP_CanTypeDef *msg );

STATIC APP_state Evaluate_Date_Parameters( APP_CanTypeDef *msg );

STATIC APP_state Evaluate_Alarm_Parameters( APP_CanTypeDef *msg );

STATIC void Send_Ok_Message( void );

STATIC void Send_Error_Message( void );

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
 * The state machine implementation is made using a switch statement, in each case a function
 * is called depending the state to do the message processing.
*/
void Serial_PeriodicTask( void )
{
    static APP_state state = IDLE;

    while ( ( HIL_QUEUE_isQueueEmptyISR( &queue ) == FALSE ) || ( state != IDLE ) )
    {
        switch ( state )
        {
            case IDLE:
     
                state = MESSAGE;  

                break;
            
            case MESSAGE:

                state = Evaluate_Msg( &queue );

                break;
            
            case TIME:

                state = Evaluate_Time_Parameters( &readMsg );
                
                break;
            
            case DATE:

                state = Evaluate_Date_Parameters( &readMsg );
                
                break;
            
            case ALARM:

                state = Evaluate_Alarm_Parameters( &readMsg );

                break;

            case OK:
                
                Send_Ok_Message( );
                state = IDLE;                

                break;
            
            case ERROR_:

                Send_Error_Message( );
                state = IDLE;

                break;
            
            default:
                break;

        }
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
        APP_CanTypeDef msg;
        /*structure CAN Rx Header*/
        FDCAN_RxHeaderTypeDef CANRxHeader;

        /*get the msg from fifo0*/
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, msg.bytes );

        /*evaluate if its a valid CAN-TP single frame*/
        if ( Serial_SingleFrameRx( msg.bytes, &msg.lenght ) != FALSE )     
        {
            msg.id = CANRxHeader.Identifier;                   /*get msg ID*/
            (void) HIL_QUEUE_writeDataISR( &queue, &msg );     /*add msg to queue*/
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
 * @brief   Function to check if the year is leap or not.
 * 
 * @param   year[in] year value to be check.
 * 
 * @retval  TRUE if its a leap year and FALSE if not. 
*/
STATIC unsigned char Validate_LeapYear( unsigned short year )
{
    return ( (year % 400u) == 0u ) || ( ( (year % 4u) == 0u ) && ( (year % 100u) != 0u ) );
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
    uint8_t varRet;
    *size = data[0] & LS_NIBBLE_MASK;

    if ( ( (data[0] >> 4u) == 0u ) && (*size > 0u) && (*size < N_BYTES_CAN_MSG) )  /*check if its a valid CAN-TP single frame*/
    {
        varRet = TRUE;                         /*if it is, return the size*/
        for (uint8_t i = 0u; i < *size; i++)     /*and remove the first byte of data*/
        {
            data[ i ] = data[ i + 1u ];
        }
        
    }else{
        varRet = FALSE;                             /*if it isn't just return 0*/
    }
    
    return varRet;
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
STATIC unsigned char WeekDay( unsigned char days, unsigned char month, unsigned short year ){
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
STATIC unsigned char Validate_Time (unsigned char hour, unsigned char minutes, unsigned char seconds)
{
    return ( hour < 24u ) && ( minutes < 60u ) && ( seconds < 60u );
}

/**
 * @brief   Function to evaluate a msg id and return the corresponding state.
 * 
 * This function read a msg from the queue and then evaluate the msg ID to return the next state
 * depending the msg ID read.
 * 
 * @param   hqueue [in] queue where the msg will be read.
 * 
 * @retval  return the next state, it can be TIME, DATE, ALARM and ERROR_
 * 
*/
STATIC APP_state Evaluate_Msg( AppQue_Queue *hqueue )
{
    APP_state stateRet;

    (void) HIL_QUEUE_readDataISR( hqueue, &readMsg );
    
    switch( readMsg.id )
    {
        case ID_TIME_MSG:
            stateRet = TIME; 
            break;

        case ID_DATE_MSG:
            stateRet = DATE;
            break;
        
        case ID_ALARM_MSG:
            stateRet = ALARM;
            break;

        default:
            stateRet = ERROR_;
            break;
    }

    return stateRet;
}

/**
 * @brief   Function to evaluate the time parameters of a message.
 * 
 * This function receive the memmory address of a read message and uses the MACRO BCD_TO_BIN
 * to convert the message parameter and then evaluate if are valid, in true case save the time
 * in the tm_msg struct and return OK state, in false case just return ERRROR_ state.  
 * 
 * @param   msg [in] is the message with time parameters.
 * 
 * @retval  return the next state, it can be OK and ERROR_
 * 
*/
STATIC APP_state Evaluate_Time_Parameters( APP_CanTypeDef *msg )
{
    APP_state stateRet = ERROR_;
    
    uint8_t hour    = BCD_TO_BIN( msg->bytes[ PARAMETER_1 ] );     /*time parameter 1*/
    uint8_t minutes = BCD_TO_BIN( msg->bytes[ PARAMETER_2 ] );     /*time parameter 2*/
    uint8_t seconds = BCD_TO_BIN( msg->bytes[ PARAMETER_3 ] );     /*time parameter 3*/

    if ( Validate_Time( hour, minutes, seconds ) == TRUE )
    {
        stateRet          = OK;
        tm_msg.msg        = SERIAL_MSG_TIME;
        tm_msg.tm.tm_hour = hour;
        tm_msg.tm.tm_min  = minutes;
        tm_msg.tm.tm_sec  = seconds;
    }

    return stateRet;
}

/**
 * @brief   Function to evaluate the date parameters of a message.
 * 
 * This function receive the memmory address of a read message and uses the MACRO BCD_TO_BIN
 * to convert the message parameter and then evaluate if are valid, in true case save the time
 * in the tm_msg struct and return OK state, in false case just return ERRROR_ state.  
 * 
 * @param   msg [in] is the message with date parameters.
 * 
 * @retval  return the next state, it can be OK and ERROR_
 * 
*/
STATIC APP_state Evaluate_Date_Parameters( APP_CanTypeDef *msg )
{
    APP_state stateRet = ERROR_;

    uint8_t day   = BCD_TO_BIN( msg->bytes[ PARAMETER_1 ] );           /*date parameter 1*/
    uint8_t month = BCD_TO_BIN( msg->bytes[ PARAMETER_2 ] );           /*date parameter 2*/
    uint16_t year = BCD_TO_BIN( msg->bytes[ PARAMETER_3 ] ) * 100u;     /*param 3 * 100 to get two most significant figures of the year */
    year += BCD_TO_BIN( msg->bytes[ PARAMETER_4 ] );                   /*add param 4 */
    
    if( Validate_Date( day, month, year ) == TRUE )
    {
        stateRet = OK;
        tm_msg.msg        = SERIAL_MSG_DATE;
        tm_msg.tm.tm_mday = day;
        tm_msg.tm.tm_mon  = month;
        tm_msg.tm.tm_year = year;
        tm_msg.tm.tm_wday = WeekDay( day, month, year );
    }

    return stateRet;
}

/**
 * @brief   Function to evaluate the alarm parameters of a message.
 * 
 * This function receive the memmory address of a read message and uses the MACRO BCD_TO_BIN
 * to convert the message parameters and then evaluate if are valid, in true case save the time
 * in the tm_msg struct and return OK state, in false case just return ERRROR_ state.  
 * 
 * @param   msg [in] is the message with date parameters.
 * 
 * @retval  return the next state, it can be OK and ERROR_
 * 
*/
STATIC APP_state Evaluate_Alarm_Parameters( APP_CanTypeDef *msg )
{
    APP_state stateRet = ERROR_;

    uint8_t hour    = BCD_TO_BIN( msg->bytes[ PARAMETER_1 ] );     /*Alarm parameter 1*/
    uint8_t minutes = BCD_TO_BIN( msg->bytes[ PARAMETER_2 ] );     /*Alarm parameter 2*/

    if ( Validate_Time( hour, minutes, VALID_SECONDS_PARAM ) == TRUE )
    {
        stateRet = OK;
        tm_msg.msg = SERIAL_MSG_ALARM;
    }

    return stateRet;
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
STATIC void Send_Ok_Message( void )
{
    uint8_t data[ N_BYTES_CAN_MSG ] = {0};
    data[ PARAMETER_1 ] = OK_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}

/**
 * @brief   Function to send an "ERROR" message.
 * 
 * This function define an array and append to it in the parameter 1 the ERROR message that is a value
 * of 0xAA and uses the Serial_SingleFrameTx to pack it in the CAN-TP format, this msg is sent with
 * the RESPONSE_ID (0x122).  
 * 
 * 
*/
STATIC void Send_Error_Message( void )
{ 
    uint8_t data[ N_BYTES_CAN_MSG ] = {0};
    data[ PARAMETER_1 ] = ERROR_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}