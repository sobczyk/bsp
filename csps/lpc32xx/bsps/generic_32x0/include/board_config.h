/***********************************************************************
 * $Id:: board_config.h 3388 2010-05-06 00:17:50Z usb10132             $
 *
 * Project: Board support package configuration options
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

#ifndef BOARD_CONFIG_H
#define BOARD_CONFIG_H

#ifdef __cplusplus
extern "C"
{
#endif

/* For systems that use SPI, define the SPI clock rate here */
#define SPICLKRATE 5000000

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

#ifdef __cplusplus
}
#endif

#endif /* BOARD_CONFIG_H */
