#include "serial.h"
#include "bsp.h" 

#define BCD_TO_BIN( x ) ( ( ( x >> 4 ) * 10 ) + ( x & 0x0F ) )

#define FILTERS_N           3u
#define ID_TIME_MSG         0x111u
#define ID_DATE_MSG         0x127u
#define ID_ALARM_MSG        0x101u
#define FILTER_MASK         0x7FFu
#define MESSAGES_N          0x23u   /*35 queue messages*/
#define VALID_SECONDS_PARAM 0x00
#define RESPONSE_ID         0x122u
#define OK_RESPONSE         0x55u
#define ERROR_RESPONSE      0xAAu
#define N_BYTES_RESPONSE    0x01u

typedef struct _App_CanTypeDef
{
    uint16_t id;        /*CAN message ID*/
    uint8_t bytes[8];   /*CAN message*/
    uint8_t lenght;     /*CAN messsge lenght*/
} APP_CanTypeDef;

/*structure fort CAN initialization*/
FDCAN_HandleTypeDef CANHandler;
/*structure CAN Rx Header*/
FDCAN_RxHeaderTypeDef CANRxHeader;
/*CAN header structure*/
FDCAN_TxHeaderTypeDef CANTxHeader;
/*structure to config CAN filters*/
FDCAN_FilterTypeDef CANFilter;

static AppQue_Queue queue;
static APP_CanTypeDef messages[ MESSAGES_N ];

static APP_state state = IDLE;
static APP_CanTypeDef readMsg;

static void Serial_SingleFrameTx( uint8_t *data, uint8_t size )
{
    if(size > 7)
    {
        size = 7;
    }

    for (uint8_t i = size; i > 0; i--)
    {
        *(data + i) = *(data + i -1);
    }

    *(data) = size;
}

static uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size)
{
    uint8_t varRet;
    *size = data[0] & 0x0F;

    if ( ( (data[0] >> 4) == 0 ) && ( (*size > 0) || (*size < 8) ) )  /*check if its a valid single frame*/
    {
        varRet = *size;                         /*if it is, return the size*/
        for (uint8_t i = 0; i < *size; i++)     /*and remove the first byte of data*/
        {
            data[i] = data[i + 1];
        }
        
    }else{
        varRet = 0;                             /*if it isn't just return 0*/
    }
    
    return varRet;
}

static unsigned char Validate_LeapYear( unsigned short year ){
    return ( (year % 4u) == 0u ) && ( ( (year % 100u) != 0u ) || ( (year % 400u) == 0u ) );
}

static unsigned char Validate_Date( unsigned char days, unsigned char month, unsigned short year )
{
    unsigned char m_days [12u] = {31u, 28u, 31u, 30u, 31u, 30u, 31u, 31u, 30u, 31u, 30u, 31u};

    unsigned char leapYear = Validate_LeapYear( year );

    if ( leapYear == TRUE )  //if its a leap year February has 29 days
    {
        m_days[ 1u ] = 29u;
    }

    return ( days > 0u ) && ( days <= m_days[ month - 1u ] ) && ( month > 0u ) && ( month <= 12u ) && ( year >= 1900u ) && ( year <= 2100u );
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

    HIL_QUEUE_readDataISR( &queue, &readMsg);
    
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

static uint8_t Evaluate_Time_Parameters( void )
{
    uint8_t hour = BCD_TO_BIN( readMsg.bytes[ 0 ] );        /*time parameter 1*/
    uint8_t minutes = BCD_TO_BIN( readMsg.bytes[ 1 ] );     /*time parameter 2*/
    uint8_t seconds = BCD_TO_BIN( readMsg.bytes[ 2 ] );     /*time parameter 3*/

    return Validate_Time( hour, minutes, seconds );
}

static uint8_t Evaluate_Date_Parameters( void )
{
    uint8_t day = BCD_TO_BIN( readMsg.bytes[ 0 ] );             /*date parameter 1*/
    uint8_t month = BCD_TO_BIN( readMsg.bytes[ 1 ] );           /*date parameter 2*/
    uint16_t year = BCD_TO_BIN( readMsg.bytes[ 2 ] ) * 100;     /*param 3 * 100 to get two most significant figures of the year */
    year += BCD_TO_BIN( readMsg.bytes[ 3 ] );                   /*add param 4 */
    
    return Validate_Date( day, month, year );
}

static uint8_t Evaluate_Alarm_Parameters( void )
{
    uint8_t hour = BCD_TO_BIN( readMsg.bytes[ 0 ] );        /*Alarm parameter 1*/
    uint8_t minutes = BCD_TO_BIN( readMsg.bytes[ 1 ] );     /*Alarm parameter 2*/

    return Validate_Time( hour, minutes, VALID_SECONDS_PARAM );
}

static void Send_Ok_Message( void )
{
    uint8_t data[8] = {0};
    data[ 0 ] = OK_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}

static void Send_Error_Message( void )
{ 
    uint8_t data[8] = {0};
    data[ 0 ] = ERROR_RESPONSE;

    Serial_SingleFrameTx( data, N_BYTES_RESPONSE );

    HAL_FDCAN_AddMessageToTxFifoQ( &CANHandler, &CANTxHeader, data );
}


void Serial_InitTask( void )
{
    HAL_Init( );

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

    CANFilter.FilterIndex   = 1;
    CANFilter.FilterID1     = ID_DATE_MSG;
    HAL_FDCAN_ConfigFilter ( &CANHandler, &CANFilter );

    CANFilter.FilterIndex   = 2;
    CANFilter.FilterID1     = ID_ALARM_MSG;
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
     switch ( state )
     {
        case IDLE:

            if( !HIL_QUEUE_isQueueEmptyISR( &queue ) )
            {
                state = MESSAGE;
            }

            break;
        
        case MESSAGE:

            state = Evaluate_Msg();

            break;
        
        case TIME:

            if ( Evaluate_Time_Parameters() )
            {
                state = OK;
            }else{
                state = ERROR_;
            }
            
            break;
        
        case DATE:

            if( Evaluate_Date_Parameters() )
            {
                state = OK;
            }else{
                state = ERROR_;
            }
            
            break;
        
        case ALARM:

            if( Evaluate_Alarm_Parameters() )
            {
                state = OK;
            }else{
                state = ERROR_;
            }

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

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs )
{
    uint8_t data[ 8 ] = {0};
    APP_CanTypeDef msg;

    /*get the msg from fifo0*/
    HAL_FDCAN_GetRxMessage( hfdcan, FDCAN_RX_FIFO0, &CANRxHeader, data );

    memcpy( msg.bytes, data, 8 );
    
    /*evaluate if its a valid CAN-TP single frame*/
    if ( Serial_SingleFrameRx( msg.bytes, &msg.lenght ) != 0 )     
    {
        msg.id = CANRxHeader.Identifier;            /*get the msg ID*/
        HIL_QUEUE_writeDataISR( &queue, &msg );     /*add the msg to queue*/
    }

}