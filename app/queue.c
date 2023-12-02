/**
 * @file    queue.c
 * @brief   Queue to handle any type of data.
 *
 * This queue make a copy of the elements to be written and always make a copy of the elements read, 
 * the interface contain fuctions to initialize the queue, write data, read data, to know if the
 * queue is empty and a function to flush the queue.
 * 
 */

#include <string.h>
#include "queue.h"
#include "bsp.h"

/** 
  * @defgroup BoleanValues 
  @{ */
#define FALSE   0u       /*!< Replace the word FALSE with 0 */
#define TRUE    1u       /*!< Replace the word TRUE with 1 */
/**
  @} */


/**
 * @brief   Interface to initialize the queue.
 *
 * This interface initialize the queue, setting the elements Tail and Head to zero, and the flags Empty
 * and Full with the values TRUE and FALSE, respectively. 
 *
 * @param   queue [in] It's the memory address of the queue to access the elements.
 *
 *
 * @note Before using this function it's mandatory initialized the elements: Buffer, Elements and Size.
 */
void AppQueue_initQueue( AppQue_Queue *queue )
{
    queue->Head = 0;      //Setting index Tail and Head to zero
    queue->Tail = 0;
    queue->Empty = TRUE;  //Empty flag to TRUE and Full flg to FALSE
    queue->Full = FALSE;
}

/**
 * @brief   Copy the given data in the queue buffer.
 *
 * First of all, verify if the queue isn't Full to write data, and as a void pointers is received, it's 
 * necessary cast it to use pointer's arithmetic and to be able to copy the data in the buffer, later
 * increment Head, and to know if reach the Tail compare them, in an TRUE case set the Full flag to TRUE.
 *
 * @param   queue [in] It's the memory address of the queue to access the elements.
 * @param   data [in] Memory address where is the data to be written.
 *
 * @retval  Return the success of the write action, TRUE if the data was written, and FALSE in case the
 * queue is Full.
 *
 */
unsigned char AppQueue_writeData( AppQue_Queue *queue, const void *data )
{   
    unsigned char varRet = FALSE;

    if ( queue->Full == FALSE)
    {
        queue->Empty = FALSE;       //if access to write then the queue will no longer be empty

        /*cppcheck-suppress misra-c2012-11.5 ; 
        The function receive a void pointer to the buffer
        if this is changed, the queue can no longer handle any type of data.*/
        unsigned char *ptrBuffer = (unsigned char*) queue->Buffer;

        (void) memcpy( &ptrBuffer[queue->Head * queue->Size], data, queue->Size );
        
        queue->Head++;
        queue->Head = queue->Head % queue->Elements;

        if ( queue->Head == queue->Tail )
        {
            queue->Full = TRUE;
        }
        
        varRet = TRUE;
    }
    
    return varRet;
}

/**
 * @brief   Copy the data of the buffer in the memmory address given.
 *
 * First verify if the queue has at least one element, then make a copy of data in the queue, increment
 * Tail, reset it if it's necessary, and check if the queue now its empty to set the corresponding flag.
 *
 * @param   queue [in] It's the memory address of the queue to access the elements.
 * @param   data [out] Memory address where the read data will be copied.
 *
 * @retval  Return the success of the read action, TRUE if the data was read, and FALSE in case the
 * queue is Empty.
 *
 */
unsigned char AppQueue_readData( AppQue_Queue *queue, void *data )
{
    unsigned char varRet = FALSE;

    if ( queue->Empty == FALSE)
    {
        queue->Full = FALSE;

        /*cppcheck-suppress misra-c2012-11.5 ; 
        The function receive a void pointer to the buffer
        if this is changed, the queue can no longer handle any type of data.*/
        const unsigned char *ptrBuffer = (unsigned char*) queue->Buffer;

        (void) memcpy( data, &ptrBuffer[queue->Tail * queue->Size], queue->Size );

        queue->Tail++;
        queue->Tail = queue->Tail % queue->Elements;

        if ( queue->Tail == queue->Head )
        {
            queue->Empty = TRUE;
        }

        varRet = TRUE;
    }
    
    return varRet;
}

/**
 * @brief   Indicate if the queue is Empty or not
 *
 * Just return value of the element (flag) Empty.
 *
 * @param   queue [in] It's the memory address of the queue to access the elements.
 *
 * @retval  Return the actual value of the flag Empty, TRUE when the queue is empty, and FALSE when
 * the queue has at least one element.
 *
 */
unsigned char AppQueue_isQueueEmpty( const AppQue_Queue *queue )
{
    return queue->Empty;
}

/**
 * @brief   This function empty the queue.
 *
 * Reset the elements Tail and Head to zero, and the flags Empty and Full to TRUE and FALSE,
 * respectively, this causes the information contained in the queue to be discarded.
 * 
 * 
 * @param   queue [in] It's the memory address of the queue to access the elements.
 *
 */
void AppQueue_flushQueue( AppQue_Queue *queue )
{
    queue->Head = 0;      //Setting index Tail and Head to zero
    queue->Tail = 0;
    queue->Empty = TRUE;  //Empty flag to TRUE and Full flg to FALSE
    queue->Full = FALSE;
}


/**
 * @brief   Copy the given data in the queue buffer.
 *
 * First of all, verify if the queue isn't Full to write data, and as a void pointers is received, it's 
 * necessary cast it to use pointer's arithmetic and to be able to copy the data in the buffer, later
 * increment Head, and to know if reach the Tail compare them, in an TRUE case set the Full flag to TRUE.
 * This function disable the all Interrupts at the beggining and enable them when the task has been 
 * completed.
 * 
 * @param   queue [in] It's the memory address of the queue to access the elements.
 * @param   data [in] Memory address where is the data to be written.
 *
 * @retval  Return the success of the write action, TRUE if the data was written, and FALSE in case the
 * queue is Full.
 *
 */
unsigned char HIL_QUEUE_writeDataISR( AppQue_Queue *queue, const void *data )
{   
    __disable_irq( );

    unsigned char varRet = AppQueue_writeData( queue, data );
    
    __enable_irq( );
    
    return varRet;
}

/**
 * @brief   Copy the data of the buffer in the memmory address given.
 *
 * First verify if the queue has at least one element, then make a copy of data in the queue, increment
 * Tail, reset it if it's necessary, and check if the queue now its empty to set the corresponding flag.  
 * This function disable the all Interrupts at the beggining and enable them when the task has been 
 * completed.
 *
 * @param   queue [in] It's the memory address of the queue to access the elements.
 * @param   data [out] Memory address where the read data will be copied.
 *
 * @retval  Return the success of the read action, TRUE if the data was read, and FALSE in case the
 * queue is Empty.
 *
 */
unsigned char HIL_QUEUE_readDataISR( AppQue_Queue *queue, void *data )
{
    __disable_irq( );

    unsigned char varRet = AppQueue_readData( queue, data );
    
    __enable_irq( );

    return varRet;
}

/**
 * @brief   Indicate if the queue is Empty or not
 *
 * Just return value of the element (flag) Empty.
 * This function disable the all Interrupts at the beggining and enable them when the task has been 
 * completed.
 *
 * @param   queue [in] It's the memory address of the queue to access the elements.
 *
 * @retval  Return the actual value of the flag Empty, TRUE when the queue is empty, and FALSE when
 * the queue has at least one element.
 *
 */
unsigned char HIL_QUEUE_isQueueEmptyISR( const AppQue_Queue *queue )
{
    __disable_irq( );
    __enable_irq( );
    return queue->Empty;
}

/**
 * @brief   This function empty the queue.
 *
 * Reset the elements Tail and Head to zero, and the flags Empty and Full to TRUE and FALSE,
 * respectively, this causes the information contained in the queue to be discarded.
 * This function disable the all Interrupts at the beggining and enable them when the task has been 
 * completed.
 * 
 * 
 * @param   queue [in] It's the memory address of the queue to access the elements.
 *
 */
void HIL_QUEUE_flushQueueISR( AppQue_Queue *queue )
{
    __disable_irq( );
    AppQueue_flushQueue( queue );
    __enable_irq( );
}