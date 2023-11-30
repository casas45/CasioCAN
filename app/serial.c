#include "serial.h"
#include "bsp.h" 

#define FILTERS_N       3U
#define ID_TIME_MSG     0x111U
#define ID_DATE_MSG     0x127U
#define ID_ALARM_MSG    0x101U
#define FILTER_MASK     0x7FFU



/*structure fort CAN initialization*/
FDCAN_HandleTypeDef CANHandler;
/*structure CAN Rx Header*/
FDCAN_RxHeaderTypeDef CANRxHeader;
/*structure to config CAN filters*/
FDCAN_FilterTypeDef CANFilter;

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

    
}

void Serial_PeriodicTask( void )
{
    
}