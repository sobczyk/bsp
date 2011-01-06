/***********************************************************************
* $Id:: lpc32xx_sdcard.h 974 2008-07-28 21:07:32Z wellsk              $
*
* Project: LPC32XX SD card controller definitions
*
* Description:
*     This file contains the structure definitions and manifest
*     constants for the LPC32xx chip family component:
*         SD card controller
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

#ifndef LPC32XX_SDCARD_H
#define LPC32XX_SDCARD_H

#include "lpc_types.h"
#include "lpc32xx_chip.h"

#ifdef __cplusplus
extern "C"
{
#endif

/**********************************************************************
* SD Card controller register structures
**********************************************************************/

/* SD Card controller module register structures */
typedef struct
{
  volatile UNS_32 sd_power;     /* SD power control reg */
  volatile UNS_32 sd_clock;     /* SD clock control reg */
  volatile UNS_32 sd_arg;       /* SD argument reg */
  volatile UNS_32 sd_cmd;       /* SD command reg */
  volatile UNS_32 sd_respcmd;   /* SD command response reg */
  volatile UNS_32 sd_resp [4];  /* SD response registers */
  volatile UNS_32 sd_dtimer;    /* SD data timeout reg */
  volatile UNS_32 sd_dlen;      /* SD data length reg */
  volatile UNS_32 sd_dctrl;     /* SD data control reg */
  volatile UNS_32 sd_dcnt;      /* SD data counter reg */
  volatile UNS_32 sd_status;    /* SD status reg */
  volatile UNS_32 sd_clear;     /* SD clear reg */
  volatile UNS_32 sd_mask0;     /* SD int mask 0 reg */
  volatile UNS_32 sd_mask1;     /* SD int mask 1 reg */
  volatile UNS_32 reserved1;
  volatile UNS_32 sd_fifocnt;   /* SD FIFO counter reg */
  volatile UNS_32 reserved2 [13];
  volatile UNS_32 sd_fifo [16]; /* SD FIFO */
} SDCARD_REGS_T;

/**********************************************************************
* sd_power register definitions
**********************************************************************/
/* SD bit for enabling open drain mode (1) or pushpull mode (0) */
#define SD_OPENDRAIN_EN            _BIT(6)
/* SD power control mode: power off */
#define SD_POWER_OFF_MODE          0x0
/* SD power control mode: power up */
#define SD_POWER_UP_MODE           0x2
/* SD power control mode: power on */
#define SD_POWER_ON_MODE           0x3
/* SD power control mode mask */
#define SD_POWER_MODE_MASK         0x3

/**********************************************************************
* sd_clock register definitions
**********************************************************************/
/* SD bit for enabling side bus mode */
#define SD_WIDEBUSMODE_EN          _BIT(11)
/* SD bit for enabling SDCLK clock bypass */
#define SD_SDCLK_BYPASS            _BIT(10)
/* SD bit for enabling clock throttling during idle states */
#define SD_SDCLK_PWRSAVE           _BIT(9)
/* SD bit for enabling the SD clock */
#define SD_SDCLK_EN                _BIT(8)
/* SD clock divider bit mask */
#define SD_CLKDIV_MASK             0xFF

/**********************************************************************
* sd_cmd register definitions
**********************************************************************/
/* SD bit for enabling command path state machine */
#define SD_CPST_EN                 _BIT(10)
/* SD bit for wait for CMDPEND prior to sending command */
#define SD_CMDPEND_WAIT            _BIT(9)
/* SD bit for enabling card interrupt request (without timeout) */
#define SD_INTERRUPT_EN            _BIT(8)
/* SD bit for enabling 136-bit response support */
#define SD_LONGRESP_EN             _BIT(7)
/* SD bit for enabling response support */
#define SD_RESPONSE                _BIT(6)
/* SD command mask */
#define SD_CMD_MASK                0x3F

/**********************************************************************
* sd_dctrl register definitions
**********************************************************************/
/* SD data transfer blocksize of 1 byte */
#define SD_BLKSIZE_1BYTE           0x00
/* SD data transfer blocksize of 2 bytes */
#define SD_BLKSIZE_2BYTES          0x10
/* SD data transfer blocksize of 4 bytes */
#define SD_BLKSIZE_4BYTES          0x20
/* SD data transfer blocksize of 8 bytes */
#define SD_BLKSIZE_8BYTES          0x30
/* SD data transfer blocksize of 16 bytes */
#define SD_BLKSIZE_16BYTES         0x40
/* SD data transfer blocksize of 32 bytes */
#define SD_BLKSIZE_32BYTES         0x50
/* SD data transfer blocksize of 64 bytes */
#define SD_BLKSIZE_64BYTES         0x60
/* SD data transfer blocksize of 128 bytes */
#define SD_BLKSIZE_128BYTES        0x70
/* SD data transfer blocksize of 256 bytes */
#define SD_BLKSIZE_256BYTES        0x80
/* SD data transfer blocksize of 512 bytes */
#define SD_BLKSIZE_512BYTES        0x90
/* SD data transfer blocksize of 1024 bytes */
#define SD_BLKSIZE_1024BYTES       0xA0
/* SD data transfer blocksize of 2048 bytes */
#define SD_BLKSIZE_2048BYTES       0xB0
/* SD bit for enabling DMA */
#define SD_DMA_EN                  _BIT(3)
/* SD bit for enabling a stream transfer */
#define SD_STREAM_EN               _BIT(2)
/* SD direction bit (1 = receive, 0 = transmit) */
#define SD_DIR_FROMCARD            _BIT(1)
/* SD data transfer enable bit */
#define SD_DATATRANSFER_EN         _BIT(0)

/**********************************************************************
* sd_status register definitions
* sd_clear register definitions (bits 0..10 only)
* sd_mask0, sd_mask1 register definitions
**********************************************************************/
/* SD bit for data receive FIFO NOT empty status */
#define SD_FIFO_RXDATA_AVAIL       _BIT(21)
/* SD bit for data transmit FIFO NOT empty status */
#define SD_FIFO_TXDATA_AVAIL       _BIT(20)
/* SD bit for data receive FIFO empty status */
#define SD_FIFO_RXDATA_EMPTY       _BIT(19)
/* SD bit for data transmit FIFO empty status */
#define SD_FIFO_TXDATA_EMPTY       _BIT(18)
/* SD bit for data receive FIFO full status */
#define SD_FIFO_RXDATA_FULL        _BIT(17)
/* SD bit for data transmit FIFO full status */
#define SD_FIFO_TXDATA_FULL        _BIT(16)
/* SD bit for data receive FIFO half-full status */
#define SD_FIFO_RXDATA_HFULL       _BIT(15)
/* SD bit for data transmit FIFO half-empty status */
#define SD_FIFO_TXDATA_HEMPTY      _BIT(14)
/* SD bit for data receive in progress status */
#define SD_RX_INPROGRESS           _BIT(13)
/* SD bit for data transmit in progress status */
#define SD_TX_INPROGRESS           _BIT(12)
/* SD bit for command transfer in progress status */
#define SD_CMD_INPROGRESS          _BIT(11)
/* SD bit for data block send/received complete (CRC good) status */
#define SD_DATABLK_END             _BIT(10)
/* SD bit for start bit detection error status */
#define SD_STARTBIT_ERR            _BIT(9)
/* SD bit for data end (data counter is 0) status */
#define SD_DATA_END                _BIT(8)
/* SD bit for command sent status */
#define SD_CMD_SENT                _BIT(7)
/* SD bit for command response received (CRC good) status */
#define SD_CMD_RESP_RECEIVED       _BIT(6)
/* SD bit for data receive FIFO overflow status */
#define SD_FIFO_RXDATA_OFLOW       _BIT(5)
/* SD bit for data transmit FIFO underflow status */
#define SD_FIFO_TXDATA_UFLOW       _BIT(4)
/* SD bit for data timeout status */
#define SD_DATA_TIMEOUT            _BIT(3)
/* SD bit for command timeout status */
#define SD_CMD_TIMEOUT             _BIT(2)
/* SD bit for data CRC failure status */
#define SD_DATA_CRC_FAIL           _BIT(1)
/* SD bit for command CRC failure status */
#define SD_CMD_CRC_FAIL            _BIT(0)

/* Macro pointing to SD card controller registers */
#define SDCARD ((SDCARD_REGS_T *)(SD_BASE))

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SDCARD_H */
