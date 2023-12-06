#include "unity.h"
#include "queue.h"

#define TRUE 1
#define FALSE 0

typedef struct 
{
    unsigned char msg;
    unsigned int val;
} Msg;

/*Global variables*/
AppQue_Queue queueRead;    //queue read Test
Msg bufferRead[ 8 ];

AppQue_Queue hqueue;   
uint8_t buffer[ 8 ];

AppQue_Queue queueFull;   
uint8_t bufferFull[ 2 ];

AppQue_Queue s_queue; 
Msg bufferS[ 2 ];

uint8_t dataW = 'A';    //variable to Write in char queue
uint8_t dataR = 0;      //variable to save data read from char queue

Msg msgRead = {0};      //variable type Msg to write in buffer Msg
Msg msgWrite = {0};     //variable type Msg to read from buffer Msg


void setUp(void)
{
    /*Queue with uint8 buffer*/
    dataW = 'A';

    hqueue.Buffer   = buffer;
    hqueue.Elements = 8;
    hqueue.Size     = 1;
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
    queueRead.Elements = 8;
    queueRead.Size     = sizeof(Msg);
    
    msgWrite.msg = 'D'; /*Data to write in the buffer*/
    msgWrite.val = 99;

    AppQueue_initQueue( &queueRead );
    AppQueue_writeData( &queueRead, &msgWrite);
}

void tearDown(void)
{
}


void test__AppQueue_initQueue__char_queue( void )
{
    AppQueue_initQueue( &hqueue );

    TEST_ASSERT_EQUAL_PTR( buffer, hqueue.Buffer );
    TEST_ASSERT_EQUAL( 8, hqueue.Elements );
    TEST_ASSERT_EQUAL( 1, hqueue.Size );
    TEST_ASSERT_EQUAL( 0, hqueue.Head );
    TEST_ASSERT_EQUAL( 0, hqueue.Tail );
    TEST_ASSERT_TRUE( hqueue.Empty );
    TEST_ASSERT_FALSE( hqueue.Full );
}

void test__AppQueue_writeData__writting_single_data( void )
{
    uint8_t retWriteData;

    retWriteData = AppQueue_writeData( &hqueue, &dataW );

    TEST_ASSERT_TRUE( retWriteData );
    TEST_ASSERT_FALSE( hqueue.Empty );
    TEST_ASSERT_EQUAL( 1, hqueue.Head );
    TEST_ASSERT_EQUAL( dataW, buffer[ hqueue.Head - 1 ] );
}

void test__AppQueue_readData__reading_Single_Char_Data( void )
{    
    uint8_t dataR, retReadData;

    buffer[ hqueue.Head ] = dataW;
    hqueue.Empty = FALSE;
    hqueue.Head++;

    retReadData = AppQueue_readData( &hqueue, &dataR );

    TEST_ASSERT_EQUAL( TRUE, retReadData );
    TEST_ASSERT_EQUAL( 1, hqueue.Tail );
    TEST_ASSERT_EQUAL( dataW, dataR );
    TEST_ASSERT_EQUAL( TRUE, hqueue.Empty );
}

void test__AppQueue_writeData__filling_the_Queue( void )
{
    for (uint8_t i = 0; i < hqueue.Elements; i++)
    {
        AppQueue_writeData( &hqueue, &dataW );
        dataW++;
    }
     
    TEST_ASSERT_EQUAL( TRUE, hqueue.Full );
    TEST_ASSERT_EQUAL( 0, hqueue.Head );
}

void test__AppQueue_writeData__trying_to_write_in_FullQueue(void)
{
    uint8_t varRet;
    
    varRet = AppQueue_writeData( &queueFull, &dataW );
    
    TEST_ASSERT_FALSE( varRet );
}


void test__AppQueue_flushQueue__flush_Full_Queue( void )
{
    HIL_QUEUE_flushQueueISR( &queueFull );

    TEST_ASSERT_FALSE( queueFull.Full );
    TEST_ASSERT_TRUE( queueFull.Empty );
    TEST_ASSERT_EQUAL( 0, queueFull.Head );
    TEST_ASSERT_EQUAL( 0, queueFull.Tail );
}

void test__AppQueue_writeData__write_in_a_struct_queue(void)
{
    HIL_QUEUE_writeDataISR( &s_queue, &msgWrite );

    TEST_ASSERT_EQUAL( 'D', bufferS[0].msg );
    TEST_ASSERT_EQUAL( 99, bufferS[0].val );
}

void test__AppQueue_readData__succesfull_read_single_data_structureQueue(void)
{
    uint8_t varRet;

    varRet = HIL_QUEUE_readDataISR( &queueRead, &msgRead);

    TEST_ASSERT_TRUE( varRet );
    TEST_ASSERT_EQUAL_MEMORY( &msgWrite, &msgRead, sizeof(Msg) );
}

void test__AppQueue_isQueueEmpty__verify_an_empty_queue(void)
{
    uint8_t varRet;

    varRet = HIL_QUEUE_isQueueEmptyISR( &hqueue );

    TEST_ASSERT_TRUE( varRet );
}

void test__AppQueue_isQueueEmpty__verify_a_no_empty_queue(void)
{
    uint8_t varRet;

    varRet = AppQueue_isQueueEmpty( &queueFull );

    TEST_ASSERT_FALSE( varRet );
}

void test__AppQueue_readData__read_from_empty_queue( void )
{
    uint8_t varRet;

    varRet = AppQueue_readData( &hqueue, &dataR );

    TEST_ASSERT_FALSE( varRet );
}

void test__AppQueue_readData__readAllData_from_full_queue( void )
{
    for (uint8_t i = 0; i < queueFull.Elements; i++)
    {
        AppQueue_readData( &queueFull, &dataR);
    }

    TEST_ASSERT_TRUE( queueFull.Empty );
}