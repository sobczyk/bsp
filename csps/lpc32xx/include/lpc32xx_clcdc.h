/***********************************************************************
 * $Id:: lpc32xx_clcdc.h 721 2008-05-08 17:01:39Z kendwyer             $
 *
 * Project: LPC32xx Color LCD controller definitions
 *
 * Description:
 *     This file contains the structure definitions and manifest
 *     constants for the LPC32xx components:
 *         Color LCD Controller
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

#ifndef LPC32xx_CLCDC_H
#define LPC32xx_CLCDC_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

/***********************************************************************
 * Color LCD Controller Register Structure
 **********************************************************************/

/* Color LCD controller Module Register Structure */
typedef struct
{
  volatile UNS_32 lcdtimh;               /* LCD timing 0 register */
  volatile UNS_32 lcdtimv;               /* LCD timing 1 register */
  volatile UNS_32 lcdpol;                /* LCD timing 2 register */
  volatile UNS_32 lcdle;                 /* LCD timing 3 register */
  volatile UNS_32 lcdupbase;             /* LCD upper frame buffer
                                           address register */
  volatile UNS_32 lcdlpbase;             /* LCD LCD lower frame buffer
                                           address register */

  volatile UNS_32 lcdctrl;               /* LCD control register */

  volatile UNS_32 lcdintrenable;         /* LCD int enable register */
  volatile UNS_32 lcdinterrupt;          /* LCD interrupt register */
  volatile UNS_32 lcdstatus;             /* LCD status register */
  volatile UNS_32 lcdintclr;             /* LCD int clear register */

  volatile UNS_32 lcdupcurr;             /* LCD upper frame buffer
                                           current address register */
  volatile UNS_32 lcdlpcurr;             /* LCD lower frame buffer
                                           current address register */

  volatile UNS_32 rsrved1clcdc[115];     /* LCD reserved */
  volatile UNS_32 lcdpalette[0x80];      /* LCD palette registers */
  volatile UNS_32 rsrved2clcdc[0x100];   /* LCD reserved */
  volatile UNS_32 lcdcrsrimage[0x100];   /* LCD cursor image RAM */
  volatile UNS_32 lcdcrsrcntl;           /* LCD cursor control 
                                            register */
  volatile UNS_32 lcdcrsrcnfg;           /* LCD cursor config 
                                            register */

  volatile UNS_32 lcdcrsrpal0;           /* LCD cursor pallette0 
                                            register */
  volatile UNS_32 lcdcrsrpal1;           /* LCD cursor pallette1 
                                            register */
  volatile UNS_32 lcdcrsrxy;             /* LCD cursor xy registers */
  volatile UNS_32 lcdcrsrclip;           /* LCD cursor clip registers */

  volatile UNS_32 rsrved3clcdc[2];       /* LCD reserved */

  volatile UNS_32 lcdcrsrintmsk;         /* LCD cursor interrupt 
                                            mask register */
  volatile UNS_32 lcdcrsrintclr;         /* LCD cursor interrupt 
                                            clear register */
  volatile UNS_32 lcdcrsrintraw;         /* LCD cursor interrupt 
                                            raw register */
  volatile UNS_32 lcdcrsrintstat;        /* LCD cursor interrupt 
                                            status register */
} CLCDC_REGS_T;

/***********************************************************************
 * Color LCD controller timing 0 register definitions
 **********************************************************************/

/* LCD controller timing 0 pixel per line load macro */
#define CLCDC_LCDTIMING0_PPL(n) _SBF(2, (((n) / 16) - 1) & 0x0000003F)
/* LCD controller timing 0 HSYNC pulse width load macro */
#define CLCDC_LCDTIMING0_HSW(n) _SBF(8, ((n) - 1) & 0x000000FF)
/* LCD controller timing 0 horizontal front porch load macro */
#define CLCDC_LCDTIMING0_HFP(n)	_SBF(16, ((n) - 1) & 0x000000FF)
/* LCD controller timing 0 horizontal back porch load macro */
#define CLCDC_LCDTIMING0_HBP(n)	_SBF(24, ((n) - 1) & 0x000000FF)

/***********************************************************************
 * Color LCD controller timing 1 register definitions
 **********************************************************************/

/* LCD controller timing 1 lines per panel load macro */
#define CLCDC_LCDTIMING1_LPP(n)	_SBF(0, ((n) - 1) & 0x000003FF)
/* LCD controller timing 1 VSYNC pulse width load macro */
#define CLCDC_LCDTIMING1_VSW(n)	_SBF(10, ((n) - 1) & 0x0000003F)
/* LCD controller timing 1 vertical front porch load macro */
#define CLCDC_LCDTIMING1_VFP(n) _SBF(16, (n & 0x000000FF))
/* LCD controller timing 1 vertical back porch load macro */
#define CLCDC_LCDTIMING1_VBP(n) _SBF(24, (n & 0x000000FF))

/***********************************************************************
 * Color LCD controller timing 2 register definitions
 **********************************************************************/

/* LCD controller timing 2 panel clock divisor load macro */
#define CLCDC_LCDTIMING2_PCD(n)	 CLCDC_LCDTIMING2_PCD_hi(n) | CLCDC_LCDTIMING2_PCD_lo(n)
#define CLCDC_LCDTIMING2_PCD_hi(n)	 (_SBF(22,((n) - 2 )) & 0xf8000000)
#define CLCDC_LCDTIMING2_PCD_lo(n)	 (_SBF(0, ((n) - 2 )) & 0x0000001f)
/* LCD controller timing 2 pixel clock selector bit */
#define CLCDC_LCDTIMING2_CLKSEL 0x00000020
/* LCD controller timing 2 AC bias frequency load macro */
#define CLCDC_LCDTIMING2_ACB(n)	_SBF(6, ((n) - 1) & 0x0000001F)
/* LCD controller timing 2 VSYNC invert bit */
#define CLCDC_LCDTIMING2_IVS    0x00000800
/* LCD controller timing 2 HSYNC invert bit */
#define CLCDC_LCDTIMING2_IHS    0x00001000
/* LCD controller timing 2 clock invert bit */
#define CLCDC_LCDTIMING2_IPC    0x00002000
/* LCD controller timing 2 output enable invert bit */
#define CLCDC_LCDTIMING2_IOE    0x00004000
/* LCD controller timing 2 clocks per line load macro */
#define CLCDC_LCDTIMING2_CPL(n)	_SBF(16, (n) & 0x000003FF)
/* LCD controller timing 2 bypass pixel divider bit */
#define CLCDC_LCDTIMING2_BCD 	0x04000000

/***********************************************************************
 * Color LCD controller interrupt enable/status register definitions
 **********************************************************************/


/* LCDIntrEnable, LCDInterrupt, LCDStatus FIFO underflow */
#define CLCDC_LCDSTATUS_FUF   0x00000002
/* LCDIntrEnable, LCDInterrupt, LCDStatus load next base bit */
#define CLCDC_LCDSTATUS_LNBU 	0x00000004
/* LCDIntrEnable, LCDInterrupt, LCDStatus vertical compare bit */
#define CLCDC_LCDSTATUS_VCOMP 	0x00000008
/* LCDIntrEnable, LCDInterrupt, LCDStatus aHB bus error bit */
#define CLCDC_LCDSTATUS_MBERROR	0x00000010

/***********************************************************************
 * Color LCD controller control register definitions
 **********************************************************************/

/* LCD control enable bit */
#define CLCDC_LCDCTRL_ENABLE	0x00000001
/* LCD control 1 bit per pixel bit field */
#define CLCDC_LCDCTRL_BPP1      0x00000000
/* LCD control 2 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP2      0x00000002
/* LCD control 4 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP4      0x00000004
/* LCD control 8 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP8      0x00000006
/* LCD control 16 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP16     0x00000008
/* LCD control 24 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP24     0x0000000A
/* LCD control 16 bits per pixel bit field (565 mode) */
#define CLCDC_LCDCTRL_BPP16_565 0x0000000C
/* LCD control 12 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP12     0x0000000E
/* LCD control 16 bits per pixel bit field */
#define CLCDC_LCDCTRL_BPP_MASK  0x0000000E
/* LCD control mono select bit */
#define CLCDC_LCDCTRL_BW_MONO   0x00000010
/* LCD controler TFT select bit */
#define CLCDC_LCDCTRL_TFT       0x00000020
/* LCD control monochrome LCD has 4-bit/8-bit select bit */
#define CLCDC_LCDCTRL_MONO8     0x00000040
/* LCD control dual panel select bit */
#define CLCDC_LCDCTRL_DUAL      0x00000080
/* LCD control normal RGB bit */
#define CLCDC_LCDCTRL_RGB       0x00000100
/* LCD control swap RGB (555 BGR mode) bit */
#define CLCDC_LCDCTRL_BGR       0x00000000
/* LCD control power enable bit */
#define CLCDC_LCDCTRL_PWR       0x00000800
/* LCD control VCOMP interrupt is start of VSYNC */
#define CLCDC_LCDCTRL_VCOMP_VS  0x00000000
/* LCD control VCOMP interrupt is start of back porch */
#define CLCDC_LCDCTRL_VCOMP_BP  0x00001000
/* LCD control VCOMP interrupt is start of video */
#define CLCDC_LCDCTRL_VCOMP_AV  0x00002000
/* LCD control VCOMP interrupt is start of front porch */
#define CLCDC_LCDCTRL_VCOMP_FP  0x00003000
/* LCD control interrupt condition bits mask */
#define CLCDC_LCDCTRL_VCOMP_IC  0x00003000
/* LCD control watermark is 4 or 8 words free mask */
#define CLCDC_LCDCTRL_WATERMARK 0x00010000

/* Macro pointing to the color LCD controller registers */
#define CLCDC    ((CLCDC_REGS_T *)(LCD_BASE))

#endif /* LPC32xx_CLCDC_H */
