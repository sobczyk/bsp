/***********************************************************************
* $Id:: lpc32xx_hsuart.h 1027 2008-08-06 22:26:25Z wellsk             $
*
* Project: LPC3xxx high speed UART definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC3xxx chip family component:
*         high speed UART definitions
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

#ifndef LPC32XX_HIGH_SPEED_UART_H
#define LPC32XX_HIGH_SPEED_UART_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************************
* High speed UART register structures
**********************************************************************/

/* High speed UART register structures */ 
typedef struct {
    volatile UNS_32 txrx_fifo;      /* HS UART Tx/Rx FIFO */
    volatile UNS_32 level;          /* HS FIFO Level Register */
    volatile UNS_32 iir;            /* HS UART Int Identification Reg */
    volatile UNS_32 ctrl;           /* HS UART Control Register */
    volatile UNS_32 rate;           /* HS UART Rate Control Register */
} HSUART_REGS_T;

/* UART control structure */
typedef struct {
    volatile UNS_32 ctrl;         /* General UART control register */
	volatile UNS_32 clkmode;      /* UART clock control register */
    volatile UNS_32 loop;         /* UART loopmode enable/disable */
} HSUART_CNTL_REGS_T;

/**********************************************************************
* high speed UART receiver FIFO register definitions
**********************************************************************/
#define HSU_BREAK_DATA      _BIT(10)    /* break condition indicator*/
#define HSU_ERROR_DATA      _BIT(9)     /* framing error indicator */
#define HSU_RX_EMPTY        _BIT(8)     /* Rx FIFO empty status */

/**********************************************************************
* high speed UART level register definitions
**********************************************************************/
#define HSU_TX_LEV(n)       ((n>>8) & 0xFF)
#define HSU_RX_LEV(n)       ((n) & 0xFF)

/**********************************************************************
* high speed UART interrupt identification register definitions
**********************************************************************/
#define HSU_TX_INT_SET      _BIT(6)     /* set the Tx int flag */
#define HSU_RX_OE_INT       _BIT(5)     /* overrun int flag */
#define HSU_BRK_INT         _BIT(4)     /* break int flag */
#define HSU_FE_INT          _BIT(3)     /* framing error flag */
#define HSU_RX_TIMEOUT_INT  _BIT(2)     /* Rx timeout int flag */
#define HSU_RX_TRIG_INT     _BIT(1)     /* Rx FIFO trig level ind */
#define HSU_TX_INT          _BIT(0)     /* Tx interrupt flag */                  

/**********************************************************************
* high speed UART control register definitions
**********************************************************************/
#define HSU_HRTS_INV        _BIT(21)            /* HRTS invert ctrl */
#define HSU_HRTS_TRIG(n)    (_SBF(19,(n&0x3)))  /* HRTS trig level */
#define HSU_HRTS_TRIG_8B    0                   /* HRTS trig lev 8B */
#define HSU_HRTS_TRIG_16B   _BIT(19)            /* HRTS trig lev 16B */
#define HSU_HRTS_TRIG_32B   _BIT(20)            /* HRTS trig lev 32B */
#define HSU_HRTS_TRIG_48B   _SBF(19,0x3)        /* HRTS trig lev 48B */
#define HSU_HRTS_EN         _BIT(18)            /* HRTS enable ctrl */
#define HSU_TMO_CONFIG(n)   ((n) & _SBF(16,0x3))/* tout intconfig */
#define HSU_TMO_DISABLED    0                   /* Rx tmo disabled */
#define HSU_TMO_INACT_4B    _BIT(16)            /* tmo after 4 bits */
#define HSU_TMO_INACT_8B    _BIT(17)            /* tmo after 8 bits */
#define HSU_TMO_INACT_16B   _SBF(16,0x3)        /* tmo after 16 bits */
#define HSU_HCTS_INV        _BIT(15)            /* HCTS inverted */
#define HSU_HCTS_EN         _BIT(14)            /* Tx flow control */
#define HSU_OFFSET(n)       _SBF(9,n)           /* 1st samplingpoint */
#define HSU_BREAK           _BIT(8)             /* break control */
#define HSU_ERR_INT_EN      _BIT(7)             /* HSUART err int en */
#define HSU_RX_INT_EN       _BIT(6)             /* HSUART Rx int en */
#define HSU_TX_INT_EN       _BIT(5)             /* HSUART Tx int en */
#define HSU_RX_TRIG(n)      ((n) & _SBF(2,0x7)) /* Rx FIFO trig lev */
#define HSU_RX_TL1B         _SBF(2,0x0)         /* Rx FIFO trig 1B  */
#define HSU_RX_TL4B         _SBF(2,0x1)         /* Rx FIFO trig 4B */
#define HSU_RX_TL8B         _SBF(2,0x2)         /* Rx FIFO trig 8B */
#define HSU_RX_TL16B        _SBF(2,0x3)         /* Rx FIFO trig 16B */
#define HSU_RX_TL32B        _SBF(2,0x4)         /* Rx FIFO trig 32B */
#define HSU_RX_TL48B        _SBF(2,0x5)         /* Rx FIFO trig 48B */
#define HSU_TX_TRIG(n)      ((n) & _SBF(0,0x3)) /* Tx FIFO trig lev */
#define HSU_TX_TLEMPTY      _SBF(0,0x0)         /* Tx FIFO trig empty */
#define HSU_TX_TL0B         _SBF(0,0x0)         /* Tx FIFO trig empty */
#define HSU_TX_TL4B         _SBF(0,0x1)         /* Tx FIFO trig 4B */
#define HSU_TX_TL8B         _SBF(0,0x2)         /* Tx FIFO trig 8B */
#define HSU_TX_TL16B        _SBF(0,0x3)         /* Tx FIFO trig 16B */

/* Macros pointing to high speed UART registers */
#define UART1 ((HSUART_REGS_T *)(HS_UART1_BASE))
#define UART2 ((HSUART_REGS_T *)(HS_UART2_BASE))
#define UART7 ((HSUART_REGS_T *)(HS_UART7_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_HIGH_SPEED_UART_H */ 
