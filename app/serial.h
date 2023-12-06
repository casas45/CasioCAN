#ifndef SERIAL_H__
#define SERIAL_H__

#ifndef UTEST
#define STATIC static
#else
#define STATIC
#endif

#define FILTERS_N           3u
#define ID_TIME_MSG         0x111u
#define ID_DATE_MSG         0x127u
#define ID_ALARM_MSG        0x101u
#define FILTER_MASK         0x7FFu
#define MESSAGES_N          0x0Au   /*10 messages can arrive in 10ms*/
#define VALID_SECONDS_PARAM 0x00
#define RESPONSE_ID         0x122u
#define OK_RESPONSE         0x55u
#define ERROR_RESPONSE      0xAAu
#define N_BYTES_RESPONSE    0x01u
#define N_BYTES_CAN_MSG     0x08u
#define PARAMETER_1         0x00u
#define PARAMETER_2         0x01u
#define PARAMETER_3         0x02u
#define PARAMETER_4         0x03u
#define MONTHS              0x0Cu
#define MONTH_31_D          0x1Fu
#define MONTH_30_D          0X1Eu
#define FEB_29              0x1Du
#define FEB_28              0x1Cu
#define FEB                 0x01u
#define YEAR_MAX            0x833u  /*2099*/
#define YEAR_MIN            0x76Du  /*1901*/
#define MS_NIBBLE_MASK      0xF0u
#define LS_NIBBLE_MASK      0x0Fu


typedef enum _App_state
{
    IDLE = 0,
    MESSAGE,
    TIME,
    DATE,
    ALARM,
    OK,
    ERROR_
} APP_state;

void Serial_InitTask( void );

void Serial_PeriodicTask( void );

#endif