/**
 * @file queue.h
 * 
 * @brief Here is defined the AppQue_Queue struct, and the functions prototypes of
 * the queue.c file 
*/
#ifndef QUEUE_H_
#define QUEUE_H_

/**
 * @struct AppQue_Queue
 * 
 * Struct with the essential elements to work with a queue.
 * 
*/
typedef struct
{
    void *Buffer;                 /*!< pointer to array that store buffer data */ 
    unsigned long    Elements;    /*!< number of elements to store (the queue lenght)*/ 
    unsigned char     Size;       /*!< size of the elements to store*/ 
    unsigned char     Head;       /*!< variable to signal the next queue space to write*/  
    unsigned char     Tail;       /*!< variable to signal the next queue space to read*/
    unsigned char     Empty;      /*!< flag to indicate if the queue is empty*/
    unsigned char     Full;       /*!< flag to indicate if the queue is full*/
} AppQue_Queue;


void AppQueue_initQueue( AppQue_Queue *queue );

unsigned char AppQueue_writeData( AppQue_Queue *queue, const void *data );

unsigned char AppQueue_readData( AppQue_Queue *queue, void *data );

unsigned char AppQueue_isQueueEmpty( const AppQue_Queue *queue );

void AppQueue_flushQueue( AppQue_Queue *queue );

unsigned char HIL_QUEUE_writeDataISR( AppQue_Queue *queue, const void *data );

unsigned char HIL_QUEUE_readDataISR( AppQue_Queue *queue, void *data );

unsigned char HIL_QUEUE_isQueueEmptyISR( const AppQue_Queue *queue );

void HIL_QUEUE_flushQueueISR( AppQue_Queue *queue );

#endif