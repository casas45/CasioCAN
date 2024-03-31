/**
 * @file    hel_lcd.c
 * 
 * @brief   Driver to control the LCD with ST7032 controller using SPI.
*/
#include "hel_lcd.h"

#define FIRST_PART_CMDS     7u      /*!< number of commands of the first part */
#define SECOND_PART_CMDS    10u     /*!< number of commands of the second part */

/**
 * @brief   Initialization routine of the LCD.
 * 
 * Here the SPI module is initialized, and the commands to initialize the LCD are sent through
 * SPI, setting an optimum contrast level and the maximum internal frequency.
 * 
 * @param   hlcd Pointer to the LCD handle structure.
 * 
 * @retval  HAL_OK if the SPI initialization was successful, otherwise HAL_ERROR.
*/
uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd )
{
    uint8_t retValue = HAL_OK;
    uint8_t commands [ 10 ];
    uint8_t i = 0u;

    /* Initialization commands */
    commands[ 0 ] = CMD_WAKEUP;
    commands[ 1 ] = CMD_WAKEUP;
    commands[ 2 ] = FUNCTION_SET | ( 1u << DL_POS ) | ( 1u << N_POS ) | ( 0u << DH_POS ) | ( 1u << IS_POS );
    commands[ 3 ] = OSC_FREQUENCY | ( 0u << BS_POS ) | ( 1u << F2_POS ) | ( 1u << F1_POS ) | ( 1u << F0_POS );
    commands[ 4 ] = PWR_ICON_CONTRAST | ( 0u << ION_POS ) | ( 1u << BON_POS ) | ( 1u << C5_POS ) | ( 0u << C4_POS );
    commands[ 5 ] = FOLLOWER_CONTROL | ( 1u << FON_POS ) | ( 1u << RAB2_POS ) | ( 0u << RAB1_POS ) | ( 1u << RAB0_POS );
    commands[ 6 ] = CONTRAST_SET | ( 0u << C3_POS ) | ( 0u << C2_POS ) | ( 0u << C1_POS ) | ( 0u << C0_POS );
    commands[ 7 ] = DISPLAY_ON_OFF | ( 1u << D_POS ) | ( 0u << C_POS ) | ( 0u << B_POS );
    commands[ 8 ] = ENTRY_MODE | ( 1u << I_D_POS ) | ( 0u << S_POS );
    commands[ 9 ] = CMD_CLEAR_DISPLAY;

    HEL_LCD_MspInit( hlcd );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );        /*CS off*/
    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );      /*RS instruction*/

    HAL_GPIO_WritePin( hlcd->RstPort, hlcd->RstPin, RESET );    /*Reset*/
    
    HAL_Delay( 2u );

    HAL_GPIO_WritePin( hlcd->RstPort, hlcd->RstPin, SET );      /*clear Reset*/
    
    HAL_Delay( 20u );
    
    retValue = HEL_LCD_Command( hlcd, CMD_WAKEUP );
    
    HAL_Delay( 2u );

    while ( ( retValue == HAL_OK ) && ( i < FIRST_PART_CMDS ) )    /* send first 7 initialization commands */
    {
        retValue = HEL_LCD_Command( hlcd, commands[ i ] );
        i++;
    }

    HAL_Delay( 200u );

    while( ( retValue == HAL_OK ) && ( i < SECOND_PART_CMDS ) )    /* send the last 3 initialization commands */
    {
        retValue = HEL_LCD_Command( hlcd, commands[ i ] );
        i++;
    }

    HAL_Delay( 2u );

    if( retValue == HAL_OK )     /* If the initialization routine has been executed correctly.. */
    {
        retValue = HEL_LCD_Command( hlcd, SET_DDRAM_ADDRESS );  /*Set DDRAM address 0x00*/
    }

    return retValue;
}

/* cppcheck-suppress misra-c2012-8.6 ; here it's defined as weak */
/**
 * @brief   Function to add unique code inside the application.
 * 
 * This function is defined as weak to allow that it can be redefined within the application.
*/
__attribute__((weak)) void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd )
{
    (void) hlcd;
}

/**
 * @brief   Function to send an instruction command.
 * 
 * This function sends a command to the LCD.
 * 
 * @param   hlcd Pointer to the LCD handle structure.
 * @param   cmd Command to send.
 * 
 * @return HAL_OK if the operation was successful, otherwise HAL_ERROR.
*/
uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd )
{
    uint8_t retValue = HAL_ERROR;

    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, RESET );  /*Command mode*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS on*/

    retValue = HAL_SPI_Transmit( hlcd->spiHandler, &cmd, 1, 100 );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );    /*CS off*/

    return retValue;
}

/**
 * @brief   Function to send a character.
 * 
 * This function sends a character to the LCD through SPI.
 * 
 * @param   hlcd Pointer to the LCD handle structure.
 * @param   data character to send.
 * 
 * @return HAL_OK if the operation was successful, otherwise HAL_ERROR.
*/
uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data )
{
    uint8_t retValue = HAL_ERROR;

    HAL_GPIO_WritePin( hlcd->RsPort, hlcd->RsPin, SET );    /*data mode*/
    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, RESET );  /*CS on*/

    retValue = HAL_SPI_Transmit( hlcd->spiHandler, &data, 1, 100 );

    HAL_GPIO_WritePin( hlcd->CsPort, hlcd->CsPin, SET );    /*CS off*/

    return retValue;
}

/**
 * @brief   Function to send a string.
 * 
 * This function sends a string to the LCD using SPI, sendinng character by character, if the string
 * has over 16 elements just send 16 and the others are ignored.
 * 
 * @param   hlcd Pointer to the LCD handle structure.
 * @param   str Pointer to the string to send.
 * 
 * @return HAL_OK if the operation was successful, otherwise HAL_ERROR.
*/
uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, const char *str )
{
    uint8_t retValue = HAL_OK;

    uint8_t i = 0u;
    
    while ( ( i <= MAX_COL ) && ( str[ i ] != '\0' ) )
    {
        retValue |= HEL_LCD_Data( hlcd, str[ i ] );

        i++;
    }
    
    return retValue;
}

/**
 * @brief Sets the cursor position on the LCD display.
 *
 * This function allows setting the cursor position to a specified row and column on the LCD display
 * to start writing.
 *
 * @param hlcd Pointer to the LCD handle structure.
 * @param row The row number (0 to 1) where the cursor will be positioned.
 * @param col The column number (0 to 15) where the cursor will be positioned. 
 *
 * @return HAL_OK if the operation was successful, otherwise HAL_ERROR.
 * 
 * @note If the row is greater than 1, it will be set to 1, and if the column is greater than 15, 
 * it will be set to 0.
*/
uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col )
{
    uint8_t retValue = HAL_ERROR;
    uint8_t command = col;      /*add the column number to the command to write*/

    if ( col > MAX_COL )        /*set col to 0 if it's greater than 15*/
    {
        command = COL_0;
    }
    
    if( row > ROW_0 )          /*upset to indicate that is the second row*/
    {
        command += SET_CURSOR_ROW_1;
    }

    command |= SET_DDRAM_ADDRESS;   /*add the specific bit to the command*/
    
    retValue = HEL_LCD_Command( hlcd, command );

    return retValue;
}

/**
 * @brief Controls the backlight of an LCD.
 *
 * This function allows controlling the backlight state of the LCD based on the provided state
 * parameter.
 *
 * @param hlcd Pointer to the LCD handle structure.
 * @param state The desired backlight state: LCD_ON to turn on, LCD_OFF to turn off, LCD_TOGGLE to
 * toggle the state.
 * 
 * @return HAL_OK if the operation was successful, otherwise HAL_ERROR.
 */
uint8_t HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state )
{
    uint8_t retVal = HAL_ERROR;
    static uint8_t current_state = LCD_ON;

    switch ( state )
    {
        case LCD_ON:

            retVal = HAL_TIM_PWM_Start( hlcd->TimHandler, TIM_CHANNEL_1 );
            current_state = LCD_ON;

            break;

        case LCD_OFF:

            retVal = HAL_TIM_PWM_Stop( hlcd->TimHandler, TIM_CHANNEL_1 );
            current_state = LCD_OFF;

            break;

        case LCD_TOGGLE:

            if ( current_state == LCD_ON )
            {
                retVal = HAL_TIM_PWM_Stop( hlcd->TimHandler, TIM_CHANNEL_1 );
                current_state = LCD_OFF;
            }else{
                retVal = HAL_TIM_PWM_Start( hlcd->TimHandler, TIM_CHANNEL_1 );
                current_state = LCD_ON;
            }

            break;

        default:
            break;
    }

    return retVal;
}

/**
 * @brief   Adjust LCD contrast level.
 * 
 * This function adjust the LCD contrast using the function HEL_LCD_Command to send the value provided
 * that will be set, there are 16 different levels from 0 to 15.
 * 
 * @param hlcd Pointer to the LCD handle structure.
 * @param contrast The contrast level to be set. Should be in the range LCD_CONTRAST_LVL_1 to 
 * LCD_CONTRAST_LVL_16.
 *  
 * @return HAL_OK if the operation was successful, otherwise HAL_ERROR.
*/
uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast )
{
    uint8_t retValue = HAL_ERROR;
    
    if ( contrast <= LCD_CONTRAST_16 )
    {
        uint8_t contrast_cmd = contrast;

        contrast_cmd |= CONTRAST_SET;           /*Add the command bits to set the contrast*/
        retValue = HEL_LCD_Command( hlcd, contrast_cmd );
    }
    
    return retValue;
}

/**
 * @brief   Control the intensity of the backlight.
 * 
 * The new pulse value is loaded into the Capture Compare Register 1, if the intensity level is out of
 * range the function returns false.
 * 
 * @param hlcd Pointer to the LCD handle structure.
 * @param intensity The intensity level to be set. Should be from 0 to 100.
 * 
 * @retval  The operation succes, return true if the intensity level is lower than 100, otherwise false.
*/
uint8_t HEL_LCD_Intensity( LCD_HandleTypeDef *hlcd, uint8_t intensity )
{
    uint8_t retValue = false;

    if ( intensity <= MAX_INTENSITY )
    {
        retValue = true;

        #ifndef UTEST
        hlcd->TimHandler->Instance->CCR1 = intensity;
        #endif
    }
    
    return retValue;
}