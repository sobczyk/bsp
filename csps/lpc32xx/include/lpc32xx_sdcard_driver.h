/***********************************************************************
* $Id:: lpc32xx_sdcard_driver.h 1089 2008-08-18 22:16:30Z wellsk      $
*
* Project: LPC32XX SD card controller driver
*
* Description:
*     This file contains driver support for the LPC32xx SD card
*     controller.
*
* Notes on driver limitations:
*     STREAMING READ and WRITE is not supported by this driver.
*     After a WRITE_SINGLE command, use SEND_STATUS to determine when
*     it is ok to perform the next data transaction to the card. This
*     is due to the lack of a BUSY status.
*     Only use WRITE_SINGLE, READ_SINGLE, and READ MULTIPLE when not
*     in DMA mode.
*     In DMA mode, WRITE_SINGLE, READ_SINGLE, READ_MULTIPLE are
*     supported.
*     For READ_MULTPLE, setup the pending command to automatically
*     stop the transfer after the desired amount of data has been
*     received.The driver will always issue the command at the start
*     of the last block transferred.
*     If DMA is enabled and a linked list is not used, then the passed
*     buffer is assumed to be a contiguous data region with a physical
*     address. Non-DMA mode always used virtual addresses.
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

#ifndef LPC32XX_SDCARD_DRIVER_H
#define LPC32XX_SDCARD_DRIVER_H

#include "lpc_types.h"
#include "lpc32xx_dma_driver.h"
#include "lpc32xx_sdcard.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * SD card controller device configuration commands (IOCTL commands
 * and arguments)
 **********************************************************************/

/* SD card controller device commands (IOCTL commands) */
typedef enum
{
  /* Setup some SD parameters, use arg as a pointer to type
     SDC_PRMS_T */
  SD_SETUP_PARAMS,
  /* Setup data transfer mode. This sets up the driver for either
     interrupt or DMA transfer modes. Use arg as a pointer to type
     SDC_XFER_SETUP_T */
  SD_SETUP_DATA_XFER,
  /* Issue a SD card controller command or command/data request, use
     arg as a pointer to SD_CMDDATA_T. This issues and starts the
     command without blocking. Use the command and data callbacks
     to determine when the command and data portions have been
     completed by the controller and the SD_GET_CMD_RESP command to
     get the command and data status and response. */
  SD_ISSUE_CMD,
  /* Get last command/data response, use arg as a pointer to type
     SD_CMDRESP_T to be populated by the driver */
  SD_GET_CMD_RESP,
  /* Get status of last SDMMC operation, use arg as a value of type
     SD_STATUS_T */
  SD_GET_STATUS
} SDC_IOCTL_CMD_T;

/* SD command and response type */
typedef struct
{
  UNS_32  cmd;             /* SDMMC command */
  UNS_32  arg;             /* SDMMC argument */
  INT_32  cmd_resp_size;   /* Expected size of response (in bits) */
} SD_CMD_T;

/* Operation type for the data portion of the transfer */
typedef enum
{
  SD_DATAOP_NONE,     /* No data associated with this operation */
  SD_DATAOP_READ,     /* Data operation is read */
  SD_DATAOP_WRITE     /* Data operation is write */
} SD_DATAOP_T;

/* Structure used to define data transfers with the SD_DATA_READ and
   SD_DATA_WRITE IOCTL commands */
typedef struct
{
  /* Indicator for determining type of data transfer. For operations
     without data, this must be set to SD_DATAOP_NONE */
  SD_DATAOP_T dataop;
  /* Number of blocks to transfer. Each block must be sized to the
      blocksize value used with the SD_SETUP_DATA_XFER IOCTL
      command. The total number of blocks times the size of the block
      should not exceed 64KBytes. */
  UNS_32 blocks;
  /* Pointer to user buffer used in non-DMA and DMA modes. This must
     be aligned on a 32-bit boundary. The buffer should be big enough
     to handle the requested number of blocks. For DMA operation,
     this should be a physcial address. */
  UNS_32 *buff;
  /* If this value is TRUE and the transfer is a data transfer, then
     the command (ppendcmd) will be issued at the end of the data
     transfer by the SD card controller's command state machine using
     the command pending logic. This is useful for issuing the STOP
     TRANSFER command when using streaming or multiple block. */
  BOOL_32 usependcmd;
  /* Pending command to use */
  SD_CMD_T pendcmd;
  /* Streaming flag - if the command uses streaming mode (ie, READ
     UNTIL STOP command), set this flag to non-zero. */
  BOOL_32 stream;
} SD_XFER_T;

/* Command and data transfer structure used for the SD_ISSUE_CMD IOCTL
   command. */
typedef struct
{
  /* Command, argument, and response type to use */
  SD_CMD_T  cmd;
  /* Identifies data transfer type (if used) */
  SD_XFER_T data;
} SD_CMDDATA_T;

/* SDMMC command/data status and response type - pass a pointer to
   this structure as an arg to the SD_GET_CMD_RESP IOCTL command to
   get the last command/data status */
typedef struct
{
  UNS_32 cmd_status;       /* Last command related status */
  UNS_32 data_status;      /* Last data related status */
  UNS_32 cmd_resp [5];     /* Returned command status */
} SD_CMDRESP_T;

/* General SD controller parameters */
typedef struct
{
  UNS_32  sdclk_rate;      /* Target SD clock rate */
  BOOL_32 use_wide;        /* Enable wide bus mode */
  BOOL_32 pwrsave;         /* Enables powersave mode */
  BOOL_32 opendrain;       /* Enables push pull/open drain mode */
  UNS_32  powermode;       /* Selected power control mode, must be a
	                            value of type SD_POWER_xxx_MODE, where
	                            xxx = OFF, UP, or ON */
  INT_32  pullup0;         /* Enable pullup for data signal 0 */
  INT_32  pullup1;         /* Enable pullup for data signal 1 */
  INT_32  pullup23;        /* Enable pullup for data signals 2/3 */
} SDC_PRMS_T;

/* Structure used to setup the data transfer mode */
typedef struct
{
  /* Pointer to data callback function. This function is called when
     a data transfer has completed successfully or with an error. */
  PFV     data_callback;
  /* Pointer to command callback function. This function is called
     when a command has completed successfully or with an error. */
  PFV     cmd_callback;
  /* Size of a block in bytes used for block transfers. This value
     is normally 512 bytes, but some cards may have different block
     sizes */
  UNS_32  blocksize;
  /* Data timeout period (in SD clocks) */
  UNS_32  data_to;
  /* Use DMA for transferring data. If FALSE, interrupt mode is
     used instead. */
  BOOL_32 use_dma;
} SDC_XFER_SETUP_T;

/* Possible returned status - use with the SD_GET_STATUS ioctl */
typedef enum
{
  /* Returns raw SD controller status, use the SD controller status
     register bits to determine status */
  SD_RAW_STATUS,
  /* Returns allocated channel when DMA is supported */
  SD_GET_DMA_CHANNEL,
  /* Returns current programmed SD card clock rate in Hz */
  SD_GET_CLOCK
} SD_STATUS_T;

/***********************************************************************
 * SD card controller driver API functions
 **********************************************************************/

/* Open the SD card controller driver, arg is 0 */
INT_32 sdcard_open(void *ipbase,
                   INT_32 arg);

/* Close the SD card controller driver */
STATUS sdcard_close(INT_32 devid);

/* SD card controller driver configuration block */
STATUS sdcard_ioctl(INT_32 devid,
                    INT_32 cmd,
                    INT_32 arg);

/* SD card controller driver read function (stub only) */
INT_32 sdcard_read(INT_32 devid,
                   void *buffer,
                   INT_32 max_bytes);

/* SD card controller driver write function (stub only) */
INT_32 sdcard_write(INT_32 devid,
                    void *buffer,
                    INT_32 n_bytes);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_SDCARD_DRIVER_H */
