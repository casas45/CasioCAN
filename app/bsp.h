#ifndef BSP_H_
#define BSP_H_

#include "stm32g0xx.h"
#include <stdint.h>
#include <string.h>
#include "scheduler.h"
#include "queue.h"

typedef enum
{
    SERIAL_MSG_NONE = 0,
    SERIAL_MSG_TIME,
    SERIAL_MSG_DATE,
    SERIAL_MSG_ALARM
} APP_Messages;

typedef struct _APP_TmTypeDef
{
    uint8_t tm_sec;     /*seconds, range 0 to 59*/
    uint8_t tm_min;     /*minutes, range 0 to 59*/
    uint8_t tm_hour;    /*hours, range 0 to 23*/
    uint8_t tm_mday;    /*day of the month, range 1 to 31*/
    uint8_t tm_mon;     /*month, tange 0 to 11*/
    uint16_t tm_year;   /*years, range 1900 to 2100*/
    uint8_t tm_wday;    /*day of the week range 0 to 6*/
} APP_TmTypeDef;

typedef struct _APP_MsgTypeDef
{
    uint8_t msg;        /*Store the message type to send*/
    APP_TmTypeDef tm;   /*time and date structure*/
} APP_MsgTypeDef;

typedef struct _App_CanTypeDef
{
    uint16_t id;        /*CAN message ID*/
    uint8_t bytes[ 8u ];   /*CAN message*/
    uint8_t lenght;     /*CAN messsge lenght*/
} APP_CanTypeDef;


#endif
