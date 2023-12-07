#include "unity.h"
#include "serial.h"
#include "bsp.h"
#include "queue.h"
#include <stdint.h>

#include "mock_stm32g0xx_hal_fdcan.h"

#define BYTES_CAN_MESSAGE       0x08u
#define SINGLE_FRAME_7_PAYLOAD  0x07u
#define FIRST_FRAME_CAN_TP      0x17u
#define VALID_BCD_HOUR              0x08u
#define VALID_BCD_MIN               0x00u
#define VALID_BCD_SEC               0x00u
#define NO_VALID_BCD_HOUR           0x30u
#define NO_VALID_BCD_MIN            0x60u
#define NO_VALID_BCD_SEC            0x60u
#define UNKNOW_ID               0x0FFu
#define VALID_BCD_DAY           0x30u
#define VALID_BCD_MONTH         0x11u
#define VALID_BCD_YEAR_MS       0x20u   
#define VALID_BCD_YEAR_LS       0x21u
#define NO_VALID_BCD_DAY        0x35u
#define NO_VALID_BCD_MONTH      0x13u
#define NO_VALID_BCD_YEAR_MS    0x50u

#define VALID_BCD_DAY_LEAP           0x29u
#define VALID_BCD_MONTH_LEAP         0x02u
#define VALID_BCD_YEAR_MS_LEAP       0x20u   
#define VALID_BCD_YEAR_LS_LEAP       0x20u

/*structure fort CAN initialization*/
extern FDCAN_HandleTypeDef CANHandler;
/*CAN header structure*/
extern FDCAN_TxHeaderTypeDef CANTxHeader;

extern AppQue_Queue queue;
APP_CanTypeDef messages2[8];

AppQue_Queue queueAux;
APP_CanTypeDef msgWrite;
APP_CanTypeDef messagesW[ 4 ];


void setUp( void )
{
    /*OxFF are don't care values*/
    const uint8_t dataTime[BYTES_CAN_MESSAGE] = {VALID_BCD_HOUR, VALID_BCD_MIN, VALID_BCD_SEC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t dataDate[BYTES_CAN_MESSAGE] = {VALID_BCD_DAY, VALID_BCD_MONTH, VALID_BCD_YEAR_MS, VALID_BCD_YEAR_LS, 0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t dataAlarm[BYTES_CAN_MESSAGE] = {VALID_BCD_HOUR, VALID_BCD_MIN, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    const uint8_t dataTime_noValid[BYTES_CAN_MESSAGE] = {NO_VALID_BCD_HOUR, VALID_BCD_MIN, VALID_BCD_SEC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    /*Modifying serial file queue*/
    queue.Buffer    = messages2;
    queue.Elements  = 8;
    queue.Size      = sizeof( APP_CanTypeDef );
    AppQueue_initQueue( &queue );
    /*Write valid time msg in queue*/
    msgWrite.id = ID_TIME_MSG;
    memcpy( msgWrite.bytes, dataTime , BYTES_CAN_MESSAGE);
    AppQueue_writeData( &queue, &msgWrite );

    /*Write valid date msg in queue*/
    msgWrite.id = ID_DATE_MSG;
    memcpy( msgWrite.bytes, dataDate , BYTES_CAN_MESSAGE);
    AppQueue_writeData( &queue, &msgWrite );

    /*Write valid alarm msg in queue*/
    msgWrite.id = ID_ALARM_MSG;
    memcpy( msgWrite.bytes, dataAlarm , BYTES_CAN_MESSAGE);
    AppQueue_writeData( &queue, &msgWrite );

    /*Write no valid time msg in queue*/
    msgWrite.id = ID_TIME_MSG;
    memcpy( msgWrite.bytes, dataTime_noValid , BYTES_CAN_MESSAGE);
    AppQueue_writeData( &queue, &msgWrite );

    /*Queue aux to Evaluate_Msg function configuration*/
    queueAux.Buffer    = messagesW;
    queueAux.Elements  = 4;
    queueAux.Size      = sizeof( APP_CanTypeDef );
    AppQueue_initQueue( &queueAux );

    msgWrite.id = ID_TIME_MSG;
    AppQueue_writeData( &queueAux, &msgWrite );

    msgWrite.id = ID_DATE_MSG;
    AppQueue_writeData( &queueAux, &msgWrite );

    msgWrite.id = ID_ALARM_MSG;
    AppQueue_writeData( &queueAux, &msgWrite );

    msgWrite.id = UNKNOW_ID;
    AppQueue_writeData( &queueAux, &msgWrite );

}

void tearDown( void )
{

}

void test__Serial_InitTask__init_FDCAN_module( void )
{
    HAL_FDCAN_Init_ExpectAndReturn( &CANHandler, HAL_OK );
    HAL_FDCAN_ConfigGlobalFilter_ExpectAndReturn( &CANHandler, FDCAN_REJECT, FDCAN_REJECT, FDCAN_FILTER_REMOTE, FDCAN_FILTER_REMOTE, HAL_OK );
    HAL_FDCAN_ConfigFilter_IgnoreAndReturn( HAL_OK );
    HAL_FDCAN_Start_ExpectAndReturn( &CANHandler, HAL_OK );
    HAL_FDCAN_ActivateNotification_ExpectAndReturn( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0, HAL_OK );

    Serial_InitTask( );
}

void test__Serial_PeriodicTask__queue_with_time_msg( void )
{
    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    Serial_PeriodicTask( );
}


STATIC void Serial_SingleFrameTx( uint8_t *data, uint8_t size );

void test__Serial_SingleFrameTx__pack_msg_7bytes_payload( void )
{
    uint8_t data[BYTES_CAN_MESSAGE] = {'H', 'I', 'W', 'O', 'R', 'L', 'D', 0xFF}; /*0xFF is a don't care value*/         
    const uint8_t data_expected[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};

    Serial_SingleFrameTx( data, 7u );

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE( data_expected, data, BYTES_CAN_MESSAGE, "Message isn't a CAN-TP single frame" );
}

void test__Serial_SingleFrameTx__pack_msg_set_size_over_7bytes( void )
{
    uint8_t data[BYTES_CAN_MESSAGE] = {'H', 'I', 'W', 'O', 'R', 'L', 'D', 0xFF}; /*0xFF is a don't care value*/           
    const uint8_t data_expected[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};

    Serial_SingleFrameTx( data, 8u );

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE( data_expected, data, BYTES_CAN_MESSAGE, "Message isn't valid a CAN-TP single frame" );
}

STATIC uint8_t Serial_SingleFrameRx( uint8_t *data, uint8_t *size);

void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_check_size( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;

    Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_EQUAL( SINGLE_FRAME_7_PAYLOAD, size );
}

void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_check_if_its_valid( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_TRUE( varRet );
}

void test__Serial_SingleFrameRx__unpack_msg_no_valid_CAN_TP_single_frame_check_if_its_valid( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {FIRST_FRAME_CAN_TP, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_FALSE( varRet );
}

void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_wPayload_zero_return_FALSE( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {0x00, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_FALSE( varRet );
}


void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_wPayload_eight_return_FALSE( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {0x08, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_FALSE( varRet );
}


STATIC APP_state Evaluate_Msg( AppQue_Queue *hqueue );

void test__Evaluate_Msg__msg_TimeID_return_TIME_state( void )
{
    APP_state stateRet;

    stateRet = Evaluate_Msg( &queueAux );

    TEST_ASSERT_EQUAL( stateRet, TIME );
}

void test__Evaluate_Msg__msg_DateID_return_DATE_state( void )
{
APP_state stateRet;

Evaluate_Msg( &queueAux );

stateRet = Evaluate_Msg( &queueAux );

TEST_ASSERT_EQUAL( stateRet, DATE );
}

void test__Evaluate_Msg__msg_AlarmID_return_ALARM_state( void )
{
APP_state stateRet;

for (uint8_t i = 0; i < 2; i++)
{
   Evaluate_Msg( &queueAux );
}

stateRet = Evaluate_Msg( &queueAux );

TEST_ASSERT_EQUAL( stateRet, ALARM );
}

void test__Evaluate_Msg__msg_UnknownID_return_ERROR_state( void )
{
APP_state stateRet;

for (uint8_t i = 0; i < 3; i++)
{
   Evaluate_Msg( &queueAux );
}

stateRet = Evaluate_Msg( &queueAux );

TEST_ASSERT_EQUAL( stateRet, ERROR_ );
}

STATIC APP_state Evaluate_Time_Parameters( APP_CanTypeDef *msg );

void test__Evaluate_Time_Parameters__valid_time_return_OK_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_SEC;

    stateRet = Evaluate_Time_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, OK );
}

void test__Evaluate_Time_Parameters__no_valid_hour_return_ERROR_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = NO_VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_SEC;

    stateRet = Evaluate_Time_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, ERROR_ );
}

STATIC APP_state Evaluate_Date_Parameters( APP_CanTypeDef *msg );

void test__Evaluate_Date_Parameters__valid_date_return_OK_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_DAY;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MONTH;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_YEAR_MS;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS;

    stateRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, OK );
}

void test__Evaluate_Date_Parameters__valid_date_leap_year_return_OK_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_DAY_LEAP;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MONTH_LEAP;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_YEAR_MS_LEAP;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS_LEAP;

    stateRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, OK );
}

void test__Evaluate_Date_Parameters__no_valid_date_return_ERROR_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = NO_VALID_BCD_DAY;
    msgRead.bytes[ PARAMETER_2 ] = NO_VALID_BCD_MONTH;
    msgRead.bytes[ PARAMETER_3 ] = NO_VALID_BCD_YEAR_MS;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS;

    stateRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, ERROR_ );
}

STATIC APP_state Evaluate_Alarm_Parameters( APP_CanTypeDef *msg );

void test__Evaluate_Alarm_Parameters__valid_Alarm_return_OK_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;

    stateRet = Evaluate_Alarm_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, OK );
}

void test__Evaluate_Alarm_Parameters__no_valid_Alarm_return_ERROR_state( void )
{
    APP_CanTypeDef msgRead;
    APP_state stateRet;

    msgRead.bytes[ PARAMETER_1 ] = NO_VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = NO_VALID_BCD_MIN;

    stateRet = Evaluate_Alarm_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( stateRet, ERROR_ );
}

STATIC void Send_Ok_Message( void );

void test__Send_Ok_Message( void )
{
    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    Send_Ok_Message();
}

STATIC void Send_Error_Message( void );

void test__Send_Error_Message( void )
{
    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    Send_Error_Message();
}

void test__HAL_FDCAN_RxFifo0Callback__receive_single_frame_CAN_TP_msg( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {SINGLE_FRAME_7_PAYLOAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

void test__HAL_FDCAN_RxFifo0Callback__receive_first_frame_CAN_TP_msg( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {FIRST_FRAME_CAN_TP, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    
    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

void test__HAL_FDCAN_RxFifo0Callback__function_called_by_FIFO0full_interrupt( void )
{
    HAL_FDCAN_GetRxMessage_IgnoreAndReturn( HAL_OK );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_FULL );
}


/*Tests Validate_Date function*/
STATIC unsigned char Validate_Date( unsigned char days, unsigned char month, unsigned short year );

void test__Validate_Date__no_valid_days_0_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 0u;
    const uint8_t month = 10u;
    const uint16_t year = 2023;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_Date__no_valid_month_0_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 10u;
    const uint8_t month = 0u;
    const uint16_t year = 2023u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_Date__no_valid_year_2110_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 10u;
    const uint8_t month = 11u;
    const uint16_t year = 2110u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_Date__no_valid_year_0_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 10u;
    const uint8_t month = 10u;
    const uint16_t year = 0u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_Date__no_valid_day_greater_than_mdays_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 35u;   
    const uint8_t month = 10u;
    const uint16_t year = 2023u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}


/*Tests Validate_LeapYear function*/
STATIC uint8_t Validate_LeapYear( unsigned short year );

void test__Validate_LeapYear__2024_leap_year_return_true( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2024 );

    TEST_ASSERT_TRUE( varRet );
}

void test__Validate_LeapYear__2000_leap_year_return_true( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2000 );

    TEST_ASSERT_TRUE( varRet );
}

void test__Validate_LeapYear__2023_no_leap_year_return_false( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2023 );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_LeapYear__2100_no_leap_year_return_false( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2100 );

    TEST_ASSERT_FALSE( varRet );
}


/*Tests Validate_Time function*/
STATIC unsigned char Validate_Time (unsigned char hour, unsigned char minutes, unsigned char seconds);

void test__Validate_Time__all_parameters_ok_return_True( void )
{
    uint8_t varRet;

    varRet = Validate_Time( VALID_BCD_HOUR, VALID_BCD_MIN, VALID_BCD_SEC );

    TEST_ASSERT_TRUE( varRet );
}

void test__Validate_Time__no_valid_hour_return_False( void )
{
    uint8_t varRet;

    varRet = Validate_Time( NO_VALID_BCD_HOUR, VALID_BCD_MIN, VALID_BCD_SEC );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_Time__no_valid_minutes_return_False( void )
{
    uint8_t varRet;

    varRet = Validate_Time( VALID_BCD_HOUR, NO_VALID_BCD_MIN, VALID_BCD_SEC );

    TEST_ASSERT_FALSE( varRet );
}

void test__Validate_Time__no_valid_seconds_return_False( void )
{
    uint8_t varRet;

    varRet = Validate_Time( VALID_BCD_HOUR, VALID_BCD_MIN, NO_VALID_BCD_SEC );

    TEST_ASSERT_FALSE( varRet );
}