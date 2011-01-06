/***********************************************************************
 * $Id:: sysapi_blkdev.c 3394 2010-05-06 17:56:27Z usb10132            $
 *
 * Project: Block device (SDMMC) support functions
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

#include "sys.h"
#include "lpc_string.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_sdcard_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_sdmmc.h"

/***********************************************************************
 * SD/MMC controller and loader support below
 **********************************************************************/

/* Operating condition register value */
#define OCRVAL OCR_VOLTAGE_RANGE_MSK /* around 3.1v */

/* SDMMC Block (sector) size in bytes */
#define SDMMC_BLK_SIZE    512

/* SDMMC OCR sequence clock speed - also the default clock speed of
   the bus whenever a new card is detected and configured */
#define SDMMC_OCR_CLOCK   390000

/* Normal clock speeds - the FIFOs may not be able to be maintained
   without using DMA at these clock speeds. */
#define SDMMC_NORM_CLOCK  5000000

#define INIT_OP_RETRIES   10  /* initial OP_COND retries */
#define SET_OP_RETRIES    200 /* set OP_COND retries */

/* card type defines */
#define CARD_TYPE_SD    (1 << 0)
#define CARD_TYPE_HC    (OCR_HC_CCS) /* high capacity card > 2GB */

static volatile INT_32 cmdresp, datadone;
static INT_32 sddev = 0;
static UNS_32 rca, sdcardtype, cid[4], csd[4];

/***********************************************************************
 *
 * Function: wait4cmddone
 *
 * Purpose: Sets command completion flag (callback)
 *
 * Processing:
 *     Sets the cmdresp flag to 1.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
 static void wait4cmddone(void)
 {
 	cmdresp = 1;
 }

/***********************************************************************
 *
 * Function: wait4datadone
 *
 * Purpose: Sets data completion flag (callback)
 *
 * Processing:
 *     Sets the datadone flag to 1.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
 static void wait4datadone(void)
 {
 	datadone = 1;
 }

/***********************************************************************
 *
 * Function: sdmmc_cmd_setup
 *
 * Purpose: Setup and SD/MMC command structure
 *
 * Processing:
 *     From the passed arguments, sets up the command structure for the
 *     SD_ISSUE_CMD IOCTL call to the SD card driver.
 *
 * Parameters:
 *     pcmdsetup : Pointer to command structure to fill
 *     cmd       : Command to send
 *     arg       : Argument to send
 *     resp_type : Expected response type for this command
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void sdmmc_cmd_setup(SD_CMD_T *pcmdsetup,
                            UNS_32 cmd,
                            UNS_32 arg)
{
	INT_32 tmp = 0;

	/* Determine response size */
	switch (cmd & CMD_MASK_RESP)
	{
		case CMD_RESP_R0:
			break;

		case CMD_RESP_R2:
			tmp = 136;
			break;

		default:
			tmp = 48;
			break;
	}

	/* Setup SD command structure */
	pcmdsetup->cmd = cmd & CMD_MASK_CMD;
	pcmdsetup->arg = arg;
	pcmdsetup->cmd_resp_size = tmp;
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_send
 *
 * Purpose: Process a SDMMC command and response (without data transfer)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd       : Command to send
 *     arg       : Argument to send
 *     resp_type : Expected response type for this command
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void sdmmc_cmd_send(UNS_32 cmd,
                           UNS_32 arg,
                           SD_CMDRESP_T *resp)
{
    SD_CMDDATA_T sdcmd;

	if (cmd & CMD_BIT_APP) {
		/* Needs APP command first */
		sdmmc_cmd_setup(&sdcmd.cmd, (MMC_APP_CMD | CMD_RESP_R1),
			(rca << 16));

		/* No data for this setup */
		sdcmd.data.dataop = SD_DATAOP_NONE;

		/* Issue command and wait for it to complete */
		cmdresp = 0;
		sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) &sdcmd);
		while (cmdresp == 0);

	    sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
		if ((resp->cmd_status & SD_CMD_RESP_RECEIVED) == 0) {
					return;
		}
	}

	/* Perform command setup from standard MMC command table */
	sdmmc_cmd_setup(&sdcmd.cmd, cmd, arg);

	/* No data for this setup */
	sdcmd.data.dataop = SD_DATAOP_NONE;

	/* Issue command and wait for it to complete */
	cmdresp = 0;
	sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) &sdcmd);
	while (cmdresp == 0);

    sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
}

/***********************************************************************
 *
 * Function: sdmmc_cmd_start_data
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     cmd  : Pointer to command structure
 *     resp : Pointer to response buffer
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 sdmmc_cmd_start_data(SD_CMDDATA_T *cmd,
                                   SD_CMDRESP_T *resp)
{
	INT_32 status = 0;

	/* Issue command and wait for it to complete */
	datadone = 0;
	sdcard_ioctl(sddev, SD_ISSUE_CMD, (INT_32) cmd);
	while (datadone == 0);

	/* Get the data transfer state */
	sdcard_ioctl(sddev, SD_GET_CMD_RESP, (INT_32) resp);
	if ((resp->data_status & SD_DATABLK_END) == 0)
	{
		status = -1;
	}

	return status;
}

/***********************************************************************
 *
 * Function: sdmmc_read_block
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff    : Pointer to data buffer
 *     numblks : Number of blocks to read
 *     index   : Block read index
 *     resp    :  Pointer to response buffers
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 sdmmc_read_block(UNS_32 *buff,
                               INT_32 numblks, /* Must be 1 */
                               UNS_32 index,
                               SD_CMDRESP_T *resp)
{
    SD_CMDDATA_T sdcmd;

	/* Setup read data */
	sdcmd.data.dataop = SD_DATAOP_READ;
	sdcmd.data.blocks = numblks;
	sdcmd.data.buff = buff;
	sdcmd.data.usependcmd = FALSE;
	sdcmd.data.stream = FALSE;

	/* Perform command setup from standard MMC command table */
	sdmmc_cmd_setup(&sdcmd.cmd, MMC_READ_SINGLE_BLOCK | CMD_RESP_R1,
		index);

	/* Read data from the SD card */
	return sdmmc_cmd_start_data(&sdcmd, resp);
}

/***********************************************************************
 *
 * Function: process_csd
 *
 * Purpose: Function to process the CSD & EXT-CSD of the card.
 *
 * Processing:
 *     Computes the card paramaters such as size, read block length,
 * 	 no. of blocks etc. based on cards CSD & EXT-CSD response.
 *
 * Parameters:
 *     pdev: Pointer to card info structure
 *
 * Outputs: None
 *
 * Returns: None.
 *
 * Notes: Function not needed. Repurposed.
 *
 **********************************************************************/
static void process_csd(void)
{
    SD_CMDRESP_T resp;

	/* put card in trans state */
	sdmmc_cmd_send(CMD_SELECT_CARD, (rca << 16), &resp);

	/* Setup block count to data size */
	sdmmc_cmd_send(CMD_SET_BLOCKLEN, SDMMC_BLK_SIZE, &resp);
}

/***********************************************************************
 *
 * Function: blkdev_deinit
 *
 * Purpose: SDMMC deinit
 *
 * Processing:
 *     De-initialize the SDMMC card
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_deinit(void) 
{
	if (sddev != 0) 
	{
		sdcard_close(sddev);
		sddev = 0;
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: blkdev_init
 *
 * Purpose: SDMMC init
 *
 * Processing:
 *     Initialize the SDMMC card
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_init(void) 
{
    SD_CMDRESP_T resp;
    SDC_PRMS_T params;
    SDC_XFER_SETUP_T dataset;
	int state = 0, tries = 0;
	UNS_32 command = 0, r, ocr = OCRVAL;

	/* Open SD card controller driver */
	sddev = sdcard_open(SDCARD, 0);
	if (sddev == 0)
	{
		return FALSE;
	}

	/* Setup controller parameters */
	params.opendrain = TRUE;
	params.powermode = SD_POWER_ON_MODE;
	params.pullup0 = 1;
	params.pullup1 = 1;
	params.pullup23 = 1;
	params.pwrsave = FALSE;
	params.sdclk_rate = SDMMC_OCR_CLOCK;
	params.use_wide = FALSE;
	if (sdcard_ioctl(sddev, SD_SETUP_PARAMS,
		(INT_32) &params) == _ERROR)
	{
		blkdev_deinit();
		return FALSE;
	}

	/* Setup data transfer paramaters */
	dataset.data_callback = (PFV) wait4datadone;
	dataset.cmd_callback = (PFV) wait4cmddone;
	dataset.blocksize = SDMMC_BLK_SIZE;
	dataset.data_to = 0x001FFFFF; /* Long timeout for slow MMC cards */
	dataset.use_dma = FALSE;
	sdcard_ioctl(sddev, SD_SETUP_DATA_XFER, (INT_32) &dataset);

	/* Issue IDLE command */
	sdmmc_cmd_send(CMD_IDLE, 0, &resp);

	/* After the IDLE command, a small wait is needed to allow the cards
	   to initialize */
    timer_wait_ms(TIMER_CNTR1, 100);

	sdcardtype = 0;
	rca = 0;
	while (state < 100)
	{
		switch (state)
		{
			case 0:
				sdmmc_cmd_send(CMD_SD_SEND_IF_COND, SD_SEND_IF_ARG, &resp);
				if ((resp.cmd_status & SD_CMD_RESP_RECEIVED) != 0)
				{
					/* check response has same echo pattern */
						if ((resp.cmd_resp[1] & SD_SEND_IF_ECHO_MSK) == SD_SEND_IF_RESP) {
						/* it is SD 2.0 card so indicate we are SDHC capable*/
						ocr |= CARD_TYPE_HC;
					}
				}
		        ++state;
				command = CMD_SD_OP_COND;
				tries = INIT_OP_RETRIES;
				/* assume SD card */
				sdcardtype |= CARD_TYPE_SD;
				break;

			case 10:      /* Setup for MMC */
				/* start fresh for MMC crds */
				sdcardtype &= ~CARD_TYPE_SD;
				sdmmc_cmd_send(CMD_IDLE, 0, &resp);
		        command = CMD_MMC_OP_COND;
		        tries = INIT_OP_RETRIES;
		        ocr |= OCR_HC_CCS;
		        ++state;
		        break;

			case 1:
			case 11:
				sdmmc_cmd_send(command, 0, &resp);

				/* If a command CRC error occurs on tne OCR (it always does!),
				   We weon't get a valid response flag OR a response timeout,
				   so we need to verify the repsonse good and timeout flags
				   before we move ahead */
				if (((resp.cmd_status & SD_CMD_RESP_RECEIVED) == 0) &&
					((resp.cmd_status & SD_CMD_TIMEOUT) == 0) &&
					((resp.cmd_status & SD_CMD_CRC_FAIL) != 0))
					++state;
				else
					state += 9;
				break;

			case 2:			/* Initial OCR check  */
			case 12:
				ocr = resp.cmd_resp[1] | (ocr & OCR_HC_CCS);
				if (ocr & OCR_ALL_READY)
					++state;
				else
					state += 2;
				break;

			case 3:			/* Initial wait for OCR clear */
			case 13:
				while ((ocr & OCR_ALL_READY) && --tries > 0)
				{
					timer_wait_ms(TIMER_CNTR1, 100);
					sdmmc_cmd_send(command, 0, &resp);
					ocr = resp.cmd_resp[1] | (ocr & OCR_HC_CCS);
				}
				if (ocr & OCR_ALL_READY)
					state += 7;
				else
					++state;
				break;

			case 14:
				/* for MMC cards set high capacity bit */
				ocr |= OCR_HC_CCS;
			case 4:     /* Assign OCR */
				tries = SET_OP_RETRIES;
				ocr &= OCRVAL | OCR_HC_CCS;	/* Mask for the bits we care about */
				do
				{
					timer_wait_ms(TIMER_CNTR1, 100);
					sdmmc_cmd_send(command, ocr, &resp);
					r = resp.cmd_resp[1];
				}
				while (!(r & OCR_ALL_READY) && --tries > 0);
				if (r & OCR_ALL_READY)
				{
					/* is it high capacity card */
					sdcardtype |= (r & CARD_TYPE_HC);
					++state;
				}
				else
					state += 6;
		        break;

			case 5:     /* CID polling */
			case 15:
				/* Enter push-pull mode and switch to fast clock */
				sdmmc_cmd_send(CMD_ALL_SEND_CID, 0, &resp);
		        memcpy(cid, &resp.cmd_resp[1], 16);
		        ++state;
				break;

			case 6:     /* RCA send, for SD get RCA */
				sdmmc_cmd_send(CMD_SD_SEND_RCA, 0, &resp);
				rca = (resp.cmd_resp [1] >> 16) & 0xFFFF;
				++state;
			break;

			case 16:      /* RCA assignment for MMC set to 1 */
				rca = 1;
				sdmmc_cmd_send(CMD_MMC_SET_RCA, (rca << 16), &resp);
				++state;
			break;

			case 7:
			case 17:
				sdmmc_cmd_send(CMD_SEND_CSD, (rca << 16), &resp);
				memcpy(csd, &resp.cmd_resp[1], 16);
				state = 100;
				break;

			default:
		        state += 100; /* break from while loop */
			    break;
		}
	}

    process_csd();

	if (sdcardtype & CARD_TYPE_SD)
	{
		/* Set bus width to 4 bits */
		sdmmc_cmd_send(CMD_SD_SET_WIDTH, 2, &resp);

		/* Switch controller to 4-bit data bus */
		params.use_wide = TRUE;
		params.opendrain = FALSE;
		params.sdclk_rate = SDMMC_NORM_CLOCK;
		sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: blkdev_read
 *
 * Purpose: Reads a sector
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Returns TRUE if the block was read ok
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 blkdev_read(void *buff, UNS_32 sector) 
{
    SD_CMDRESP_T resp;
	BOOL_32 good = TRUE;

	/* if high capacity card use block indexing */
	if (!(sdcardtype & CARD_TYPE_HC))
		sector = sector * SDMMC_BLK_SIZE;

	if (sdmmc_read_block(buff, 1, sector, &resp) < 0)
	{
		blkdev_deinit();
		good = FALSE;
	}

	return good;
}
