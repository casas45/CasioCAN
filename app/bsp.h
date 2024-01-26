/**
 * @file    bsp.h
 * @brief   file where the libraries are included and the structures used are defined.
*/
#ifndef BSP_H_
#define BSP_H_

#include "stm32g0xx.h"
#include <stdint.h>
#include <string.h>
#include "queue.h"
#include "scheduler.h"

#define assert_error( expr, error)      ( (expr) ? (void)0U : safe_state(( uint8_t *)__FILE__, __LINE__, (error)))

extern void safe_state( uint8_t *file, uint32_t line, uint8_t error );

/**
 * @brief   Variable with external linkage that is used to configure interrupt in ints.c file.
*/
extern FDCAN_HandleTypeDef CANHandler;

/** @brief  ClockQueue external reference */
extern AppQue_Queue ClockQueue;

/** @brief  DisplayQueue external reference */
extern AppQue_Queue DisplayQueue;

/** @brief  Scheduler external reference */
extern AppSched_Scheduler Scheduler;

/** @brief  LCD Handler external reference */
extern LCD_HandleTypeDef LCD_Handler;

/** @brief  RTC Handler external reference */
extern RTC_HandleTypeDef hrtc;

/** @brief  WWDG Handler external reference */
extern WWDG_HandleTypeDef h_watchdog;

extern uint8_t UpdateTimerID;

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

/**
 * @enum    ClkMessages
 * 
 * @brief   Enum to clasify the clock messages.
*/
/* cppcheck-suppress misra-c2012-2.4 ; this enum is only used to clasify the clock messages */
typedef enum
{
    CLOCK_MSG_TIME = 0,     /*!< Msg to update RTC time */
    CLOCK_MSG_DATE,         /*!< Msg to update RTC date */
    CLOCK_MSG_ALARM,        /*!< Msg to update RTC alarm */
    CLOCK_MSG_DISPLAY,      /*!< Msg to update display */
    N_CLK_STATES            /*!< Number of events in clock event machine*/
} ClkMessages;

/**
 * @enum    DisplayMessages
 * 
 * @brief   Enum to clasify the Display messages.
*/
/* cppcheck-suppress misra-c2012-2.4 ; this enum is only used to clasify the Display messages */
typedef enum
{
    DISPLAY_MSG_UPDATE = 0,         /*!< Msg to update display */
    N_DISPLAY_EVENTS                /*!< Number of events in Display event machine*/
} DisplayMessages;


typedef enum _App_ErrorsCode
{
    WWDG_RET_ERROR = 1u,
    RCC_RET_ERROR,
    PWR_RET_ERROR,
    FDCAN_RET_ERROR,
    RTC_RET_ERROR,
    SPI_RET_ERROR,  
    SCHE_RET_ERROR,
    QUEUE_PAR_ERROR,
    SCHE_PAR_ERROR,
    WWDG_RESET_ERROR,
    HARD_FAULT_ERROR,
    



} App_ErrorsCode;

#endif
