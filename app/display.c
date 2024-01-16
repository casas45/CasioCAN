/**
 * @file    display.c
 * 
 * @brief   file where are the functions that controls the LCD.
*/
#include "display.h"
#include "bsp.h"

#define BCD_TO_BIN( x ) ( ( ( (x) >> 4u ) * 10u ) + ( (x) & 0x0Fu ) ) /*!< Macro to conver BCD data to an integer */

/**
 * @brief Queue to communicate clock and display tasks.
*/
AppQue_Queue DisplayQueue;

/**
 * @brief LCD Handler.
*/
static LCD_HandleTypeDef LCD_Handler;

static void UpdateDisplay ( APP_MsgTypeDef *pDisplayMsg );
static void TimeString( char *string, uint8_t hours, uint8_t minutes, uint8_t seconds );
static void DateString( char *string, uint8_t month, uint8_t day, uint16_t year, uint8_t weekday );

/**
 * @brief   Initialize all required to work with the LCD.
 * 
 * Initialize the DisplayQueue, and write a message of type CLOCK_MSG_DISPLAY in the ClockQueue
 * to get the time and date, updating the display after its initialization. Additionally, configure
 * the SPI module to initialize the LCD.
*/
void Display_InitTask( void )
{
    /*Display queue configuration*/
    static APP_MsgTypeDef DisplayMsgs[ N_DISPLAY_MSGS ];

    DisplayQueue.Buffer     = DisplayMsgs;
    DisplayQueue.Elements   = N_DISPLAY_MSGS;
    DisplayQueue.Size       = sizeof( APP_MsgTypeDef );

    AppQueue_initQueue( &DisplayQueue );

    /* Write a msg to update the display after the initialization  */
    APP_MsgTypeDef nextEvent;
    nextEvent.msg = CLOCK_MSG_DISPLAY;
    (void) HIL_QUEUE_writeDataISR( &ClockQueue, &nextEvent );

    /*SPI configuration*/
    static SPI_HandleTypeDef SPI_Handler;

    SPI_Handler.Instance                = SPI1;
    SPI_Handler.Init.Mode               = SPI_MODE_MASTER;
    SPI_Handler.Init.Direction          = SPI_DIRECTION_1LINE;
    SPI_Handler.Init.BaudRatePrescaler  = SPI_BAUDRATEPRESCALER_8;
    SPI_Handler.Init.DataSize           = SPI_DATASIZE_8BIT;
    SPI_Handler.Init.CLKPolarity        = SPI_POLARITY_HIGH;
    SPI_Handler.Init.CLKPhase           = SPI_PHASE_2EDGE;
    SPI_Handler.Init.FirstBit           = SPI_FIRSTBIT_MSB;
    SPI_Handler.Init.NSS                = SPI_NSS_SOFT;
    SPI_Handler.Init.TIMode             = SPI_TIMODE_DISABLED;
    SPI_Handler.Init.CRCCalculation     = SPI_CRCCALCULATION_DISABLED;

    /*LCD Handler configuration*/
    LCD_Handler.spiHandler = &SPI_Handler;

    (void) HEL_LCD_Init( &LCD_Handler );

    HEL_LCD_Backlight( &LCD_Handler, LCD_ON );
}

/**
 * @brief   Function where the display event machine it's implemented.
*/
void Display_PeriodicTask( void )
{
    void (*DisplayEventMachine[ N_DISPLAY_EVENTS ])( APP_MsgTypeDef *DisplayMsg ) =
    {
        UpdateDisplay
    };

    APP_MsgTypeDef readMsg;

    while ( AppQueue_isQueueEmpty( &DisplayQueue ) == FALSE )
    {
        (void) AppQueue_readData( &DisplayQueue, &readMsg );
        
        if ( readMsg.msg < (uint8_t) N_DISPLAY_EVENTS )
        {
            DisplayEventMachine[ readMsg.msg ]( &readMsg );
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
*/
static void UpdateDisplay ( APP_MsgTypeDef *pDisplayMsg )
{
    char lcd_row_0_date[ LCD_CHARACTERS ];
    char lcd_row_1_time[ LCD_CHARACTERS ];

    DateString( lcd_row_0_date, pDisplayMsg->tm.tm_mon, pDisplayMsg->tm.tm_mday, pDisplayMsg->tm.tm_year, pDisplayMsg->tm.tm_wday );
    TimeString( lcd_row_1_time, pDisplayMsg->tm.tm_hour, pDisplayMsg->tm.tm_min, pDisplayMsg->tm.tm_sec );

    (void) HEL_LCD_SetCursor( &LCD_Handler, 0u, 1u );
    (void) HEL_LCD_String( &LCD_Handler, lcd_row_0_date );

    (void) HEL_LCD_SetCursor( &LCD_Handler, 1u, 3u );
    (void) HEL_LCD_String( &LCD_Handler, lcd_row_1_time );
}

/**
 * @brief   Set the time parameters into a string with a specific format.
 * 
 * This function takes the hours, minutes, and seconds as input and formats
 * them into a string in the "Hr:Mi:Se" format.
 * 
 * @param[out]   string  Pointer to the character array where the formatted time string will be stored.
 * @param[in] hours The hours component of the time.
 * @param[in] minutes The minutes component of the time.
 * @param[in] seconds The seconds component of the time.
 * 
 * @note The output string must have sufficient space (at least 9 characters) to accommodate the 
 * formatted time string.
*/
static void TimeString( char *string, uint8_t hours, uint8_t minutes, uint8_t seconds )
{
    /*Format "Hr:Mi:Se" */
    string [0] = ( hours / 10u ) + UPSET_ASCII_NUM;
    string [1] = ( hours % 10u )+ UPSET_ASCII_NUM;
    string [2] = ':';

    string [3] =  ( minutes / 10u ) + UPSET_ASCII_NUM;
    string [4] = ( minutes % 10u ) + UPSET_ASCII_NUM;
    string [5] = ':';

    string [6] = ( seconds / 10u ) + UPSET_ASCII_NUM;
    string [7] = ( seconds % 10u ) + UPSET_ASCII_NUM;

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
static void DateString( char *string, uint8_t month, uint8_t day, uint16_t year, uint8_t weekday )
{
    //Format : “Mon,dd yyyy Dw“
    const char months[ N_MONTHS ] [ MONTH_N_CHARACTERS ] = 
    { "ENE,", "FEB,", "MAR,", "ABR,", "MAY,", "JUN,", "JUL,", "AGO,", "SEP,", "OCT,", "NOV,", "DIC," };

    const char wdays[ N_WDAYS ][ WDAY_N_CHARACTERS ] = { "Lu", "Ma", "Mi", "Ju", "Vi", "Sa", "Do" };

    /* Add "Mon," */
    (void) strncpy( string, months[ BCD_TO_BIN( month ) - OFFSET_ARRAY ], MONTH_N_CHARACTERS );

    /* Add "dd " */
    string[4u] = ( day / 10u ) + UPSET_ASCII_NUM;
    string[5u] = ( day % 10u ) + UPSET_ASCII_NUM;
    string[6u] = ' ';

    /* Add "yyyy" */
    string[7u]  = ( year / 1000u ) + UPSET_ASCII_NUM;
    string[8u]  = ( ( year % 1000u ) / 100u ) + UPSET_ASCII_NUM;
    string[9u]  = ( ( (year % 1000u) % 100u ) / 10u ) + UPSET_ASCII_NUM;
    string[10u] = ( ( (year % 1000u) % 100u ) % 10u ) + UPSET_ASCII_NUM;
    string[11u] = ' ';

    /* Add "Dw" */
    (void) strncpy( &string[12], wdays[ weekday - OFFSET_ARRAY ], WDAY_N_CHARACTERS );

    /* Add null character */
    string[14] = '\0';
}