/**
 * @file    bsp.h
 * @brief   file where the libraries are included and the structures used are defined.
*/
#ifndef BSP_H_
#define BSP_H_

#include "stm32g0xx.h"
#include <stdint.h>
#include <string.h>
/* cppcheck-suppress misra-c2012-21.6 ; semihosting purpose */
#include <stdio.h>
#include "serial.h"
#include "scheduler.h"
#include "queue.h"
#include "clock.h"

/**
 * @brief   Variable with external linkage that is used to configure interrupt in ints.c file.
*/
extern FDCAN_HandleTypeDef CANHandler;

/** @brief  ClockQueue external reference */
extern AppQue_Queue ClockQueue;

/** @brief  Scheduler external reference */
extern AppSched_Scheduler Scheduler;


/**
 * @brief   List of messages types.
*/
typedef enum {
    SERIAL_MSG_TIME = 0,    /*!< Msg type time */
    SERIAL_MSG_DATE,        /*!< Msg type date */
    SERIAL_MSG_ALARM,       /*!< Msg type alarm */
    SERIAL_MSG_OK,          /*!< Msg type ok */
    SERIAL_MSG_ERROR,       /*!< Msg type error */
    SERIAL_N_EVENTS,        /*!< Number of events */
    SERIAL_MSG_DISPLAY,     /*!< Msg to update display */
    SERIAL_MSG_NONE         /*!< Msg type none */
} APP_Messages;

/**
 * @brief   struct to save time and date.
*/
typedef struct _APP_TmTypeDef
{
    uint8_t tm_sec;     /*!< seconds, range 0 to 59*/
    uint8_t tm_min;     /*!< minutes, range 0 to 59*/
    uint8_t tm_hour;    /*!< hours, range 0 to 23*/
    uint8_t tm_mday;    /*!< day of the month, range 1 to 31*/
    uint8_t tm_mon;     /*!< month, tange 0 to 11*/
    uint16_t tm_year;   /*!< years, range 1901 to 2099*/
    uint8_t tm_wday;    /*!< day of the week range 0 to 6*/
} APP_TmTypeDef;

/**
 * @brief   Struct to place the information once is processed and accepted.
*/
typedef struct _APP_MsgTypeDef
{
    uint8_t msg;        /*!< Store the message type to send*/
    APP_TmTypeDef tm;   /*!< time and date structure*/
} APP_MsgTypeDef;

/**
 * @brief   Struct to pass messages from CAN interrupt to SerialTask through queue.
*/
typedef struct _App_CanTypeDef
{
    uint16_t id;            /*!< CAN message ID*/
    uint8_t bytes[ 8u ];    /*!< CAN message*/
    uint8_t lenght;         /*!< CAN messsge lenght*/
} APP_CanTypeDef;


#endif
