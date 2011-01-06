/***********************************************************************
 * $Id:: sysapi_misc.c 5065 2010-09-28 19:58:10Z usb10132              $
 *
 * Project: Command processor
 *
 * Description:
 *     Implements the following functions required for the S1L API
 *         ucmd_init
 *         cmd_info_extend
 *         get_seconds
 *         sys_up
 *         sys_down
 *         get_rt_s1lsys_cfg
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
#include "lpc_arm922t_cp15_driver.h"
#include "lpc_irq_fiq.h"
#include "lpc_string.h"
#include "lpc32xx_emc.h"
#include "lpc32xx_rtc.h"
#include "sys.h"
#include "s1l_cmds.h"
#include "s1l_sys_inf.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_mstimer_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "startup.h"
#include "dram_configs.h"
#include "phy3250_board.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Millisecond timer device handles */
static INT_32 mstimerdev;

volatile UNS_32 tick_10ms;

static UNS_8 mmu1_msg[] = "MMU : Enabled";
static UNS_8 mmu0_msg[] = "MMU : Disabled";
static UNS_8 armclk_msg[]  = "ARM system clock (Hz) = ";
static UNS_8 hclkclk_msg[] = "HCLK (Hz)             = ";
static UNS_8 pclkclk_msg[] = "Peripheral clock (Hz) = ";

/* clock command */
BOOL_32 cmd_clock(void);
static UNS_32 cmd_clock_plist[] =
{
	(PARSE_TYPE_STR), /* The "clock" command */
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC),
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T hw_clock_cmd =
{
	(UNS_8 *) "clock",
	cmd_clock,
	(UNS_8 *) "Sets the system clock frequencies",
	(UNS_8 *) "clock [ARM clock (MHz)][HCLK divider(1, 2, or 4)][PCLK divider(1-32)]",
	cmd_clock_plist,
	NULL
};

/* bberase command */
BOOL_32 cmd_bberase(void);
static UNS_32 cmd_bberase_plist[] =
{
	PARSE_TYPE_STR, /* The "bberase" command */
	(PARSE_TYPE_DEC | PARSE_TYPE_END)
};
static CMD_ROUTE_T core_bberase_cmd =
{
	(UNS_8 *) "bberase",
	cmd_bberase,
	(UNS_8 *) "Sets the bad block erase option for the erase command",
	(UNS_8 *) "bberase [0 = skip bad blocks, 1 = erase bad blocks]",
	cmd_bberase_plist,
	NULL
};

BOOL_32 cmd_reset(void);
static UNS_32 cmd_reset_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_END) /* The reset" command */
};
static CMD_ROUTE_T core_reset_cmd =
{
	(UNS_8 *) "reset",
	cmd_reset,
	(UNS_8 *) "Resets the system",
	(UNS_8 *) "reset",
	cmd_reset_plist,
	NULL
};

static UNS_8 clockerr_msg[] = "Error: ARM clock rate must be between "
	"33MHz and 550MHz";
static UNS_8 clock_msg[] = "New programmed ARM clock frequency is ";
static UNS_8 clockbad_msg[] =
	"Error couldn't find a close speed for that rate";

#ifdef ENABLE_DEBUG

/* rbswap command */
BOOL_32 cmd_rbswap(void);
static UNS_32 cmd_rbswap_plist[] =
{
	(PARSE_TYPE_STR | PARSE_TYPE_END) /* The "rbswap" command */
};
static CMD_ROUTE_T core_rbswap_cmd =
{
	(UNS_8 *) "rbswap",
	cmd_rbswap,
	(UNS_8 *) "Switches low power or performance SDRAM mode",
	(UNS_8 *) "rbswap",
	cmd_rbswap_plist,
	NULL
};
	
/***********************************************************************
 *
 * Function: output_char_multiple
 *
 * Purpose: Outputs a string of the same character
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     value: Character value to output
 *     rep  : Number of times to repeat the character
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void output_char_multiple(UNS_8 value, int rep)
{
	UNS_8 ch[2];

	ch[0] = value;
	ch[1] = '\0';
	while (rep > 0)
	{
		term_dat_out(ch);
		rep--;
	}
}

/***********************************************************************
 *
 * Function: Output_bit_multiple
 *
 * Purpose: Outputs a string of the same character
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     value: Value to perform bit operations on
 *     bits : Number of bits to output
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void Output_bit_multiple(UNS_32 value, int bits)
{
	int shift = bits - 1;
	UNS_8 ch0[] = "0";
	UNS_8 ch1[] = "1";

	while (bits > 0)
	{
		if (value & (1 << shift))
		{
			term_dat_out(ch1);
		}
		else
		{
			term_dat_out(ch0);
		}
		bits--;
		shift--;
	}
}

/***********************************************************************
 *
 * Function: output_formatted_mode
 *
 * Purpose: Outputs a formatted mode word in bitwise format
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     modestr: Mode string
 *     mode   : Mode word to output
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void output_formatted_mode(char *modestr, UNS_32 mode)
{
	UNS_32 hiperfmode =
		((EMC->emcdynamicconfig0 & ((1 << 5) << 7)) == 0);

	/* Output mode word name */
	term_dat_out_crlf((UNS_8 *) modestr);

	/* Depending on the current mode, the bus and rows may be swapped */
	term_dat_out((UNS_8 *) "  ");
	if (hiperfmode)
	{
		/* Row - bank - column mode */
		output_char_multiple((UNS_8) 'R', SDRAM_ROWS);
		term_dat_out((UNS_8 *) " ");
		output_char_multiple((UNS_8) 'B', SDRAM_BANK_BITS);
		term_dat_out((UNS_8 *) " ");
		output_char_multiple((UNS_8) 'C', SDRAM_COLS);
		term_dat_out((UNS_8 *) " ");
		output_char_multiple((UNS_8) '0', (bus32 + 1));
		term_dat_out_crlf((UNS_8 *) "");
		term_dat_out((UNS_8 *) "  ");

		Output_bit_multiple((mode >> modeshift), SDRAM_ROWS);
		term_dat_out((UNS_8 *) " ");
		Output_bit_multiple((mode >> bankshift), SDRAM_BANK_BITS);
	}
	else
	{
		/* Bank - row - column mode */
		output_char_multiple((UNS_8) 'B', SDRAM_BANK_BITS);
		term_dat_out((UNS_8 *) " ");
		output_char_multiple((UNS_8) 'R', SDRAM_ROWS);
		term_dat_out((UNS_8 *) " ");
		output_char_multiple((UNS_8) 'C', SDRAM_COLS);
		term_dat_out((UNS_8 *) " ");
		output_char_multiple((UNS_8) '0', (bus32 + 1));
		term_dat_out_crlf((UNS_8 *) "");
		term_dat_out((UNS_8 *) "  ");

		Output_bit_multiple((mode >> bankshift), SDRAM_BANK_BITS);
		term_dat_out((UNS_8 *) " ");
		Output_bit_multiple((mode >> modeshift), SDRAM_ROWS);
	}

	/* Dump columns */
	term_dat_out((UNS_8 *) " ");
	Output_bit_multiple((mode >> (bus32 + 1)), SDRAM_COLS);
	term_dat_out((UNS_8 *) " ");
	output_char_multiple((UNS_8) '0', (bus32 + 1));

	term_dat_out_crlf((UNS_8 *) "");
}

/***********************************************************************
 *
 * Function: cmd_rbswap
 *
 * Purpose: Swaps BRC and RBC modes in the SDRAM controller
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
BOOL_32 cmd_rbswap(void)
{
	UNS_32 emsdynstate = EMC->emcdynamicconfig0;

	if (emsdynstate & ((1 << 5) << 7))
	{
		EMC->emcdynamicconfig0 = emsdynstate & ~((1 << 5) << 7);
	}
	else
	{
		EMC->emcdynamicconfig0 = emsdynstate | ((1 << 5) << 7);
	}

	return TRUE;
}
#endif

/***********************************************************************
 *
 * Function: cmd_clock
 *
 * Purpose: Clock command
 *
 * Processing:
 *     Parse the string elements for the clock command and sets the
 *     new clock value.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the command was processed, otherwise FALSE
 *
 * Notes: May not work in DDR systems.
 *
 **********************************************************************/
BOOL_32 cmd_clock(void) 
{
	UNS_32 freqr, armclk, hclkdiv, pclkdiv;
	UNS_8 dclk [32];
	BOOL_32 processed = TRUE;

	/* Get arguments */
	armclk = cmd_get_field_val(1);
	hclkdiv = cmd_get_field_val(2);
	pclkdiv = cmd_get_field_val(3);

	if (!((hclkdiv == 1) || (hclkdiv == 2) || (hclkdiv == 4))) 
	{
		processed = FALSE;
	}
	if ((pclkdiv < 1) || (pclkdiv > 32)) 
	{
		processed = FALSE;
	}
	if ((armclk < 33) || (armclk > 550))
	{
		term_dat_out_crlf(clockerr_msg);
		processed = FALSE;
	}
	else 
	{
		armclk = armclk * 1000 * 1000;
	}

	if (processed == TRUE) 
	{
		s1l_board_cfg.armclk = armclk;
		s1l_board_cfg.hclkdiv = hclkdiv;
		s1l_board_cfg.pclkdiv = pclkdiv;
		freqr = clock_adjust();
		if (freqr == 0)
		{
			term_dat_out_crlf(clockbad_msg);
		}
		else
		{
			term_dat_out(clock_msg);
			str_makedec(dclk, freqr);
			term_dat_out_crlf(dclk);
			cfg_save(&syscfg);
		}
	}

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_bberase
 *
 * Purpose: bberase command
 *
 * Processing:
 *     See function
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: TRUE if the command was processed, otherwise FALSE
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 cmd_bberase(void) 
{
	erasebadblocks = cmd_get_field_val(1);

	return TRUE;
}

/***********************************************************************
 *
 * Function: cmd_reset
 *
 * Purpose: Resets the system
 *
 * Processing:
 *     Calsl board level reset function.
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
 BOOL_32 cmd_reset(void) {
	/* Function never returns */
	board_reset();

	return TRUE;
}

/***********************************************************************
 *
 * Function: mstimer_user_interrupt
 *
 * Purpose: MSTIMER interrupt handler
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
static void mstimer_user_interrupt(void)
{
    /* Clear latched mstimer interrupt */
    mstimer_ioctl(mstimerdev, MST_CLEAR_INT, 0);

	tick_10ms++;
}

/***********************************************************************
 *
 * Function: ucmd_init
 *
 * Purpose: Initialize user commands
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
void ucmd_init(void)
{
	mmu_cmd_group_init();

#ifdef ENABLE_DEBUG
	cmd_add_new_command(&core_group, &core_rbswap_cmd);
#endif
	cmd_add_new_command(&core_group, &hw_clock_cmd);
	cmd_add_new_command(&core_group, &core_bberase_cmd);
	cmd_add_new_command(&core_group, &core_reset_cmd);
}

/***********************************************************************
 *
 * Function: get_seconds
 *
 * Purpose: Get current hardware count value (in seconds)
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: A second reference value
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 get_seconds(void)
{
	return RTC->ucount;
}

/***********************************************************************
 *
 * Function: get_rt_s1lsys_cfg
 *
 * Purpose: Populate runtime system configuration data
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     psysrt : Pointer to structure to populate
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void get_rt_s1lsys_cfg(S1L_SYS_RTCFG_T *psysrt)
{
	str_copy(psysrt->default_prompt, DEFPROMPT);
	str_copy(psysrt->system_name, SYSHEADER);
	psysrt->bl_num_blks = BL_NUM_BLOCKS;
	psysrt->app_num_blks = FLASHAPP_MAX_BLOCKS;
}

/***********************************************************************
 *
 * Function: sys_up
 *
 * Purpose: Sets up system
 *
 * Processing:
 *     Sets up MS timer to generate an interrupt at 2Hz.
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
void sys_up(void)
{
	MST_MATCH_SETUP_T mstp;

    /* Disable interrupts in ARM core */
    disable_irq_fiq();

    /* Set virtual address of MMU table in last 16K of IRAM */
    cp15_set_vmmu_addr((void *)
		(IRAM_BASE + (IRAM_SIZE) - (16 * 1024)));

	/* Initialize interrupt system */
    int_initialize(0xFFFFFFFF);

    /* Install standard IRQ dispatcher at ARM IRQ vector */
    int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

	/* Install exception handler */
    int_install_arm_vec_handler(UNDEFINED_INST_VEC, (PFV) arm9_exchand);
    int_install_arm_vec_handler(PREFETCH_ABORT_VEC, (PFV) arm9_exchand);
    int_install_arm_vec_handler(DATA_ABORT_VEC, (PFV) arm9_exchand);

	dcache_flush();
	icache_inval();
    enable_irq_fiq();

    /* Install mstimer interrupts handlers as a IRQ interrupts */
    int_install_irq_handler(IRQ_MSTIMER, (PFV) mstimer_user_interrupt);

    /* Setup MS timer */
    mstimerdev = mstimer_open(MSTIMER, 0);
	if (mstimerdev != 0)
	{
		/* Set a 10mS match rate */
		mstp.timer_num      = 0;
		mstp.use_int        = TRUE;
		mstp.stop_on_match  = FALSE;
		mstp.reset_on_match = TRUE;
		mstp.tick_val       = 0;
		mstp.ms_val         = 10;
		mstimer_ioctl(mstimerdev, MST_CLEAR_INT, 0);
		mstimer_ioctl(mstimerdev, MST_TMR_SETUP_MATCH, (INT_32) &mstp);

		/* Reset terminal count */
		mstimer_ioctl(mstimerdev, MST_TMR_RESET, 0);

		/* Enable mstimer (starts counting) */
	    mstimer_ioctl(mstimerdev, MST_TMR_ENABLE, 1);

		/* Enable mstimer interrupts in the interrupt controller */
		int_enable(IRQ_MSTIMER);
	}

	/* Initialize terminal I/O */
	term_init();
}

/***********************************************************************
 *
 * Function: sys_down
 *
 * Purpose: Closes down system
 *
 * Processing:
 *     Disable MS timer and timer interrupt.
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
void sys_down(void)
{
	/* Shut down terminal I/O */
	term_deinit();

	/* Disable mstimer interrupts in the interrupt controller */
	int_disable(IRQ_MSTIMER);

	/* Close mstimer */
    mstimer_close(mstimerdev);

    /* Disable interrupts in ARM core */
    disable_irq_fiq();
}

/***********************************************************************
 *
 * Function: cmd_info_extend
 *
 * Purpose: Extended data on the info command
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
void cmd_info_extend(void)
{
	UNS_8 str [128];
	UNS_32 tmp;

	/* MMU enabled? */
	if (cp15_mmu_enabled() == FALSE) 
	{
		term_dat_out_crlf(mmu0_msg);
	}
	else 
	{
		term_dat_out_crlf(mmu1_msg);
	}

	/* System clock speeds */
	term_dat_out(armclk_msg);
	tmp = clkpwr_get_base_clock_rate(CLKPWR_ARM_CLK);
	str_makedec(str, tmp);
	term_dat_out_crlf(str);
	term_dat_out(hclkclk_msg);
	tmp = clkpwr_get_base_clock_rate(CLKPWR_HCLK);
	str_makedec(str, tmp);
	term_dat_out_crlf(str);
	term_dat_out(pclkclk_msg);
	tmp = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);
	str_makedec(str, tmp);
	term_dat_out_crlf(str);

#ifdef ENABLE_DEBUG
	/* Also dump SDRAM information */
	term_dat_out((UNS_8 *) "Rows/Columns/Bank bits : ");
	str_makehex(str, (UNS_32) SDRAM_ROWS, 1);
	term_dat_out(str);
	term_dat_out((UNS_8 *) "/");
	str_makehex(str, (UNS_32) SDRAM_COLS, 1);
	term_dat_out(str);
	term_dat_out((UNS_8 *) "/");
	str_makehex(str, (UNS_32) SDRAM_BANK_BITS, 1);
	term_dat_out_crlf(str);
	term_dat_out((UNS_8 *) "SDRAM size (bytes)     : ");
	str_makehex(str, SDRAM_SIZE, 8);
	term_dat_out_crlf(str);
	term_dat_out((UNS_8 *) "DRAM performance mode  : ");
	tmp = ((EMC->emcdynamicconfig0 & ((1 << 5) << 7)) == 0);
	str_makehex(str, tmp, 1);
	term_dat_out_crlf(str);
	term_dat_out((UNS_8 *) "SDRAM address mapping  : ");
	str_makehex(str, sdram_find_config(), 2);
	term_dat_out_crlf(str);
	term_dat_out((UNS_8 *) "Mode shift value       : ");
	str_makehex(str, (UNS_32) modeshift, 2);
	term_dat_out_crlf(str);
	term_dat_out((UNS_8 *) "Bank shift value       : ");
	str_makehex(str, (UNS_32) bankshift, 2);
	term_dat_out_crlf(str);

	/* These values are only valid if the stage 1 application initializes
	   SDRAM. So it's possible they may be 0. */
	term_dat_out((UNS_8 *) "Mode write address     : ");
	str_makehex(str, modeaddr, 8);
	term_dat_out_crlf(str);
	term_dat_out((UNS_8 *) "Ext mode write address : ");
	str_makehex(str, extmodeaddr, 8);
	term_dat_out_crlf(str);
	output_formatted_mode("Mode word", modeaddr);
	output_formatted_mode("Ext Mode word", extmodeaddr);
#endif
}

/***********************************************************************
 *
 * Function: jumptoprog
 *
 * Purpose: Jump to passed address
 *
 * Processing:
 *     Jump to passed address (execute program), requires cache flush
 *     on some systems
 *
 * Parameters:
 *     progaddr: Address to jump to
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void jumptoprog(PFV progaddr)
{
	/* Execute new program */
	dcache_flush();
	icache_inval();
	progaddr();
}
