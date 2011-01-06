/***********************************************************************
 * $Id:: lpc32xx_clkpwr_driver.c 2398 2009-10-28 00:21:31Z wellsk      $
 *
 * Project: LPC32xx SLC NAND controller driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx SLC NAND
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

#include "lpc32xx_clkpwr_driver.h"

#ifdef __ICCARM__
#include "intrinsics.h"
#endif

/***********************************************************************
 * SLC NAND controller driver package data
***********************************************************************/

/* Structure containing event register pointers */
typedef struct
{
  volatile UNS_32 *start_er;
  volatile UNS_32 *start_sr;
  volatile UNS_32 *start_rsr;
  volatile UNS_32 *start_apr;
} CLKPWR_EVENTREG_GROUP_T;

/* Normal base clocks for peripherals */
CLKPWR_BASE_CLOCK_T clkpwr_base_clk[CLKPWR_LAST_CLK] =
{
  CLKPWR_HCLK,       /* CLKPWR_USB_HCLK */
  CLKPWR_HCLK,       /* CLKPWR_LCD_CLK */
  CLKPWR_HCLK,       /* CLKPWR_SSP1_CLK */
  CLKPWR_HCLK,       /* CLKPWR_SSP0_CLK */
  CLKPWR_HCLK,       /* CLKPWR_I2S1_CLK */
  CLKPWR_HCLK,       /* CLKPWR_I2S0_CLK */
  CLKPWR_ARM_CLK,    /* CLKPWR_MSCARD_CLK */
  CLKPWR_HCLK,       /* CLKPWR_MAC_DMA_CLK */
  CLKPWR_HCLK,       /* CLKPWR_MAC_MMIO_CLK */
  CLKPWR_HCLK,       /* CLKPWR_MAC_HRC_CLK */
  CLKPWR_HCLK,       /* CLKPWR_I2C2_CLK */
  CLKPWR_HCLK,       /* CLKPWR_I2C1_CLK */
  CLKPWR_RTC_CLK,    /* CLKPWR_KEYSCAN_CLK */
  CLKPWR_RTC_CLK,    /* CLKPWR_ADC_CLK */
  CLKPWR_RTC_CLK,    /* CLKPWR_PWM2_CLK */
  CLKPWR_RTC_CLK,    /* CLKPWR_PWM1_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_HSTIMER_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_WDOG_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_TIMER3_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_TIMER2_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_TIMER1_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_TIMER0_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_PWM4_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_PWM3_CLK */
  CLKPWR_HCLK,       /* CLKPWR_SPI2_CLK */
  CLKPWR_HCLK,       /* CLKPWR_SPI1_CLK */
  CLKPWR_HCLK,       /* CLKPWR_NAND_SLC_CLK */
  CLKPWR_HCLK,       /* CLKPWR_NAND_MLC_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_UART6_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_UART5_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_UART4_CLK */
  CLKPWR_PERIPH_CLK, /* CLKPWR_UART3_CLK */
  CLKPWR_HCLK,       /* CLKPWR_DMA_CLK */
  CLKPWR_DDR_CLK     /* CLKPWR_SDRAMDDR_CLK */
};

/* Post divider values for PLLs based on selected register value */
static UNS_32 pll_postdivs[4] =
{
  1, 2, 4, 8
};

/* CLK divider values for HCLK based on selected register value */
static UNS_32 hclkdivs[4] =
{
  1, 2, 4, 4
};

/***********************************************************************
 * CLKPWR controller driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: clkpwr_abs
 *
 * Purpose: ABS difference function
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     v1 : Value 1 for ABS
 *     v2 : Value 2 for ABS
 *
 * Outputs: None
 *
 * Returns: Absolute difference between the 2 values
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 clkpwr_abs(INT_32 v1, INT_32 v2)
{
  if (v1 > v2)
  {
    return v1 - v2;
  }

  return v2 - v1;
}

/***********************************************************************
 *
 * Function: clkpwr_check_pll_setup
 *
 * Purpose: Determines if a PLL setup is valid
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ifreq    : PLL input frequency
 *     pllsetup : Pointer to PLL setup structure
 *
 * Outputs: None
 *
 * Returns: Computed PLL frequency or 0 if invalid
 *
 * Notes: Used for PLL setup value verification.
 *
 **********************************************************************/
UNS_32 clkpwr_check_pll_setup(UNS_32 ifreq,
                              CLKPWR_HCLK_PLL_SETUP_T *pllsetup)
{
  UNS_64 i64freq, p, m, n, fcco = 0, fref = 0, cfreq = 0;
  INT_32 mode;

  /* PLL requirements */
  /* ifreq must be >= 1MHz and <= 20MHz */
  /* FCCO must be >= 156MHz and <= 320MHz */
  /* FREF must be >= 1MHz and <= 27MHz. */
  /* Assume the passed input data is not valid */

  /* Work with 64-bit values to prevent overflow */
  i64freq = (UNS_64) ifreq;
  m = (UNS_64) pllsetup->pll_m;
  n = (UNS_64) pllsetup->pll_n;
  p = (UNS_64) pllsetup->pll_p;

  /* Get components of the PLL register */
  mode = (pllsetup->cco_bypass_b15 << 2) |
         (pllsetup->direct_output_b14 << 1) |
         pllsetup->fdbk_div_ctrl_b13;
  switch (mode)
  {
    case 0x0: /* Non-integer mode */
      cfreq = (m * i64freq) / (2 * p * n);
      fcco = (m * i64freq) / n;
      fref = i64freq / n;
      break;

    case 0x1: /* integer mode */
      cfreq = (m * i64freq) / n;
      fcco = (m * i64freq) / (n * 2 * p);
      fref = i64freq / n;
      break;

    case 0x2:
    case 0x3: /* Direct mode */
      cfreq = (m * i64freq) / n;
      fcco = cfreq;
      fref = i64freq / n;
      break;

    case 0x4:
    case 0x5: /* Bypass mode */
      cfreq = i64freq / (2 * p);
      fcco = 156000000;
      fref = 1000000;
      break;

    case 0x6:
    case 0x7: /* Direct bypass mode */
      cfreq = i64freq;
      fcco = 156000000;
      fref = 1000000;
      break;
  }

  if ((fcco < 156000000) || (fcco > 320000000))
  {
    /* not a valid range */
    cfreq = 0;
  }

  if ((fref < 1000000) || (fref > 27000000))
  {
    /* not a valid range */
    cfreq = 0;
  }

  return (INT_32) cfreq;
}

/***********************************************************************
 *
 * Function: clkpwr_pll_rate_from_val
 *
 * Purpose: Compute a PLL's frequency from a PLL register value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     osc_rate : PLL input clock in Hz
 *     val      : 32-bit register value
 *
 * Outputs: None
 *
 * Returns: The PLL output frequency in Hz, or 0
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clkpwr_pll_rate_from_val(UNS_32 osc_rate,
                                UNS_32 val)
{
  CLKPWR_HCLK_PLL_SETUP_T pllcfg;

  /* Get components of the PLL register */
  pllcfg.cco_bypass_b15 = 0;
  pllcfg.direct_output_b14 = 0;
  pllcfg.fdbk_div_ctrl_b13 = 0;
  if ((val & CLKPWR_HCLKPLL_CCO_BYPASS) != 0)
  {
    pllcfg.cco_bypass_b15 = 1;
  }
  if ((val & CLKPWR_HCLKPLL_POSTDIV_BYPASS) != 0)
  {
    pllcfg.direct_output_b14 = 1;
  }
  if ((val & CLKPWR_HCLKPLL_FDBK_SEL_FCLK) != 0)
  {
    pllcfg.fdbk_div_ctrl_b13 = 1;
  }
  pllcfg.pll_m = 1 + ((val >> 1) & 0xFF);
  pllcfg.pll_n = 1 + ((val >> 9) & 0x3);
  pllcfg.pll_p = pll_postdivs[((val >> 11) & 0x3)];

  return clkpwr_check_pll_setup(osc_rate, &pllcfg);
}

/***********************************************************************
 *
 * Function: clkpwr_pll_rate
 *
 * Purpose: Compute a PLL's frequency
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     osc_rate : PLL input clock in Hz
 *     pPllreg  : Pointer to PLL register set
 *
 * Outputs: None
 *
 * Returns: The PLL output frequency in Hz, or 0
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clkpwr_pll_rate(UNS_32 osc_rate,
                       UNS_32 *pPllreg)
{
  return clkpwr_pll_rate_from_val(osc_rate, *pPllreg);
}

/***********************************************************************
 *
 * Function: clkpwr_mask_and_set
 *
 * Purpose: Set or mask off a specific field or bit
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pReg : Pointer to address to modify
 *     mask : Mask to apply to value
 *     set  : '1' = mask and set, '0' = mask only
 *
 * Outputs: None
 *
 * Returns: '1' if the selected PLL oscillator is locked, otherwise 0
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_mask_and_set(volatile UNS_32 *pReg,
                         UNS_32 mask,
                         UNS_32 set)
{
  UNS_32 tmp;

  tmp = *pReg & ~mask;
  if (set != 0)
  {
    tmp |= mask;
  }
  *pReg = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_get_event_field
 *
 * Purpose: Get register and bit position for an event ID
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     event_id : Event enumeration to change
 *     pReg     : Pointer to where to save register addresses
 *     bitnum   : Bit number mapped to event
 *
 * Outputs: None
 *
 * Returns: 1 if a valid event_id, otherwise 0
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 clkpwr_get_event_field(CLKPWR_EVENT_T event_id,
                              CLKPWR_EVENTREG_GROUP_T *pReg,
                              INT_32 *bitnum)
{
  INT_32 status = 0;

  if (event_id < CLKPWR_SOURCES_EXT_BASE)
  {
    pReg->start_er = &CLKPWR->clkpwr_int_er;
    pReg->start_sr = &CLKPWR->clkpwr_int_sr;
    pReg->start_rsr = &CLKPWR->clkpwr_int_rs;
    pReg->start_apr = &CLKPWR->clkpwr_int_ap;
    *bitnum = (INT_32)event_id;
    status = 1;
  }
  else if (event_id < CLKPWR_SOURCES_GPIO_BASE)
  {
    pReg->start_er = &CLKPWR->clkpwr_pin_er;
    pReg->start_sr = &CLKPWR->clkpwr_pin_sr;
    pReg->start_rsr = &CLKPWR->clkpwr_pin_rs;
    pReg->start_apr = &CLKPWR->clkpwr_pin_ap;
    *bitnum = (INT_32)(CLKPWR_SOURCES_GPIO_BASE - event_id);
    status = 1;
  }
  else if (event_id < CLKPWR_EVENT_LAST)
  {
    pReg->start_er = &CLKPWR->clkpwr_p01_er;
    pReg->start_sr = NULL;
    pReg->start_rsr = NULL;
    pReg->start_apr = NULL;
    *bitnum = (INT_32)(CLKPWR_EVENT_LAST - event_id);
    status = 1;
  }

  return status;
}

/***********************************************************************
 *
 * Function: clkpwr_get_base_clock
 *
 * Purpose: Get the base clock for a main clock
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipclk : Clock to get the rate of
 *
 * Outputs: None
 *
 * Returns: The base clock for the selected clock.
 *
 * Notes: None
 *
 **********************************************************************/
CLKPWR_BASE_CLOCK_T clkpwr_get_base_clock(CLKPWR_CLK_T ipclk)
{
  CLKPWR_BASE_CLOCK_T baseclk = CLKPWR_BASE_INVALID;

  if (ipclk <= CLKPWR_LAST_CLK)
  {
    /* Get base clock for the selected clock */
    baseclk = clkpwr_base_clk[ipclk];

    /* Do default clocks need to be overrided? */
    switch (ipclk)
    {
      case CLKPWR_ADC_CLK:
        /* Is the peripheral clock used? */
        if ((CLKPWR->clkpwr_adc_clk_ctrl_1 &
             CLKPWR_ADCCTRL1_PCLK_SEL) != 0)
        {
          baseclk = CLKPWR_PERIPH_CLK;
        }
        break;

      case CLKPWR_PWM2_CLK:
        /* Is the peripheral clock used? */
        if ((CLKPWR->clkpwr_pwm_clk_ctrl &
             CLKPWR_PWMCLK_PWM2SEL_PCLK) != 0)
        {
          baseclk = CLKPWR_PERIPH_CLK;
        }
        break;

      case CLKPWR_PWM1_CLK:
        /* Is the peripheral clock used? */
        if ((CLKPWR->clkpwr_pwm_clk_ctrl &
             CLKPWR_PWMCLK_PWM1SEL_PCLK) != 0)
        {
          baseclk = CLKPWR_PERIPH_CLK;
        }
        break;

      default:
        break;
    }

  }

  return baseclk;
}

/**********************************************************************
 * Clock control functions
 *********************************************************************/

/***********************************************************************
 *
 * Function: clkpwr_find_pll_cfg
 *
 * Purpose:
 *     Find a PLL configuration for the selected frequency and
 *     tolerance (in .1% increments), returns the actual frequency of
 *     the PLL for the configuration
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pllin_freq  : Frequency into the PLL
 *     target_freq : Frequency in Hz to compute PLL values for
 *     tol_001     : Tolerance in 1/10th of a percent for target frequency
 *     pllsetup    : Pointer to PLL config structure to fill
 *
 * Outputs: None
 *
 * Returns: New configured frequency, or '0' if an error occurs
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clkpwr_find_pll_cfg(UNS_32 pllin_freq,
                           UNS_32 target_freq,
                           INT_32 tol_001,
                           CLKPWR_HCLK_PLL_SETUP_T *pllsetup)
{
  UNS_32 ifreq, freqtol, m, n, p, fclkout = 0;
  UNS_32 flag = 0, freqret = 0;
  UNS_64 lfreqtol;

  /* Determine frequency tolerance limits */
  lfreqtol = ((UNS_64) target_freq * (UNS_64) tol_001) / 1000;
  freqtol = (INT_32) lfreqtol;

  /* Get PLL clock */
  ifreq = pllin_freq;

  /* Is direct bypass mode possible? */
  if (clkpwr_abs(pllin_freq, target_freq) <= freqtol)
  {
    flag = 1;
    pllsetup->analog_on = 0;
    pllsetup->cco_bypass_b15 = 1; /* Bypass CCO */
    pllsetup->direct_output_b14 = 1; /* Bypass post divider */
    pllsetup->fdbk_div_ctrl_b13 = 1;
    pllsetup->pll_p = pll_postdivs[0]; /* Doesn't matter */
    pllsetup->pll_n = 1; /* Doesn't matter */
    pllsetup->pll_m = 1; /* Doesn't matter */
    fclkout = clkpwr_check_pll_setup(ifreq, pllsetup);
  }
  else if (target_freq <= ifreq) /* Is bypass mode possible? */
  {
    pllsetup->analog_on = 0;
    pllsetup->cco_bypass_b15 = 1; /* Bypass CCO */
    pllsetup->direct_output_b14 = 0;
    pllsetup->fdbk_div_ctrl_b13 = 1;
    pllsetup->pll_n = 1; /* Doesn't matter */
    pllsetup->pll_m = 1; /* Doesn't matter */
    for (p = 0; ((p <= 3) && (flag == 0)); p++)
    {
      pllsetup->pll_p = pll_postdivs[p];
      fclkout = clkpwr_check_pll_setup(ifreq, pllsetup);
      if (clkpwr_abs(target_freq, fclkout) <= freqtol)
      {
        /* Found a matching frequency */
        flag = 1;
      }
    }
  }

  /* Is direct mode possible? */
  if (flag == 0)
  {
    pllsetup->analog_on = 1;
    pllsetup->cco_bypass_b15 = 0; /* Bypass CCO */
    pllsetup->direct_output_b14 = 1;
    pllsetup->fdbk_div_ctrl_b13 = 0;
    pllsetup->pll_p = pll_postdivs[0];
    for (m = 1; ((m <= 256) && (flag == 0)); m++)
    {
      for (n = 1; ((n <= 4) && (flag == 0)); n++)
      {
        /* Compute output frequency for this value */
        pllsetup->pll_n = n;
        pllsetup->pll_m = m;
        fclkout = clkpwr_check_pll_setup(ifreq, pllsetup);
        if (clkpwr_abs(target_freq, fclkout) <= freqtol)
        {
          flag = 1;
        }
      }
    }
  }

  /* Is integer mode possible? */
  if (flag == 0)
  {
    /* Bypass and direct modes won't work with this frequency, so
       integer mode may need to be used */
    pllsetup->analog_on = 1;
    pllsetup->cco_bypass_b15 = 0;
    pllsetup->direct_output_b14 = 0;
    pllsetup->fdbk_div_ctrl_b13 = 1;
    for (m = 1; ((m <= 256) && (flag == 0)); m++)
    {
      for (n = 1; ((n <= 4) && (flag == 0)); n++)
      {
        for (p = 0; ((p < 4) && (flag == 0)); p++)
        {
          /* Compute output frequency for this value */
          pllsetup->pll_p = pll_postdivs[p];
          pllsetup->pll_n = n;
          pllsetup->pll_m = m;
          fclkout = clkpwr_check_pll_setup(ifreq, pllsetup);
          if (clkpwr_abs(target_freq, fclkout) <= freqtol)
          {
            flag = 1;
          }
        }
      }
    }
  }

  if (flag == 0)
  {
    /* Try non-integer mode */
    pllsetup->analog_on = 1;
    pllsetup->cco_bypass_b15 = 0;
    pllsetup->direct_output_b14 = 0;
    pllsetup->fdbk_div_ctrl_b13 = 0;
    for (m = 1; ((m <= 256) && (flag == 0)); m++)
    {
      for (n = 1; ((n <= 4) && (flag == 0)); n++)
      {
        for (p = 0; ((p < 4) && (flag == 0)); p++)
        {
          /* Compute output frequency for this value */
          pllsetup->pll_p = pll_postdivs[p];
          pllsetup->pll_n = n;
          pllsetup->pll_m = m;
          fclkout = clkpwr_check_pll_setup(ifreq, pllsetup);
          if (clkpwr_abs(target_freq, fclkout) <= freqtol)
          {
            flag = 1;
          }
        }
      }
    }
  }

  if (flag == 1)
  {
    freqret = fclkout;
  }

  return freqret;
}

/***********************************************************************
 *
 * Function: clkpwr_pll397_setup
 *
 * Purpose: Setup the PLL397 oscillator
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     bypass_enable : (1) to enable bypass, (0) to disable bypass mode
 *     bias : Must be a macro of type CLKPWR_PLL397_BIAS_xxxx
 *     enable : (1) to enable, (0) to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_pll397_setup(INT_32 bypass_enable,
                         UNS_32 bias,
                         INT_32 pll_enable)
{
  volatile UNS_32 tmp;

  tmp = CLKPWR->clkpwr_pll397_ctrl & ~(CLKPWR_PLL397_BYPASS |
                                       CLKPWR_PLL397_BIAS_MASK |
									   CLKPWR_SYSCTRL_PLL397_DIS);

  if (bypass_enable != 0)
  {
    tmp = tmp | CLKPWR_PLL397_BYPASS;
  }
  tmp = tmp | bias;

  if (pll_enable == 0)
  {
    tmp = tmp | CLKPWR_SYSCTRL_PLL397_DIS;
  }

  CLKPWR->clkpwr_pll397_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_hclkpll_setup
 *
 * Purpose: Setup the HCLK PLL with a PLL structure
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pHCLKPllSetup : Pointer to a HCLK PLL setup structure
 *
 * Outputs: None
 *
 * Returns: 1 if the PLL was setup ok, otherwise 0 if no change
 *
 * Notes: The PLL will be disabled out of this function.
 *
 **********************************************************************/
INT_32 clkpwr_hclkpll_setup(CLKPWR_HCLK_PLL_SETUP_T *pHCLKPllSetup)
{
  UNS_32 tv, tmp = 0;

  if (pHCLKPllSetup->analog_on != 0)
  {
    tmp |= CLKPWR_HCLKPLL_POWER_UP;
  }
  if (pHCLKPllSetup->cco_bypass_b15 != 0)
  {
    tmp |= CLKPWR_HCLKPLL_CCO_BYPASS;
  }
  if (pHCLKPllSetup->direct_output_b14 != 0)
  {
    tmp |= CLKPWR_HCLKPLL_POSTDIV_BYPASS;
  }
  if (pHCLKPllSetup->fdbk_div_ctrl_b13 != 0)
  {
    tmp |= CLKPWR_HCLKPLL_FDBK_SEL_FCLK;
  }

  switch (pHCLKPllSetup->pll_p)
  {
    case 1:
      tv = 0;
      break;

    case 2:
      tv = 1;
      break;

    case 4:
      tv = 2;
      break;

    case 8:
      tv = 3;
      break;

    default:
      return 0;
  }
  tmp |= CLKPWR_HCLKPLL_POSTDIV_2POW(tv);
  tmp |= CLKPWR_HCLKPLL_PREDIV_PLUS1(pHCLKPllSetup->pll_n - 1);
  tmp |= CLKPWR_HCLKPLL_PLLM(pHCLKPllSetup->pll_m - 1);

  CLKPWR->clkpwr_hclkpll_ctrl = tmp;

  return clkpwr_check_pll_setup(
           clkpwr_get_base_clock_rate(CLKPWR_SYSCLK), pHCLKPllSetup);
}

/***********************************************************************
 *
 * Function: clkpwr_usbclkpll_setup
 *
 * Purpose: Setup the USB PLL with a PLL structure
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pHCLKPllSetup : Pointer to a HCLK PLL setup structure
 *
 * Outputs: None
 *
 * Returns: 1 if the PLL was setup ok, otherwise 0 if no change
 *
 * Notes: The PLL will be disabled out of this function. USB and HCLK
 *        PLL structures are identical.
 *
 **********************************************************************/
INT_32 clkpwr_usbclkpll_setup(CLKPWR_HCLK_PLL_SETUP_T *pHCLKPllSetup)
{
  UNS_32 tv, tmp = 0;

  if (pHCLKPllSetup->analog_on != 0)
  {
    tmp |= CLKPWR_HCLKPLL_POWER_UP;
  }
  if (pHCLKPllSetup->cco_bypass_b15 != 0)
  {
    tmp |= CLKPWR_HCLKPLL_CCO_BYPASS;
  }
  if (pHCLKPllSetup->direct_output_b14 != 0)
  {
    tmp |= CLKPWR_HCLKPLL_POSTDIV_BYPASS;
  }
  if (pHCLKPllSetup->fdbk_div_ctrl_b13 != 0)
  {
    tmp |= CLKPWR_HCLKPLL_FDBK_SEL_FCLK;
  }

  switch (pHCLKPllSetup->pll_p)
  {
    case 1:
      tv = 0;
      break;

    case 2:
      tv = 1;
      break;

    case 4:
      tv = 2;
      break;

    case 8:
      tv = 3;
      break;

    default:
      return 0;
  }
  tmp |= CLKPWR_HCLKPLL_POSTDIV_2POW(tv);
  tmp |= CLKPWR_HCLKPLL_PREDIV_PLUS1(pHCLKPllSetup->pll_n - 1);
  tmp |= CLKPWR_HCLKPLL_PLLM(pHCLKPllSetup->pll_m - 1);

  CLKPWR->clkpwr_usb_ctrl = tmp;

//  return 1;
  return clkpwr_check_pll_setup(
           clkpwr_get_base_clock_rate(CLKPWR_SYSCLK), pHCLKPllSetup);
}

/***********************************************************************
 *
 * Function: clkpwr_hclkpll_direct_setup
 *
 * Purpose: Setup the HCLK PLL with a direct value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pllsetupval : PLL direct setup value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: The PLL will be disabled out of this function.
 *
 **********************************************************************/
void clkpwr_hclkpll_direct_setup(UNS_32 pllsetupval)
{
  CLKPWR->clkpwr_hclkpll_ctrl = pllsetupval;
}

/***********************************************************************
 *
 * Function: clkpwr_pll_dis_en
 *
 * Purpose: Enable or disable a PLL
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pll    : PLL to enable or disable
 *     enable : 0 to disable, 1 to enable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_pll_dis_en(CLKPWR_PLL_T pll,
                       INT_32 enable)
{
  switch (pll)
  {
    case CLKPWR_PLL397:
      if (enable == 0)
      {
        CLKPWR->clkpwr_pll397_ctrl |= CLKPWR_SYSCTRL_PLL397_DIS;
      }
      else
      {
        CLKPWR->clkpwr_pll397_ctrl &= ~CLKPWR_SYSCTRL_PLL397_DIS;
      }
      break;

    case CLKPWR_HCLK_PLL:
      if (enable != 0)
      {
        CLKPWR->clkpwr_hclkpll_ctrl |= CLKPWR_HCLKPLL_POWER_UP;
      }
      else
      {
        CLKPWR->clkpwr_hclkpll_ctrl &= ~CLKPWR_HCLKPLL_POWER_UP;
      }
      break;

    case CLKPWR_USB_PLL:
      if (enable != 0)
      {
        CLKPWR->clkpwr_usb_ctrl |= CLKPWR_USBCTRL_PLL_PWRUP;
      }
      else
      {
        CLKPWR->clkpwr_usb_ctrl &= ~CLKPWR_USBCTRL_PLL_PWRUP;
      }
      break;

    default:
      break;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_is_pll_locked
 *
 * Purpose: Get a PLL lock status
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pll : PLL to check the status of
 *
 * Outputs: None
 *
 * Returns: '1' if the selected PLL oscillator is locked, otherwise 0
 *
 * Notes:
 *     If the analog is disabled for the HCKL or USB PLL, it may still
 *     provide a clock, but never a locked status. In these cases, a
 *     simulated lock status is returned.
 *
 **********************************************************************/
INT_32 clkpwr_is_pll_locked(CLKPWR_PLL_T pll)
{
  INT_32 locked = 0;

  switch (pll)
  {
    case CLKPWR_PLL397:
      if ((CLKPWR->clkpwr_pll397_ctrl &
           CLKPWR_SYSCTRL_PLL397_STS) != 0)
      {
        locked = 1;
      }
      break;

    case CLKPWR_HCLK_PLL:
      if ((CLKPWR->clkpwr_hclkpll_ctrl &
           CLKPWR_HCLKPLL_PLL_STS) != 0)
      {
        locked = 1;
      }
      else if ((CLKPWR->clkpwr_hclkpll_ctrl &
                CLKPWR_HCLKPLL_POWER_UP) == 0)
      {
        /* Simulated status */
        locked = 1;
      }
      break;

    case CLKPWR_USB_PLL:
      if ((CLKPWR->clkpwr_usb_ctrl &
           CLKPWR_USBCTRL_PLL_STS) != 0)
      {
        locked = 1;
      }
      else if ((CLKPWR->clkpwr_usb_ctrl &
                CLKPWR_USBCTRL_PLL_PWRUP) == 0)
      {
        /* Simulated status */
        locked = 1;
      }
      break;

    default:
      break;
  }

  return locked;
}

/***********************************************************************
 *
 * Function: clkpwr_mainosc_setup
 *
 * Purpose: Setup the main oscillator
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     capload_add : adds 0 to 12.7pF, or 0.1pF per increment
 *     osc_enable  : (1) to enable, (0) to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_mainosc_setup(UNS_32 capload_add,
                          INT_32 osc_enable)
{
  volatile UNS_32 tmp;

  /* Read current register value and mask off cap load value */
  tmp = CLKPWR->clkpwr_main_osc_ctrl & ~(CLKPWR_MOSC_CAP_MASK |
                                         CLKPWR_MOSC_DISABLE |
										 CLKPWR_SYSCTRL_BP_MASK);

  /* Add in new cap load value */
  tmp = tmp | CLKPWR_MOSC_ADD_CAP(capload_add);

  if (osc_enable == 0)
  {
    tmp = tmp | CLKPWR_MOSC_DISABLE;
  }

  CLKPWR->clkpwr_main_osc_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_sysclk_setup
 *
 * Purpose: Setup the system clocking
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     osc   : Selected oscillator to use
 *     bpval : Bad phase clock switch value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_sysclk_setup(CLKPWR_OSC_T osc,
                         INT_32 bpval)
{
  volatile UNS_32 tmp;

  tmp = CLKPWR->clkpwr_sysclk_ctrl & ~(CLKPWR_SYSCTRL_BP_MASK |
                                       CLKPWR_SYSCTRL_USEPLL397);

  tmp = tmp | CLKPWR_SYSCTRL_BP_TRIG(bpval);
  switch (osc)
  {
    case CLKPWR_PLL397_OSC:
      tmp = tmp | CLKPWR_SYSCTRL_USEPLL397;
      break;

    case CLKPWR_MAIN_OSC:
    default:
      break;
  }

  CLKPWR->clkpwr_sysclk_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_get_osc
 *
 * Purpose: Determine which oscillator is selected
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Oscillator enumeration for active oscillator
 *
 * Notes: None
 *
 **********************************************************************/
CLKPWR_OSC_T clkpwr_get_osc(void)
{
  CLKPWR_OSC_T sosc = CLKPWR_MAIN_OSC;
  if ((CLKPWR->clkpwr_sysclk_ctrl & CLKPWR_SYSCTRL_SYSCLKMUX) != 0)
  {
    sosc = CLKPWR_PLL397_OSC;
  }

  return sosc;
}

/***********************************************************************
 *
 * Function: clkpwr_set_hclk_divs
 *
 * Purpose: Setup the HCLK dividers
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     dram_clk_div   : CLKPWR_HCLKDIV_DDRCLK_xx value
 *     periph_clk_div : Must be 1 to 256, peripheral clock divider
 *     hclk_div_val   : Must be 1, 2, or 4
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_set_hclk_divs(UNS_32 dram_clk_div,
                          INT_32 periph_clk_div,
                          INT_32 hclk_div_val)
{
  UNS_32 hclkval = 0;

  switch (hclk_div_val)
  {
    case 1:
      hclkval = 0;
      break;

    case 2:
      hclkval = 1;
      break;

    case 4:
    default:
      hclkval = 2;
      break;
  }

  CLKPWR->clkpwr_hclk_div = (dram_clk_div |
    CLKPWR_HCLKDIV_PCLK_DIV(periph_clk_div - 1) |
    CLKPWR_HCLKDIV_DIV_2POW(hclkval));
}

/***********************************************************************
 *
 * Function: clkpwr_get_base_clock_rate
 *
 * Purpose: Get the clock frequency for a base clock
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     baseclk : Base clock to get the rate of
 *
 * Outputs: None
 *
 * Returns:
 *     The current clock rate for the selected peripheral in Hz, or 0
 *     if the clock rate can't be determined.
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clkpwr_get_base_clock_rate(CLKPWR_BASE_CLOCK_T baseclk)
{
  UNS_32 sys_clk, ddr_clock, ddr_hclk_div, hclkpll_clk, periph_clk;
  UNS_32 tmp, hclk1_clk, arm1_clk, hclk_clk, arm_clk, clkrate;

  /* Is PLL397 oscillator being used? */
  if ((CLKPWR->clkpwr_sysclk_ctrl & CLKPWR_SYSCTRL_USEPLL397) != 0)
  {
    /* PLL397 is used */
    sys_clk = CLOCK_OSC_FREQ * 397;
  }
  else
  {
    sys_clk = MAIN_OSC_FREQ;
  }

  /* Compute HCLK DDR divider */
  ddr_hclk_div = 0;
  if ((CLKPWR->clkpwr_sdramclk_ctrl & CLKPWR_SDRCLK_USE_DDR) != 0)
  {
    /* DDR is being used */
    if ((CLKPWR->clkpwr_hclk_div & CLKPWR_HCLKDIV_DDRCLK_NORM) != 0)
    {
      ddr_hclk_div = 1;
    }
    else if ((CLKPWR->clkpwr_hclk_div &
              CLKPWR_HCLKDIV_DDRCLK_HALF) != 0)
    {
      ddr_hclk_div = 2;
    }
  }
  else
  {
    /* SDRAM is being used */
    tmp = CLKPWR->clkpwr_hclk_div & CLKPWR_HCLKDIV_DIV_2POW(0x3);
    ddr_hclk_div = hclkdivs[tmp] - 1;
  }

  /* Is the device in run mode? */
  if ((CLKPWR->clkpwr_pwr_ctrl & CLKPWR_SELECT_RUN_MODE) != 0)
  {
    /* In run mode */

    /* Compute HCLK PLL rate */
    hclkpll_clk = clkpwr_pll_rate(sys_clk,
      (UNS_32 *) & CLKPWR->clkpwr_hclkpll_ctrl);

    /* Base DDR rate */
    ddr_clock = hclkpll_clk;

    /* Base peripheral clock rate */
    tmp = 1 + ((CLKPWR->clkpwr_hclk_div >> 2) & 0x1F);
    periph_clk = hclkpll_clk / tmp;

    /* Base HCLK rate (when not using peripheral clock */
    hclk1_clk = hclkpll_clk /
     hclkdivs[CLKPWR_HCLKDIV_DIV_2POW(CLKPWR->clkpwr_hclk_div)];

    /* Base ARM clock (when not using peripheral clock */
    arm1_clk = hclkpll_clk;
  }
  else
  {
    /* In direct-run mode */

    /* Base DDR rate */
    ddr_clock = sys_clk;

    /* Base peripheral clock rate */
    periph_clk = sys_clk;

    /* Base HCLK rate (when not using peripheral clock */
    hclk1_clk = sys_clk;

    /* Base ARM clock (when not using peripheral clock */
    arm1_clk = sys_clk;
  }

  /* Compute SDRAM/DDR clock */
  ddr_clock = ddr_clock / (ddr_hclk_div + 1);

  /* Compute HCLK and ARM clock rates */
  if ((CLKPWR->clkpwr_pwr_ctrl & CLKPWR_CTRL_FORCE_PCLK) != 0)
  {
    /* HCLK and ARM clock run from peripheral clock */
    hclk_clk = periph_clk;
    arm_clk = periph_clk;
  }
  else
  {
    /* Normal clock is used for HCLK and ARM clock */
    hclk_clk = hclk1_clk;
    arm_clk = arm1_clk;
  }

  /* Determine rates */
  switch (baseclk)
  {
    case CLKPWR_MAINOSC_CLK:
      /* Main oscillator rate */
      clkrate = MAIN_OSC_FREQ;
      break;

    case CLKPWR_RTC_CLK:
      /* RTC oscillator rate */
      clkrate = CLOCK_OSC_FREQ;
      break;

    case CLKPWR_SYSCLK:
      /* System oscillator (main osc or PLL397) rate */
      clkrate = sys_clk;
      break;

    case CLKPWR_ARM_CLK:
      clkrate = arm_clk;
      break;

    case CLKPWR_HCLK:
      clkrate = hclk_clk;
      break;

    case CLKPWR_PERIPH_CLK:
      clkrate = periph_clk;
      break;

    case CLKPWR_USB_HCLK_SYS:
      clkrate = 0; // Not supported
      break;

    case CLKPWR_48M_CLK:
      clkrate = 0; // Not supported
      break;

    case CLKPWR_DDR_CLK:
      clkrate = ddr_clock;
      break;

    case CLKPWR_MSSD_CLK:
      clkrate = hclk_clk;
      break;

    default:
      clkrate = 0;
      break;
  }

  return clkrate;
}

/***********************************************************************
 *
 * Function: clkpwr_force_arm_hclk_to_pclk
 *
 * Purpose: Force (or unforce) HCLK and ARM_CLK to run from PERIPH_CLK
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     force : '1' to force HCLK and ARM clock to PERIPH_CLK, '0' normal
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_force_arm_hclk_to_pclk(INT_32 force)
{
  UNS_32 tmp;

  tmp = CLKPWR->clkpwr_pwr_ctrl & ~CLKPWR_CTRL_FORCE_PCLK;
  if (force != 0)
  {
    tmp |= CLKPWR_CTRL_FORCE_PCLK;
  }

  CLKPWR->clkpwr_pwr_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_set_mode
 *
 * Purpose: Select operational mode (run, direct run, or stop)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     freq    : Frequency in Hz to compute PLL values for
 *     tol     : Tolerance in percent for target frequency
 *     pllsetup: Pointer to PLL config structure to fill
 *
 * Outputs: None
 *
 * Returns: '1' if the PLL config is valid, otherwise 0
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_set_mode(CLKPWR_MODE_T mode)
{
  UNS_32 tmp;

  switch (mode)
  {
    case CLKPWR_MD_RUN:
      tmp = CLKPWR->clkpwr_pwr_ctrl | CLKPWR_SELECT_RUN_MODE;
      CLKPWR->clkpwr_pwr_ctrl = tmp;
      break;

    case CLKPWR_MD_DIRECTRUN:
      tmp = CLKPWR->clkpwr_pwr_ctrl & ~CLKPWR_SELECT_RUN_MODE;
      CLKPWR->clkpwr_pwr_ctrl = tmp;
      break;

    case CLKPWR_MODE_STOP:
      tmp = CLKPWR->clkpwr_pwr_ctrl | CLKPWR_STOP_MODE_CTRL;
      CLKPWR->clkpwr_pwr_ctrl = tmp;
      break;

    default:
      break;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_clk_en_dis
 *
 * Purpose: Enable or disable a clock
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clk    : Clock to enable or disable
 *     enable : '1' to enable, '0' to disable
 *
 * Outputs: None
 *
 * Returns: '1' if the selected PLL oscillator is locked, otherwise 0
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_clk_en_dis(CLKPWR_CLK_T clk,
                       INT_32 enable)
{
  switch (clk)
  {
#if 0 // Not supported
    case CLKPWR_USB_HCLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_pwr_ctrl,
                          CLKPWR_DISABLE_USB_HCLK, !enable);
      break;
#endif
    case CLKPWR_LCD_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_lcdclk_ctrl,
                          CLKPWR_LCDCTRL_CLK_EN, enable);
      break;

    case CLKPWR_SSP1_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_ssp_blk_ctrl,
                          CLKPWR_SSPCTRL_SSPCLK1_EN, enable);
      break;

    case CLKPWR_SSP0_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_ssp_blk_ctrl,
                          CLKPWR_SSPCTRL_SSPCLK0_EN, enable);
      break;

    case CLKPWR_I2S1_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_i2s_clk_ctrl,
                          CLKPWR_I2SCTRL_I2SCLK1_EN, enable);
      break;

    case CLKPWR_I2S0_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_i2s_clk_ctrl,
                          CLKPWR_I2SCTRL_I2SCLK0_EN, enable);
      break;

    case CLKPWR_MSCARD_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_ms_ctrl,
                          CLKPWR_MSCARD_SDCARD_EN, enable);
      if ((CLKPWR->clkpwr_ms_ctrl &
           CLKPWR_MSCARD_SDCARD_DIV(0xF)) == 0)
      {
        /* Set fastest clock */
        CLKPWR->clkpwr_ms_ctrl |= CLKPWR_MSCARD_SDCARD_DIV(1);
      }
      break;

    case CLKPWR_MAC_DMA_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_macclk_ctrl,
                          CLKPWR_MACCTRL_DMACLK_EN, enable);
      break;

    case CLKPWR_MAC_MMIO_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_macclk_ctrl,
                          CLKPWR_MACCTRL_MMIOCLK_EN, enable);
      break;

    case CLKPWR_MAC_HRC_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_macclk_ctrl,
                          CLKPWR_MACCTRL_HRCCLK_EN, enable);
      break;

    case CLKPWR_I2C2_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_i2c_clk_ctrl,
                          CLKPWR_I2CCLK_I2C2CLK_EN, enable);
      break;

    case CLKPWR_I2C1_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_i2c_clk_ctrl,
                          CLKPWR_I2CCLK_I2C1CLK_EN, enable);
      break;

    case CLKPWR_KEYSCAN_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_key_clk_ctrl,
                          CLKPWR_KEYCLKCTRL_CLK_EN, enable);
      break;

    case CLKPWR_ADC_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_adc_clk_ctrl,
                          CLKPWR_ADC32CLKCTRL_CLK_EN, enable);
      break;

    case CLKPWR_PWM2_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_pwm_clk_ctrl,
                          CLKPWR_PWMCLK_PWM2CLK_EN, enable);
      break;

    case CLKPWR_PWM1_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_pwm_clk_ctrl,
                          CLKPWR_PWMCLK_PWM1CLK_EN, enable);
      break;

    case CLKPWR_HSTIMER_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timer_clk_ctrl,
                          CLKPWR_PWMCLK_HSTIMER_EN, enable);
      break;

    case CLKPWR_WDOG_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timer_clk_ctrl,
                          CLKPWR_PWMCLK_WDOG_EN, enable);
      break;

    case CLKPWR_TIMER3_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timers_pwms_clk_ctrl_1,
                          CLKPWR_TMRPWMCLK_TIMER3_EN, enable);
      break;

    case CLKPWR_TIMER2_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timers_pwms_clk_ctrl_1,
                          CLKPWR_TMRPWMCLK_TIMER2_EN, enable);
      break;

    case CLKPWR_TIMER1_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timers_pwms_clk_ctrl_1,
                          CLKPWR_TMRPWMCLK_TIMER1_EN, enable);
      break;

    case CLKPWR_TIMER0_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timers_pwms_clk_ctrl_1,
                          CLKPWR_TMRPWMCLK_TIMER0_EN, enable);
      break;

    case CLKPWR_PWM4_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timers_pwms_clk_ctrl_1,
                          CLKPWR_TMRPWMCLK_PWM4_EN, enable);
      break;

    case CLKPWR_PWM3_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_timers_pwms_clk_ctrl_1,
                          CLKPWR_PWM3_CLK, enable);
      break;

    case CLKPWR_SPI2_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_spi_clk_ctrl,
                          CLKPWR_SPICLK_SPI2CLK_EN, enable);
      break;

    case CLKPWR_SPI1_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_spi_clk_ctrl,
                          CLKPWR_SPICLK_SPI1CLK_EN, enable);
      break;

    case CLKPWR_NAND_SLC_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_nand_clk_ctrl,
                          CLKPWR_NANDCLK_SLCCLK_EN, enable);
      break;

    case CLKPWR_NAND_MLC_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_nand_clk_ctrl,
                          CLKPWR_NANDCLK_MLCCLK_EN, enable);
      break;

    case CLKPWR_UART6_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_uart_clk_ctrl,
                          CLKPWR_UARTCLKCTRL_UART6_EN, enable);
      break;

    case CLKPWR_UART5_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_uart_clk_ctrl,
                          CLKPWR_UARTCLKCTRL_UART5_EN, enable);
      break;

    case CLKPWR_UART4_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_uart_clk_ctrl,
                          CLKPWR_UARTCLKCTRL_UART4_EN, enable);
      break;

    case CLKPWR_UART3_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_uart_clk_ctrl,
                          CLKPWR_UARTCLKCTRL_UART3_EN, enable);
      break;

    case CLKPWR_DMA_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_dmaclk_ctrl,
                          CLKPWR_DMACLKCTRL_CLK_EN, enable);
      break;

    case CLKPWR_SDRAMDDR_CLK:
      clkpwr_mask_and_set(&CLKPWR->clkpwr_sdramclk_ctrl,
                          CLKPWR_SDRCLK_CLK_DIS, !enable);
      break;

    default:
      break;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_autoclk_en_dis
 *
 * Purpose: Enable or disable a autoclock
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     clk    : Clock to set
 *     enable : '1' to enable autoclock, 0 to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_autoclk_en_dis(CLKPWR_AUTOCLK_T clk,
                           INT_32 enable)
{
  volatile UNS_32 tmp;

  switch (clk)
  {
    case CLKPWR_ACLK_USB_DEV:
      tmp = (CLKPWR->clkpwr_autoclock & ~CLKPWR_AUTOCLK_USB_EN);
      if (enable != 0)
      {
        tmp |= CLKPWR_AUTOCLK_USB_EN;
      }
      CLKPWR->clkpwr_autoclock = tmp;
      break;

    case CLKPWR_ACLK_IRAM:
      tmp = (CLKPWR->clkpwr_autoclock & ~CLKPWR_AUTOCLK_IRAM_EN);
      if (enable != 0)
      {
        tmp |= CLKPWR_AUTOCLK_IRAM_EN;
      }
      CLKPWR->clkpwr_autoclock = tmp;
      break;

    case CLKPWR_ACLK_IROM:
      tmp = (CLKPWR->clkpwr_autoclock & ~CLKPWR_AUTOCLK_IROM_EN);
      if (enable != 0)
      {
        tmp |= CLKPWR_AUTOCLK_IROM_EN;
      }
      CLKPWR->clkpwr_autoclock = tmp;
      break;

    default:
      break;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_get_clock_rate
 *
 * Purpose: Get the clock frequency for a specific clocked peripheral
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ipclk : Clock to get the rate of
 *
 * Outputs: None
 *
 * Returns:
 *     The current clock rate for the selected peripheral in Hz, or 0
 *     if the clock rate can't be determined.
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 clkpwr_get_clock_rate(CLKPWR_CLK_T ipclk)
{
  CLKPWR_BASE_CLOCK_T baseclk;
  UNS_32 tmp, baseclkrate = 0, divrate = 1;

  /* Get base clock for the selected clock */
  baseclk = clkpwr_get_base_clock(ipclk);
  if (baseclk != CLKPWR_BASE_INVALID)
  {
    /* Based on ipclk, determine possible divders at the clock
    and power control block that apply to the IP clock */
    baseclkrate = clkpwr_get_base_clock_rate(baseclk);
    switch (ipclk)
    {
      case CLKPWR_MSCARD_CLK:
        tmp = CLKPWR->clkpwr_ms_ctrl &
              CLKPWR_MSCARD_SDCARD_DIV(0xF);
        if (tmp == 0)
        {
          baseclkrate = 0;
        }
        else
        {
          divrate = tmp;
        }
        break;

      case CLKPWR_LCD_CLK:
        tmp = (CLKPWR->clkpwr_lcdclk_ctrl &
               CLKPWR_LCDCTRL_PSCALE_MSK);
        divrate = 1 + tmp;
        break;

      case CLKPWR_ADC_CLK:
        /* Clock divider only applies when peripheral clock
           is used for ADC clock */
        tmp = CLKPWR->clkpwr_adc_clk_ctrl_1;
        if ((tmp & CLKPWR_ADCCTRL1_PCLK_SEL) != 0)
        {
          tmp = (tmp & CLKPWR_ADCCTRL1_RTDIV(0xF) >> 8);
          if (tmp == 0)
          {
            baseclkrate = 0;
          }
          else
          {
            divrate = tmp;
          }
        }
        break;

      case CLKPWR_PWM2_CLK:
        tmp = (CLKPWR->clkpwr_pwm_clk_ctrl &
               CLKPWR_PWMCLK_PWM2_DIV(0xF) >> 8);
        if (tmp == 0)
        {
          baseclkrate = 0;
        }
        else
        {
          divrate = tmp;
        }
        break;

      case CLKPWR_PWM1_CLK:
        tmp = (CLKPWR->clkpwr_pwm_clk_ctrl &
               CLKPWR_PWMCLK_PWM1_DIV(0xF) >> 4);
        if (tmp == 0)
        {
          baseclkrate = 0;
        }
        else
        {
          divrate = tmp;
        }
        break;

      default:
        break;
    }
  }

  return (baseclkrate / divrate);
}

/***********************************************************************
 *
 * Function: clkpwr_wk_event_en_dis
 *
 * Purpose: Enable or disable a wakeup event
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     event_id : Event enumeration to change
 *     enable  : '1' to enable the event for wakeup, '0' to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_wk_event_en_dis(CLKPWR_EVENT_T event_id,
                            INT_32 enable)
{
  volatile UNS_32 tmp;
  INT_32 bitnum = 0;
  CLKPWR_EVENTREG_GROUP_T *pRegs = NULL;

  if (clkpwr_get_event_field(event_id, pRegs, &bitnum) == 0)
  {
    /* Invalid */
    return;
  }

  tmp = *pRegs->start_er;
  if (enable != 0)
  {
    tmp |= _BIT(bitnum);
  }
  else
  {
    tmp &= ~_BIT(bitnum);
  }

  *pRegs->start_er = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_is_raw_event_active
 *
 * Purpose: Get the raw captured status of an event
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     event_id : Event enumeration to check
 *
 * Outputs: None
 *
 * Returns: '1' if the selected event is active, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 clkpwr_is_raw_event_active(CLKPWR_EVENT_T event_id)
{
  INT_32 event_sts = 0;
  INT_32 bitnum = 0;
  CLKPWR_EVENTREG_GROUP_T *pRegs = NULL;

  if (clkpwr_get_event_field(event_id, pRegs, &bitnum) != 0)
  {
    if ((*pRegs->start_rsr & _BIT(bitnum)) != 0)
    {
      event_sts = 1;
    }
  }

  return event_sts;
}

/***********************************************************************
 *
 * Function: clkpwr_is_msk_event_active
 *
 * Purpose: Get the masked captured status of an event
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     event_id : Event enumeration to check
 *
 * Outputs: None
 *
 * Returns: '1' if the selected event is active, otherwise '0'
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 clkpwr_is_msk_event_active(CLKPWR_EVENT_T event_id)
{
  INT_32 event_sts = 0;
  INT_32 bitnum = 0;
  CLKPWR_EVENTREG_GROUP_T *pRegs = NULL;

  if (clkpwr_get_event_field(event_id, pRegs, &bitnum) == 0)
  {
    if ((*pRegs->start_sr & _BIT(bitnum)) != 0)
    {
      event_sts = 1;
    }
  }

  return event_sts;
}

/***********************************************************************
 *
 * Function: clkpwr_clear_event
 *
 * Purpose: Clear a captured event status
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     event_id : Event enumeration to  clear
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_clear_event(CLKPWR_EVENT_T event_id)
{
  INT_32 bitnum = 0;
  CLKPWR_EVENTREG_GROUP_T *pRegs = NULL;

  if (clkpwr_get_event_field(event_id, pRegs, &bitnum) == 0)
  {
    /* Invalid */
    return;
  }

  /* Clear selected event */
  *pRegs->start_rsr = _BIT(bitnum);
}

/***********************************************************************
 *
 * Function: clkpwr_set_event_pol
 *
 * Purpose:
 *     Set event signal polarity (0) = falling edge, (1) = rising edge
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     event_id : Event enumeration to set polarity of
 *     high     : '1' for high [polarity, '0' for low polarity
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_set_event_pol(CLKPWR_EVENT_T event_id,
                          INT_32 high)
{
  volatile UNS_32 tmp;
  INT_32 bitnum = 0;
  CLKPWR_EVENTREG_GROUP_T *pRegs = NULL;

  if (clkpwr_get_event_field(event_id, pRegs, &bitnum) == 0)
  {
    /* Invalid */
    return;
  }

  /* Set polarity high or low */
  tmp = *pRegs->start_apr & ~_BIT(bitnum);
  if (high != 0)
  {
    tmp |= _BIT(bitnum);
  }
  *pRegs->start_apr = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_get_uid
 *
 * Purpose:
 *     Returns the chip's unoqie ID into the passed array.
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     uid: Array to fill with 4 32-bit unique ID words
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_get_uid(UNS_32 uid[4]) {
	uid[0] = CLKPWR->clkpwr_uid[0];
	uid[1] = CLKPWR->clkpwr_uid[1];
	uid[2] = CLKPWR->clkpwr_uid[2];
	uid[3] = CLKPWR->clkpwr_uid[3];
}

/***********************************************************************
 *
 * Function: clkpwr_set_mux
 *
 * Purpose:
 *     Set a muxed GPO signal to it's default (0) or GPO state (1)
 *
 * Processing:
 *     Change the function of the selected muxed pin based on the
 *     passed input value.
 *
 * Parameters:
 *     mux_signal: Muxed signal enumeration to change
 *     mux1      : Set to non-zero value to enable muxed state
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_set_mux(CLKPWR_MUX_STATE_T mux_signal,
                    INT_32 mux1)
{
  switch (mux_signal)
  {
    case CLKPWR_HIGHCORE:
      if (mux1 == 0)
      {
        CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_HIGHCORE_GPIO_EN;
      }
      else
      {
        CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_HIGHCORE_GPIO_EN;
      }
      break;

    case CLKPWR_SYSCLKEN:
      if (mux1 == 0)
      {
        CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_SYSCLKEN_GPIO_EN;
      }
      else
      {
        CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_SYSCLKEN_GPIO_EN;
      }
      break;

    case CLKPWR_TEST_CLK1:
      if (mux1 == 0)
      {
        CLKPWR->clkpwr_test_clk_sel &=
          ~CLKPWR_TESTCLK_TESTCLK1_EN;
      }
      else
      {
        CLKPWR->clkpwr_test_clk_sel |=
          CLKPWR_TESTCLK_TESTCLK1_EN;
      }
      break;

    case CLKPWR_TEST_CLK2:
      if (mux1 == 0)
      {
        CLKPWR->clkpwr_test_clk_sel &=
          ~CLKPWR_TESTCLK_TESTCLK2_EN;
      }
      else
      {
        CLKPWR->clkpwr_test_clk_sel |=
          CLKPWR_TESTCLK_TESTCLK2_EN;
      }
      break;

    case CLKPWR_SPI2_DATIO:
    case CLKPWR_SPI2_CLK_PAD:
      if (mux1 == 0)
      {
        CLKPWR->clkpwr_spi_clk_ctrl &= ~CLKPWR_SPICLK_USE_SPI2;
      }
      else
      {
        CLKPWR->clkpwr_spi_clk_ctrl |= CLKPWR_SPICLK_USE_SPI2;
      }
      break;

    case CLKPWR_SPI1_DATIO:
    case CLKPWR_SPI1_CLK_PAD:
      if (mux1 == 0)
      {
        CLKPWR->clkpwr_spi_clk_ctrl &= ~CLKPWR_SPICLK_USE_SPI1;
      }
      else
      {
        CLKPWR->clkpwr_spi_clk_ctrl |= CLKPWR_SPICLK_USE_SPI1;
      }
      break;

    default:
      break;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_set_mux_state
 *
 * Purpose: Set a muxed signal to passed value
 *
 * Processing:
 *     Set a muxed signal state for the passed selections to the
 *     passed state.
 *
 * Parameters:
 *     mux_signal: Muxed signal enumeration to change
 *     state     : Value to set the GPO output to
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_set_mux_state(CLKPWR_MUX_STATE_T mux_signal,
                          INT_32 state)
{
  volatile UNS_32 tmp;

  switch (mux_signal)
  {
    case CLKPWR_HIGHCORE:
      if (state == 0)
      {
        CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_HIGHCORE_STATE_BIT;
      }
      else
      {
        CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_HIGHCORE_STATE_BIT;
      }
      break;

    case CLKPWR_SYSCLKEN:
      if (state == 0)
      {
        CLKPWR->clkpwr_pwr_ctrl &= ~CLKPWR_SYSCLKEN_STATE_BIT;
      }
      else
      {
        CLKPWR->clkpwr_pwr_ctrl |= CLKPWR_SYSCLKEN_STATE_BIT;
      }
      break;

    case CLKPWR_TEST_CLK1:
      tmp = (CLKPWR->clkpwr_test_clk_sel &
             ~CLKPWR_TESTCLK1_SEL_MASK) | (UNS_32) state;
      CLKPWR->clkpwr_test_clk_sel = tmp;
      break;

    case CLKPWR_TEST_CLK2:
      tmp = (CLKPWR->clkpwr_test_clk_sel &
             ~CLKPWR_TESTCLK2_SEL_MASK) | (UNS_32) state;
      CLKPWR->clkpwr_test_clk_sel = tmp;
      break;

    case CLKPWR_SPI2_DATIO:
      if (state == 0)
      {
        CLKPWR->clkpwr_spi_clk_ctrl &=
          ~CLKPWR_SPICLK_SET_SPI2DATIO;
      }
      else
      {
        CLKPWR->clkpwr_spi_clk_ctrl |=
          CLKPWR_SPICLK_SET_SPI2DATIO;
      }
      break;

    case CLKPWR_SPI2_CLK_PAD:
      if (state == 0)
      {
        CLKPWR->clkpwr_spi_clk_ctrl &=
          ~CLKPWR_SPICLK_SET_SPI2CLK;
      }
      else
      {
        CLKPWR->clkpwr_spi_clk_ctrl |=
          CLKPWR_SPICLK_SET_SPI2CLK;
      }
      break;

    case CLKPWR_SPI1_DATIO:
      if (state == 0)
      {
        CLKPWR->clkpwr_spi_clk_ctrl &=
          ~CLKPWR_SPICLK_SET_SPI1DATIO;
      }
      else
      {
        CLKPWR->clkpwr_spi_clk_ctrl |=
          CLKPWR_SPICLK_SET_SPI1DATIO;
      }
      break;

    case CLKPWR_SPI1_CLK_PAD:
      if (state == 0)
      {
        CLKPWR->clkpwr_spi_clk_ctrl &=
          ~CLKPWR_SPICLK_SET_SPI1CLK;
      }
      else
      {
        CLKPWR->clkpwr_spi_clk_ctrl |=
          CLKPWR_SPICLK_SET_SPI1CLK;
      }
      break;

    default:
      break;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_setup_mcard_ctrlr
 *
 * Purpose: Memory card (MSSDIO) pad and clock setup
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     enable_pullups   : '1 to enable all pullups, '0' to disable
 *     dat2_3_pullup_on : '1' to enable pullups on data 2 and 3 pads
 *     dat1_pullup_on   : '1' to enable pullups on data 1 pad
 *     dat0_pullup_on   : '1' to enable pullups on data 0 pad
 *     freq_div         : 'Must be 1 to 15, or 0 to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_setup_mcard_ctrlr(INT_32 enable_pullups,
                              INT_32 dat2_3_pullup_on,
                              INT_32 dat1_pullup_on,
                              INT_32 dat0_pullup_on,
                              INT_32 freq_div)
{
  UNS_32 tmp;

  tmp = CLKPWR->clkpwr_ms_ctrl & CLKPWR_MSCARD_SDCARD_EN;
  if (enable_pullups != 0)
  {
    tmp |= CLKPWR_MSCARD_MSDIO_PU_EN;
  }
  if (dat2_3_pullup_on == 0)
  {
    tmp |= CLKPWR_MSCARD_MSDIO23_DIS;
  }
  if (dat1_pullup_on == 0)
  {
    tmp |= CLKPWR_MSCARD_MSDIO1_DIS;
  }
  if (dat0_pullup_on == 0)
  {
    tmp |= CLKPWR_MSCARD_MSDIO0_DIS;
  }
  tmp |= CLKPWR_MSCARD_SDCARD_DIV(freq_div);
  CLKPWR->clkpwr_ms_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_setup_pwm
 *
 * Purpose: Set the drive strength of I2C signals
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     i2c_num : Must be 1 or 2
 *     high    : '1' for high driver
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_set_i2c_driver(INT_32 i2c_num,
                           INT_32 high)
{
  volatile UNS_32 tmp;

  if (i2c_num == 1)
  {
    tmp = CLKPWR->clkpwr_i2c_clk_ctrl & ~CLKPWR_I2CCLK_I2C1HI_DRIVE;
    if (high != 0)
    {
      tmp |= CLKPWR_I2CCLK_I2C1HI_DRIVE;
    }
    CLKPWR->clkpwr_i2c_clk_ctrl = tmp;
  }
  else if (i2c_num == 2)
  {
    tmp = CLKPWR->clkpwr_i2c_clk_ctrl & ~CLKPWR_I2CCLK_I2C2HI_DRIVE;
    if (high != 0)
    {
      tmp |= CLKPWR_I2CCLK_I2C2HI_DRIVE;
    }
    CLKPWR->clkpwr_i2c_clk_ctrl = tmp;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_setup_pwm
 *
 * Purpose:
 *     Configure PWM 1 or 2 with a clock source and divider value
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     pwm_num  : Must be 1 or 2
 *     clk_src  : '0' for 32KHz clock, 1 for PERIPH_CLK
 *     freq_dev : Must be 1 to 15, or 0 to disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_setup_pwm(INT_32 pwm_num,
                      INT_32 clk_src,
                      INT_32 freq_dev)
{
  volatile UNS_32 tmp;

  if (pwm_num == 1)
  {
    tmp = CLKPWR->clkpwr_pwm_clk_ctrl &
          ~(CLKPWR_PWMCLK_PWM1_DIV(0xF) | CLKPWR_PWMCLK_PWM1SEL_PCLK);
    if (clk_src != 0)
    {
      tmp |= CLKPWR_PWMCLK_PWM1SEL_PCLK;
    }
    tmp |= CLKPWR_PWMCLK_PWM1_DIV(freq_dev);
    CLKPWR->clkpwr_pwm_clk_ctrl = tmp;
  }
  else if (pwm_num == 2)
  {
    tmp = CLKPWR->clkpwr_pwm_clk_ctrl &
          ~(CLKPWR_PWMCLK_PWM2_DIV(0xF) | CLKPWR_PWMCLK_PWM2SEL_PCLK);
    if (clk_src != 0)
    {
      tmp |= CLKPWR_PWMCLK_PWM2SEL_PCLK;
    }
    tmp |= CLKPWR_PWMCLK_PWM2_DIV(freq_dev);
    CLKPWR->clkpwr_pwm_clk_ctrl = tmp;
  }
}

/***********************************************************************
 *
 * Function: clkpwr_setup_nand_ctrlr
 *
 * Purpose: NAND FLASH controller setup (except clocks)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     use_slc            : '1' for SLC, '0' for MLC
 *     use_dma_req_on_rnb : Enable DMA_REQ signal on NAND_RnB
 *     use_dma_req_on_int : Enable DMA_REQ signal on NAND_INT
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_setup_nand_ctrlr(INT_32 use_slc,
                             INT_32 use_dma_req_on_rnb,
                             INT_32 use_dma_req_on_int)
{
  UNS_32 tmp;

  if (use_slc != 0)
  {
    /* Select SLC interface and SLC interrupt */
    tmp = CLKPWR_NANDCLK_SEL_SLC;
  }
  else
  {
    /* Select MLC interface and MLC interrupt */
    tmp = CLKPWR_NANDCLK_INTSEL_MLC;
  }

  /* Enable DMA support */
  if (use_dma_req_on_rnb != 0)
  {
    tmp |= CLKPWR_NANDCLK_DMA_RNB;
  }
  if (use_dma_req_on_int != 0)
  {
    tmp |= CLKPWR_NANDCLK_DMA_INT;
  }

  CLKPWR->clkpwr_nand_clk_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_setup_adc_ts
 *
 * Purpose: ADC/touchscreen controller setup
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     sel_periph_clk : '1' for PERIPH_CLK, '0' for RTC
 *     clkdiv         : ADC clock divider, 0 to 15, or 0 for off
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_setup_adc_ts(INT_32 sel_periph_clk,
                         INT_32 clkdiv)
{
  UNS_32 tmp;

  tmp = CLKPWR_ADCCTRL1_RTDIV(clkdiv);
  if (sel_periph_clk != 0)
  {
    tmp |= CLKPWR_ADCCTRL1_PCLK_SEL;
  }

  CLKPWR->clkpwr_adc_clk_ctrl_1 = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_setup_ssp
 *
 * Purpose: SSP controller setup
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     ssp1_rx_dma_en : 1 = Enable SSP1 RX DMA, 0 = disable
 *     ssp1_tx_dma_en : 1 = Enable SSP1 RX DMA, 0 = disable
 *     ssp0_rx_dma_en : 1 = Enable SSP1 RX DMA, 0 = disable
 *     ssp0_tx_dma_en : 1 = Enable SSP1 RX DMA, 0 = disable
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Enabling SSP DMA will disable DMA for SPI peripherals.
 *
 **********************************************************************/
void clkpwr_setup_ssp(INT_32 ssp1_rx_dma_en,
                      INT_32 ssp1_tx_dma_en,
                      INT_32 ssp0_rx_dma_en,
                      INT_32 ssp0_tx_dma_en)
{
  volatile UNS_32 tmp;

  tmp = CLKPWR->clkpwr_ssp_blk_ctrl & ~(CLKPWR_SSPCTRL_DMA_SSP1RX |
    CLKPWR_SSPCTRL_DMA_SSP1TX | CLKPWR_SSPCTRL_DMA_SSP0RX |
    CLKPWR_SSPCTRL_DMA_SSP0TX);
  if (ssp1_rx_dma_en != 0)
  {
    tmp |= CLKPWR_SSPCTRL_DMA_SSP1RX;
  }
  if (ssp1_tx_dma_en != 0)
  {
    tmp |= CLKPWR_SSPCTRL_DMA_SSP1TX;
  }
  if (ssp0_rx_dma_en != 0)
  {
    tmp |= CLKPWR_SSPCTRL_DMA_SSP0RX;
  }
  if (ssp0_tx_dma_en != 0)
  {
    tmp |= CLKPWR_SSPCTRL_DMA_SSP0TX;
  }

  CLKPWR->clkpwr_ssp_blk_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_setup_i2s
 *
 * Purpose: I2S controller setup
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     i2s1_tx_clk_from_rx : 1 = I2S1 TX clock = RX clock, 0 = swap
 *     i2s1_rx_clk_from_tx : 1 = I2S1 RX clock = TX clock, 0 = swap
 *     i2s1_use_dma        : 1 = Enable I2S1 RX DMA, 0 = UART7 DMA
 *     i2s0_rx_clk_from_tx : 1 = I2S0 TX clock = RX clock, 0 = swap
 *     i2s0_tx_clk_from_rx : 1 = I2S0 RX clock = TX clock, 0 = swap
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Enabling I2S1 DMA will disable DMA for UART7 peripherals.
 *
 **********************************************************************/
void clkpwr_setup_i2s(INT_32 i2s1_tx_clk_from_rx,
                      INT_32 i2s1_rx_clk_from_tx,
                      INT_32 i2s1_use_dma,
                      INT_32 i2s0_rx_clk_from_tx,
                      INT_32 i2s0_tx_clk_from_rx)
{
  volatile UNS_32 tmp;

  tmp = CLKPWR->clkpwr_i2s_clk_ctrl & ~(CLKPWR_I2SCTRL_I2S1_RX_FOR_TX |
    CLKPWR_I2SCTRL_I2S1_TX_FOR_RX | CLKPWR_I2SCTRL_I2S1_USE_DMA |
    CLKPWR_I2SCTRL_I2S0_RX_FOR_TX | CLKPWR_I2SCTRL_I2S0_TX_FOR_RX);
  if (i2s1_tx_clk_from_rx != 0)
  {
    tmp |= CLKPWR_I2SCTRL_I2S1_RX_FOR_TX;
  }
  if (i2s1_rx_clk_from_tx != 0)
  {
    tmp |= CLKPWR_I2SCTRL_I2S1_TX_FOR_RX;
  }
  if (i2s1_use_dma != 0)
  {
    tmp |= CLKPWR_I2SCTRL_I2S1_USE_DMA;
  }
  if (i2s0_tx_clk_from_rx != 0)
  {
    tmp |= CLKPWR_I2SCTRL_I2S0_RX_FOR_TX;
  }
  if (i2s0_rx_clk_from_tx != 0)
  {
    tmp |= CLKPWR_I2SCTRL_I2S0_TX_FOR_RX;
  }

  CLKPWR->clkpwr_i2s_clk_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_setup_lcd
 *
 * Purpose: LCD controller setup
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     lcdpins    : Select pin mux group
 *     clkdiv     : Must be 1 to 32
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_setup_lcd(LCD_PIN_MUX_T lcdpins,
                      INT_32 clkdiv)
{
  volatile UNS_32 tmp;

  tmp = CLKPWR->clkpwr_lcdclk_ctrl & ~(CLKPWR_LCDCTRL_LCDTYPE_MSK |
                                       CLKPWR_LCDCTRL_PSCALE_MSK);
  switch (lcdpins)
  {
    case CLKPWR_LCDMUX_TFT12:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_TFT12;
      break;

    case CLKPWR_LCDMUX_TFT16:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_TFT16;
      break;

    case CLKPWR_LCDMUX_TFT15:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_TFT15;
      break;

    case CLKPWR_LCDMUX_TFT24:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_TFT24;
      break;

    case CLKPWR_LCDMUX_STN4M:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_STN4M;
      break;

    case CLKPWR_LCDMUX_STN8C:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_STN8C;
      break;

    case CLKPWR_LCDMUX_DSTN4M:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_DSTN4M;
      break;

    case CLKPWR_LCDMUX_DSTN8C:
      tmp = CLKPWR_LCDCTRL_LCDTYPE_DSTN8C;
      break;

    default:
      break;
  }

  /* Prescaler */
  tmp |= CLKPWR_LCDCTRL_SET_PSCALE(clkdiv);
  CLKPWR->clkpwr_lcdclk_ctrl = tmp;
}

/***********************************************************************
 *
 * Function: clkpwr_select_enet_iface
 *
 * Purpose: Ethernet enable and select MII/RMII interface selection
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     enable   : 1 = enable, 0 = disable
 *     use_rmii : 1 = RMII, 0 = MII
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void clkpwr_select_enet_iface(INT_32 enable,
                              INT_32 use_rmii)
{
  volatile UNS_32 tmp;

  tmp = CLKPWR->clkpwr_macclk_ctrl & ~CLKPWR_MACCTRL_PINS_MSK;
  if (enable == 0)
  {
    tmp |= CLKPWR_MACCTRL_NO_ENET_PIS;
  }
  else if (use_rmii == 0)
  {
    tmp |= CLKPWR_MACCTRL_USE_RMII_PINS;
  }
  else
  {
    tmp |= CLKPWR_MACCTRL_USE_MII_PINS;
  }

  CLKPWR->clkpwr_macclk_ctrl = tmp;
}



/***********************************************************************
 *
 * Function: clkpwr_halt_cpu
 *
 * Purpose: Enter low power mode (clocks disabled) in the CPU core
 *
 * Processing:
 *     Issue the coprocessor CP15 command to the CPU core to halt the
 *     CPU clocks until an interrupt occurs.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     See the ARM926EJ-S TRM for more information on the wait
 *     for inetrrupt command.
 *
 **********************************************************************/
void clkpwr_halt_cpu(void)
{
  register UNS_32 status = 0;

  /* Read the MMU control register */
#ifdef __GNUC__
asm("MCR p15, 0, %0, c7, c0, 4" : "=r"(status));
#endif
#ifdef __arm
  __asm
  {
    MCR p15, 0, status, c7, c0, 4;
  }
#endif
#ifdef __ICCARM__
  /* IAR CC includes some "intrinsic" functions to access ARM CP regs */
  __MCR(15, 0, status, 7, 0, 4);
#endif
}
