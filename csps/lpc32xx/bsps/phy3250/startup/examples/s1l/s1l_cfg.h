/***********************************************************************
 * $Id:: s1l_cfg.h 3396 2010-05-06 18:03:32Z usb10132                  $
 *
 * Project: S1L board configuration
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

#include "lpc_types.h"

#ifndef S1L_CFG_H
#define S1L_CFG_H

/***********************************************************************
 * Misc S1L configuration
 **********************************************************************/
/* Default S1L prompt for this system */
#define DEFPROMPT "PHY3250>"

/* System S1L startup message */
#define SYSHEADER "Phytec 3250 Board"

/***********************************************************************
 * S1L FLASH configuration
 **********************************************************************/

/* Uncomment this define to disable all NAND FLASH support */
#define S1L_SUPPORT_NAND

/* This define is used for picking either large or small block NAND
   FLASH support. Uncomment this define to use the large block SLC
   NAND driver */
#define USE_SMALL_BLOCK

/* Number of blocks plus one dedicated to Stage 1 application. This
   includes one extra block to the kickstart loader, so if 24 blocks
   are needed, set this to 25. */
#define BL_NUM_BLOCKS 25

/* Maximum amount of blocks available for saved FLASH applications
   stored at block (BL_FIRST_BLOCK + BL_NUM_BLOCKS), use 0xFFFFFFFF
   for maximum blocks */
#define FLASHAPP_MAX_BLOCKS 100

/* Structure used to save clock data across power cycles */
typedef struct S1L_BOARD_CFG_TYPE
{
	UNS_32 armclk;
	UNS_32 hclkdiv;
	UNS_32 pclkdiv;
} S1L_BOARD_CFG_T;
extern S1L_BOARD_CFG_T s1l_board_cfg;

/* Clock update function */
UNS_32 clock_adjust(void);

#endif /* S1L_CFG_H */
