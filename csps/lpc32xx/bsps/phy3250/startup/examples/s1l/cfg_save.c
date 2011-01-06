/***********************************************************************
 * $Id:: sysapi_eeprom.c 875 2008-07-08 17:27:04Z wellsk               $
 *
 * Project: System configuration save and restore functions
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         cfg_save
 *         cfg_load
 *         cfg_override
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

#include "lpc_string.h"
#include "sys.h"
#include "s1l_sys.h"
#include "s1l_sys_inf.h"
#include "phy3250_board.h"
#include "board_spi_flash_driver.h"

#define KEYCHECKVAL 0x1352EADB

/* Offset where S1L Cfg Verification Tag is stored */
const UNS_32 cfg_check_offset = PHY3250_SEEPROM_S1L_CFG_OFF;
const UNS_32 board_cfg_offset = PHY3250_SEEPROM_S1L_CFG_OFF +
	sizeof(S1L_CFG_T);
const UNS_32 key_offset = PHY3250_SEEPROM_S1L_CFG_OFF +
	sizeof(S1L_CFG_T) + sizeof(S1L_BOARD_CFG_T);

S1L_BOARD_CFG_T s1l_board_cfg;
static UNS_32 cfg_check_data;

/***********************************************************************
 *
 * Function: cfg_save
 *
 * Purpose: Save a S1L configuration to non-volatile memory
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pCfg : Pointer to config structure to save
 *
 * Outputs: None
 *
 * Returns: Always returns TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_save(S1L_CFG_T *pCfg)
{
	UNS_32 idx;
	UNS_8 *psspcfg = (UNS_8 *) pCfg;
	UNS_8 *data = (UNS_8*)&cfg_check_data;

	/* Config SSP for correct mode */
	board_spi_config();
	cfg_check_data = KEYCHECKVAL;

	/* Write configuration structure */
	for (idx = 0; idx < sizeof(S1L_CFG_T); idx++) 
	{
		board_spi_write(*psspcfg, (cfg_check_offset + idx));
		psspcfg++;
	}

	/* Write board configuration structure */
	psspcfg = (UNS_8 *) &s1l_board_cfg;
	for (idx = 0; idx < sizeof(S1L_BOARD_CFG_T); idx++) 
	{
		board_spi_write(*psspcfg, (board_cfg_offset + idx));
		psspcfg++;
	}

	/* Write Configuration Verification Tag */
	for(idx = 0; idx < 4; idx++)
	{
		board_spi_write(data[idx], key_offset + idx);
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cfg_load
 *
 * Purpose: Load an S1L conmfiguariton from non-volatile memory
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pCfg : Pointer to config structure to populate
 *
 * Outputs: None
 *
 * Returns: FALSE if the structure wasn't loaded, otherwise TRUE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_load(S1L_CFG_T *pCfg)
{
	UNS_8 *psspcfg = (UNS_8 *) pCfg;
	UNS_8 *cfg = (UNS_8*)&cfg_check_data;
	UNS_32 idx = 0;

	/* Config SSP for correct mode */
	board_spi_config();

	/* Read Dummy Byte to Clear FIFO */
	*cfg = board_spi_read(idx);
	
	/* Read S1L Configuration & Verification Tag */
	for (idx = 0; idx < 4; idx++) 
	{
		*cfg = board_spi_read(key_offset + idx);
		cfg++;
	}

	/* Return False if Verification tag doesn't match */
	if (cfg_check_data != KEYCHECKVAL)
		return FALSE;

	/* Read configuration structure */
	for (idx = 0; idx < sizeof(S1L_CFG_T); idx++) 
	{
		*psspcfg = board_spi_read(cfg_check_offset + idx);
		psspcfg++;
	}

	/* Read board configuration structure */
	psspcfg = (UNS_8 *) &s1l_board_cfg;
	for (idx = 0; idx < sizeof(S1L_BOARD_CFG_T); idx++) 
	{
		*psspcfg = board_spi_read(board_cfg_offset + idx);
		psspcfg++;
	}

	clock_adjust();

	return TRUE;
}

/***********************************************************************
 *
 * Function: cfg_load
 *
 * Purpose:
 *     If this is returned as TRUE (when S1L is booted), the S1L
 *     configuration will return to the default configuration, can be
 *     tied to a pushbutton state.
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the config reset button is pressed, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cfg_override(void)
{
	BOOL_32 pressed = FALSE;

	if (phy3250_button_state(PHY3250_BTN1) != 0)
	{
		pressed = TRUE;
	}

	return pressed;
}

/***********************************************************************
 *
 * Function: cfg_user_reset
 *
 * Purpose: Reset user configuration data
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
void cfg_user_reset(void)
{
	s1l_board_cfg.armclk = 208000000;
	s1l_board_cfg.hclkdiv = 2;
	s1l_board_cfg.pclkdiv = 16;

	clock_adjust();

	cfg_save(&syscfg);
}
