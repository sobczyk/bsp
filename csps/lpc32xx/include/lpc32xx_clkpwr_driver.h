/***********************************************************************
 * $Id:: lpc32xx_clkpwr_driver.h 4932 2010-09-16 00:38:51Z usb10132    $
 *
 * Project: LPC32XX Clock and Power controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32XX Clock and Power
 *     controller.
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
 *********************************************************************/

#ifndef LPC32XX_CLKPWR_DRIVER_H
#define LPC32XX_CLKPWR_DRIVER_H

#include "lpc32xx_clkpwr.h"

#ifdef __cplusplus
extern "C"
{
#endif

/* Enumeration for clock enable/disable selection and determining base
   clock rates and sources */
typedef enum
{
  CLKPWR_FIRST_CLK = 0,
  CLKPWR_USB_HCLK = CLKPWR_FIRST_CLK,
  CLKPWR_LCD_CLK,
  CLKPWR_SSP1_CLK,
  CLKPWR_SSP0_CLK,
  CLKPWR_I2S1_CLK,
  CLKPWR_I2S0_CLK,
  CLKPWR_MSCARD_CLK,
  CLKPWR_MAC_DMA_CLK,
  CLKPWR_MAC_MMIO_CLK,
  CLKPWR_MAC_HRC_CLK,
  CLKPWR_I2C2_CLK,
  CLKPWR_I2C1_CLK,
  CLKPWR_KEYSCAN_CLK,
  CLKPWR_ADC_CLK,
  CLKPWR_PWM2_CLK,
  CLKPWR_PWM1_CLK,
  CLKPWR_HSTIMER_CLK,
  CLKPWR_WDOG_CLK,
  CLKPWR_TIMER3_CLK,
  CLKPWR_TIMER2_CLK,
  CLKPWR_TIMER1_CLK,
  CLKPWR_TIMER0_CLK,
  CLKPWR_PWM4_CLK,
  CLKPWR_PWM3_CLK,
  CLKPWR_SPI2_CLK,
  CLKPWR_SPI1_CLK,
  CLKPWR_NAND_SLC_CLK,
  CLKPWR_NAND_MLC_CLK,
  CLKPWR_UART6_CLK,
  CLKPWR_UART5_CLK,
  CLKPWR_UART4_CLK,
  CLKPWR_UART3_CLK,
  CLKPWR_DMA_CLK,
  CLKPWR_SDRAMDDR_CLK,
  CLKPWR_LAST_CLK
} CLKPWR_CLK_T;

/* Main system clocks */
typedef enum
{
  /* Main oscillator clock */
  CLKPWR_MAINOSC_CLK,
  /* RTC clock */
  CLKPWR_RTC_CLK,
  /* System clock (Main oscillator or PLL397) */
  CLKPWR_SYSCLK,
  /* ARM clock, either HCLK(PLL), SYSCLK, or PERIPH_CLK */
  CLKPWR_ARM_CLK,
  /* HCLK (HCLKPLL divided, SYSCLK, or PERIPH_CLK) */
  CLKPWR_HCLK,
  /* Peripheral clock (HCLKPLL divided or SYSCLK) */
  CLKPWR_PERIPH_CLK,
  /* USB HCLK (???) */
  CLKPWR_USB_HCLK_SYS,
  /* USB PLL clock */
  CLKPWR_48M_CLK,
  /* DDR clock (HCLKPLL divided or SYSCLK) */
  CLKPWR_DDR_CLK,
  /* Sd card controller */
  CLKPWR_MSSD_CLK,
  CLKPWR_BASE_INVALID
} CLKPWR_BASE_CLOCK_T;

/* Enumeration to control output muxes of several signals */
typedef enum
{
  CLKPWR_HIGHCORE = 0,
  CLKPWR_SYSCLKEN,
  CLKPWR_TEST_CLK1,
  CLKPWR_TEST_CLK2,
  CLKPWR_SPI2_DATIO,
  CLKPWR_SPI2_CLK_PAD,
  CLKPWR_SPI1_DATIO,
  CLKPWR_SPI1_CLK_PAD
} CLKPWR_MUX_STATE_T;

/* Main system oscillators */
typedef enum
{
  CLKPWR_MAIN_OSC = 0,
  CLKPWR_PLL397_OSC
} CLKPWR_OSC_T;

/* PLL enumerations */
typedef enum
{
  CLKPWR_PLL397 = 0,
  CLKPWR_HCLK_PLL,
  CLKPWR_USB_PLL
} CLKPWR_PLL_T;

/* Structure used for setting up the HCLK PLL */
typedef struct
{
  /* (0) = analog off, (!0) = on */
  INT_32 analog_on;
  /* (0) = CCO clock sent to post divider, (!0) = PLL input clock sent
  to post div */
  INT_32 cco_bypass_b15;
  /* (0) = PLL out from post divider, (!0) = PLL out bypasses post
  divider */
  INT_32 direct_output_b14;
  /* (0) = use CCO clock, (!0) = use FCLKOUT */
  INT_32 fdbk_div_ctrl_b13;
  /* Must be 1, 2, 4, or 8 */
  INT_32 pll_p;
  /* Must be 1, 2, 3, or 4 */
  INT_32 pll_n;
  /* Feedback multiplier 1-256 */
  UNS_32 pll_m;
} CLKPWR_HCLK_PLL_SETUP_T;

/* Autoclock type enumeration */
typedef enum
{
  CLKPWR_ACLK_USB_DEV = 0,
  CLKPWR_ACLK_IRAM,
  CLKPWR_ACLK_IROM
} CLKPWR_AUTOCLK_T;

/* Interrupt event sources */
#define CLKPWR_SOURCES_INT_BASE  0
#define CLKPWR_SOURCES_EXT_BASE  32
#define CLKPWR_SOURCES_GPIO_BASE 64
typedef enum
{
  /* Internal sources */
  CLKPWR_EVENT_INT_GPIO_00 = CLKPWR_SOURCES_INT_BASE,
  CLKPWR_EVENT_INT_GPIO_01,
  CLKPWR_EVENT_INT_GPIO_02,
  CLKPWR_EVENT_INT_GPIO_03,
  CLKPWR_EVENT_INT_GPIO_04,
  CLKPWR_EVENT_INT_GPIO_05,
  CLKPWR_EVENT_INT_GPIO_P0_P1,
  CLKPWR_EVENT_INT_ENET_MAC,
  CLKPWR_EVENT_INT_KEY = CLKPWR_SOURCES_INT_BASE + 16,
  CLKPWR_EVENT_INT_USBATXINT = CLKPWR_SOURCES_INT_BASE + 19,
  CLKPWR_EVENT_INT_USBOTGTIMER,
  CLKPWR_EVENT_INT_USB_I2C,
  CLKPWR_EVENT_INT_USB,
  CLKPWR_EVENT_INT_USBNEEDCLK,
  CLKPWR_EVENT_INT_RTC,
  CLKPWR_EVENT_INT_MSTIMER,
  CLKPWR_EVENT_INT_USBAHNEEDCLK,
  CLKPWR_EVENT_INT_AUX = CLKPWR_SOURCES_INT_BASE + 29,
  CLKPWR_EVENT_INT_PD = CLKPWR_SOURCES_INT_BASE + 30,
  CLKPWR_EVENT_INT_ADC = CLKPWR_SOURCES_INT_BASE + 31,

  /* External sources */
  CLKPWR_EVENT_EXT_GPIO_O8 = CLKPWR_SOURCES_EXT_BASE,
  CLKPWR_EVENT_EXT_GPIO_O9,
  CLKPWR_EVENT_EXT_GPIO_10,
  CLKPWR_EVENT_EXT_SPI2_DATIN,
  CLKPWR_EVENT_EXT_GPIO_O7,
  CLKPWR_EVENT_EXT_SPI1_DATIN,
  CLKPWR_EVENT_EXT_SYSCLKEN,
  CLKPWR_EVENT_EXT_GPIO_O0,
  CLKPWR_EVENT_EXT_GPIO_O1,
  CLKPWR_EVENT_EXT_GPIO_O2,
  CLKPWR_EVENT_EXT_GPIO_O3,
  CLKPWR_EVENT_EXT_GPIO_O4,
  CLKPWR_EVENT_EXT_GPIO_O5,
  CLKPWR_EVENT_EXT_GPIO_O6,
  CLKPWR_EVENT_EXT_MSDIO_ST,
  CLKPWR_EVENT_EXT_SDIO,
  CLKPWR_EVENT_EXT_U1_RX = CLKPWR_SOURCES_EXT_BASE + 21,
  CLKPWR_EVENT_EXT_U2_RX,
  CLKPWR_EVENT_EXT_U3_HCTS,
  CLKPWR_EVENT_EXT_U3_RX,
  CLKPWR_EVENT_EXT_GPI_11,
  CLKPWR_EVENT_EXT_U5_RX,
  CLKPWR_EVENT_EXT_U6_IRRX = CLKPWR_SOURCES_EXT_BASE + 28,
  CLKPWR_EVENT_EXT_U7_HXTC = CLKPWR_SOURCES_EXT_BASE + 30,
  CLKPWR_EVENT_EXT_U7_RX,

  /* GPIO sources */
  CLKPWR_EVENT_GPIO_P0_00 = CLKPWR_SOURCES_GPIO_BASE,
  CLKPWR_EVENT_GPIO_P0_01,
  CLKPWR_EVENT_GPIO_P0_02,
  CLKPWR_EVENT_GPIO_P0_03,
  CLKPWR_EVENT_GPIO_P0_04,
  CLKPWR_EVENT_GPIO_P0_05,
  CLKPWR_EVENT_GPIO_P0_06,
  CLKPWR_EVENT_GPIO_P0_07,
  CLKPWR_EVENT_GPIO_P1_00,
  CLKPWR_EVENT_GPIO_P1_01,
  CLKPWR_EVENT_GPIO_P1_02,
  CLKPWR_EVENT_GPIO_P1_03,
  CLKPWR_EVENT_GPIO_P1_04,
  CLKPWR_EVENT_GPIO_P1_05,
  CLKPWR_EVENT_GPIO_P1_06,
  CLKPWR_EVENT_GPIO_P1_07,
  CLKPWR_EVENT_GPIO_P1_08,
  CLKPWR_EVENT_GPIO_P1_09,
  CLKPWR_EVENT_GPIO_P1_10,
  CLKPWR_EVENT_GPIO_P1_11,
  CLKPWR_EVENT_GPIO_P1_12,
  CLKPWR_EVENT_GPIO_P1_13,
  CLKPWR_EVENT_GPIO_P1_14,
  CLKPWR_EVENT_GPIO_P1_15,
  CLKPWR_EVENT_GPIO_P1_16,
  CLKPWR_EVENT_GPIO_P1_17,
  CLKPWR_EVENT_GPIO_P1_18,
  CLKPWR_EVENT_GPIO_P1_19,
  CLKPWR_EVENT_GPIO_P1_20,
  CLKPWR_EVENT_GPIO_P1_21,
  CLKPWR_EVENT_GPIO_P1_22,
  CLKPWR_EVENT_GPIO_P1_23,
  CLKPWR_EVENT_LAST
} CLKPWR_EVENT_T;

/* High level chip clocking modes */
typedef enum
{
  CLKPWR_MD_RUN,
  CLKPWR_MD_DIRECTRUN,
  CLKPWR_MODE_STOP
} CLKPWR_MODE_T;

/**********************************************************************
 * PLL control functions
 *********************************************************************/

/* Find a PLL configuration for the selected frequency and tolerance
   (in .1% increments), returns the actual frequency of the PLL for
   the configuration */
UNS_32 clkpwr_find_pll_cfg(UNS_32 pllin_freq,
                           UNS_32 target_freq,
                           INT_32 tol_001,
                           CLKPWR_HCLK_PLL_SETUP_T *pllsetup);

/* Setup the PLL397 oscillator */
void clkpwr_pll397_setup(INT_32 bypass_enable,
                         UNS_32 bias,
                         INT_32 pll_enable);

/* Setup the HCLK PLL with a PLL structure */
INT_32 clkpwr_hclkpll_setup(CLKPWR_HCLK_PLL_SETUP_T *pHCLKPllSetup);

/* Setup the HCLK PLL with a direct value */
void clkpwr_hclkpll_direct_setup(UNS_32 pllsetupval);

/* Enable or disable a PLL (analog portion of PLL only) - do not use
   this function after a call to clkpwr_hclkpll_direct_setup() or
   clkpwr_hclkpll_setup() as they will enable the analog portion if
   needed. */
void clkpwr_pll_dis_en(CLKPWR_PLL_T pll,
                       INT_32 enable);

/* Get a PLL lock status */
INT_32 clkpwr_is_pll_locked(CLKPWR_PLL_T pll);

/**********************************************************************
 * System clock control and query functions
 *********************************************************************/

/* Setup the main oscillator */
void clkpwr_mainosc_setup(UNS_32 capload_add,
                          INT_32 osc_enable);

/* Setup the system clocking, selects the main or PLL397 oscillator */
void clkpwr_sysclk_setup(CLKPWR_OSC_T osc,
                         INT_32 bpval);

/* Determine which oscillator is selected */
CLKPWR_OSC_T clkpwr_get_osc(void);

/* Setup the HCLK dividers */
void clkpwr_set_hclk_divs(
  UNS_32 dram_clk_div,    /* A value of CLKPWR_HCLKDIV_DDRCLK_xx */
  INT_32 periph_clk_div,  /* Must be 1 to 32 */
  INT_32 hclk_div_val);   /* Must be 1, 2, or 4 */

/* Get the clock frequency for a system base clock (such as HCLK PLL,
   main oscillator, PERIPH_CLK, etc.) */
UNS_32 clkpwr_get_base_clock_rate(CLKPWR_BASE_CLOCK_T baseclk);

/* Force (or unforce) HCLK and ARM_CLK to run from PERIPH_CLK */
void clkpwr_force_arm_hclk_to_pclk(INT_32 force);

/* Select operational mode (run, direct run, or stop) */
void clkpwr_set_mode(CLKPWR_MODE_T mode);

/**********************************************************************
 * Individual peripheral clock control and query functions
 *********************************************************************/

/* Enable or disable a clock to a specific clocked peripheral */
void clkpwr_clk_en_dis(CLKPWR_CLK_T clk,
                       INT_32 enable);

/* Enable or disable a autoclock for a supported peripheral */
void clkpwr_autoclk_en_dis(CLKPWR_AUTOCLK_T clk,
                           INT_32 enable);

/* Get the clock frequency for a specific clocked peripheral */
UNS_32 clkpwr_get_clock_rate(CLKPWR_CLK_T ipclk);

/**********************************************************************
 * Wakeup control functions
 *********************************************************************/

/* Enable or disable a wakeup event */
void clkpwr_wk_event_en_dis(CLKPWR_EVENT_T event_id,
                            INT_32 enable);

/* Get the raw captured status of an event */
INT_32 clkpwr_is_raw_event_active(CLKPWR_EVENT_T event_id);

/* Get the masked captured status of an event */
INT_32 clkpwr_is_msk_event_active(CLKPWR_EVENT_T event_id);

/* Clear a captured event status */
void clkpwr_clear_event(CLKPWR_EVENT_T event_id);

/* Set event signal polarity (0) = falling edge, (1) = rising edge */
void clkpwr_set_event_pol(CLKPWR_EVENT_T event_id,
                          INT_32 high);

/**********************************************************************
 * Miscellaneous functions
 *********************************************************************/

/* Return the chip's unique ID */
void clkpwr_get_uid(UNS_32 uid[4]);


/* Set a muxed signal to it's default (0) or muxed state (1) */
void clkpwr_set_mux(CLKPWR_MUX_STATE_T mux_signal,
                    INT_32 mux1);

/* Set a muxed signal to passed value */
void clkpwr_set_mux_state(CLKPWR_MUX_STATE_T mux_signal,
                          INT_32 state);

/* Memory card (MSSDIO) pad and clock setup */
void clkpwr_setup_mcard_ctrlr(INT_32 enable_pullups,
                              INT_32 dat2_3_pullup_on,
                              INT_32 dat1_pullup_on,
                              INT_32 dat0_pullup_on,
                              INT_32 freq_div);

/* Set the drive strength of I2C signals */
void clkpwr_set_i2c_driver(INT_32 i2c_num, /* Must be 1 or 2 */
                           INT_32 high); /* '1' for high driver */

/* Configure PWM 1 or 2 with a clock source and divider value */
void clkpwr_setup_pwm(INT_32 pwm_num,   /* Must be 1 or 2 */
                      INT_32 clk_src,   /* '0' for 32KHz clock, 1 for
								        PERIPH_CLK */
                      INT_32 freq_dev); /* Must be 1 to 15, or 0 to
									    disable */

/* NAND FLASH controller setup */
void clkpwr_setup_nand_ctrlr(INT_32 use_slc,
                             INT_32 use_dma_req_on_rnb,
                             INT_32 use_dma_req_on_int);

/* ADC/touchscreen controller setup */
void clkpwr_setup_adc_ts(INT_32 sel_periph_clk,
                         INT_32 clkdiv);

/* SSP controller setup */
void clkpwr_setup_ssp(INT_32 ssp1_rx_dma_en,  /* Will disble SPI2 DMA */
                      INT_32 ssp1_tx_dma_en,  /* Will disble SPI1 DMA */
                      INT_32 ssp0_rx_dma_en,  /* Will disble SPI3 DMA */
                      INT_32 ssp0_tx_dma_en); /* Will disble SPI4 DMA */

/* I2S controller setup */
void clkpwr_setup_i2s(INT_32 i2s1_tx_clk_from_rx,
                      INT_32 i2s1_rx_clk_from_tx,
                      INT_32 i2s1_use_dma,
                      INT_32 i2s0_use_dma,
                      INT_32 i2s0_tx_clk_from_rx);

/* LCD pin mux group selection enumeration */
typedef enum
{
  CLKPWR_LCDMUX_TFT12 = CLKPWR_LCDCTRL_LCDTYPE_TFT12, /* 12-bit TFT */
  CLKPWR_LCDMUX_TFT16 = CLKPWR_LCDCTRL_LCDTYPE_TFT16, /* 16-bit TFT */
  CLKPWR_LCDMUX_TFT15 = CLKPWR_LCDCTRL_LCDTYPE_TFT15, /* 15-bit TFT */
  CLKPWR_LCDMUX_TFT24 = CLKPWR_LCDCTRL_LCDTYPE_TFT24, /* 24-bit TFT */
  CLKPWR_LCDMUX_STN4M = CLKPWR_LCDCTRL_LCDTYPE_STN4M, /* Mono4 STN */
  CLKPWR_LCDMUX_STN8C = CLKPWR_LCDCTRL_LCDTYPE_STN8C, /* Color8 STN */
  CLKPWR_LCDMUX_DSTN4M = CLKPWR_LCDCTRL_LCDTYPE_DSTN4M, /* M8 DSTN */
  CLKPWR_LCDMUX_DSTN8C = CLKPWR_LCDCTRL_LCDTYPE_DSTN8C, /* C8 DSTN */
} LCD_PIN_MUX_T;

/* LCD controller setup */
void clkpwr_setup_lcd(LCD_PIN_MUX_T lcdpins,
                      INT_32 clkdiv); /* 1 to 32 */

/* Ethernet enable and select MII/RMII interface selection */
void clkpwr_select_enet_iface(INT_32 enable, /* 1 = enable, 0 = dis */
                              INT_32 use_rmii); /* 1 = RMII, 0 = MII */

/* Enter low power mode (clocks disabled) in the CPU core. This
   function will block until a CPU niterrupt occurs */
void clkpwr_halt_cpu(void);

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_CLKPWR_DRIVER_H */

