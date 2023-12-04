#include "serial.h"
#include "bsp.h" 

#define BCD_TO_BIN( x ) ( ( ( (x) >> 4 ) * 10 ) + ( (x) & 0x0F ) )

typedef struct _App_CanTypeDef
{
    uint16_t id;        /*CAN message ID*/
    uint8_t bytes[ N_BYTES_CAN_MSG ];   /*CAN message*/
    uint8_t lenght;     /*CAN messsge lenght*/
} APP_CanTypeDef;

/*structure fort CAN initialization*/
static FDCAN_HandleTypeDef CANHandler;
/*CAN header structure*/
static FDCAN_TxHeaderTypeDef CANTxHeader;

static APP_MsgTypeDef tm_msg;

static AppQue_Queue queue;

static APP_CanTypeDef readMsg;

/*Functions prototypes*/
static void Serial_SingleFrameTx( uint8_t *data, uint8_t size );

static uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size);

static unsigned char Validate_LeapYear( unsigned short year );

static unsigned char Validate_Date( unsigned char days, unsigned char month, unsigned short year );

static unsigned char WeekDay( unsigned char days, unsigned char month, unsigned short year );

static unsigned char Validate_Time (unsigned char hour, unsigned char minutes, unsigned char seconds);

static APP_state Evaluate_Msg( void );

static APP_state Evaluate_Time_Parameters( void );

static APP_state Evaluate_Date_Parameters( void );

static APP_state Evaluate_Alarm_Parameters( void );

static void Send_Ok_Message( void );

static void Send_Error_Message( void );


void Serial_InitTask( void )
{
    /*structure to config CAN filters*/
    FDCAN_FilterTypeDef CANFilter;
    
    static APP_CanTypeDef messages[ MESSAGES_N ];   /*queue buffer*/

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

                state = Evaluate_Msg();

                break;
            
            case TIME:

                state = Evaluate_Time_Parameters( );
                
                break;
            
            case DATE:

                state = Evaluate_Date_Parameters( );
                
                break;
            
            case ALARM:

                state = Evaluate_Alarm_Parameters( );

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

/* cppcheck-suppress misra-c2012-8.4 ; its external linkage is in HAL library*/
void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs )
{
    if ( TxEventFifoITs == FDCAN_IT_RX_FIFO0_NEW_MESSAGE )
    {
        uint8_t data[ N_BYTES_CAN_MSG ] = {0};
        APP_CanTypeDef msg;
        /*structure CAN Rx Header*/
        FDCAN_RxHeaderTypeDef CANRxHeader;

        /*get the msg from fifo0*/
        HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, data );

        (void) memcpy( msg.bytes, data, N_BYTES_CAN_MSG );
    
        /*evaluate if its a valid CAN-TP single frame*/
        if ( Serial_SingleFrameRx( msg.bytes, &msg.lenght ) != FALSE )     
        {
            msg.id = CANRxHeader.Identifier;            /*get the msg ID*/
            (void) HIL_QUEUE_writeDataISR( &queue, &msg );     /*add the msg to queue*/
        }
    }
}

static void Serial_SingleFrameTx( uint8_t *data, uint8_t size )
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

    data[ 0 ] = size;
}

static uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size)
{
    uint8_t varRet;
    *size = data[0] & LS_NIBBLE_MASK;

    if ( ( (data[0] >> 4u) == 0u ) && ( (*size > 0u) && (*size < N_BYTES_CAN_MSG) ) )  /*check if its a valid single frame*/
    {
        varRet = *size;                         /*if it is, return the size*/
        for (uint8_t i = 0u; i < *size; i++)     /*and remove the first byte of data*/
        {
            data[ i ] = data[ i + 1u ];
        }
        
    }else{
        varRet = FALSE;                             /*if it isn't just return 0*/
    }
    
    return varRet;
}

static unsigned char Validate_LeapYear( unsigned short year ){
    return ( (year % 400u) == 0u ) || ( ( (year % 4u) == 0u ) && ( (year % 100u) != 0u ) );
}

static unsigned char Validate_Date( unsigned char days, unsigned char month, unsigned short year )
{
    unsigned char m_days [ MONTHS ] = {MONTH_31_D, FEB_28, MONTH_31_D, MONTH_30_D, MONTH_31_D, MONTH_30_D, MONTH_31_D, MONTH_31_D, MONTH_30_D, MONTH_31_D, MONTH_30_D, MONTH_31_D};

    unsigned char leapYear = Validate_LeapYear( year );

    if ( leapYear == TRUE )  //if its a leap year February has 29 days
    {
        m_days[ FEB ] = FEB_29;
    }

    return ( days > 0u ) && ( days <= m_days[ month - 1u ] ) && ( month > 0u ) && ( month <= MONTHS ) && ( year >= YEAR_MIN ) && ( year <= YEAR_MAX );
}

static unsigned char WeekDay( unsigned char days, unsigned char month, unsigned short year ){
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

static unsigned char Validate_Time (unsigned char hour, unsigned char minutes, unsigned char seconds){
    return ( hour < 24u ) && ( minutes < 60u ) && ( seconds < 60u );
}

static APP_state Evaluate_Msg( void )
{
    APP_state stateRet;

    (void) HIL_QUEUE_readDataISR( &queue, &readMsg);
    
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

static APP_state Evaluate_Time_Parameters( void )
{
    APP_state stateRet = ERROR_;
    
    uint8_t hour    = BCD_TO_BIN( readMsg.bytes[ PARAMETER_1 ] );     /*time parameter 1*/
    uint8_t minutes = BCD_TO_BIN( readMsg.bytes[ PARAMETER_2 ] );     /*time parameter 2*/
    uint8_t seconds = BCD_TO_BIN( readMsg.bytes[ PARAMETER_3 ] );     /*time parameter 3*/

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

static APP_state Evaluate_Date_Parameters( void )
{
    APP_state stateRet = ERROR_;

    uint8_t day   = BCD_TO_BIN( readMsg.bytes[ PARAMETER_1 ] );           /*date parameter 1*/
    uint8_t month = BCD_TO_BIN( readMsg.bytes[ PARAMETER_2 ] );           /*date parameter 2*/
    uint16_t year = BCD_TO_BIN( readMsg.bytes[ PARAMETER_3 ] ) * 100;     /*param 3 * 100 to get two most significant figures of the year */
    year += BCD_TO_BIN( readMsg.bytes[ PARAMETER_4 ] );                   /*add param 4 */
    
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

static APP_state Evaluate_Alarm_Parameters( void )
{
    APP_state stateRet = ERROR_;

    uint8_t hour    = BCD_TO_BIN( readMsg.bytes[ PARAMETER_1 ] );     /*Alarm parameter 1*/
    uint8_t minutes = BCD_TO_BIN( readMsg.bytes[ PARAMETER_2 ] );     /*Alarm parameter 2*/

    if ( Validate_Time( hour, minutes, VALID_SECONDS_PARAM ) == TRUE )
    {
        stateRet = OK;
        tm_msg.msg = SERIAL_MSG_ALARM;
    }

    return stateRet;
}

static void Send_Ok_Message( void )
{
    uint8_t data[ N_BYTES_CAN_MSG ] = {0};
    data[ PARAMETER_1 ] = OK_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}

static void Send_Error_Message( void )
{ 
    uint8_t data[ N_BYTES_CAN_MSG ] = {0};
    data[ PARAMETER_1 ] = ERROR_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}