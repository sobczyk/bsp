/***********************************************************************
 * $Id:: nand_slc_common.c 4867 2010-09-09 06:41:58Z ing03005          $
 *
 * Project: NAND SLC support functions
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

#include "nand_slc_common.h"
#include "lpc32xx_slcnand.h"
#include "lpc_nandflash_params.h"
#include "lpc_lbecc.h"

/* Layout of ECC in NAND flash OOB */
static int sp_ooblayout[] = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
static int lp_ooblayout[] = {
	40, 41, 42, 43, 44, 45,
	46, 47, 48, 49, 50, 51,
	52, 53, 54, 55, 56, 57,
	58, 59, 60, 61, 62, 63
	};

/***********************************************************************
 *
 * Function: nand_slc_bit_cnt16
 *
 * Purpose: Support function for bit counts
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ch : Value to check
 *
 * Outputs: None
 *
 * Returns: computed value
 *
 * Notes: None
 *
 **********************************************************************/
static UNS_8 nand_slc_bit_cnt16(UNS_16 ch)
{
    ch = (ch & 0x5555) + ((ch & ~0x5555) >> 1);
    ch = (ch & 0x3333) + ((ch & ~0x3333) >> 2);
    ch = (ch & 0x0F0F) + ((ch & ~0x0F0F) >> 4);

	return (ch + (ch >> 8)) & 0xFF;
}

/***********************************************************************
 *
 * Function: nand_slc_bit_cnt32
 *
 * Purpose: Counts the number of bits set in a given UINT_32
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     val: int for which the bits to be counted
 *
 * Outputs: None
 *
 * Returns: Number of bits set in val
 *
 * Notes: None
 *
 **********************************************************************/
static UNS_8 bit_cnt32(UNS_32 val)
{
    return nand_slc_bit_cnt16(val & 0xFFFF) +
		nand_slc_bit_cnt16(val >> 16);
}

/***********************************************************************
 *
 * Function: slc_cmd
 *
 * Purpose: Issue a command to the SLC NAND device
 *
 * Processing:
 *     Write the command to the SLC command register.
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
void slc_cmd(UNS_8 cmd)
{
	SLCNAND->slc_cmd = (UNS_32) cmd;
}

/***********************************************************************
 *
 * Function: slc_addr
 *
 * Purpose: Issue a address to the SLC NAND device
 *
 * Processing:
 *     Write the address to the SLC address register.
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
void slc_addr(UNS_8 addr)
{
	SLCNAND->slc_addr = (UNS_32) addr;
}

/***********************************************************************
 *
 * Function: slc_wait_ready
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
void slc_wait_ready(void)
{
	while ((SLCNAND->slc_stat & SLCSTAT_NAND_READY) == 0);
}

/***********************************************************************
 *
 * Function: slc_sb_set_cs
 *
 * Purpose: Assert or deassert NAND chip select state
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     low: TRUE to set low, FALSE to set high
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void slc_sb_set_cs(BOOL_32 low)
{
	if (low == TRUE)
	{
		SLCNAND->slc_cfg |= SLCCFG_CE_LOW;
	}
	else
	{
		SLCNAND->slc_cfg &= ~SLCCFG_CE_LOW;
	}
}

/***********************************************************************
 *
 * Function: slc_get_status
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
UNS_8 slc_get_status(void)
{
	slc_cmd(LPCNAND_CMD_STATUS);
	slc_wait_ready();

	return (UNS_8) SLCNAND->slc_data;
}

/***********************************************************************
 *
 * Function: slc_ecc_copy_to_buffer
 *
 * Purpose: Copies ECC data from 32-Bit integer array to OOB buffer
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     spare : pointer to spare area buffer where ECC will be stored
 *     ecc : ECC stored (Probably read from H/W)
 *     count : Count of ECC (8 - Large page, 2 - Small page)
 *
 * Outputs: OOB area will be filled with apropriate ECC
 *
 * Returns:
 *        0 - Failure
 *        1 - True
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slc_ecc_copy_to_buffer(UNS_8 * spare, const UNS_32 * ecc, INT_32 count)
{
	int * offset, i;
	if (count == 2){ /* SMALL Page */
		offset = sp_ooblayout;
	} else if (count == 8) { /* Large Page */
		offset = lp_ooblayout;
	} else {
		return 0;
	}

	for (i = 0; i < (count * 3); i += 3) {
		UNS_32 ce = ecc[i/3];
		ce = ~(ce << 2) & 0xFFFFFF;
		spare[offset[i+2]] = (UNS_8)(ce & 0xFF); ce >>= 8;
		spare[offset[i+1]] = (UNS_8)(ce & 0xFF); ce >>= 8;
		spare[offset[i]]   = (UNS_8)(ce & 0xFF);
	}
	return 1;
}

/***********************************************************************
 *
 * Function: slc_ecc_copy_from_buffer
 *
 * Purpose: Copies ECC data from OOB buffer into an 32-Bit integer array
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     spare : pointer to spare area buffer
 *     ecc : Memory to which the integer ECC be copied
 *     count : Count of ECC (8 - Large page, 2 - Small page)
 *
 * Outputs: ecc will be filled with 24 bit ECC data read from OOB area
 *
 * Returns:
 *        0 - Failure
 *        1 - Success
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 slc_ecc_copy_from_buffer(const UNS_8 * spare, UNS_32 * ecc, INT_32 count)
{
	int * offset, i;
	if (count == 2){ /* Small Page */
		offset = sp_ooblayout;
	} else if (count == 8) { /* Large Page */
		offset = lp_ooblayout;
	} else {
		return 0; /* FALSE */
	}

	for (i = 0; i < (count * 3); i += 3) {
		UNS_32 ce = 0;
		ce |= spare[offset[i]]; ce <<= 8;
		ce |= spare[offset[i+1]]; ce <<= 8;
		ce |= spare[offset[i+2]];
		ecc[i/3] = (~ce >> 2) & 0x3FFFFF;
	}
	return 1; /* TRUE */
}

/***********************************************************************
 *
 * Function: nand_slc_correct_ecc
 *
 * Purpose: Locate the ECC error and correct it (256 byte block)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ecc_gen : ECC generated by hardware
 *     ecc_stored : ECC stored in spare area
 *     buf : pointer to data
 *
 * Outputs: Corrected buffer data, in case of correctable ECC error
 *
 * Returns:
 *     LPC_ECC_CORRECTED      -> Error corrected 
 *     LPC_ECC_NOERR          -> No ECC error correction needed
 *     LPC_ECC_NOTCORRECTABLE -> Un-correctable error 
 *     LPC_ECC_AREA_ERR       -> Error in spare area
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 nand_slc_correct_ecc(UNS_32 *ecc_gen, UNS_32 *ecc_stored,
							UNS_8 * buf)
{
	UNS_32 tmp, err, byte, bit = 0;
	err = *ecc_stored ^ *ecc_gen;

	/* Return if no ECC errors detected */
	if (!err)
		return LPC_ECC_NOERR;

	/* Get parity error count */
	tmp = bit_cnt32(err);

	/* If Error is in Spare area return here */
	if (tmp == 1)
		return LPC_ECC_AREA_ERR;

	/*
	 * If Error is in Data area and
	 * more than 1 bits has problem,
	 * we cannot correct it
	 */
	if (tmp != 11)
		return LPC_ECC_NOTCORRECTABLE;

	/* We have a correctable error, lets do the correction */
	byte = err >> 6;
	bit = ((err & _BIT(1)) >> 1) | ((err & _BIT(3)) >> 2) |
	((err & _BIT(5)) >> 3);

	/* Calculate Byte offset */
	byte = ((byte & _BIT(1)) >> 1) | ((byte & _BIT(3)) >> 2) |
	        ((byte & _BIT(5)) >> 3) | ((byte & _BIT(7)) >> 4) |
	        ((byte & _BIT(9)) >> 5) | ((byte & _BIT(11)) >> 6) |
	        ((byte & _BIT(13)) >> 7) | ((byte & _BIT(15)) >> 8);

	/* Do the correction */
	buf[byte] ^= _BIT(bit);

	return LPC_ECC_CORRECTED;
}
