#include "serial.h"
#include "bsp.h" 

#define BCD_TO_BIN( x ) ( ( ( x >> 4 ) * 10 ) + ( x & 0x0F ) )

#define FILTERS_N       3u
#define ID_TIME_MSG     0x111u
#define ID_DATE_MSG     0x127u
#define ID_ALARM_MSG    0x101u
#define FILTER_MASK     0x7FFu
#define MESSAGES_N      0x23u   /*35 queue messages*/



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
/*structure to config CAN filters*/
FDCAN_FilterTypeDef CANFilter;

AppQue_Queue queue;
APP_CanTypeDef messages[ MESSAGES_N ];

APP_state state = IDLE;


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

    if ( ( (data[0] & 0xF0) == 0 ) || ( (*size > 0) || (*size < 8) ) )  /*check if its a valid single frame*/
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

static APP_state Evaluate_Msg( void )
{
    APP_state stateRet;
    APP_CanTypeDef msg;

    HIL_QUEUE_readDataISR( &queue, &msg );

    switch( msg.id )
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


void Serial_InitTask( void )
{
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
            
            break;
        
        case DATE:
            
            break;
        
        case ALARM:

            break;

        case OK:

            break;
        
        case ERROR_:

            break;

        default:
            break;
     }

}

void HAL_FDCAN_RxFifo0Callback( FDCAN_HandleTypeDef *hfdcan, uint32_t TxEventFifoITs )
{
    uint8_t data[ 8 ];
    APP_CanTypeDef msg;

    /*get the msg from fifo0*/
    HAL_FDCAN_GetRxMessage( &CANHandler, FDCAN_RX_FIFO0, &CANRxHeader, data );

    memcpy( msg.bytes, data, 8 );
    
    /*evaluate if its a valid CAN-TP single frame*/
    if ( Serial_SingleFrameRx( &msg.bytes, &msg.lenght ) != 0 )     
    {
        msg.id = CANRxHeader.Identifier;            /*get the msg ID*/
        HIL_QUEUE_writeDataISR( &queue, &msg );     /*add the msg to the queue*/
    }
    

}