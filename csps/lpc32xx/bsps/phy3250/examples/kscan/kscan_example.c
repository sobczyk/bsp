/***********************************************************************
 * $Id:: kscan_example.c 1261 2008-10-28 23:20:59Z wellsk              $
 *
 * Project: Keyboard scanner example
 *
 * Description:
 *     Keyboard scanner example using interrupts.
 *
 * Notes:
 *     Use of this example may require changing jumpers or
 *     configuration of the Phytec board.
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
#include "lpc_swim.h"
#include "lpc_swim_font.h"
#include "lpc_lcd_params.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_kscan_driver.h"
#include "lpc32xx_clcdc_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_rtc.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Number of key matrices this example supports */
#define MAXKEYS 1

/* Interrupt capture flag and capture buffer */
static volatile INT_32 cap = 0;
volatile UNS_32 sts [MAXKEYS];

/* Last statuses of all key captures */
static UNS_32 last_sts [MAXKEYS];

/* Key scanner driver ID */
static INT_32 kscan_id;
static BOOL_32 exitff;
static SWIM_WINDOW_T win1;

/* The display is used to output messages about the keyboard. This
   string matrix maps to a specific key being pressed. */
const char keyvals [MAXKEYS][MAXKEYS] =
{
#if MAXKEYS==1
  {'1'},
#endif
#if MAXKEYS==2
  {'1', '2'},
  {'4', '5'}
#endif
#if MAXKEYS==3
  {'1', '2', '3'},
  {'4', '5', '6'},
  {'7', '8', '9'}
#endif
#if MAXKEYS==4
  {'1', '2', '3', ' '},
  {'4', '5', '6', ' '},
  {'7', '8', '9', ' '},
  {'*', '0', '#', ' '}
#endif
};

#define PHY_LCD_FRAME_BUF 0x81000000
#define LCD_DISPLAY       hitachi_tx09d71

/***********************************************************************
 *
 * Function: kscan_user_interrupt
 *
 * Purpose: Keyboard scanner interrupt handler
 *
 * Processing:
 *     Saves the keyscan data and clears the interrupt.
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
static void kscan_user_interrupt(void)
{
  INT_32 idx;

  for (idx = 0; idx < MAXKEYS; idx++)
  {
    sts[idx] = KSCAN->ks_data[idx];
  }

  KSCAN->ks_irq = KSCAN_IRQ_PENDING_CLR;
  cap = 1;
}

/***********************************************************************
 *
 * Function: disp_key_chg
 *
 * Purpose: Determine changed key and state
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     matidx : Row that has a changed value
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void disp_key_chg(INT_32 matidx)
{
  INT_32 bitpos = 0;
  char str [3];

  /* Find changed bit positions */
  while (bitpos < MAXKEYS)
  {
    if ((sts[matidx] & _BIT(bitpos)) !=
        (last_sts[matidx] & _BIT(bitpos)))
    {
      /* Is this a key up or down state? */
      if ((last_sts[matidx] & _BIT(bitpos)) == 0)
      {
        /* Down state */
        swim_put_ltext(&win1, "Key pressed:");
      }
      else
      {
        /* Up state */
        swim_put_ltext(&win1, "Key released:");
      }

      str [0] = keyvals [matidx][bitpos];
      str [1] = '\n';
      swim_put_ltext(&win1, str);
    }

    bitpos++;
  }
}

/***********************************************************************
 *
 * Function: c_entry
 *
 * Purpose: Application entry point from the startup code
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
void c_entry(void)
{
  volatile UNS_32 status, newstatus;
  KSCAN_SETUP_T kstp;
  COLOR_T *fblog;
  INT_32 lcddev, idx;

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

  /* Install key scanner interrupt handlers as an IRQ interrupt */
  int_install_irq_handler(IRQ_KEY, (PFV) kscan_user_interrupt);

  /* Disable ethernet (which uses a lot of KSCAN pins) */
  clkpwr_select_enet_iface(0, 0);

  /* Setup LCD muxing for STN Color 16BPP */
  clkpwr_setup_lcd(CLKPWR_LCDMUX_TFT16, 1);

  /* Enable clock to LCD block (HCLK_EN)*/
  clkpwr_clk_en_dis(CLKPWR_LCD_CLK, 1);

  /* Setup LCD paramaters in the LCD controller */
  lcddev = lcd_open(CLCDC, (INT_32) & LCD_DISPLAY);

  /* Upper Panel Frame Base Address register */
  lcd_ioctl(lcddev, LCD_SET_UP_FB, PHY_LCD_FRAME_BUF);

  /* Enable LCD controller and power signals */
  lcd_ioctl(lcddev, LCD_PWENABLE, 1);

  /* Enable LCD backlight */
  phy3250_lcd_backlight_enable(TRUE);

  /* Enable LCD power */
  phy3250_lcd_power_enable(TRUE);

  /* Set frame buffer address */
  fblog = (COLOR_T *) cp15_map_physical_to_virtual(PHY_LCD_FRAME_BUF);

  /* Create a SWIM window */
  swim_window_open(&win1, LCD_DISPLAY.pixels_per_line,
                   LCD_DISPLAY.lines_per_panel, fblog, 0, 0,
                   (LCD_DISPLAY.pixels_per_line - 1),
                   (LCD_DISPLAY.lines_per_panel - 1),
                   1, WHITE, BLACK, BLACK);
  swim_put_ltext(&win1, "Key scanner example - press keys to see "
                 "key state changes.\n");

  /* Open key scan driver */
  kscan_id = kscan_open(KSCAN, 0);
  if (kscan_id == 0)
  {
    return;
  }

  /* Setup key scanner for a MAXKEYSxMAXKEYS matrix */
  kstp.deb_clks = 3; /* 3 debouce clocks */
  kstp.scan_delay_clks = 5; /* 5*32 clocks between scan states */
  kstp.pclk_sel = 1; /* RTC */
  kstp.matrix_size = MAXKEYS; /* MAXKEYSxMAXKEYS matrix */
  kscan_ioctl(kscan_id, KSCAN_SETUP, (INT_32) &kstp);

  /* Clear pending key scanner interrupt */
  kscan_ioctl(kscan_id, KSCAN_CLEAR_INT, 0);

  /* Clear initial key states */
  for (idx = 0; idx < MAXKEYS; idx++)
  {
    last_sts [idx] = sts [idx] = 0;
  }

  /* Enable key scanner interrupts in the interrupt controller */
  int_enable(IRQ_KEY);

  /* Enable IRQ interrupts in the ARM core */
  enable_irq();

  /* Loop for 30 seconds and dislpay key changes */
  exitff = FALSE;
  while (exitff == FALSE)
  {
    if (cap != 0)
    {
      /* Display changed keys */
      for (idx = 0; idx < MAXKEYS; idx++)
      {
        if (last_sts [idx] != sts [idx])
        {
          /* Find changed key */
          disp_key_chg(idx);
          last_sts [idx] = sts [idx];
        }
      }

      cap = 0;
    }
  }

  /* Disable KSCAN interrupts in the interrupt controller */
  int_disable(IRQ_KEY);

  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  /* Close KSCAN driver */
  kscan_close(kscan_id);
}
