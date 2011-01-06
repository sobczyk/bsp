/***********************************************************************
 * $Id:: phy3250_board.h 4873 2010-09-09 21:04:43Z usb10132            $
 *
 * Project: Phytec 3250 board definitions
 *
 * Description:
 *     This file contains board specific information such as the
 *     chip select wait states, and other board specific information.
 *
 ***********************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
 **********************************************************************/

#ifndef PHY3250_BOARD_H
#define PHY3250_BOARD_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* For systems that use SPI, define the SPI clock rate here */
#define SPICLKRATE 10000000

/* Timing setup for the NAND MLC controller timing registers. These
   values can be adjusted to optimize the program time using the
   burner software with NAND. If your not worried about how long it
   takes to program your kickstart loader (or if your not using a
   kickstart loader), don't worry about changing these values. If you
   need to burn the kickstart, this can be speeded up by tweaking
   these values. See the 32x0 user's guide for info on these
   values. These should be programmed to work with the selected bus
   (HCLK) speed - because the burn image is usually small (under 54K),
   there really is not reason to change these values. */
#define MLC_TCEA_TIME   0x3
#define MLC_TWBTRB_TIME 0xF
#define MLC_TRHZ_TIME   0x3
#define MLC_TREH_TIME   0x7
#define MLC_TRP_TIME    0x7
#define MLC_TWH_TIME    0x7
#define MLC_TWP_TIME    0x7

/* Timing setup for the NAND SLC controller timing registers. On
   systems using NAND, these values effect how fast the kickstart
   loader loads the stage 1 application or how fast the S1L
   application handles NAND operations. See the 32x0 user's guide for
   info on these values. These should be programmed to work with the
   selected bus (HCLK) speed. */
#define SLC_NAND_W_RDY    0xF
#define SLC_NAND_W_WIDTH  0xF
#define SLC_NAND_W_HOLD   0xF
#define SLC_NAND_W_SETUP  0xF
#define SLC_NAND_R_RDY    0xF
#define SLC_NAND_R_WIDTH  0xF
#define SLC_NAND_R_HOLD   0xF
#define SLC_NAND_R_SETUP  0xF

/* External static memory timings used for chip select 0 (see the users
   guide for what these values do). Optimizing these values will help
   with NOR program and boot speed. These should be programmed to work
   with the selected bus (HCLK) speed. */
#define EMCSTATICWAITWEN_CLKS  0xF
#define EMCSTATICWAITOEN_CLKS  0xF
#define EMCSTATICWAITRD_CLKS   0x1F
#define EMCSTATICWAITPAGE_CLKS 0x1F
#define EMCSTATICWAITWR_CLKS   0x1F
#define EMCSTATICWAITTURN_CLKS 0xF

/***********************************************************************
 * System configuration defines
 **********************************************************************/
/* Carrier board revision selection - The carrier board revision is a
   value of 1305.x, where x = 0 to 3 */
#define PHY3250_CARRIERBOARD_1305_X 2

/* Module board revision selection - The module board revision is a
   value of 1304.x, where x = 0 to 1 */
#define PHY3250_MODULEBOARD_1304_X 1

/* LCD revision selection - The LCD revision is a value of 1307.x,
   where x = 0 to 1 */
#define PHY3250_LCD_1307_X 1

/***********************************************************************
 * Serial EEPROM via SPI
 **********************************************************************/

/* Size of serial EEPROM */
#define PHY3250_SEEPROM_SIZE  0x8000

/* Offset Where S1L Configuration is Stored */
#define PHY3250_SEEPROM_S1L_CFG_OFF	(PHY3250_SEEPROM_SIZE - 0x800)

/***********************************************************************
 * Functions
 **********************************************************************/

/* Miscellaneous board setup functions */
void phy3250_board_init(void);

/* LED toggle */
void phy3250_toggle_led(BOOL_32 on);

/* Read a button state */
typedef enum {PHY3250_BTN1, PHY3250_BTN2} PHY_BUTTON_T;
INT_32 phy3250_button_state(PHY_BUTTON_T button);

/* Enable or disable SDMMC power */
void phy3250_sdpower_enable(BOOL_32 enable);

/* Returns card inserted status */
BOOL_32 phy3250_sdmmc_card_inserted(void);

/* Enables or disables the LCD backlight */
void phy3250_lcd_backlight_enable(BOOL_32 enable);

/* Enables or disables the LCD power */
void phy3250_lcd_power_enable(BOOL_32 enable);

/* Reset the board */
void board_reset(void);

#ifdef __cplusplus
}
#endif

#endif /* PHY3250_BOARD_H */
