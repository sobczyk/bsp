/***********************************************************************
* $Id:: lpc32xx_i2c.h 1028 2008-08-06 22:26:55Z wellsk                $
*
* Project: LPC32XX i2c controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32XX chip family component:
*         I2C controller
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

#ifndef LPC32XX_I2C_H
#define LPC32XX_I2C_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* I2C controller register structures
**********************************************************************/

/* I2C controller module register structures */
typedef struct
{
  volatile UNS_32 i2c_txrx;      /* I2C Tx/Rx Data FIFO */
  volatile UNS_32 i2c_stat;      /* I2C Status Register */
  volatile UNS_32 i2c_ctrl;      /* I2C Control Register */
  volatile UNS_32 i2c_clk_hi;    /* I2C Clock Divider high */
  volatile UNS_32 i2c_clk_lo;    /* I2C Clock Divider low */
  volatile UNS_32 i2c_adr;	   /* I2C Slave Address */
  volatile UNS_32 i2c_rxfl;	   /* I2C Rx FIFO level */
  volatile UNS_32 i2c_txfl;	   /* I2C Tx FIFO level */
  volatile UNS_32 i2c_rxb;       /* I2C Number of bytes received */
  volatile UNS_32 i2c_txb;       /* I2C Number of bytes transmitted */
  volatile UNS_32 i2c_stx;       /* Slave Transmit FIFO */
  volatile UNS_32 i2c_stxfl;	   /* Slave Transmit FIFO level */
} I2C_REGS_T;

/**********************************************************************
* i2c_txrx register definitions
**********************************************************************/
#define I2C_START    _BIT(8)		/* generate a START before this B*/
#define I2C_STOP     _BIT(9)		/* generate a STOP after this B */

/**********************************************************************
* i2c_stat register definitions
**********************************************************************/
#define I2C_TDI     _BIT(0)         /* Transaction Done Interrupt */
#define I2C_AFI     _BIT(1)         /* Arbitration Failure Interrupt */
#define I2C_NAI     _BIT(2)         /* No Acknowledge Interrupt */
#define I2C_DRMI    _BIT(3)         /* Master Data Request Interrupt */
#define I2C_DRSI    _BIT(4)         /* Slave Data Request Interrupt */
#define I2C_ACTIVE  _BIT(5)         /* Busy bus indicator */
#define I2C_SCL     _BIT(6)         /* The current SCL signal value */
#define I2C_SDA     _BIT(7)         /* The current SDA signal value */
#define I2C_RFF     _BIT(8)         /* Receive FIFO Full */
#define I2C_RFE     _BIT(9)         /* Receive FIFO Empty */
#define I2C_TFF     _BIT(10)        /* Transmit FIFO Full */
#define I2C_TFE     _BIT(11)        /* Transmit FIFO Empty */
#define I2C_TFFS    _BIT(12)        /* Slave Transmit FIFO Full */
#define I2C_TFES    _BIT(13)        /* Slave Transmit FIFO Empty */

/**********************************************************************
* i2c_ctrl register definitions
**********************************************************************/
#define I2C_TDIE    _BIT(0)         /* Transaction Done Int Enable */
#define I2C_AFIE    _BIT(1)         /* Arbitration Failure Int Ena */
#define I2C_NAIE    _BIT(2)         /* No Acknowledge Int Enable */
#define I2C_DRMIE   _BIT(3)         /* Master Data Request Int Ena */
#define I2C_DRSIE   _BIT(4)         /* Slave Data Request Int Ena */
#define I2C_RFFIE   _BIT(5)         /* Receive FIFO Full Int Ena */
#define I2C_RFDAIE  _BIT(6)         /* Rcv Data Available Int Ena */
#define I2C_TFFIE   _BIT(7)         /* Trnsmt FIFO Not Full Int Ena */
#define I2C_RESET   _BIT(8)         /* Soft Reset */
#define I2C_SEVEN   _BIT(9)         /* Seven-bit slave address */
#define I2C_TFFSIE  _BIT(10)        /* Slave Trnsmt FIFO Not Full IE */

/**********************************************************************
* i2c channel select
**********************************************************************/

/* Macro pointing to I2C controller registers */
#define I2C1  ((I2C_REGS_T  *)(I2C1_BASE))
#define I2C2  ((I2C_REGS_T  *)(I2C2_BASE))

#define I2C_RX_BUFFER_SIZE  4
#define I2C_TX_BUFFER_SIZE  4
#define I2C_STX_BUFFER_SIZE 4

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_I2C_H */
