/**
 * @file    test_queue.c
 * 
 * @brief   Unit test cases for the functions from queue file.
*/
#include "unity.h"
#include "queue.h"

 #define HQUEUE_ELEM    8u      /*!< Number of elements in hqueue*/

/**
 * @struct  Msg 
 * @brief   Struct used to test a queue with differents types of data.
*/
typedef struct 
{
    unsigned char msg;  /*!< element msg to save a letter */
    unsigned int val;   /*!< element val to save an uint8 */
} Msg;

/** @brief  queue where only one message will be written */
AppQue_Queue queueRead;
/** @brief  buffer type Msg for queueRead */
Msg bufferRead[ 2 ];

/** @brief  queue to test init and writeMessage functions */
AppQue_Queue hqueue;
/** @brief  buffer type uint8 to hqueue */   
uint8_t buffer[ HQUEUE_ELEM ];

/** @brief  queue to be filled */
AppQue_Queue queueFull;
/** @brief  buffer type uint8 to bufferFull */   
uint8_t bufferFull[ 2 ];

/** @brief  queue with buffer type Msg */
AppQue_Queue s_queue;
/** @brief  buffer type Msg to s_queue */
Msg bufferS[ 2 ];

/** @brief  variable to Write in char queue */
uint8_t dataW = 'A';
/** @brief  variable to save data read from char queue */    
uint8_t dataR = 0;

/** @brief  variable type Msg to write in buffer Msg */
Msg msgRead = {0};      
/** @brief  variable type Msg to read from buffer Msg */
Msg msgWrite = {0};   

/**
 * @brief   function that is executed before any unit test function.
 * 
 * In this function is the configuration and initialization of each queue used in the tests.
 * the queueFull is filled and in the queueRead is written only one element.
*/
void setUp(void)
{
    hqueue.Buffer   = buffer;
    hqueue.Elements = HQUEUE_ELEM;
    hqueue.Size     = sizeof( unsigned char );
    AppQueue_initQueue( &hqueue );

    /*Queue full to test read Function*/
    queueFull.Buffer   = bufferFull;
    queueFull.Elements = 2;
    queueFull.Size     = 1;
    AppQueue_initQueue( &queueFull );
    /*cycle to full the queue*/
    for (uint8_t i = 0; i < queueFull.Elements; i++)
    {
        AppQueue_writeData( &queueFull, &dataW);
    }

    /*Queue with  Msg buffer*/
    s_queue.Buffer   = bufferS;
    s_queue.Elements = 2;
    s_queue.Size     = sizeof(Msg);
    AppQueue_initQueue( &s_queue );

    /*Queue with  Msg buffer to test Read Functions*/
    queueRead.Buffer   = bufferRead;
    queueRead.Elements = 2;
    queueRead.Size     = sizeof(Msg);
    
    msgWrite.msg = 'D'; /*Data to write in the buffer*/
    msgWrite.val = 99;

    AppQueue_initQueue( &queueRead );
    AppQueue_writeData( &queueRead, &msgWrite);
}

/**
 * @brief   function that is executed after any unit test function.
*/
void tearDown(void)
{
}

/**
 * @brief   test AppQueue_initQueue, check the number of elements
 * 
 * In this test the hqueue is initialized and then the number of elements is test to know if
 * the number is the same that is initialized in setUp function.
*/
void test__AppQueue_initQueue__char_queue_check_elements_number( void )
{
    AppQueue_initQueue( &hqueue );

    TEST_ASSERT_EQUAL( HQUEUE_ELEM, hqueue.Elements );
}

/**
 * @brief   test AppQueue_initQueue, check queue size
 * 
 * In this test the hqueue is initialized and then the size of queue is test to know if
 * the number is the same that is initialized in setUp function.
*/
void test__AppQueue_initQueue__char_queue_check_elements_size( void )
{
    AppQueue_initQueue( &hqueue );

    TEST_ASSERT_EQUAL( sizeof( char ), hqueue.Size );
}

/**
 * @brief   test AppQueue_initQueue, check tail index.
 * 
 * In this test the hqueue is initialized and then the tail index is test to know if
 * this index set correctly.
*/
void test__AppQueue_initQueue__char_queue_check_Tail_index( void )
{
    AppQueue_initQueue( &hqueue );

    TEST_ASSERT_EQUAL( 0, hqueue.Tail );
}

/**
 * @brief   test AppQueue_initQueue, check head index.
 * 
 * In this test the hqueue is initialized and then the head index is test to know if
 * this index set correctly.
*/
void test__AppQueue_initQueue__char_queue_check_Head_index( void )
{
    AppQueue_initQueue( &hqueue );

    TEST_ASSERT_EQUAL( 0, hqueue.Head );
}

/**
 * @brief   test AppQueue_initQueue, check empty flag.
 * 
 * In this test the hqueue is initialized and then empty flag is test to know if this flag is set True.
*/
void test__AppQueue_initQueue__char_queue_check_empty_flag_True( void )
{
    AppQueue_initQueue( &hqueue );

    TEST_ASSERT_TRUE( hqueue.Empty );
}

/**
 * @brief   test AppQueue_initQueue, check full flag.
 * 
 * In this test the hqueue is initialized and then full flag is test to know if this flag is set False.
*/
void test__AppQueue_initQueue__char_queue_check_full_flag_False( void )
{
    AppQueue_initQueue( &hqueue );
 
    TEST_ASSERT_FALSE( hqueue.Full );
}

/**
 * @brief   test AppQueue_writeData write single data and check the returned value.
 * 
 * In the hqueue is written a single data and the returned value is saved in variable retWriteData
 * then using TEST_ASSERT_TRUE assertion check if the value is true.
*/
void test__AppQueue_writeData__writting_single_data_check_retVal_True( void )
{
    uint8_t retWriteData;

    retWriteData = AppQueue_writeData( &hqueue, &dataW );

    TEST_ASSERT_TRUE( retWriteData );
}

/**
 * @brief   test AppQueue_writeData write single data and check the empty flag.
 * 
 * In the hqueue is written a single data and with TEST_ASSERT_FALSE is tested the empty flag
 * to check that the value is False.
*/
void test__AppQueue_writeData__writting_single_data_check_Empty_flag_False( void )
{
    AppQueue_writeData( &hqueue, &dataW );

    TEST_ASSERT_FALSE( hqueue.Empty );
}

/**
 * @brief   test AppQueue_writeData write single data and check the value written in the buffer.
 * 
 * In the hqueue is written a single data and how its a queue with uint8 buffer the written value
 * is test with the TEST_ASSERT_EQUAL assertion.
*/
void test__AppQueue_writeData__writting_single_data_check_value_written_in_buffer( void )
{
    AppQueue_writeData( &hqueue, &dataW );

    TEST_ASSERT_EQUAL( dataW, buffer[ hqueue.Tail ] );
}

/**
 * @brief   test AppQueue_readData read single data and check the returned value.
 * 
 * The returned value is storaged in the variable retReadData and is tested with TEST_ASSERT_TRUE
 * to know if the read operation was succesfull.
*/
void test__AppQueue_readData__reading_single_Char_Data_check_retval_True( void )
{    
    uint8_t dataR;
    uint8_t retReadData;

    retReadData = AppQueue_readData( &queueFull, &dataR );

    TEST_ASSERT_TRUE( retReadData );
}

/**
 * @brief   AppQueue_readData read single data and check the returned value.
 * 
 * After read a single date from queueFull the Tail index shall be 1, and its tested with
 * TEST_ASSERT_EQUAL assertion.
*/
void test__AppQueue_readData__reading_single_Char_Data_check_Tail_index( void )
{    
    uint8_t dataR;

    AppQueue_readData( &queueFull, &dataR );

    TEST_ASSERT_EQUAL( 1, queueFull.Tail );
}

/**
 * @brief   AppQueue_readData read single data and check the returned value.
 * 
 * In the setUp function the queueFull is filled with the variable dataW ('A'), and in this
 * test is used the TEST_ASSERT_EQUAL to compare the value written and the value read.
*/
void test__AppQueue_readData__reading_single_Char_Data_check_dataW_equal_dataR( void )
{    
    uint8_t dataR;

    AppQueue_readData( &queueFull, &dataR );

    TEST_ASSERT_EQUAL( dataW, dataR );
}

/**
 * @brief   test AppQueue_writeData filling the queue and check Full flag
 * 
 * The hqueue is filled with the global variable dataW and the is tested the Full flag that shall
 * be True, to check this is used the assertion TEST_ASSERT_TRUE.
*/
void test__AppQueue_writeData__filling_the_Queue_check_Full_flag( void )
{
    for (uint8_t i = 0; i < hqueue.Elements; i++)
    {
        AppQueue_writeData( &hqueue, &dataW );
    }
     
    TEST_ASSERT_TRUE( hqueue.Full );
}

/**
 * @brief   test AppQueue_writeData filling the queue and check Head index
 * 
 * The hqueue is filled with the global variable dataW and the is tested the Head index that shall
 * be 0 because when the queue is full for first time the Head index is reset, to check this is used
 * the assertion TEST_ASSERT_EQUAL.
*/
void test__AppQueue_writeData__filling_the_Queue_check_Head_index_reset_to_0( void )
{
    for (uint8_t i = 0; i < hqueue.Elements; i++)
    {
        AppQueue_writeData( &hqueue, &dataW );
    }
     
    TEST_ASSERT_EQUAL( 0, hqueue.Head );
}

/**
 * @brief   test AppQueue_writeData trying to write in a full queue.
 * 
 * Using the variable varRet is storaged the value returned by the function, that value shall be
 * False because the queueFull is filled in the setUp function.
*/
void test__AppQueue_writeData__trying_to_write_in_FullQueue_return_False(void)
{
    uint8_t varRet;
    
    varRet = AppQueue_writeData( &queueFull, &dataW );
    
    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppQueue_flushQueue and check Full flag.
 * 
 * The queueFull is previously filled in the setUp function, after using the function to flush queue
 * is tested the Full flag that shall be False.
*/
void test__AppQueue_flushQueue__flush_Full_Queue_check_Full_flag_False( void )
{
    HIL_QUEUE_flushQueueISR( &queueFull );

    TEST_ASSERT_FALSE( queueFull.Full );
}

/**
 * @brief   test AppQueue_flushQueue and check Empty flag.
 * 
 * The queueFull is previously filled in the setUp function, after using the function to flush queue
 * is tested the Empty flag that shall be True.
*/
void test__AppQueue_flushQueue__flush_Full_Queue_check_Empty_flag_True( void )
{
    HIL_QUEUE_flushQueueISR( &queueFull );

    TEST_ASSERT_TRUE( queueFull.Empty );
}

/**
 * @brief   test AppQueue_writeData compared msg writed in the queue.
 * 
 * First is defined a Msg variable, then this variable is written in s_queue, after this using 
 * TEST_ASSERTION_EQUAL_MEMORY to compare the msg written and the s_queue buffer in the first
 * position.
*/
void test__AppQueue_writeData__write_in_a_struct_queue_compare_memory_in_the_buffer(void)
{
    Msg msg_expected = { 'D', 99 };

    HIL_QUEUE_writeDataISR( &s_queue, &msg_expected );

    TEST_ASSERT_EQUAL_MEMORY( &msg_expected, &bufferS[ s_queue.Tail ], sizeof(Msg) );
}

/**
 * @brief   test AppQueue_readData check returned value.
 * 
 * The value returned by the function is saved in varRet and shall be True because the queueRead
 * has at least one element in the buffer.
*/
void test__AppQueue_readData__succesfull_read_single_data_structureQueue_check_returned_value_True(void)
{
    uint8_t varRet;

    varRet = HIL_QUEUE_readDataISR( &queueRead, &msgRead);

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test AppQueue_readData compared the read msg.
 * 
 * In the setUp function the msgWrite is written in the queueRead, and in this test the msg read is
 * storaged in msgRead then this variables are compared with TEST_ASSERT_EQUAL_MEMORY assertion.  
*/
void test__AppQueue_readData__succesfull_read_single_data_structureQueue_compare_msgWrited_with_msgRead(void)
{
    HIL_QUEUE_readDataISR( &queueRead, &msgRead);

    TEST_ASSERT_EQUAL_MEMORY( &msgWrite, &msgRead, sizeof(Msg) );
}

/**
 * @brief   test AppQueue_isQueueEmpty in a empty queue.
 * 
 * The hqueue has no elements in the buffer, the returned value by the function shall be True.
*/
void test__AppQueue_isQueueEmpty__verify_an_empty_queue_return_True(void)
{
    uint8_t varRet;

    varRet = HIL_QUEUE_isQueueEmptyISR( &hqueue );

    TEST_ASSERT_TRUE( varRet );
}

/**
 * @brief   test AppQueue_isQueueEmpty in a full queue.
 * 
 * The queueFull is full, the returned value by the function shall be False.
*/
void test__AppQueue_isQueueEmpty__verify_a_no_empty_queue_return_False(void)
{
    uint8_t varRet;

    varRet = AppQueue_isQueueEmpty( &queueFull );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppQueue_readData trying to read from a empty queue.
 * 
 * The hqueue has no elements in the buffer, then the value returned by the function shall be
 * False.
*/
void test__AppQueue_readData__read_from_empty_queue_return_False( void )
{
    uint8_t varRet;

    varRet = AppQueue_readData( &hqueue, &dataR );

    TEST_ASSERT_FALSE( varRet );
}

/**
 * @brief   test AppQueue_readData read all elements in a buffer.
 * 
 * The queueFull is filled previously in the setUp function, in this test all elements from this
 * queue are read, then the Empty flag shall be True.
*/
void test__AppQueue_readData__readAllData_from_full_queue_return_False( void )
{
    for (uint8_t i = 0; i < queueFull.Elements; i++)
    {
        AppQueue_readData( &queueFull, &dataR);
    }

    TEST_ASSERT_TRUE( queueFull.Empty );
}