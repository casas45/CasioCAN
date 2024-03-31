/**
 * @file  hel_lcd.h
 * 
 * @brief Contain functions prototypes of the LCD driver and LCD instructions defines.
*/
#ifndef HEL_LCD_H_
#define HEL_LCD_H_

#include "stm32g0xx.h"
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define MAX_INTENSITY       100u    /*!< Maximum intensity value */

#define MAX_COL   15u     /*!< Maximum column number */
#define ROW_0     0u      /*!< Row 0 value */
#define COL_0     0u      /*!< Column 0 value */

/** 
  * @defgroup Bkl_states LCD backlight states.
  @{ */
#define LCD_ON      0u    /*!< Backlight ON */
#define LCD_OFF     1u    /*!< Backlight OFF */
#define LCD_TOGGLE  3u    /*!< Toggle Backlight */
/**
  @} */

/** 
  * @defgroup init_CMDs Initialization commands 
  @{ */
#define CMD_WAKEUP              0x30u     /*!< Wakeup command */
#define CMD_CLEAR_DISPLAY       0x01u     /*!< Clear display command */
/**
  @} */

/** 
  * @defgroup DDRAM_init Set DDRAM address instructions  
  @{ */
#define SET_DDRAM_ADDRESS       0x80u   /*!< Set DDRAM address command */
#define SET_CURSOR_ROW_1        0x40u   /*!< Upset to second line memory address */
/**
  @} */

/** 
  * @defgroup FunctionSet Function set command and configuration bits.
  @{ */
#define FUNCTION_SET      0x20u     /*!< Function set command */
#define DL_POS            4u        /*!< DL bit positon */
#define N_POS             3u        /*!< N bit positon */
#define DH_POS            2u        /*!< DH bit positon */
#define IS_POS            0u        /*!< IS bit positon */
/**
  @} */

/** 
  * @defgroup OSCfrequency Osc frequency command and configuration bits.
  @{ */
#define OSC_FREQUENCY     0x10u   /*!< Internal oscillator freq command */
#define BS_POS            3u      /*!< BS bit position */
#define F2_POS            2u      /*!< F2 bit position */
#define F1_POS            1u      /*!< F1 bit position */
#define F0_POS            0u      /*!< F0 bit position */
/**
  @} */

/** 
  * @defgroup PwrIconContrast PwrIconContrast command and configuration bits.
  @{ */
#define PWR_ICON_CONTRAST     0x50u   /*!< Pwr / ICONcontrol / Contrast set command */
#define ION_POS               3u      /*!< Ion bit position */
#define BON_POS               2u      /*!< Bon bit position */
#define C5_POS                1u      /*!< C5 bit position */
#define C4_POS                0u      /*!< C4 bit position */
/**
  @} */

/** 
  * @defgroup ContrastSet ContrastSet command and configuration bits.
  @{ */
#define CONTRAST_SET      0x70u   /*!< Contrast set command */
#define C3_POS            3u      /*!< C3 bit position */
#define C2_POS            2u      /*!< C2 bit position */
#define C1_POS            1u      /*!< C1 bit position */
#define C0_POS            0u      /*!< C0 bit position */
/**
  @} */

/** 
  * @defgroup ContrastLevels Contrast levels max and min.
  @{ */
#define LCD_CONTRAST_1    0x00u   /*!< LCD contrast level 1 */
#define LCD_CONTRAST_16   0x0Fu   /*!< LCD contrast level 16 */
/**
  @} */ 

/** 
  * @defgroup FollowerControl FollowerControl command and configuration bits.
  @{ */
#define FOLLOWER_CONTROL  0x60u     /*!< Follower control command */
#define FON_POS           3u        /*!< fon bit position */
#define RAB2_POS          2u        /*!< Rab2 bit position*/
#define RAB1_POS          1u        /*!< Rab1 bit position*/
#define RAB0_POS          0u        /*!< Rab0 bit position*/
/**
  @} */

/** 
  * @defgroup DisplayOnOff DisplayOnOff command and configuration bits.
  @{ */
#define DISPLAY_ON_OFF    0x08u   /*!< Display ON/OFF command */
#define D_POS             2u      /*!< D bit position*/
#define C_POS             1u      /*!< C bit position*/
#define B_POS             0u      /*!< B bit position*/
/**
  @} */

/** 
  * @defgroup EntryMode Entry mode command and configuration bits.
  @{ */
#define ENTRY_MODE    0x04u   /*!< Entry mode command */
#define I_D_POS       1u      /*!< I-D bit position*/
#define S_POS         0u      /*!< S bit position*/
/**
  @} */

/**
 * @struct  LCD_HandleTypeDef
 * 
 * @brief Struct to handle the ports, pins and SPI struct related to the LCD.
*/
typedef struct
{
    SPI_HandleTypeDef       *spiHandler;    /*!< Pointer to SPI handler used for comunication with the LCD  */
    GPIO_TypeDef            *RstPort;       /*!< GPIO port for the Reset pin */
    uint32_t                RstPin;         /*!< GPIO pin for the Reset pin */
    GPIO_TypeDef            *RsPort;        /*!< GPIO port for the Rs pin */
    uint32_t                RsPin;          /*!< GPIO pin for the Rs pin */
    GPIO_TypeDef            *CsPort;        /*!< GPIO port for the Chip Select (CS) pin */
    uint32_t                CsPin;          /*!< GPIO pin for the Chip Select (CS) pin */
    GPIO_TypeDef            *BklPort;       /*!< GPIO port for the Backlight pin */
    uint32_t                BklPin;         /*!< GPIO pin for the Backlight pin */
    TIM_HandleTypeDef       *TimHandler;     /*!< Pointer to the TIM Handler to control the intensity */

} LCD_HandleTypeDef;

uint8_t HEL_LCD_Init( LCD_HandleTypeDef *hlcd );

__attribute__((weak)) void HEL_LCD_MspInit( LCD_HandleTypeDef *hlcd );

uint8_t HEL_LCD_Command( LCD_HandleTypeDef *hlcd, uint8_t cmd );

uint8_t HEL_LCD_Data( LCD_HandleTypeDef *hlcd, uint8_t data );

uint8_t HEL_LCD_String( LCD_HandleTypeDef *hlcd, const char *str );

uint8_t HEL_LCD_SetCursor( LCD_HandleTypeDef *hlcd, uint8_t row, uint8_t col );

uint8_t HEL_LCD_Backlight( LCD_HandleTypeDef *hlcd, uint8_t state );

uint8_t HEL_LCD_Contrast( LCD_HandleTypeDef *hlcd, uint8_t contrast );

uint8_t HEL_LCD_Intensity( LCD_HandleTypeDef *hlcd, uint8_t intensity );

#endif