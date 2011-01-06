/***********************************************************************
 * $Id:: lpc32xx_clcdc_driver.h 702 2008-05-07 20:16:37Z kendwyer      $
 *
 * Project: lpc32xx CLCDC driver
 *
 * Description:
 *     This file contains driver support for the CLCDC module on the
 *     lpc32xx
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

#ifndef LPC32xx_CLCDC_DRIVER_H
#define LPC32xx_CLCDC_DRIVER_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "lpc_lcd_params.h"
#include "lpc32xx_clcdc.h"

/***********************************************************************
 * CLCDC device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* CLCDC device commands (IOCTL commands) */
typedef enum
{
  LCD_CONFIG,      /* Configure the LCD controller with a predefined
                        LCD configuration structure, use arg as the
                        pointer to the structure */
  LCD_ENABLE,      /* Enable or disable LCD controller, use arg = 0
                        to disable, arg = 1 to enable */
  LCD_PWENABLE,    /* Enable or disable LCD power, use arg = 0
                        to disable, arg = 1 to enable */
  LCD_SET_UP_FB,   /* Set upper frame buffer address, use arg as
                        the address of the frame buffer */
  LCD_SET_LW_FB,   /* Set lower frame buffer address, use arg as
                        the address of the frame buffer */
  LCD_SET_OV_FB,   /* Set overflow frame buffer address, use arg as
                        the address of the frame buffer */
  LCD_ENABLE_INTS, /* Enable LCD interrupts, use arg as an 'OR'ed
                        combination of
                        CLCDC_LCDSTATUS_FUF, CLCDC_LCDSTATUS_LNBU,
                        CLCDC_LCDSTATUS_VCOMP, and
                        CLCDC_LCDSTATUS_MBERROR */
  LCD_DISABLE_INTS, /* Disable LCD interrupts, use arg as an 'OR'ed
                         combination of CLCDC_LCDSTATUS_FUFUP,
                         CLCDC_LCDSTATUS_FUFLP, CLCDC_LCDSTATUS_LNBU,
                         CLCDC_LCDSTATUS_VCOMP, and
                         CLCDC_LCDSTATUS_MBERROR */
  LCD_PICK_INT,    /* Select the interrupt condition that generates
                        the CLCDC_LCDSTATUS_VCOMP interrupt, use arg as
                        a type of LCD_INTERRUPT_T */
  LCD_CLEAR_INT,   /* Clear the latched LCD interrupt condition, use
                        arg = 0 */
  LCD_DMA_ON_4MT,  /* Set DMA requests to start when 4 or 8 entries
                        are free in the CLCDC FIFOs, use arg = 1 for
                        4 entries, use arg = 0 for 8 entries */
  LCD_SWAP_RGB,    /* Swaps the red and green colors in a 16-bit
                        color value (555 displays only), use arg = 1 to
                        swap red and green, use arg = 0 to not swap */
  LCD_SET_BPP,     /* Set bits per pixel, arg must be 1, 2, 4, 8, or
                        16 */
  LCD_SET_CLOCK,   /* Sets or updates the LCD pixel clock, use arg as
                        a new clock rate in Hz */
  LCD_GET_STATUS,  /* Get an LCD status, use an argument type of
                        LCD_IOCTL_STS_T as the argument to return the
                        correct status */

  LCD_CRSR_EN,     /* Enable the LCD cursor, arg values are 0,1,2,3 */

  LCD_CRSR_XY,     /* Set the Cursor X/Y position. 
                      X,Y is passed in as arg */

  LCD_CRSR_PAL0,   /* Set the Cursor pallette 0 value. 
                      pallete is passed in as arg */

  LCD_CRSR_PAL1,   /* Set the Cursor pallette 1 value. 
                      pallete is passed in as arg */

  LCD_CRSR_INTMSK,  /* Sets (arg = 1) or clears the 
                      interrupt mask flag for cursur interrupts*/

  LCD_CRSR_INIT_IMG, /* initialize the LCD cursor image, 
                        arg is array or pointer to cursor 
						data image*/
  
  LCD_INIT_IMG_PALL, /* initialize the LCD pallette, 
                        arg is array or pointer to cursor 
						pallette data */

} LCD_IOCTL_CMD_T;

/* CLCDC arguments for LCD_PICK_INT command (IOCTL arguments) */
typedef enum
{
  VSYNC_START = 0,    /* Interrupt on start of VSYNC */
  BACK_PORCH_START,   /* Interrupt on start of vertical back porch */
  VIDEO_START,        /* Interrupt on start of video */
  FRONT_PORCH_START   /* Interrupt on start of vertical front porch */
} LCD_INTERRUPT_T;

/* CLCDC device arguments for LCD_GET_STATUS command (IOCTL
   arguments) */
typedef enum
{
  LCD_ENABLE_ST,  /* Returns LCD enabled status (1 = enabled, 0
                       = disabled */
  LCD_PWENABLE_ST, /* Returns LCD power enabled status (1 = enabled,
                        0 = disabled */
  LCD_UP_FB,      /* Returns address of primary (upper) frame
                       buffer */
  LCD_LW_FB,      /* Returns address of lower panel buffer (dual panel
                       display mode only) */
  LCD_OVR_FB,     /* Returns address of overflow buffer */
  LCD_PANEL_TYPE, /* Returns the LCD type, an enumeration of type
                       LCD_PANEL_T */
  LCD_CLOCK,      /* Returns the pixel clock in Hz */
  LCD_XSIZE,      /* Returns the display horizontal size in pixels */
  LCD_YSIZE,      /* Returns the display certical size in pixels */
  LCD_COLOR_DEPTH /* Returns the display color depth, either 1, 2, 4
                       8, or 16 bits per pixel */
} LCD_IOCTL_STS_T;

/***********************************************************************
 * CLCDC driver API functions
 **********************************************************************/

/* Open the LCD */
INT_32 lcd_open(void *ipbase, INT_32 arg);

/* Close the LCD */
STATUS lcd_close(INT_32 devid);

/* LCD configuration block */
STATUS lcd_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* LCD read function (stub only) */
INT_32 lcd_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes);

/* LCD write function (stub only) */
INT_32 lcd_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32xx_CLCDC_DRIVER_H */
