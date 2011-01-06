/***********************************************************************
 * $Id:: common_funcs.h 3391 2010-05-06 16:03:54Z usb10132             $
 *
 * Project: Common functions used by multiple applications
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

#ifndef COMMON_FUNCS_H
#define COMMON_FUNCS_H

/* User defined functions for disabling write protect (if needed) for
   NAND/SPI/NOR FLASH */
void nand_flash_wp_disable(void);
void nor_flash_wp_disable(void);
void spi_flash_wp_disable(void);

/* Purpose: Outputs some string data on the UART */
void uart_output(UNS_8 *buff);

/* Initialize UART output */
void uart_output_init(void);

/* Read data from the UART */
INT_32 uart_input(void *buffer, INT_32 max_bytes);

#endif /* COMMON_FUNCS_H */
