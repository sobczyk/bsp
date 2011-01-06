/***********************************************************************
 * $Id:: ddr_st_sdram.c 3377 2010-05-05 22:29:38Z usb10132             $
 *
 * Project: Standard (2.5v) DDR setup code
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

#include "startup.h"
#include "dram_configs.h"
#include "lpc32xx_emc.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_timer_driver.h"

/***********************************************************************
 *
 * Function: ddr_if_init
 *
 * Purpose: Sets up DDR interface and initial calibration
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *    cfg: Dynamic configuration value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
static void ddr_if_init(UNS_32 cfg)
{
	UNS_32 tmp, ringosccount;
	int idx;

	/* Use nominal rate for DDR clocking */
	tmp = CLKPWR->clkpwr_hclk_div & ~(0x3 << 0x7);
	CLKPWR->clkpwr_hclk_div = tmp | CLKPWR_HCLKDIV_DDRCLK_NORM;

	/* Resync DDR clocks */
	ddr_clock_resync(cfg);

	/* Enable calibration logic with low calibration sensitivity and
	   a guessed first DQSIN delay value. This is just temporary and
	   is used for the next step. */
	CLKPWR->clkpwr_sdramclk_ctrl |= CLKPWR_SDRCLK_USE_CAL |
		CLKPWR_SDRCLK_SENS_FACT(7) | CLKPWR_SDRCLK_DQS_DLY(0xF);

	/* Force calibration 10 times and save the average value */
	ringosccount = 0;
	for (idx = 0; idx < 10; idx++)
	{
		CLKPWR->clkpwr_sdramclk_ctrl |= CLKPWR_SDRCLK_DO_CAL;
		CLKPWR->clkpwr_sdramclk_ctrl &= ~CLKPWR_SDRCLK_DO_CAL;
		timer_wait_us(TIMER_CNTR0, 25);
		ringosccount += CLKPWR->clkpwr_ddr_lap_count;
	}

	/* use the average for the nominal ring oscillator value */
	CLKPWR->clkpwr_ddr_lap_nom = ringosccount / 10;

	/* Enable automatic RTC tick calibration, but keep calibration off
	   for now until uncalibrated DQS delay is found. */
	CLKPWR->clkpwr_sdramclk_ctrl |= CLKPWR_SDRCLK_CAL_ON_RTC;
	CLKPWR->clkpwr_sdramclk_ctrl &= ~CLKPWR_SDRCLK_USE_CAL;

	/* Setup CAS and RAS latencies */
	EMC->emcdynamicrascas0 =
		EMC_SET_CAS_IN_HALF_CYCLES(SDRAM_CAS_LATENCY) |
		EMC_SET_RAS_IN_CYCLES(SDRAM_RAS_LATENCY);

#ifdef USE_DUAL_SDRAM_DEVICES
	EMC->emcdynamicrascas1 =
		EMC_SET_CAS_IN_HALF_CYCLES(SDRAM_CAS_LATENCY) |
		EMC_SET_RAS_IN_CYCLES(SDRAM_RAS_LATENCY);
#endif

	/* Setup DDR read strategy */
	EMC->emcdynamicreadconfig = (EMC_SDR_CLK_NODLY_CMD_DEL |
	    EMC_SDR_READCAP_POS_POL | EMC_DDR_CLK_NODLY_CMD_DEL |
		EMC_DDR_READCAP_POS_POL);

	/* Setup slew rates */
#ifdef SDRAM_USE_SLOW_SLEW
	CLKPWR->clkpwr_sdramclk_ctrl |= (CLKPWR_SDRCLK_SLOWSLEW_CLK |
		CLKPWR_SDRCLK_SLOWSLEW | CLKPWR_SDRCLK_SLOWSLEW_DAT);

#else
	CLKPWR->clkpwr_sdramclk_ctrl &= ~(CLKPWR_SDRCLK_SLOWSLEW_CLK |
		CLKPWR_SDRCLK_SLOWSLEW | CLKPWR_SDRCLK_SLOWSLEW_DAT);
#endif
}

/***********************************************************************
 *
 * Function: ddr_sdram_st_setup
 *
 * Purpose: Setup and calibrate DDR
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clk : Base SDRAM controller clock rate
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void ddr_sdram_st_setup(UNS_32 clk)
{
	UNS_32 tmp;
	volatile UNS_16 tmp16;

	/* Enables DDR clocking mode, (15*0.25nS) HCLK delay
	   Tests of HCLK delay have shown that voltage and frequency are
	   not driving factors for it's timing. Over testing, a working
	   value of 3 to 27 (average) appears to work and not effect other
	   operation. For this value, a hardcoded value of 15 is used. */
    CLKPWR->clkpwr_sdramclk_ctrl = CLKPWR_SDRCLK_USE_DDR;
    CLKPWR->clkpwr_sdramclk_ctrl = CLKPWR_SDRCLK_USE_DDR |
		CLKPWR_SDRCLK_HCLK_DLY(15);

	/* 16-bit interface for DDR memory. There's no point in using a
	   32-bit interface, as several high data lines are used for DDR
	   control signals, but this is optional */
	EMC->emcstatic_regs[0].emcstaticconfig =
		EMC->emcstatic_regs[1].emcstaticconfig =
		(EMC_STC_MEMWIDTH_16 | EMC_STC_BLS_EN_BIT);

	tmp = sdram_find_config();
	if (!tmp)
	{
		/* Nothing matches, exit, DRAM won't work */
		return;
	}

	/* Setup DDR interface */
	ddr_if_init(tmp);

	/* Setup interface timing */
	sdram_adjust_timing(clk);

	/* Issue NOP with Clock enables active */
	tmp = (EMC_DYN_CLK_ALWAYS_ON | EMC_DYN_CLKEN_ALWAYS_ON);
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_NOP_MODE);
	timer_wait_us(TIMER_CNTR0, 200);

	/* Issue a precharge all command */
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_PALL_MODE);

	/* Fast dynamic refresh */
	EMC->emcdynamicrefresh = EMC_DYN_REFRESH_IVAL(4);
	timer_wait_us(TIMER_CNTR0, 10);

	/* Extended mode word */
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_CMD_MODE);
	tmp16 = * (volatile UNS_16 *) (EMC_DYCS0_BASE +
		(SDRAM_EXT_MODE_WORD << modeshift) + (SDRAM_EXT_MODE_BB));
#ifdef ENABLE_DEBUG
		extmodeaddr = EMC_DYCS0_BASE + SDRAM_EXT_MODE_BB +
			(SDRAM_EXT_MODE_WORD << modeshift);
#endif
	timer_wait_us(TIMER_CNTR0, 1);

	/* Note special code for DLL reset here */
	/* Issue load mode command and normal mode word with DLL reset */
	tmp16 = * (volatile UNS_16 *) (EMC_DYCS0_BASE +
		(((2 << 7) + SDRAM_MODE_WORD) << modeshift));
#ifdef ENABLE_DEBUG
		modeaddr = EMC_DYCS0_BASE + (SDRAM_MODE_WORD << modeshift);
#endif
	timer_wait_us(TIMER_CNTR0, 1);

#ifdef USE_DUAL_SDRAM_DEVICES
	/* Also do this for SDRAM chip select 1 if devices are present */
	tmp16 = * (volatile UNS_16 *) (EMC_DYCS0_BASE +
		(SDRAM_EXT_MODE_WORD << modeshift) + (SDRAM_EXT_MODE_BB));
	timer_wait_us(TIMER_CNTR0, 1);

	tmp16 = * (volatile UNS_16 *) (EMC_DYCS0_BASE +
		(((2 << 7) + SDRAM_MODE_WORD) << modeshift));
	timer_wait_us(TIMER_CNTR0, 1);
#endif

	/* Issue a precharge all command and wait */
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_PALL_MODE);
	timer_wait_us(TIMER_CNTR0, 10);

	/* Fast dynamic refresh (at least 2 autorefresh commands) */
	timer_wait_us(TIMER_CNTR0, 25);

	/* Compute nominal refresh period of DDR for current clock */
	EMC->emcdynamicrefresh =
		EMC_DYN_REFRESH_IVAL(clk / SDRAM_RFSH_INTERVAL);

	/* Issue load mode command and normal mode word with DLL clear */
	EMC->emcdynamiccontrol = (tmp | EMC_DYN_CMD_MODE);
	tmp16 = * (volatile UNS_16 *) (EMC_DYCS0_BASE +
		(SDRAM_MODE_WORD << modeshift));
	timer_wait_us(TIMER_CNTR0, 1);

#ifdef USE_DUAL_SDRAM_DEVICES
	/* Also do this for SDRAM chip select 1 if devices are present */
	tmp16 = * (volatile UNS_16 *) (EMC_DYCS1_BASE +
		(SDRAM_MODE_WORD << modeshift));
	timer_wait_us(TIMER_CNTR0, 1);
#endif

	/* Normal DDR mode */
	EMC->emcdynamiccontrol = EMC_DYN_NORMAL_MODE |
		EMC_DYN_DIS_MEMCLK_IN_SFRSH;
	timer_wait_us(TIMER_CNTR0, 10);

	/* Find optimal DQSin delay and calibration sensitivity, tested
	   over DRAM bank 0 with a size of 64MBytes */
	ddr_find_dqsin_delay(EMC_DYCS0_BASE, SDRAM_SIZE);

	/* Enable automatic calibration */
	CLKPWR->clkpwr_sdramclk_ctrl |= CLKPWR_SDRCLK_USE_CAL;
}
