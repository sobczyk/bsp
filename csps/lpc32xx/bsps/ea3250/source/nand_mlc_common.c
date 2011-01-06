/***********************************************************************
 * $Id:: nand_mlc_common.c 3380 2010-05-05 23:54:23Z usb10132          $
 *
 * Project: Common MLC NAND support functions
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

#include "nand_mlc_common.h"
#include "lpc32xx_mlcnand.h"
#include "lpc_nandflash_params.h"

/***********************************************************************
 *
 * Function: mlc_cmd
 *
 * Purpose: Issue a command to the MLC NAND device
 *
 * Processing:
 *     Write the command to the MLC command register.
 *
 * Parameters:
 *     cmd : Command to issue
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void mlc_cmd(UNS_8 cmd)
{
	MLCNAND->mlc_cmd = (UNS_32) cmd;
}

/***********************************************************************
 *
 * Function: mlc_addr
 *
 * Purpose: Issue a address to the MLC NAND device
 *
 * Processing:
 *     Write the address to the MLC address register.
 *
 * Parameters:
 *     addr : Address to issue
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void mlc_addr(UNS_8 addr)
{
	MLCNAND->mlc_addr = (UNS_32) addr;
}

/***********************************************************************
 *
 * Function: mlc_wait_ready
 *
 * Purpose: Wait for device to go to the ready state
 *
 * Processing:
 *     Loop until the ready status is detected.
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
void mlc_wait_ready(void)
{
	while ((MLCNAND->mlc_isr & MLC_DEV_RDY_STS) == 0);
}

/***********************************************************************
 *
 * Function: mlc_get_status
 *
 * Purpose: Return the current NAND status
 *
 * Processing:
 *     Issue the read status command to the device. Wait until the
 *     device is ready. Read the current status and return it to the
 *     caller.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: The status read from the NAND device
 *
 * Notes: None
 *
 **********************************************************************/
UNS_8 mlc_get_status(void)
{
	mlc_cmd(LPCNAND_CMD_STATUS);
	mlc_wait_ready();

	return (UNS_8) MLCNAND->mlc_data [0];
}
