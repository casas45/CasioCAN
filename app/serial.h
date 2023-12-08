/**
 * @file serial.h
 * 
 * @brief file where are the functions protypes and MACROS of serial.c.
*/
#ifndef SERIAL_H__
#define SERIAL_H__

#ifndef UTEST
#define STATIC static
#else
#define STATIC
#endif

#define FILTERS_N           0x03u       /*!< 3 filters are configured in FDCAN module*/
#define ID_TIME_MSG         0x111u      /*!< TIME ID*/
#define ID_DATE_MSG         0x127u      /*!< DATE ID*/
#define ID_ALARM_MSG        0x101u      /*!< ALARM ID*/
#define FILTER_MASK         0x7FFu      /*!< Mask to indicate how many bit take in acount to filter*/
#define MESSAGES_N          0x14u       /*!< Number of messages that can be received in 10 ms*/
#define VALID_SECONDS_PARAM 0x00u       /*!< A valid value for seconds*/
#define RESPONSE_ID         0x122u      /*!< RESPONSE ID*/
#define OK_RESPONSE         0x55u       /*!< Parameter 1 of OK response*/
#define ERROR_RESPONSE      0xAAu       /*!< Parameter 1 of ERROR response*/
#define N_BYTES_RESPONSE    0x01u       /*!< Number of payload bytes in a response*/
#define N_BYTES_CAN_MSG     0x08u       /*!< Number of data bytes in a standard CAN message*/
#define PARAMETER_1         0x00u       /*!< Position in data of parameter 1*/
#define PARAMETER_2         0x01u       /*!< Position in data of parameter 2*/
#define PARAMETER_3         0x02u       /*!< Position in data of parameter 3*/
#define PARAMETER_4         0x03u       /*!< Position in data of parameter 4*/
#define MSG                 0x04u       /*!< Position of msg type*/
#define MONTHS              0x0Cu       /*!< Number of months in a year*/
#define MONTH_31_D          0x1Fu       /*!< Number of days (31)*/
#define MONTH_30_D          0X1Eu       /*!< Number of days (30)*/
#define FEB_29              0x1Du       /*!< Number of february days in a leap year*/
#define FEB_28              0x1Cu       /*!< Number of february days in a no leap year*/
#define FEB                 0x01u       /*!< Position of february in an array*/
#define YEAR_MAX            0x833u      /*!< Max value of a year allowed by the app*/
#define YEAR_MIN            0x76Du      /*!< Min value of a year allowed by the app*/
#define MS_NIBBLE_MASK      0xF0u       /*!< Mask to obtain most significant nibble of a byte*/
#define LS_NIBBLE_MASK      0x0Fu       /*!< Mask to obtain low significant nibble of a byte*/

/**
 * @enum APP_state
 * 
 * @brief enum to list the states of the machine
 * 
*/
typedef enum _App_state
{
    IDLE = 0,       /*!< IDLE state (0) */ 
    MESSAGE,        /*!< MESSAGE state (1) */ 
    TIME,           /*!< TIME state (2) */ 
    DATE,           /*!< DATE state (3) */ 
    ALARM,          /*!< ALARM state (4) */ 
    OK,             /*!< OK state (5) */ 
    ERROR_          /*!< ERROR_ state (6) */ 
} APP_state;

void Serial_InitTask( void );

void Serial_PeriodicTask( void );

#endif