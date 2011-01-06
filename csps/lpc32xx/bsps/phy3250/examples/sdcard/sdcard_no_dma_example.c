/***********************************************************************
 * $Id:: sdcard_no_dma_example.c 1868 2009-05-19 20:29:10Z wellsk      $
 *
 * Project: SDMMC driver example with interrupts (FIFO mode)
 *
 * Description:
 *     A SD card controller driver example using SD/MMC.
 *
 * Notes:
 *     This examples has no direct output. This code must be executed
 *     with a debugger to see how it works. The write functionality
 *     has been disabled by default to prevent unintended writes to
 *     the SD/MMC cards. To enable it, uncomment the MMCWRITE define.
 *     Be careful using MMCWRITE as it may make your cards unusable
 *     without a card reformat. Use only with cards that do not have
 *     important data!
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

#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc_arm922t_cp15_driver.h"
#include "phy3250_board.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_sdcard_driver.h"
#include "sdmmc_dma_example.h"

/* Uncomment the following line to enable SDMMC write functionality */
//#define MMCWRITE

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Blocks to transfer and first block to use */
#define BLKSTOTRANFER 4
#define BLKFIRST      0

/* This array maps the SDMMC command enumeration to the hardware SDMMC
   command index, the controller setup value, and the expected response
   type */
SDMMC_CMD_CTRL_T sdmmc_cmds[SDMMC_INVALID_CMD] =
{
  /* SDMMC_IDLE */
  {0,  SDMMC_RESPONSE_NONE},
  /* MMC_SENDOP_COND */
  {1,  SDMMC_RESPONSE_R3},
  /* SDMMC_ALL_SEND_CID */
  {2,  SDMMC_RESPONSE_R2},
  /* SDMMC_SRA */
  {3,  SDMMC_RESPONSE_R1},
  /* MMC_PROGRAM_DSR */
  {4,  SDMMC_RESPONSE_NONE},
  /* SDMMC_SELECT_CARD */
  {7,  SDMMC_RESPONSE_R1},
  /* SDMMC_SEND_CSD */
  {9,  SDMMC_RESPONSE_R2},
  /* SDMMC_SEND_CID */
  {10, SDMMC_RESPONSE_R2},
  /* SDMMC_READ_UNTIL_STOP */
  {11, SDMMC_RESPONSE_R1},
  /* SDMMC_STOP_XFER */
  {12, SDMMC_RESPONSE_R1},
  /* SDMMC_SSTAT */
  {13, SDMMC_RESPONSE_R1},
  /* SDMMC_INACTIVE */
  {15, SDMMC_RESPONSE_NONE},
  /* SDMMC_SET_BLEN */
  {16, SDMMC_RESPONSE_R1},
  /* SDMMC_READ_SINGLE */
  {17, SDMMC_RESPONSE_R1},
  /* SDMMC_READ_MULTIPLE */
  {18, SDMMC_RESPONSE_R1},
  /* SDMMC_WRITE_UNTIL_STOP */
  {20, SDMMC_RESPONSE_R1},
  /* SDMMC_SET_BLOCK_COUNT */
  {23, SDMMC_RESPONSE_R1},
  /* SDMMC_WRITE_SINGLE */
  {24, SDMMC_RESPONSE_R1},
  /* SDMMC_WRITE_MULTIPLE */
  {25, SDMMC_RESPONSE_R1},
  /* MMC_PROGRAM_CID */
  {26, SDMMC_RESPONSE_R1},
  /* SDMMC_PROGRAM_CSD */
  {27, SDMMC_RESPONSE_R1},
  /* SDMMC_SET_WR_PROT */
  {28, SDMMC_RESPONSE_R1B},
  /* SDMMC_CLEAR_WR_PROT */
  {29, SDMMC_RESPONSE_R1B},
  /* SDMMC_SEND_WR_PROT */
  {30, SDMMC_RESPONSE_R1},
  /* SD_ERASE_BLOCK_START */
  {32, SDMMC_RESPONSE_R1},
  /* SD_ERASE_BLOCK_END */
  {33, SDMMC_RESPONSE_R1},
  /* MMC_ERASE_BLOCK_START */
  {35, SDMMC_RESPONSE_R1},
  /* MMC_ERASE_BLOCK_END */
  {36, SDMMC_RESPONSE_R1},
  /* MMC_ERASE_BLOCKS */
  {38, SDMMC_RESPONSE_R1B},
  /* MMC_FAST_IO */
  {39, SDMMC_RESPONSE_R4},
  /* MMC_GO_IRQ_STATE */
  {40, SDMMC_RESPONSE_R5},
  /* MMC_LOCK_UNLOCK */
  {42, SDMMC_RESPONSE_R1B},
  /* SDMMC_APP_CMD */
  {55, SDMMC_RESPONSE_R1},
  /* SDMMC_GEN_CMD */
  {56, SDMMC_RESPONSE_R1B}
};

/* This array maps the SDMMC application specific command enumeration to
   the hardware SDMMC command index, the controller setup value, and the
   expected response type */
SDMMC_CMD_CTRL_T sdmmc_app_cmds[SD_INVALID_APP_CMD] =
{
  /* SD_SET_BUS_WIDTH */
  {6,  SDMMC_RESPONSE_R1},
  /* SD_SEND_STATUS */
  {13, SDMMC_RESPONSE_R1},
  /* SD_SEND_WR_BLOCKS */
  {22, SDMMC_RESPONSE_R1},
  /* SD_SET_ERASE_COUNT */
  {23, SDMMC_RESPONSE_R1},
  /* SD_SENDOP_COND */
  {41, SDMMC_RESPONSE_R3},
  /* SD_CLEAR_CARD_DET */
  {42, SDMMC_RESPONSE_R1},
  /* SD_SEND_SCR */
  {51, SDMMC_RESPONSE_R1}
};

static volatile INT_32 cmdresp, datadone;
INT_32 sddev;
UNS_32 rca;
UNS_32 databuff[512 * BLKSTOTRANFER / sizeof(UNS_32)];


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
void wait4cmddone(void)
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
void wait4datadone(void)
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
                            UNS_32 arg,
                            SDMMC_RESPONSE_T resp_type)
{
  INT_32 tmp = 0;

  /* Determine response size */
  switch (resp_type)
  {
    case SDMMC_RESPONSE_NONE:
      tmp = 0;
      break;

    case SDMMC_RESPONSE_R1:
    case SDMMC_RESPONSE_R1B:
    case SDMMC_RESPONSE_R3:
      tmp = 48;
      break;

    case SDMMC_RESPONSE_R2:
      tmp = 136;
      break;
  }

  /* Setup SD command structure */
  pcmdsetup->cmd = cmd;
  pcmdsetup->arg = arg;
  pcmdsetup->cmd_resp_size = tmp;
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
 *     resp : Pointer to response structure to fill
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
void sdmmc_cmd_send(SDMMC_COMMAND_T cmd,
                    UNS_32 arg,
                    SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;

  /* Perform command setup from standard MMC command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[cmd].cmd, arg,
                  sdmmc_cmds[cmd].resp);

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
 * Function: app_cmd_send
 *
 * Purpose: Process a SD APP command and response
 *
 * Processing:
 *     See function.
 *
 * Parameters: TBD
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void app_cmd_send(SD_APP_CMD_T cmd,
                  UNS_32 arg,
                  SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;

  /* Perform command setup from SD APP command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_app_cmds[cmd].cmd, arg,
                  sdmmc_app_cmds[cmd].resp);

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
 * Function: sdmmc_read_block
 *
 * Purpose: Read SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff    : Pointer to word aligned buffer
 *     numblks : Must be 1
 *     index   : Read offset on the card
 *     resp    : Pointer to repsonse structure to fill
 *
 * Outputs: None
 *
 * Returns: <1 on an error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_read_block(UNS_32 *buff,
                        INT_32 numblks, /* Must be 1 */
                        UNS_32 index,
                        SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;

  /* Setup read data */
  sdcmd.data.dataop = SD_DATAOP_READ;
  sdcmd.data.blocks = numblks;
  sdcmd.data.buff = buff;

  /* Setup block count to data size */
  sdmmc_cmd_send(SDMMC_SET_BLEN, SDMMC_BLK_SIZE, resp);

  /* Perform command setup from standard MMC command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[SDMMC_READ_SINGLE].cmd,
                  index, sdmmc_cmds[SDMMC_READ_SINGLE].resp);

  /* Read data from the SD card */
  return sdmmc_cmd_start_data(&sdcmd, resp);
}

/***********************************************************************
 *
 * Function: sdmmc_write_block
 *
 * Purpose: Write SD/MMC data blocks
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     buff    : Pointer to word aligned buffer
 *     numblks : Must be 1
 *     index   : Read offset on the card
 *     resp    : Pointer to repsonse structure to fill
 *
 * Outputs: None
 *
 * Returns: <1 on an error
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 sdmmc_write_block(UNS_32 *buff,
                         INT_32 numblks, /* Must be 1 */
                         UNS_32 index,
                         SD_CMDRESP_T *resp)
{
  SD_CMDDATA_T sdcmd;
  INT_32 status;
  UNS_32 tmp;

  /* Setup read data */
  sdcmd.data.dataop = SD_DATAOP_WRITE;
  sdcmd.data.blocks = numblks;
  sdcmd.data.buff = buff;

  /* Setup block count to data size */
  sdmmc_cmd_send(SDMMC_SET_BLEN, SDMMC_BLK_SIZE, resp);

  /* Perform command setup from standard MMC command table */
  sdmmc_cmd_setup(&sdcmd.cmd, sdmmc_cmds[SDMMC_WRITE_SINGLE].cmd,
                  index, sdmmc_cmds[SDMMC_WRITE_SINGLE].resp);

  /* Write data to the SD card */
  status = sdmmc_cmd_start_data(&sdcmd, resp);

  /* Poll status from the device until transfer is complete */
  tmp = 0;
  while ((tmp & 0x1F00) != 0x900)
  {
    /* Currently in non-TRANS state, keep polling card until it
       returns to TRANS state */
    sdmmc_cmd_send(SDMMC_SSTAT, (rca << 16), resp);
    tmp = resp->cmd_resp [1];
  }

  return status;
}

/***********************************************************************
 *
 * Function: sdmmc_setup
 *
 * Purpose: Setup SDMMC card and controller
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the card was initialized ok, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 sdmmc_setup(void)
{
  INT_32 ocrtries, sdcardtype, validocr;
  SD_CMDRESP_T resp;
  SDC_PRMS_T params;
  SDC_XFER_SETUP_T dataset;

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
  sdmmc_cmd_send(SDMMC_IDLE, 0, &resp);

  /* After the IDLE command, a small wait is needed to allow the cards
     to initialize */
  timer_wait_ms(TIMER_CNTR0, 100);

  /* Issue APP command, only SD cards will respond to this */
  sdcardtype = 0;
  sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
  if ((resp.cmd_status & SD_CMD_RESP_RECEIVED) != 0)
  {
    sdcardtype = 1;
  }

  ocrtries = SDMMC_MAX_OCR_RET;
  validocr = 0;

  /* If this is an SD card, use the SD sendop command */
  if (sdcardtype == 1)
  {
    resp.cmd_resp [1] = 0;
    while ((validocr == 0) && (ocrtries >= 0))
    {
      /* SD card init sequence */
      app_cmd_send(SD_SENDOP_COND, OCRVAL, &resp);
      if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) == 0)
      {
        /* Response received and busy, so try again */
        sdmmc_cmd_send(SDMMC_APP_CMD, 0, &resp);
      }
      else
      {
        validocr = 1;
      }

      ocrtries--;
    }

    if (validocr == 0)
    {
      return FALSE;
    }

    /* Enter push-pull mode and switch to fast clock */
    params.opendrain = FALSE;
    params.sdclk_rate = SD_NORM_CLOCK;
    sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);

    /* Get CID */
    sdmmc_cmd_send(SDMMC_ALL_SEND_CID, 0, &resp);

    /* Get relative card address */
    sdmmc_cmd_send(SDMMC_SRA, 0, &resp);
    rca = (resp.cmd_resp [1] >> 16) & 0xFFFF;

    /* Select card (required for bus width change) */
    sdmmc_cmd_send(SDMMC_SELECT_CARD, (rca << 16), &resp);

    /* Set bus width to 4 bits */
    sdmmc_cmd_send(SDMMC_APP_CMD, (rca << 16), &resp);
    app_cmd_send(SD_SET_BUS_WIDTH, 2, &resp);

    /* Switch controller to 4-bit data bus */
    params.use_wide = TRUE;
    sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);
  }
  else
  {
    resp.cmd_resp [1] = 0;
    while ((validocr == 0) && (ocrtries >= 0))
    {
      /* MMC card init sequence */
      sdmmc_cmd_send(MMC_SENDOP_COND, OCRVAL, &resp);
      if ((resp.cmd_resp [1] & SDMMC_OCR_MASK) != 0)
      {
        validocr = 1;
      }

      ocrtries--;
    }

    if (validocr == 0)
    {
      return FALSE;
    }

    /* Enter push-pull mode and switch to fast clock */
    params.opendrain = FALSE;
    params.sdclk_rate = MMC_NORM_CLOCK;
    sdcard_ioctl(sddev, SD_SETUP_PARAMS, (INT_32) &params);

    /* Get CID */
    sdmmc_cmd_send(SDMMC_ALL_SEND_CID, 0, &resp);

    /* Get relative card address */
    rca = 0x1234;
    sdmmc_cmd_send(SDMMC_SRA, (rca << 16), &resp);
  }

  /* Deselect card */
  sdmmc_cmd_send(SDMMC_SELECT_CARD, 0, &resp);

  /* Status request */
  sdmmc_cmd_send(SDMMC_SSTAT, (rca << 16), &resp);

  /* Select card */
  sdmmc_cmd_send(SDMMC_SELECT_CARD, (rca << 16), &resp);

  return TRUE;
}

/***********************************************************************
 *
 * Function: sdmmc_close
 *
 * Purpose: Close SDMMC card and controller
 *
 * Processing:
 *     See function.
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
void sdmmc_close(void)
{
  sdcard_close(sddev);
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Always returns 1
 *
 * Notes: None
 *
 **********************************************************************/
void c_entry(void)
{
  UNS_32 blk, sblk;
  SD_CMDRESP_T resp;
  UNS_32 *dbuff;

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Setup miscellaneous board functions */
  phy3250_board_init();

  /* Set virtual address of MMU table */
  cp15_set_vmmu_addr((void *)
                     (IRAM_BASE + (256 * 1024) - (16 * 1024)));

  /* Initialize interrupt system */
  int_initialize(0xFFFFFFFF);

  /* Install standard IRQ dispatcher at ARM IRQ vector */
  int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

  /* Enable SDMMC power */
  phy3250_sdpower_enable(TRUE);

  /* Exit if no SDMMC card installed */
  if (phy3250_sdmmc_card_inserted() == FALSE)
  {
    return;
  }

  enable_irq();

  /* Setup SDMMC card and controller */
  if (sdmmc_setup() == FALSE)
  {
    return;
  }

  /* Read a bunch of data blocks */
  sblk = BLKFIRST;
  dbuff = databuff;
  for (blk = 0; blk < BLKSTOTRANFER; blk++)
  {
    if (sdmmc_read_block(dbuff, 1, (sblk * SDMMC_BLK_SIZE),
                         &resp) < 0)
    {
      goto ctrller_close;
    }

    sblk++;
    dbuff += SDMMC_BLK_SIZE / sizeof(UNS_32);
  }
#ifdef MMCWRITE
  /* Write data blocks back to the SD card */
  sblk = BLKFIRST;
  dbuff = databuff;
  for (blk = 0; blk < BLKSTOTRANFER; blk++)
  {
    if (sdmmc_write_block(dbuff, 1,
                          (sblk * SDMMC_BLK_SIZE), &resp) < 0)
    {
      goto ctrller_close;
    }

    sblk++;
    dbuff += SDMMC_BLK_SIZE / sizeof(UNS_32);
  }
#endif

  /* Deselect card */
  sdmmc_cmd_send(SDMMC_SELECT_CARD, 0, &resp);

  /* Place card into idle to shut it down */
  sdmmc_cmd_send(SDMMC_IDLE, 0, &resp);

ctrller_close:
  sdmmc_close();

  /* Enable SDMMC power */
  phy3250_sdpower_enable(FALSE);

  /* Disable interrupts in ARM core */
  disable_irq_fiq();
}
