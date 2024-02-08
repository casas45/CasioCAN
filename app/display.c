/**
 * @file    display.c
 * 
 * @brief   file where are the functions that controls the LCD.
*/
#include "display.h"
#include "bsp.h"

#define BCD_TO_BIN( x ) ( ( ( (x) >> 4u ) * 10u ) + ( (x) & 0x0Fu ) ) /*!< Macro to conver BCD data to an integer */

#define GET_UNITS( x ) ( (x) % 10u )                        /*!< Operation to get the units of x */
#define GET_TENS( x ) ( ( (x) % 100u ) / 10u )              /*!< Operation to get the tens of x */
#define GET_HUNDREDS(x) ( ( (x) % 1000u ) / 100u )          /*!< Operation to get the hundreds of x */
#define GET_THOUSANDS(x) ( ( (x) % 10000u ) / 1000u )       /*!< Operation to get the thousands of x */

/**
 * @brief Queue to communicate clock and display tasks.
*/
AppQue_Queue DisplayQueue;

/**
 * @brief LCD Handler.
*/
LCD_HandleTypeDef LCD_Handler;

/** @brief SPI Handler */
SPI_HandleTypeDef SPI_Handler;

STATIC APP_MsgTypeDef UpdateDisplay ( APP_MsgTypeDef *pDisplayMsg );

STATIC APP_MsgTypeDef DisplayAlarmSet( APP_MsgTypeDef *pDisplayMsg );

STATIC APP_MsgTypeDef DisplayAlarmActive( APP_MsgTypeDef *pDisplayMsg );

STATIC void TimeString( char *string, uint8_t hours, uint8_t minutes, uint8_t seconds );

STATIC void DateString( char *string, uint8_t month, uint8_t day, uint16_t year, uint8_t weekday );

/**
 * @brief   Initialize all required to work with the LCD.
 * 
 * Initialize the DisplayQueue, and write a message of type CLOCK_MSG_DISPLAY in the ClockQueue
 * to get the time and date, updating the display after its initialization. Additionally, configure
 * the SPI module to initialize the LCD.
*/
void Display_InitTask( void )
{
    HAL_StatusTypeDef Status = HAL_ERROR;

    /*Display queue configuration*/
    static APP_MsgTypeDef DisplayMsgs[ N_DISPLAY_MSGS ];

    DisplayQueue.Buffer     = DisplayMsgs;
    DisplayQueue.Elements   = N_DISPLAY_MSGS;
    DisplayQueue.Size       = sizeof( APP_MsgTypeDef );

    AppQueue_initQueue( &DisplayQueue );

    /* Write a msg to update the display after the initialization  */
    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;

    Status = HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );
    assert_error( Status == TRUE, QUEUE_RET_ERROR );

    /*SPI configuration*/

    SPI_Handler.Instance                = SPI1;
    SPI_Handler.Init.Mode               = SPI_MODE_MASTER;
    SPI_Handler.Init.Direction          = SPI_DIRECTION_1LINE;
    SPI_Handler.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_64;
    SPI_Handler.Init.DataSize           = SPI_DATASIZE_8BIT;
    SPI_Handler.Init.CLKPolarity        = SPI_POLARITY_HIGH;
    SPI_Handler.Init.CLKPhase           = SPI_PHASE_2EDGE;
    SPI_Handler.Init.FirstBit           = SPI_FIRSTBIT_MSB;
    SPI_Handler.Init.NSS                = SPI_NSS_SOFT;
    SPI_Handler.Init.TIMode             = SPI_TIMODE_DISABLED;
    SPI_Handler.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLED;

    Status = HAL_SPI_Init( &SPI_Handler );

    assert_error( Status == HAL_OK, SPI_RET_ERROR );

    /*LCD Handler configuration*/
    LCD_Handler.spiHandler = &SPI_Handler;

    LCD_Handler.BklPort   = GPIOB;
    LCD_Handler.BklPin    = GPIO_PIN_4;

    LCD_Handler.RsPort    = GPIOD;
    LCD_Handler.RsPin     = GPIO_PIN_4;

    LCD_Handler.RstPort   = GPIOD;
    LCD_Handler.RstPin    = GPIO_PIN_2;
    
    LCD_Handler.CsPort    = GPIOD;
    LCD_Handler.CsPin     = GPIO_PIN_3;

    Status = HEL_LCD_Init( &LCD_Handler );
    assert_error( Status == HAL_OK, LCD_RET_ERROR );

    HEL_LCD_Backlight( &LCD_Handler, LCD_ON );

}

/**
 * @brief   Function where the display event machine it's implemented.
*/
void Display_PeriodicTask( void )
{
    APP_MsgTypeDef (*DisplayEventMachine[ N_DISPLAY_EVENTS ])( APP_MsgTypeDef *DisplayMsg ) =
    {
        UpdateDisplay,
        DisplayAlarmSet,
        DisplayAlarmActive
    };

    APP_MsgTypeDef readMsg = {0};

    while ( HIL_QUEUE_isQueueEmptyISR( &DisplayQueue ) == FALSE )
    {
        uint8_t Status = FALSE;
        
        Status = HIL_QUEUE_readDataISR( &DisplayQueue, &readMsg );
        assert_error( Status == TRUE, QUEUE_RET_ERROR );
        
        if ( readMsg.msg < (uint8_t) N_DISPLAY_EVENTS )
        {
            (void) DisplayEventMachine[ readMsg.msg ]( &readMsg );
        }
        
    }
}

/**
 * @brief   Sends the strings with the time and date.
 * 
 * This function updates the display getting the time and date information from the the read message
 * and utilizes the DateString and TimeString functions to set that information in the corresponding 
 * arrays.
 * 
 * @param   pDisplayMsg Pointer to the read message.
 * 
 * @return  the next event, if there is one, otherwise DISPLAY_MSG_NONE.
 * 
 * @note The year that provides the RTC function only has the last two digits. It is necessary to add
 * the value TWO_THOUSANDS to obtain a four-digit year. Please note that this assumes we are in the
 * years 2000.
*/
STATIC APP_MsgTypeDef UpdateDisplay ( APP_MsgTypeDef *pDisplayMsg )
{
    APP_MsgTypeDef nextEvent;
    nextEvent.msg = DISPLAY_MSG_NONE;

    HAL_StatusTypeDef Status = HAL_ERROR;

    char lcd_row_0_date[ LCD_CHARACTERS ];
    char lcd_row_1_time[ LCD_CHARACTERS ];

    DateString( lcd_row_0_date, BCD_TO_BIN( pDisplayMsg->tm.tm_mon ), 
    pDisplayMsg->tm.tm_mday, ( pDisplayMsg->tm.tm_year + TWO_THOUSANDS ), pDisplayMsg->tm.tm_wday );
    
    TimeString( lcd_row_1_time, pDisplayMsg->tm.tm_hour, pDisplayMsg->tm.tm_min, pDisplayMsg->tm.tm_sec );

    Status = HEL_LCD_SetCursor( &LCD_Handler, 0u, 1u );
    assert_error( Status == HAL_OK, SPI_RET_ERROR );

    Status = HEL_LCD_String( &LCD_Handler, lcd_row_0_date );
    assert_error( Status == HAL_OK, SPI_RET_ERROR );  

    Status = HEL_LCD_SetCursor( &LCD_Handler, 1u, 4u );
    assert_error( Status == HAL_OK, SPI_RET_ERROR );

    Status = HEL_LCD_String( &LCD_Handler, lcd_row_1_time );
    assert_error( Status == HAL_OK, SPI_RET_ERROR );

    return nextEvent;
}

/**
 * @brief   Display the letter A in the left-down corner.
 * 
 * Print the letter A in the display to indicate that the alarm is set.
 * 
 * @param   pDisplayMsg Pointer to read message.
 * 
 * @return  the next event, if there is one, otherwise DISPLAY_MSG_NONE.
*/
STATIC APP_MsgTypeDef DisplayAlarmSet( APP_MsgTypeDef *pDisplayMsg )
{
    (void) pDisplayMsg;

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = DISPLAY_MSG_NONE;

    HAL_StatusTypeDef Status = HAL_ERROR;

    Status = HEL_LCD_SetCursor( &LCD_Handler, 1u, 0u );   /*set cursor in the left-down corner */
    assert_error( Status == HAL_OK, LCD_RET_ERROR );

    Status = HEL_LCD_Data( &LCD_Handler, 'A' );
    assert_error( Status == HAL_OK, LCD_RET_ERROR );

    return nextEvent;
}

/**
 * @brief   Display the message "ALARM!!!" in the second row.
 * 
 * When the alarm is activated, the display shows the message "ALARM!!!" in the second line, the 
 * message is declared as a string with four blank spaces at the beginning to clear the letter
 * A, which is used to indicate that the alarm is set.
 * 
 * @param   pDisplayMsg Pointer to the read message.
 * 
 * @return  the next event, if there is one, otherwise DISPLAY_MSG_NONE.
*/
STATIC APP_MsgTypeDef DisplayAlarmActive( APP_MsgTypeDef *pDisplayMsg )
{
    (void) pDisplayMsg;

    APP_MsgTypeDef nextEvent;
    nextEvent.msg = DISPLAY_MSG_NONE;

    HAL_StatusTypeDef Status = HAL_ERROR;

    const char *AlarmMessage = "    ALARM!!!";

    Status = HEL_LCD_SetCursor( &LCD_Handler, 1u, 0u );
    assert_error( Status == HAL_OK, LCD_RET_ERROR );

    Status = HEL_LCD_String( &LCD_Handler, AlarmMessage );
    assert_error( Status == HAL_OK, LCD_RET_ERROR );

    return nextEvent;
} 

/**
 * @brief   Set the time parameters into a string with a specific format.
 * 
 * This function takes the hours, minutes, and seconds as input and formats
 * them into a string in the "Hr:Mi:Se" format.
 * 
 * @param[out] string  Pointer to the character array where the formatted time string will be stored.
 * @param[in] hours The hours component of the time.
 * @param[in] minutes The minutes component of the time.
 * @param[in] seconds The seconds component of the time.
 * 
 * @note The output string must have sufficient space (at least 9 characters) to accommodate the 
 * formatted time string.
*/
STATIC void TimeString( char *string, uint8_t hours, uint8_t minutes, uint8_t seconds )
{
    /*Format "Hr:Mi:Se" */
    string [0] = GET_TENS( hours ) + UPSET_ASCII_NUM;
    string [1] = GET_UNITS( hours ) + UPSET_ASCII_NUM;
    string [2] = ':';

    string [3] = GET_TENS( minutes ) + UPSET_ASCII_NUM;
    string [4] = GET_UNITS( minutes ) + UPSET_ASCII_NUM;
    string [5] = ':';

    string [6] = GET_TENS( seconds ) + UPSET_ASCII_NUM;
    string [7] = GET_UNITS( seconds ) + UPSET_ASCII_NUM;

    /* Add null character */
    string [8] = '\0';
}

/**
 * @brief   Set the date parameters into a string with a specific format.
 * 
 * This function formats the date information (month, day, year, and weekday) into a string
 * with the format "Mon, dd yyyy Dw".
 * 
 * @param[out] string Pointer to the character array where the formatted date string will be stored.
 * @param[in] month Month (1-12).
 * @param[in] day Month day.
 * @param[in] year The year of the date.
 * @param[in] weekday The weekday (1-7 = Monday to Sunday).
 * 
 * @note The output string must have sufficient space (at least 15 characters) to accommodate the 
 * formatted date string.
*/
STATIC void DateString( char *string, uint8_t month, uint8_t day, uint16_t year, uint8_t weekday )
{
    //Format : “Mon,dd yyyy Dw“
    const char months[ N_MONTHS ] [ MONTH_N_CHARACTERS ] = 
    { "ENE,", "FEB,", "MAR,", "ABR,", "MAY,", "JUN,", "JUL,", "AGO,", "SEP,", "OCT,", "NOV,", "DIC," };

    const char wdays[ N_WDAYS ][ WDAY_N_CHARACTERS ] = { "Lu", "Ma", "Mi", "Ju", "Vi", "Sa", "Do" };

    /* Add "Mon," if the parameter month it's 0 the offset isn't used */
    if ( month > 0u )
    {
        (void) strncpy( string, months[ month - OFFSET_ARRAY ], MONTH_N_CHARACTERS );
    }else{
        (void) strncpy( string, months[ month ], MONTH_N_CHARACTERS );
    }

    /* Add "dd " */
    string[4u] = GET_TENS( day ) + UPSET_ASCII_NUM;
    string[5u] = GET_UNITS( day ) + UPSET_ASCII_NUM;
    string[6u] = ' ';

    /* Add "yyyy " */
    string[7u]  = GET_THOUSANDS( year ) + UPSET_ASCII_NUM;
    string[8u]  = GET_HUNDREDS( year ) + UPSET_ASCII_NUM;
    string[9u]  = GET_TENS( year ) + UPSET_ASCII_NUM;
    string[10u] = GET_UNITS( year ) + UPSET_ASCII_NUM;
    string[11u] = ' ';

    /* Add "Dw" if the parameter weekday it's 0 the offset isn't used  */
    if ( weekday > 0u )
    {
        (void) strncpy( &string[12], wdays[ weekday - OFFSET_ARRAY ], WDAY_N_CHARACTERS );
    }else{
        (void) strncpy( &string[12], wdays[ weekday ], WDAY_N_CHARACTERS );
    }

    /* Add null character */
    string[14] = '\0';
}