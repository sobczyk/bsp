/***********************************************************************
 * $Id:: sdmmc_dma_example.h 943 2008-07-24 20:41:54Z wellsk           $
 *
 * Project: SD/MMC support functions
 *
 * Description:
 *     These functions are used with the SDMMC and DMA examples.
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

#ifndef SDMMCCORE_H
#define SDMMCCORE_H

#include "lpc_types.h"
#include "lpc32xx_sdcard_driver.h"
#include "lpc_sdmmc.h"

/* Operating condition register value */
#define OCRVAL 0x001C0000 /* around 3.1v */

/* SDMMC Block (sector) size in bytes */
#define SDMMC_BLK_SIZE    512

/* SDMMC OCR power-up complete mask (used against word 0) */
#define SDMMC_OCR_MASK    0x80000000

/* SDMMC maximum number of OCR request retries before considering a
   card dead */
#define SDMMC_MAX_OCR_RET 512

/* SDMMC OCR sequence clock speed - also the default clock speed of
   the bus whenever a new card is detected and configured */
#define SDMMC_OCR_CLOCK   390000

/* Normal clock speeds without DMA (interrupt mode) */
#define SD_NORM_CLOCK      5000000
#define MMC_NORM_CLOCK     5000000

/* Each command enumeration has a SDMMC command number (used by the
   card) and a SDMMC command/control word (used by the controller).
   This structure defines this word pair. */
typedef struct
{
  UNS_32 cmd;            /* Mapped SDMMC (spec) command */
  SDMMC_RESPONSE_T resp; /* Expected response type */
} SDMMC_CMD_CTRL_T;

/* Process a SDMMC command and response (without data transfer) */
void sdmmc_cmd_send(SDMMC_COMMAND_T cmd,
                    UNS_32 arg,
                    SD_CMDRESP_T *resp);

/* Process a SD APP command and response */
void app_cmd_send(SD_APP_CMD_T cmd,
                  UNS_32 arg,
                  SD_CMDRESP_T *resp);

/* Read SD/MMC data blocks */
INT_32 sdmmc_read_block(UNS_32 *buff,
                        INT_32 numblks, /* Must be 1 */
                        UNS_32 index,
                        SD_CMDRESP_T *resp);

/* Write SD/MMC data blocks */
INT_32 sdmmc_write_block(UNS_32 *buff,
                         INT_32 numblks, /* Must be 1 */
                         UNS_32 index,
                         SD_CMDRESP_T *resp);

/* Setup SDMMC card and controller */
BOOL_32 sdmmc_setup(void);

/* Close SDMMC card and controller */
void sdmmc_close(void);

#endif
