/**
 * @file    test_serial.c
 * 
 * @brief   Here are the unit test cases for serial driver functions.
 * 
*/
#include "unity.h"
#include "serial.h"
#include "bsp.h"
#include <stdint.h>

#include "mock_queue.h"
#include "mock_stm32g0xx_hal_fdcan.h"

#define BYTES_CAN_MESSAGE       0x08u   /*!< Number of bytes in a standard CAN message */
#define SINGLE_FRAME_7_PAYLOAD  0x07u   /*!< Byte 0 of a CAN-TP single frame message  */
#define FIRST_FRAME_CAN_TP      0x17u   /*!< Byte 0 of a CAN-TP first frame message */
#define VALID_BCD_HOUR          0x08u   /*!< A valid value of hour in BCD format*/
#define VALID_BCD_MIN           0x00u   /*!< A valid value of minutes in BCD format*/
#define VALID_BCD_SEC           0x00u   /*!< A valid value of seconds in BCD format*/
#define NO_VALID_BCD_HOUR       0x30u   /*!< No valid value of hour in BCD format*/
#define NO_VALID_BCD_MIN        0x60u   /*!< No valid value of minutes in BCD format*/
#define NO_VALID_BCD_SEC        0x60u   /*!< No valid value of seconds in BCD format*/
#define UNKNOW_ID               0x0FFu  /*!< ID unknown for this application */
#define VALID_BCD_DAY           0x30u   /*!< Valid value for a day in BCD format*/
#define VALID_BCD_MONTH         0x11u   /*!< Valid value for a month in BCD format*/
#define VALID_BCD_YEAR_MS       0x20u   /*!< Valid value for the two most significant figures of a year in BCD format*/
#define VALID_BCD_YEAR_LS       0x21u   /*!< Valid value for the two least significant figures of a year in BCD format*/
#define NO_VALID_BCD_DAY        0x35u   /*!< No valid value for a day in BCD format*/
#define NO_VALID_BCD_MONTH      0x13u   /*!< No valid value for a month in BCD format*/
#define NO_VALID_BCD_YEAR_MS    0x50u   /*!< No valid value for the two most significant figures of a year in BCD format*/

#define VALID_BCD_DAY_LEAP      0x29u   
#define VALID_BCD_MONTH_LEAP    0x02u
#define VALID_BCD_YEAR_MS_LEAP  0x20u   
#define VALID_BCD_YEAR_LS_LEAP  0x20u


uint8_t dataTime[BYTES_CAN_MESSAGE] = {VALID_BCD_HOUR, VALID_BCD_MIN, VALID_BCD_SEC, 0xFF, SERIAL_MSG_TIME, 0xFF, 0xFF, 0xFF};

void setUp( void )
{
}

void tearDown( void )
{
}

/**
 * @brief   Test for Serial_InitTask
 * 
 * This function uses the mock functions from FDCAN HAL library and queue.h, here there isn't something
 * to test just check if all lines in this interface are executed using code coverage.
*/
void test__Serial_InitTask__init_FDCAN_module( void )
{
    HAL_FDCAN_Init_IgnoreAndReturn( HAL_OK );
    HAL_FDCAN_ConfigGlobalFilter_IgnoreAndReturn( HAL_OK );
    HAL_FDCAN_ConfigFilter_IgnoreAndReturn( HAL_OK );
    HAL_FDCAN_Start_IgnoreAndReturn( HAL_OK );
    HAL_FDCAN_ActivateNotification_IgnoreAndReturn( HAL_OK );
    AppQueue_initQueue_Ignore( );

    Serial_InitTask( );
}

/**
 * @brief   Test for serial periodic task, mock read queue with time msg.
 * 
 * In this function it is only necessary check if all lines are executed, to do that mock functions
 * HIL_QUEUE_isQueueEmptyISR and HIL_QUEUE_writeDataISR to simulate a queue with a time message.
*/
void test__Serial_PeriodicTask__queue_with_time_msg( void )
{
    APP_CanTypeDef SerialMsg;
    dataTime[MSG] = SERIAL_MSG_TIME;
    memcpy( SerialMsg.bytes, &dataTime, BYTES_CAN_MESSAGE );

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );

    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &SerialMsg, sizeof( APP_CanTypeDef ) );

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );

    Serial_PeriodicTask( );
}


STATIC void Serial_SingleFrameTx( uint8_t *data, uint8_t size );
/**
 * @brief   Test for Serial_SingleFrameTx to pack a message.
 * 
 * Here is defined an uint8_t array with a payload of 7, and also is define an array with the first
 * byte in CAN-TP single frame format to compare the result of this interface, is used the assert
 * TEST_ASSERT_EQUAL_MEMORY to compare the eight bytes of the can msg and the result must be true.
*/
void test__Serial_SingleFrameTx__pack_msg_7bytes_payload( void )
{
    uint8_t data[BYTES_CAN_MESSAGE] = {'H', 'I', 'W', 'O', 'R', 'L', 'D', 0xFF}; /*0xFF is a don't care value*/         
    const uint8_t data_expected[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};

    Serial_SingleFrameTx( data, 7u );

    TEST_ASSERT_EQUAL_MEMORY_MESSAGE( data_expected, data, BYTES_CAN_MESSAGE, "Message isn't a CAN-TP single frame" );
}

/**
 * @brief   Test for Serial_SingleFrameTx to pack a message.
 * 
 * Here is defined an uint8_t array with a payload of 8 and the function change this value for a 7, 
 * and also is define an array with the first byte in CAN-TP single frame format to compare the 
 * result of this interface, is used the assert TEST_ASSERT_EQUAL_MEMORY to compare the eight bytes
 * of the can msg and the result must be true.
*/
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


STATIC APP_Messages Evaluate_Time_Parameters( APP_CanTypeDef *SerialMsgPtr );

void test__Evaluate_Time_Parameters__valid_time_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_SEC;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Time_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

void test__Evaluate_Time_Parameters__no_valid_hour_ERROR_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = NO_VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_SEC;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Time_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_ERROR );
}

STATIC APP_Messages Evaluate_Date_Parameters( APP_CanTypeDef *SerialMsgPtr );

void test__Evaluate_Date_Parameters__valid_date_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_DAY;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MONTH;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_YEAR_MS;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

void test__Evaluate_Date_Parameters__valid_date_leap_year_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_DAY_LEAP;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MONTH_LEAP;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_YEAR_MS_LEAP;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS_LEAP;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

void test__Evaluate_Date_Parameters__no_valid_date_ERROR_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = NO_VALID_BCD_DAY;
    msgRead.bytes[ PARAMETER_2 ] = NO_VALID_BCD_MONTH;
    msgRead.bytes[ PARAMETER_3 ] = NO_VALID_BCD_YEAR_MS;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_ERROR );
}

/*Tests function Evaluate_Alarm_Parameters*/
STATIC APP_Messages Evaluate_Alarm_Parameters( APP_CanTypeDef *SerialMsgPtr );

void test__Evaluate_Alarm_Parameters__valid_Alarm_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Alarm_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

void test__Evaluate_Alarm_Parameters__no_valid_Alarm_ERROR_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = NO_VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = NO_VALID_BCD_MIN;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Alarm_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_ERROR );
}

STATIC APP_Messages Send_Ok_Message( APP_CanTypeDef *SerialMsgPtr );

void test__Send_Ok_Message( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    eventRet = Send_Ok_Message( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_NONE );
}

STATIC APP_Messages Send_Error_Message( APP_CanTypeDef *SerialMsgPtr );

void test__Send_Error_Message( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    eventRet = Send_Error_Message( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_NONE );
}

void test__HAL_FDCAN_RxFifo0Callback__receive_single_frame_CAN_TP_msg_time( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {SINGLE_FRAME_7_PAYLOAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    FDCAN_RxHeaderTypeDef RxHeader;
    RxHeader.Identifier = ID_TIME_MSG;

    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );
    HAL_FDCAN_GetRxMessage_ReturnMemThruPtr_pRxHeader( &RxHeader, sizeof(FDCAN_RxHeaderTypeDef) );

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

void test__HAL_FDCAN_RxFifo0Callback__receive_single_frame_CAN_TP_msg_date( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {SINGLE_FRAME_7_PAYLOAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    FDCAN_RxHeaderTypeDef RxHeader;
    RxHeader.Identifier = ID_DATE_MSG;

    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );
    HAL_FDCAN_GetRxMessage_ReturnMemThruPtr_pRxHeader( &RxHeader, sizeof(FDCAN_RxHeaderTypeDef) );

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

void test__HAL_FDCAN_RxFifo0Callback__receive_single_frame_CAN_TP_msg_alarm( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {SINGLE_FRAME_7_PAYLOAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    FDCAN_RxHeaderTypeDef RxHeader;
    RxHeader.Identifier = ID_ALARM_MSG;

    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );
    HAL_FDCAN_GetRxMessage_ReturnMemThruPtr_pRxHeader( &RxHeader, sizeof(FDCAN_RxHeaderTypeDef) );

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

void test__HAL_FDCAN_RxFifo0Callback__receive_single_frame_CAN_TP_msg_id_unknown( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {SINGLE_FRAME_7_PAYLOAD, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    FDCAN_RxHeaderTypeDef RxHeader;
    RxHeader.Identifier = UNKNOW_ID;

    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );
    HAL_FDCAN_GetRxMessage_ReturnMemThruPtr_pRxHeader( &RxHeader, sizeof(FDCAN_RxHeaderTypeDef) );

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

void test__HAL_FDCAN_RxFifo0Callback__receive_first_frame_CAN_TP_msg( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {FIRST_FRAME_CAN_TP, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );
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