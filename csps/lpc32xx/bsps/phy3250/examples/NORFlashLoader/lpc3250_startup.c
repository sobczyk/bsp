/***********************************************************************
 * $Id:: lpc3250_startup.c 2442 2009-11-05 18:50:01Z wellsk            $
 *
 * Project: Phytec LPC3250 board startup code
 *
 * Description:
 *     This file contains startup code used with the Phytec
 *     LPC3250 board.
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

#include "lpc_arm922t_cp15_driver.h"
//#include "phy3250_startup.h"
#include "phy3250_board.h"
#include "lpc32xx_emc.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_gpio_driver.h"

/***********************************************************************
 * Startup code private data
 **********************************************************************/

/* Phytec LPC3250 MMU virtual mapping table */
TT_SECTION_BLOCK_T tt_init_basic[] =
{
  /* 0x00000000  0x08000000	1	cb	IRAM cached */
  {1, 0x00000000, 0x00000000,
    (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0x08000000	0x08000000	1	u	IRAM uncached */
  {1, 0x08000000, 0x08000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0x0C000000	0x0C000000	1	u	IROM uncached */
  {1, 0x0C000000, 0x0C000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0x20000000	0x20000000	0x10000000	u	Registers */
  {256, 0x20000000, 0x20000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0x30000000	0x230000000	0x10000000	u	Registers */
  {256, 0x30000000, 0x30000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0x40000000	0x40000000	0x10000000	u	Registers */
  {256, 0x40000000, 0x40000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0x80000000	0x80000000	128	u	DDR#0 cached */
  {128, 0x80000000, 0x80000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0xA0000000	0xA0000000	128	u	DDR#0 uncached */
  {128, 0xA0000000, 0x80000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0xE0000000	0xE0000000	1	u	SRAM cached */
  {8, 0xE0000000, 0xE0000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0xE0000000	0xE0000000	1	u	SRAM uncached */
  {8, 0xE8000000, 0xE0000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  /* 0xE1000000	0xE1000000	1	u	SDIO uncached */
  {1, 0xE1000000, 0xE1000000,
   (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
    ARM922T_L1D_TYPE_SECTION)},
  {0, 0, 0, 0}  // Marks end of initialization array.  Required!
};

/* MMU base table address */
TRANSTABLE_T *mmu_base_aadr;

/***********************************************************************
 * Private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: phy3250_gpio_setup
 *
 * Purpose: Setup GPIO and MUX states
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
static void phy3250_gpio_setup(void)
{
  /* P2 group muxing
     GPIO_02 / KEY_ROW6 | (ENET_MDC)      ->KEY_ROW6 | (ENET_MDC)
     GPIO_03 / KEY_ROW7 | (ENET_MDIO)     ->KEY_ROW7 | (ENET_MDIO)
     GPO_21 / U4_TX | (LCDVD[3])          ->U4_TX | (LCDVD[3])
     EMC_D_SEL                            ->(D16 ..D31 used)
     GPIO_04 / SSEL1 | (LCDVD[22])        ->SSEL1 | (LCDVD[22])
     GPIO_05 / SSEL0                      ->SSEL0 */
  GPIO->p2_mux_clr = (P2_SDRAMD19D31_GPIO | P2_GPIO05_SSEL0);
  GPIO->p2_mux_set = (P2_GPIO03_KEYROW7 | P2_GPIO02_KEYROW6 |
                       P2_GPO21_U4TX | P2_GPIO04_SSEL1);

  /* P_ group muxing
     I2S1TX_SDA / MAT3.1                  ->I2S1TX_SDA
     I2S1TX_CLK / MAT3.0                  ->I2S1TX_CLK
     I2S1TX_WS / CAP3.0                   ->I2S1TX_WS
     SPI2_DATIO / MOSI1 | (LCDVD[20])     ->MOSI1 | (LCDVD[20])
     SPI2_DATIN / MISO1 | (LCDVD[21])     ->MISO1 | (LCDVD[21])
     SPI2_CLK / SCK1 | (LCDVD[23])        ->SCK1 | (LCDVD[23])
     SPI1_DATIO / MOSI0                   ->MOSI0
     SPI1_DATIN / MISO0                   ->MISO0
     SPI1_CLK / SCK0                      ->SCK0
     (MS_BS) | MAT2.1                     ->(MS_BS)
     (MS_SCLK) | MAT2.0                   ->(MS_SCLK)
     U7_TX / MAT1.1 | (LCDVD[11])         ->MAT1.1 | (LCDVD[11])
     (MS_DIO3) | MAT0.3                   ->(MS_DIO3)
     (MS_DIO2) | MAT0.2                   ->(MS_DIO2)
     (MS_DIO1) | MAT0.1                   ->(MS_DIO1)
     (MS_DIO0) | MAT0.0                   ->(MS_DIO0) */
  GPIO->p_mux_set = (P_SPI2DATAIO_MOSI1 |
    P_SPI2DATAIN_MISO1 | P_SPI2CLK_SCK1 |
    P_SPI1DATAIO_SSP0_MOSI | P_SPI1DATAIN_SSP0_MISO |
    P_SPI1CLK_SCK0 | P_U7TX_MAT11);
  GPIO->p_mux_clr = (P_I2STXSDA1_MAT31 | P_I2STXCLK1_MAT30 |
    P_I2STXWS1_CAP30 | P_MAT20 | P_MAT20 | P_MAT03 | P_MAT02 |
    P_MAT01 | P_MAT00);

  /* P3 group muxing
     GPO_02 / MAT1.0 | (LCDVD[0])       ->GPO_02
     GPO_06 | (LCDVD[18])               ->PWM4.3 | (LCDVD[18])
     GPO_08 | (LCDVD[8])                ->PWM4.2 | (LCDVD[8])
     GPO_09 | (LCDVD[9])                ->PWM4.1 | (LCDVD[9])
     GPO_10 / MC2B | (LCDPWR)           ->PWM3.6 | (LCDPWR)
     GPO_12 / MC2A | (LCDLE)            ->PWM3.5 | (LCDLE)
     GPO_13 / MC1B | (LCDDCLK)          ->PWM3.4 | (LCDDCLK)
     GPO_15 / MC1A | (LCDFP)            ->PWM3.3 | (LCDFP)
     GPO_16 / MC0B | (LCDENAB/LCDM)     ->PWM3.2 | (LCDENAB/LCDM)
     GPO_18 / MC0A | (LCDLP)            ->PWM3.1 | (LCDLP) */
  GPIO->p3_mux_set = (P3_GPO6 | P3_GPO8 | P3_GPO9 | P3_GPO10_MC2B |
	P3_GPO12_MC2A | P3_GPO13_MC1B | P3_GPO15_MC1A | P3_GPO16_MC0B |
    P3_GPO18_MC0A);
  GPIO->p3_mux_clr = P3_GPO2_MAT10;

  /* P0 group muxing
     P0.0 / I2S1RX_CLK                    ->I2S1RX_CLK
     P0.1 / I2S1RX_WS                     ->I2S1RX_WS
     P0.2 / I2S0RX_SDA | (LCDVD[4])       ->I2S0RX_SDA | (LCDVD[4])
     P0.3 / I2S0RX_CLK | (LCDVD[5])       ->I2S0RX_CLK | (LCDVD[5])
     P0.4 / I2S0RX_WS | (LCDVD[6])        ->I2S0RX_WS | (LCDVD[6])
     P0.5 / I2S0TX_SDA | (LCDVD[7])       ->I2S0TX_SDA | (LCDVD[7])
     P0.6 / I2S0TX_CLK | (LCDVD[12])      ->I2S0TX_CLK | (LCDVD[12])
     P0.7 / I2S0TX_WS | (LCDVD[13])       ->I2S0TX_WS | (LCDVD[13])
  */
  GPIO->p0_mux_set = (P0_GPOP0_I2SRXCLK1 | P0_GPOP1_I2SRXWS1 |
    P0_GPOP2_I2SRXSDA0 | P0_GPOP3_I2SRXCLK0 | P0_GPOP4_I2SRXWS0 |
	P0_GPOP5_I2STXSDA0 | P0_GPOP6_I2STXCLK0 | P0_GPOP7_I2STXWS0);

  /* Muxing for address bus - full 24-bit address bus on
     EMC_A[0..23] */
  GPIO->p1_mux_clr = P1_ALL;

  /* Serial EEPROM SSEL signal - a GPIO is used instead of the SSP
     controlled SSEL signal, configure GPIO as a high output */
  GPIO->p2_dir_set = P2_DIR_GPIO(5);
  GPIO->p3_outp_set = P3_STATE_GPIO(5);

  /* Some GPO and GPIO states and directions needs to be setup here:
     GPO_20                      -> Output (watchdog enable) low
     GPO_19                      -> Output (NAND write protect) high
     GPO_17                      -> Output (deep sleep set) low
     GPO_11                      -> Output (deep sleep exit) low
     GPO_05                      -> Output (SDMMC power control) low
     GPO_04                      -> Output (unused) low
     GPO_02                      -> Output (audio reset) low
     GPO_01                      -> Output (LED1) low
     GPIO_1                      -> Input (MMC write protect)
     GPIO_0                      -> Input (MMC detect)
  */
  gpio_set_gpo_state(P3_STATE_GPO(19),
    (P3_STATE_GPO(20) | P3_STATE_GPO(17) | P3_STATE_GPO(11) |
    P3_STATE_GPO(5) | P3_STATE_GPO(4) | P3_STATE_GPO(2) |
    P3_STATE_GPO(1)));

  /* Set default LCD type to TFT @ 16bpp */
  clkpwr_setup_lcd(CLKPWR_LCDMUX_TFT16, 1);

  /* Select RMII ethernet interface */
  clkpwr_select_enet_iface(0, 1);
}

/***********************************************************************
 *
 * Function: phy3250_clock_setup
 *
 * Purpose: Setup system clocking
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
static void phy3250_clock_setup(void)
{
  CLKPWR_HCLK_PLL_SETUP_T pllcfg;

  /* Is the PLL397 or the oscillator being used for SYSCLK? */
  if (clkpwr_get_osc() == CLKPWR_PLL397_OSC)
  {
    /* PLL397 is being used, try to switch to the main oscillator */

    /* Enable the main oscillator */
    clkpwr_mainosc_setup(0, 1);

    /* Wait 100mS to allow the oscillator to power up */
    timer_wait_ms(TIMER_CNTR0, 100);

    /* Switch over the main oscillator and disable PLL397 */
    clkpwr_sysclk_setup(CLKPWR_MAIN_OSC, 0x50);
    clkpwr_pll397_setup(0, 0, 0);
  }
  else
  {
    /* Set bad phase timing only */
    clkpwr_sysclk_setup(CLKPWR_MAIN_OSC, 0x50);
  }

  /* Setup the HCLK PLL for 208MHz operation, but if a configuration
     can't be found, stay in direct run mode */
  if (clkpwr_find_pll_cfg(MAIN_OSC_FREQ, 208000000, 1, &pllcfg) != 0)
  {
    /* PLL configuration is valid, so program the PLL with the
       computed configuration data */
    clkpwr_hclkpll_setup(&pllcfg);

    /* Wait for PLL to lock */
    while (clkpwr_is_pll_locked(CLKPWR_HCLK_PLL) == 0);

    /* DDR divider is 2, PERIPH_CLK divider is 16, and HCLK divider
       is 2 */
    clkpwr_set_hclk_divs(CLKPWR_HCLKDIV_DDRCLK_STOP, 16, 2);

    /* Switch to run mode - the ARM core clock is HCLK_PLL (208MHz),
       HCLK is (HCLK_PLL / 2), and PERIPH_CLK is (HCLK / 16) */
    clkpwr_force_arm_hclk_to_pclk(0);
    clkpwr_set_mode(CLKPWR_MD_RUN);
  }
}

/***********************************************************************
 *
 * Function: phy3250_dram_init
 *
 * Purpose: Setup DDR
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
void phy3250_dram_init(void)
{
  int idx;
  volatile UNS_32 tmp, tmp32;

  /* Set HCLK delay */
  CLKPWR->clkpwr_sdramclk_ctrl = CLKPWR_SDRCLK_HCLK_DLY(0x7);

  /* Enable normal power mode, little-endian mode, start clocks,
     disable clock enables and self-refresh mode */
  EMC->emccontrol = EMC_DYN_SDRAM_CTRL_EN;
  EMC->emcconfig = 0;

  idx = (phyhwdesc.dramcfg & PHYHW_DRAM_SIZE_MASK) >> 2;
  EMC->emcdynamicconfig0 = dram_cfg [idx].dyncfgword << 7;
  if ((phyhwdesc.dramcfg & PHYHW_DRAM_TYPE_MASK) ==
      PHYHW_DRAM_TYPE_LPSDRAM)
  {
    EMC->emcdynamicconfig0 |= EMC_DYN_DEV_LP_SDR_SDRAM;
  }

  /* Setup CAS timing and clock edge config */
  EMC->emcdynamicrascas0 =
    (EMC_SET_CAS_IN_HALF_CYCLES(dram_cfg [idx].cas) |
     EMC_SET_RAS_IN_CYCLES(dram_cfg [idx].ras));
  EMC->emcdynamicreadconfig = (EMC_SDR_CLK_NODLY_CMD_DEL |
    EMC_SDR_READCAP_POS_POL | EMC_SDR_CLK_DLY_CMD_NODELY);

  /* Setup SDRAM timing for current HCLK clock settings */
  sdram_adjust_timing(&dram_cfg [idx]);
  timer_wait_us(TIMER_CNTR0, 10);

  /* NOP mode with clocks running for 100uS */
  tmp = (EMC_DYN_CLK_ALWAYS_ON | EMC_DYN_CLKEN_ALWAYS_ON |
         EMC_DYN_DIS_INV_MEMCLK);
  EMC->emcdynamiccontrol = (tmp | EMC_DYN_NOP_MODE);
  timer_wait_us(TIMER_CNTR0, 100);

  /* Issue a precharge all command */
  EMC->emcdynamiccontrol = (tmp | EMC_DYN_PALL_MODE);

  /* Fast dynamic refresh for at least a few SDRAM clock cycles */
  EMC->emcdynamicrefresh = EMC_DYN_REFRESH_IVAL(128);
  timer_wait_us(TIMER_CNTR0, 10);

  /* Set correct DRAM refresh timing */
  sdram_adjust_timing(&dram_cfg [idx]);
  timer_wait_us(TIMER_CNTR0, 10);

  /* Issue load mode command and normal mode word */
  EMC->emcdynamiccontrol = (tmp | EMC_DYN_CMD_MODE);
  tmp32 = * (volatile UNS_32 *)(EMC_DYCS0_BASE +
                                dram_cfg [idx].modeword);

  /* Issue load mode command and extended mode word */
  EMC->emcdynamiccontrol = (tmp | EMC_DYN_CMD_MODE);
  tmp32 = * (volatile UNS_32 *)(EMC_DYCS0_BASE +
                                dram_cfg [idx].emodeword);

  /* Normal DDR mode */
  EMC->emcdynamiccontrol = (EMC_DYN_NORMAL_MODE |
                            EMC_DYN_DIS_INV_MEMCLK);
}

/***********************************************************************
 *
 * Function: phy3250_mem_setup
 *
 * Purpose: Setup memory
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
static void phy3250_mem_setup(void)
{
  /* Mirror IRAM at address 0x0 */
  CLKPWR->clkpwr_bootmap = CLKPWR_BOOTMAP_SEL_BIT;

  /* Enable HCLK and SDRAM bus clocks */
  clkpwr_clk_en_dis(CLKPWR_SDRAMDDR_CLK, 1);

  /* Enable buffers in AHB ports */
  EMC->emcahn_regs [0].emcahbcontrol = EMC_AHB_PORTBUFF_EN;
  EMC->emcahn_regs [2].emcahbcontrol = EMC_AHB_PORTBUFF_EN;
  EMC->emcahn_regs [3].emcahbcontrol = EMC_AHB_PORTBUFF_EN;
  EMC->emcahn_regs [4].emcahbcontrol = EMC_AHB_PORTBUFF_EN;

  /* Enable port timeouts */
  EMC->emcahn_regs [0].emcahbtimeout = EMC_AHB_SET_TIMEOUT(100);
  EMC->emcahn_regs [2].emcahbtimeout = EMC_AHB_SET_TIMEOUT(400);
  EMC->emcahn_regs [3].emcahbtimeout = EMC_AHB_SET_TIMEOUT(400);
  EMC->emcahn_regs [4].emcahbtimeout = EMC_AHB_SET_TIMEOUT(400);

  /* Perform DRAM initialization */
  phy3250_dram_init();

  /* Setup SRAM timing */
  sram_adjust_timing();
}

/***********************************************************************
 *
 * Function: phy3250_misc_setup
 *
 * Purpose: Miscellaneous board setup
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
void phy3250_misc_setup(void)
{
  /* Start Activation Polarity Register for all Sources */
  (*(volatile UNS_32 *)(0x4000402C)) = 0x40000000;

  /* Disable all start enables */
  CLKPWR->clkpwr_p01_er = 0;
  CLKPWR->clkpwr_int_er = 0;
  CLKPWR->clkpwr_pin_er = 0;

  /* Set default start polarities for wakeup signals */
  CLKPWR->clkpwr_int_ap = (CLKPWR_INTSRC_TS_P_BIT |
                           CLKPWR_INTSRC_RTC_BIT | CLKPWR_INTSRC_MSTIMER_BIT);
  CLKPWR->clkpwr_pin_ap = 0;

  /* Clear and active (latched) wakeup statuses */
  CLKPWR->clkpwr_int_rs = 0xFFFFFFFF;
  CLKPWR->clkpwr_pin_rs = 0xFFFFFFFF;
}

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: phy3250_init
 *
 * Purpose: Main startup code entry point, called from reset entry code
 *
 * Processing:
 *     Call the individual board init functions to setup the system.
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
void phy3250_init(void)
{
  CLKPWR_CLK_T clk;


  /* Setup GPIO and MUX states */
  phy3250_gpio_setup();

  /* Shut down all IP clocks as the default state */
  for (clk = CLKPWR_FIRST_CLK; clk < CLKPWR_LAST_CLK; clk++)
  {
	  /* Don't disable the EMC clock or we'll lose NOR FLASH */
	  if (clk != CLKPWR_SDRAMDDR_CLK) {
        clkpwr_clk_en_dis(clk, 0);
	  }
  }

  /* Before setting a higher clock speed, change the NOR timings to
     maximum so NOR works at the new clock rate */
  EMC->emcstatic_regs[0].emcstaticwaitwen = 0x1f;
  EMC->emcstatic_regs[0].emcstaticwait0en = 0x1f;
  EMC->emcstatic_regs[0].emcstaticwaitrd = 0x1f;
  EMC->emcstatic_regs[0].emcstaticpage = 0x1f;
  EMC->emcstatic_regs[0].emcstaticwr = 0x1f;
  EMC->emcstatic_regs[0].emcstaticturn = 0x1f;

  /* Setup system clocks and run mode */
  phy3250_clock_setup();

  /* Read data from EEPROM - this needs to be done here as the
     SDRAM configuration depends on these settings. */
 /* CODE DELETED BECAUSE SDRAM IS NOT NEEDED AND NOT ALL BOARDS HAVE THE EEPROM (Code hangs up here in some other boards) */

  /* Setup memory */
  phy3250_mem_setup();

  /* Setup MMU table */
  cp15_init_mmu_trans_table(mmu_base_aadr, tt_init_basic);

  /* Setup miscellaneous functions */
  phy3250_misc_setup();
}
