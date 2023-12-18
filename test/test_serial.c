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
#define VALID_BCD_DAY_LEAP      0x29u   /*!< 29 February in a leap lear */
#define VALID_BCD_MONTH_LEAP    0x02u   /*!< Month number of February */
#define VALID_BCD_YEAR_MS_LEAP  0x20u   /*!< Two most significant figures of a year in BCD format */
#define VALID_BCD_YEAR_LS_LEAP  0x20u   /*!< Two least significant figures of a year in BCD format */

/** 
  * @defgroup   WeekDays WeekDays according WeekDay function.
  @{ */

#define SUNDAY              0x00u       /*!< Sunday (0)*/
#define MONDAY              0x01u       /*!< Monday (1)*/
#define TUESDAY             0x02u       /*!< Tuesday (2)*/
#define WEDNESDAY           0x03u       /*!< Wednesday (3)*/
#define THURSDAY            0x04u       /*!< Thursday (4)*/
#define FRIDAY              0x05u       /*!< Friday (5)*/
#define SATURDAY            0x06u       /*!< Saturday (6)*/
/**
  @} */

/**
 * @brief   reference to the ClockQueue.
*/
AppQue_Queue ClockQueue;

/**
 * @brief   variable to test SerialTask with a valid time message
*/
uint8_t dataTime[BYTES_CAN_MESSAGE] = {VALID_BCD_HOUR, VALID_BCD_MIN, VALID_BCD_SEC, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

/**
 * @brief   function that is executed before any unit test function.
*/
void setUp( void )
{
}

/**
 * @brief   function that is executed after any unit test function.
*/
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
 * In this function it is necessary check if the msg have a valid event index, to do that mock 
 * functions HIL_QUEUE_isQueueEmptyISR and HIL_QUEUE_writeDataISR to simulate a queue with a 
 * time message.
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
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );

    Serial_PeriodicTask( );
}

/**
 * @brief   Test for serial periodic task, mock read queue with none msg.
 * 
 * In this function it is necessary check if the msg have a valid event index, to do that mock 
 * functions HIL_QUEUE_isQueueEmptyISR  to simulate a queue with a message with a event index
 * of 6 (SERIAL_MSG_NONE).
*/
void test__Serial_PeriodicTask__queue_with_none_msg( void )
{
    APP_CanTypeDef SerialMsg;
    dataTime[MSG] = SERIAL_MSG_NONE;
    memcpy( SerialMsg.bytes, &dataTime, BYTES_CAN_MESSAGE );

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( FALSE );

    HIL_QUEUE_readDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_readDataISR_ReturnMemThruPtr_data( &SerialMsg, sizeof( APP_CanTypeDef ) );

    HIL_QUEUE_isQueueEmptyISR_IgnoreAndReturn( TRUE );

    Serial_PeriodicTask( );
}

/**
 * @brief   Reference for private function  Serial_SingleFrameTx
*/
STATIC void Serial_SingleFrameTx( uint8_t*, uint8_t );

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

/**
 * @brief   Reference for private function  Serial_SingleFrameRx
 * @retval  Return TRUE if its a can-tp single frame format, FALSE if it isn't.
*/
STATIC uint8_t Serial_SingleFrameRx( uint8_t*, uint8_t* );

/**
 * @brief   test Serial_SingleFrameRx with a valid CAN-TP sf message.
 * 
 * First it's declared an array of eight bytes where the first byte contain the information that
 * indicate that it is a CAN-TP single frame message with a payload of 7 bytes (0x07), also is 
 * declared a variable where the size is storaged at the moment that the function is called. In
 * this test just check if the size of the message is obtained succesfully and to do that size is
 * compared with a macro that contain number 7. 
*/
void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_check_size( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;

    Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_EQUAL( SINGLE_FRAME_7_PAYLOAD, size );
}

/**
 * @brief   test Serial_SingleFrameRx with a valid CAN-TP single frame message.
 * 
 * First it's declared an array of eight bytes where the first byte contain the information that
 * indicate that it is a CAN-TP single frame message with a payload of 7 bytes (0x07) and another
 * variable is declared to save the value returned by the function, finally this variable is checked
 * with the assertion TEST_ASSERT_TRUE to know if the function returned the correct value.
*/
void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_check_if_its_valid_return_True( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {SINGLE_FRAME_7_PAYLOAD, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test Serial_SingleFrameRx with a CAN-TP first frame message.
 * 
 * First it's declared an array of eight bytes where the first byte contain the information that
 * indicate that it is a CAN-TP fist frame message with a payload of 7 bytes (0x07) and another
 * variable is declared to save the value returned by the function, finally this variable is checked
 * with the assertion TEST_ASSERT_FALSE to know if the function returned the correct value.
*/
void test__Serial_SingleFrameRx__unpack_msg_CAN_TP_first_frame_check_if_its_valid_return_False( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {FIRST_FRAME_CAN_TP, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test Serial_SingleFrameRx with a valid CAN-TP single frame message with payload of 0.
 * 
 * First it's declared an array of eight bytes where the first byte contain the information that
 * indicate that it is a CAN-TP single frame message with a payload of 0 bytes (0x00) and another
 * variable is declared to save the value returned by the function, finally this variable is checked
 * with the assertion TEST_ASSERT_FALSE to know if the function returned the correct value.
*/
void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_wPayload_zero_return_FALSE( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {0x00, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test Serial_SingleFrameRx with a valid CAN-TP single frame message with payload of 8.
 * 
 * First it's declared an array of eight bytes where the first byte contain the information that
 * indicate that it is a CAN-TP single frame message with a payload of 8 bytes (0x08) and another
 * variable is declared to save the value returned by the function, finally this variable is checked
 * with the assertion TEST_ASSERT_FALSE to know if the function returned the correct value.
*/
void test__Serial_SingleFrameRx__unpack_msg_valid_CAN_TP_single_frame_wPayload_eight_return_FALSE( void )
{
    uint8_t data_received[BYTES_CAN_MESSAGE] = {0x08, 'H', 'I', 'W', 'O', 'R', 'L', 'D'};
    uint8_t size;
    uint8_t varRet;

    varRet = Serial_SingleFrameRx( data_received, &size);

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   Reference for private function  Evaluate_Time_Parameters
 * @retval  Return the event type that was writed in the queue, it can be Error or Ok.
*/
STATIC APP_Messages Evaluate_Time_Parameters( APP_CanTypeDef* );

/**
 * @brief   test Evaluate_Time_Parameters transition to OK event.
 * 
 * First its define a message with valid parameters of time in BCD format, the is used the mock
 * to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged in
 * eventRet variable, finally it's checked if the transition was to the OK event with the
 * TEST_ASSERT_EQUAL assertion.
*/
void test__Evaluate_Time_Parameters__valid_time_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_SEC;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Time_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

/**
 * @brief   test Evaluate_Time_Parameters transition to ERROR event.
 * 
 * First its define a message with not valid parameters of time in BCD format, the is used the
 * mock to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged 
 * in eventRet variable, finally it's checked if the transition was to the ERROR event with the
 * TEST_ASSERT_EQUAL assertion.
*/
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

/**
 * @brief   Reference for private function  Evaluate_Date_Parameters
 * @retval  Return the event type that was writed in the queue, it can be Error or Ok.
*/
STATIC APP_Messages Evaluate_Date_Parameters( APP_CanTypeDef* );

/**
 * @brief   test Evaluate_Date_Parameters transition to OK event.
 * 
 * First its define a message with valid parameters of date in BCD format, then is used the
 * mock to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged 
 * in eventRet variable, finally it's checked if the transition was to the OK event with the
 * TEST_ASSERT_EQUAL assertion.
*/
void test__Evaluate_Date_Parameters__valid_date_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_DAY;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MONTH;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_YEAR_MS;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

/**
 * @brief   test Evaluate_Date_Parameters transition to OK event.
 * 
 * First its define a message with valid parameters (leap year) of date in BCD format, then is used
 * the mock to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged 
 * in eventRet variable, finally it's checked if the transition was to the OK event with the
 * TEST_ASSERT_EQUAL assertion.
*/
void test__Evaluate_Date_Parameters__valid_date_leap_year_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_DAY_LEAP;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MONTH_LEAP;
    msgRead.bytes[ PARAMETER_3 ] = VALID_BCD_YEAR_MS_LEAP;
    msgRead.bytes[ PARAMETER_4 ] = VALID_BCD_YEAR_LS_LEAP;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Date_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

/**
 * @brief   test Evaluate_Date_Parameters transition to ERROR event.
 * 
 * First its define a message with not valid parameters of date in BCD format, then is used the
 * mock to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged 
 * in eventRet variable, finally it's checked if the transition was to the ERROR event with the
 * TEST_ASSERT_EQUAL assertion.
*/
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

/**
 * @brief   Reference for private fucntion  Evaluate_Alarm_Parameters
 * @retval  Return the event type that was writed in the queue, it can be Error or Ok.
*/
STATIC APP_Messages Evaluate_Alarm_Parameters( APP_CanTypeDef* );

/**
 * @brief   test Evaluate_Alarm_Parameters transition to OK event.
 * 
 * First its define a message with not valid parameters of alarm in BCD format, then is used the
 * mock to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged 
 * in eventRet variable, finally it's checked if the transition was to the OK event with the
 * TEST_ASSERT_EQUAL assertion.
*/
void test__Evaluate_Alarm_Parameters__valid_Alarm_OK_MSG( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    msgRead.bytes[ PARAMETER_1 ] = VALID_BCD_HOUR;
    msgRead.bytes[ PARAMETER_2 ] = VALID_BCD_MIN;

    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );
    HIL_QUEUE_writeDataISR_ExpectAnyArgsAndReturn( TRUE );

    eventRet = Evaluate_Alarm_Parameters( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_OK );
}

/**
 * @brief   test Evaluate_Alarm_Parameters transition to ERROR event.
 * 
 * First its define a message with not valid parameters of alarm in BCD format, then is used the
 * mock to HIL_QUEUE_writeDataISR function, the event that was "writed" in the queue is storaged 
 * in eventRet variable, finally it's checked if the transition was to the ERROR event with the
 * TEST_ASSERT_EQUAL assertion.
*/
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

/**
 * @brief   Reference for private fucntion  Send_Ok_Message
 * @retval  Return the event type that was writed in the queue (None).
*/
STATIC APP_Messages Send_Ok_Message( APP_CanTypeDef* );

/**
 * @brief   test Send_Ok_Message.
 * 
 * In this function there is not a transition to another event, this test is made to know
 * if all lines in the block are executed and with TEST_ASSERT_EQUAL assertion test that
 * the funtion returne SERIAL_MSG_NONE.
*/
void test__Send_Ok_Message( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    eventRet = Send_Ok_Message( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_NONE );
}

/**
 * @brief   Reference for private fucntion  Send_Error_Message
 * @retval  Return the event type that was writed in the queue (None).
*/
STATIC APP_Messages Send_Error_Message( APP_CanTypeDef* );

/**
 * @brief   test Send_Error_Message.
 * 
 * In this function there is not a transition to another event, this test is made to know
 * if all lines in the block are executed and with TEST_ASSERT_EQUAL assertion test that
 * the funtion returne SERIAL_MSG_NONE.
*/
void test__Send_Error_Message( void )
{
    APP_Messages eventRet;
    APP_CanTypeDef msgRead;

    HAL_FDCAN_AddMessageToTxFifoQ_IgnoreAndReturn( HAL_OK );

    eventRet = Send_Error_Message( &msgRead );

    TEST_ASSERT_EQUAL( eventRet, SERIAL_MSG_NONE );
}

/**
 * @brief   test HAL_FDCAN_RxFifo0Callback with a msg time with CAN-TP format.
 * 
 * The aim of this test is cover the TIME branch in the switch statement for this an array of 8 bytes
 * is define with the content of a time message in CAN-TP single frame format, and also is necessary 
 * use the mock function from HAL library to indicate the TIME ID of the received message
*/
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

/**
 * @brief   test HAL_FDCAN_RxFifo0Callback with a msg date with CAN-TP format.
 * 
 * The aim of this test is cover the DATE branch in the switch statement for this an array of 8 bytes
 * is define with the content of a time message in CAN-TP single frame format, and also is necessary 
 * use the mock function from HAL library to indicate the DATE ID of the received message
*/
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

/**
 * @brief   test HAL_FDCAN_RxFifo0Callback with a alarm msg with CAN-TP format.
 * 
 * The aim of this test is cover the ALARM branch in the switch statement for this an array of 8 bytes
 * is define with the content of a time message in CAN-TP single frame format, and also is necessary 
 * use the mock function from HAL library to indicate the ALARM ID of the received message
*/
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

/**
 * @brief   test HAL_FDCAN_RxFifo0Callback with a msg with an ID unknown.
 * 
 * The aim of this test is cover the default branch in the switch statement for this use the
 * mock function from HAL library to indicate the unknown ID of the received message
*/
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

/**
 * @brief   test HAL_FDCAN_RxFifo0Callback with time with CAN-TP first frame format.
 * 
 * The aim of this test is cover the TIME branch of the if statement for this an array of 8 bytes
 * is define with the content of a message in CAN-TP first frame format, and the function doesn't 
 * take in account to be written to the queue.
*/
void test__HAL_FDCAN_RxFifo0Callback__receive_first_frame_CAN_TP_msg( void )
{
    /*0xFF is a don't care value*/
    uint8_t msg_CanTP[ BYTES_CAN_MESSAGE ] = {FIRST_FRAME_CAN_TP, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

    HAL_FDCAN_GetRxMessage_ExpectAnyArgsAndReturn( HAL_OK );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );
    HAL_FDCAN_GetRxMessage_ReturnThruPtr_pRxData( msg_CanTP );

    HAL_FDCAN_RxFifo0Callback( &CANHandler, FDCAN_IT_RX_FIFO0_NEW_MESSAGE );
}

/**
 * @brief   Reference for private fucntion Validate_Date
 * @retval  Returns TRUE when its a valid date and FALSE when it is not.
*/
STATIC uint8_t Validate_Date( uint8_t, uint8_t, uint16_t );

/**
 * @brief test Validate_Date with day parameter not valid.
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * the parameters to test the function. Finally the returned value is tested wit TEST_ASSERT_FALSE
 * to know if the fucntion return a correct value when the day parameter is OORL;
*/
void test__Validate_Date__no_valid_days_0_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 0u;
    const uint8_t month = 10u;
    const uint16_t year = 2023u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief test Validate_Date with month parameter not valid.
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * the parameters to test the function. Finally the returned value is tested wit TEST_ASSERT_FALSE
 * to know if the function return a correct value when the month parameter is OORL;
*/
void test__Validate_Date__no_valid_month_0_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 10u;
    const uint8_t month = 0u;
    const uint16_t year = 2023u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief test Validate_Date with year parameter not valid (OORH).
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * the parameters to test the function. Finally the returned value is tested wit TEST_ASSERT_FALSE
 * to know if the function return a correct value when the year parameter is OORH;
*/
void test__Validate_Date__no_valid_year_2110_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 10u;
    const uint8_t month = 11u;
    const uint16_t year = 2110u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief test Validate_Date with year parameter not valid (OORL).
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * the parameters to test the function. Finally the returned value is tested wit TEST_ASSERT_FALSE
 * to know if the function return a correct value when the year parameter is OORL;
*/
void test__Validate_Date__no_valid_year_0_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 10u;
    const uint8_t month = 10u;
    const uint16_t year = 0u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief test Validate_Date with day parameter not valid (OORH).
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * the parameters to test the function. Finally the returned value is tested with TEST_ASSERT_FALSE
 * to know if the function return a correct value when the days parameter is OORH;
*/
void test__Validate_Date__no_valid_day_greater_than_mdays_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 35u;   
    const uint8_t month = 10u;
    const uint16_t year = 2023u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief test Validate_Date with all parameters valid.
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_TRUE
 * to know if the function return a correct value when all paratemeters are in a valid range.
*/
void test__Validate_Date__valid_parameters_return_True( void )
{
    uint8_t varRet;
    const uint8_t days = 30u;   
    const uint8_t month = 11u;
    const uint16_t year = 1999u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief test Validate_Date with all parameters valid for a leap year.
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_TRUE
 * to know if the function return a correct value when all paratemeters are in a valid range, and 
 * it is proven that it takes into account leap year.
*/
void test__Validate_Date__valid_parameters_leap_year_return_True( void )
{
    uint8_t varRet;
    const uint8_t days = 29u;   
    const uint8_t month = 2u;
    const uint16_t year = 2048u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief test Validate_Date with day parameter not valid for a leap year.
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_FALSE
 * to know if the function return a correct value when day paratemeter isn't in a valid range 
 * for a leap year.
*/
void test__Validate_Date__not_valid_day_leap_year_return_False( void )
{
    uint8_t varRet;
    const uint8_t days = 29u;   
    const uint8_t month = 2u;
    const uint16_t year = 2023u;

    varRet = Validate_Date( days, month, year );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   Reference for private fucntion  Validate_LeapYear
 * @retval  TRUE if its a leap year and FALSE if not. 
*/
STATIC uint8_t Validate_LeapYear( uint16_t );

/**
 * @brief   test Validate_LeapYear with 2024 that it's a leap year.
 * 
 * First is defined an uint8 variable to save the value returned by the function, then the value
 * of the year is used directly in the function and to test the result is used the assertion 
 * TEST_ASSERT_TRUE to check that varRet is TRUE.
*/
void test__Validate_LeapYear__2024_leap_year_return_true( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2024 );

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test Validate_LeapYear with 2000 that it's a leap year.
 * 
 * First is defined an uint8 variable to save the value returned by the function, then the value
 * of the year is used directly in the function and to test the result is used the assertion 
 * TEST_ASSERT_TRUE to check that varRet is TRUE.
*/
void test__Validate_LeapYear__2000_leap_year_return_true( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2000 );

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test Validate_LeapYear with 2023 that it is not a leap year.
 * 
 * First is defined an uint8 variable to save the value returned by the function, then the value
 * of the year is used directly in the function and to test the result is used the assertion 
 * TEST_ASSERT_FALSE to check that varRet is FALSE.
*/
void test__Validate_LeapYear__2023_no_leap_year_return_false( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2023 );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test Validate_LeapYear with 2100 that it is not a leap year.
 * 
 * First is defined an uint8 variable to save the value returned by the function, then the value
 * of the year is used directly in the function and to test the result is used the assertion 
 * TEST_ASSERT_FALSE to check that varRet is FALSE.
*/
void test__Validate_LeapYear__2100_no_leap_year_return_false( void )
{
    uint8_t varRet;

    varRet = Validate_LeapYear( 2100 );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   Reference for private fucntion  Validate_Time.
 * @retval  Returns TRUE when its a valid time and FALSE if it is not.
*/
STATIC unsigned char Validate_Time ( uint8_t, uint8_t, uint8_t );

/**
 * @brief   test Validate_Time with all parameters valid
 * 
 * There are declared four uint8 variables, one to save the returned value by the function and the
 * others to set the parameters: hour, minutes and seconds.
 * In this test case the three parameters are in the correct range of each one, and the variable that
 * saved the result is tested with the assertion TEST_ASSERT_TRUE to check if the function has the
 * correct behavior.
*/
void test__Validate_Time__all_parameters_ok_return_True( void )
{
    uint8_t varRet;
    uint8_t hour    = 13;
    uint8_t minutes = 30;
    uint8_t seconds = 50;

    varRet = Validate_Time( hour, minutes, seconds );

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test Validate_Time with hour parameter not valid (OORH).
 * 
 * There are declared four uint8 variables, one to save the returned value by the function and the
 * others to set the parameters: hour, minutes and seconds.
 * In this test case hour is out of range high, and the variable that saved the result is tested 
 * with the assertion TEST_ASSERT_FALSE to check if the function has the correct behavior.
*/
void test__Validate_Time__not_valid_hour_OORH_return_False( void )
{
    uint8_t varRet;
    uint8_t hour    = 25;
    uint8_t minutes = 30;
    uint8_t seconds = 50;

    varRet = Validate_Time( hour, minutes, seconds );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test Validate_Time with minutes parameter not valid (OORH).
 * 
 * There are declared four uint8 variables, one to save the returned value by the function and the
 * others to set the parameters: hour, minutes and seconds.
 * In this test case minutes is out of range high, and the variable that saved the result is tested 
 * with the assertion TEST_ASSERT_FALSE to check if the function has the correct behavior.
*/
void test__Validate_Time__no_valid_minutes_OORH_return_False( void )
{
    uint8_t varRet;
    uint8_t hour    = 12;
    uint8_t minutes = 65;
    uint8_t seconds = 50;

    varRet = Validate_Time( hour, minutes, seconds );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test Validate_Time with seconds parameter not valid (OORH).
 * 
 * There are declared four uint8 variables, one to save the returned value by the function and the
 * others to set the parameters: hour, minutes and seconds.
 * In this test case seconds is out of range high, and the variable that saved the result is tested 
 * with the assertion TEST_ASSERT_FALSE to check if the function has the correct behavior.
*/
void test__Validate_Time__no_valid_seconds_OORH_return_False( void )
{
    uint8_t varRet;
    uint8_t hour    = 13;
    uint8_t minutes = 30;
    uint8_t seconds = 65;

    varRet = Validate_Time( hour, minutes, seconds );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   Reference for private fucntion WeekDay.
 * @retval  Return the week day (0 to 6 -> Sunday to Monday).
*/
STATIC uint8_t WeekDay( uint8_t, uint8_t, uint16_t );

/**
 * @brief   test WeekDay with date 07/12/2023 thursday
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_EQUAL
 * to know if the fucntion return the correct day corresponding with the date.
*/
void test__WeekDay__date_7_12_2023_return_4_thursday( void )
{
    uint8_t varRet;
    const uint8_t days = 7u;   
    const uint8_t month = 12u;
    const uint16_t year = 2023u;

    varRet = WeekDay( days, month, year );

    TEST_ASSERT_EQUAL( THURSDAY, varRet );
}

/**
 * @brief   test WeekDay with date 12/01/1977 wednesday
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_EQUAL
 * to know if the fucntion return the correct day corresponding with the date.
*/
void test__WeekDay__date_12_01_1977_return_3_wednesday( void )
{
    uint8_t varRet;
    const uint8_t days = 12u;   
    const uint8_t month = 1u;
    const uint16_t year = 1977u;

    varRet = WeekDay( days, month, year );

    TEST_ASSERT_EQUAL( WEDNESDAY, varRet );
}

/**
 * @brief   test WeekDay with date 30/11/1999 tuesday
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_EQUAL
 * to know if the fucntion return the correct day corresponding with the date.
*/
void test__WeekDay__date_30_11_1999_return_2_tuesday( void )
{
    uint8_t varRet;
    const uint8_t days = 30u;   
    const uint8_t month = 11u;
    const uint16_t year = 1999u;

    varRet = WeekDay( days, month, year );

    TEST_ASSERT_EQUAL( TUESDAY, varRet );
}

/**
 * @brief   test WeekDay with date 30/11/2024 saturday
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_EQUAL
 * to know if the fucntion return the correct day corresponding with the date.
*/
void test__WeekDay__date_30_11_2024_return_6_saturday( void )
{
    uint8_t varRet;
    const uint8_t days = 30u;   
    const uint8_t month = 11u;
    const uint16_t year = 2024u;

    varRet = WeekDay( days, month, year );

    TEST_ASSERT_EQUAL( SATURDAY, varRet );
}

/**
 * @brief   test WeekDay with date 29/02/2024 thursday (leap year).
 * 
 * Declare varRet to save the value returned by the function; days, month and year are the
 * parameters to test the function. Finally the returned value is tested with TEST_ASSERT_EQUAL
 * to know if the fucntion return the correct day corresponding with the date.
*/
void test__WeekDay__date_29_02_2024_leap_year_return_4_thursday( void )
{
    uint8_t varRet;
    const uint8_t days = 29u;   
    const uint8_t month = 2u;
    const uint16_t year = 2024u;

    varRet = WeekDay( days, month, year );

    TEST_ASSERT_EQUAL( THURSDAY, varRet );
}