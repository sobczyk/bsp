/***********************************************************************
 * $Id:: misc_config.h 3391 2010-05-06 16:03:54Z usb10132              $
 *
 * Project: Misc example configuration options
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

#ifndef MISC_CONFIG_H
#define MISC_CONFIG_H

/* UART device and rate, supported UART devices are UART3, UART4,
   UART5, and UART6 */
#define UARTOUTPUTDEV UART5
#define UARTOUTPUTRATE 115200

/* Load address used by the burner software. This is usually IRAM, but
   can be SDRAM (0x80000000) for larger images if IRAM is too small
   and the burner application initializes SDRAM. */
#define BURNER_LOAD_ADDR 0x8000

/* Set these values to where the kickstart loader will load the stage
   1 application in memory and it's size. If the kickstart loader
   initializes SDRAM, a stage 1 application can be loaded into SDRAM.
   However, if the kickstart loader doesn't initialize SDRAM, the
   application can only be loaded into IRAM. Since the kickstart loads
   and executes at address 0x0, be sure to load the image past the
   area where the kickstart's code and data is loaded. */
#define STAGE1_LOAD_ADDR 0x8000  /* Load at address 0x8000 */
#define STAGE1_LOAD_SIZE 0x20000 /* Load 128k */

/* Size allocation for kickstart loader when using SPI FLASH. The SPI
   kickstart loader is programmed into the start of SPI memory, but
   the stage 1 app is programmed here. This address is used by the
   SPI burner and the SPI kickstart loader. */
#define SPI_S1APP_OFFSET 0x8000

#endif /* MISC_CONFIG_H */
