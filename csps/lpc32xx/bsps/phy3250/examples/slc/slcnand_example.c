/***********************************************************************
 * $Id:: slcnand_example.c 1263 2008-10-28 23:21:37Z wellsk            $
 *
 * Project: SLC NAND controller test
 *
 * Description:
 *     SLC NAND controller test
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
#include "phy3250_board.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_dma_driver.h"
#include "lpc_nandflash_params.h"
#include "lpc32xx_slcnand_driver.h"
#include "lpc32xx_clcdc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_lcd_params.h"
#include "lpc_swim.h"
#include "lpc_swim_font.h"
#include "lpc_string.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Device handles */
static INT_32 slcnanddev, lcddev;

/* SLC NAND flash */
#define SLCNAND_FLASH st_nand256r3a

/* SWIM window handle */
static SWIM_WINDOW_T sw;

/* Location in memory of LCD frame buffer */
#define LCD_BUFF_ADDR 0x81000000

/* Selected display type */
#define LCD_DISPLAY hitachi_tx09d71

/* FLASH data buffer */
UNS_32 buffer [528 / sizeof(UNS_32)];

/* Enable the following define to allow block erase/write operations
   to occur in the example. Enabling these operations may erase
   important information in the FLASH. Use with care! */
#define USEWRITE

/***********************************************************************
 *
 * Function: dump_data
 *
 * Purpose: Dump some values to the display
 *
 * Processing:
 *     Converts the values in the passed array to strings and displays
 *     them in the SWIM window.
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
void dump_data(char *str,
			   void *buff,
			   int bytes)
{
  int idx;
  UNS_32 tmp;
  UNS_8 gstr[32], *buff8;

  swim_put_ltext(&sw, str);
  swim_put_newline(&sw);

  /* Convert each value in the array to a string and dump the string
     to the display windows */
  idx = 0;
  buff8 = (UNS_8 *) buff;
  while (idx < bytes)
  {
	  tmp = (UNS_32) buff8 [idx];
	  str_makehex(gstr, tmp, 2);
	  swim_put_ltext(&sw, (const char *) gstr);
	  swim_put_text(&sw, " ");
	  idx++;
  }

  swim_put_newline(&sw);
}

/***********************************************************************
 *
 * Function: lcd_setup
 *
 * Purpose: LCD and SWIM setup code
 *
 * Processing:
 *     Enables the LCD and sets up a SWIM window for text output.
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
void lcd_setup(void)
{
	COLOR_T *fblog;

    /* Setup LCD muxing for STN Color 16BPP */
    clkpwr_setup_lcd(CLKPWR_LCDMUX_TFT16, 1);

    /* Enable clock to LCD block (HCLK_EN)*/
    clkpwr_clk_en_dis(CLKPWR_LCD_CLK, 1);

	/* Setup LCD paramaters in the LCD controller */
    lcddev = lcd_open(CLCDC, (INT_32) &LCD_DISPLAY);

    /* Upper Panel Frame Base Address register */
    lcd_ioctl(lcddev, LCD_SET_UP_FB, LCD_BUFF_ADDR); 

    /* Enable LCD controller and power signals */
    lcd_ioctl(lcddev, LCD_PWENABLE, 1);

    /* Enable LCD backlight */
    phy3250_lcd_backlight_enable(TRUE);

    /* Enable LCD power */
    phy3250_lcd_power_enable(TRUE);

    /* Set frame buffer address */
    fblog = (COLOR_T *) 
            cp15_map_physical_to_virtual(LCD_BUFF_ADDR);

    /* Create a SWIM window */
    swim_window_open(&sw, LCD_DISPLAY.pixels_per_line,
        LCD_DISPLAY.lines_per_panel, fblog, 0, 0,
		(LCD_DISPLAY.pixels_per_line - 1), 
		(LCD_DISPLAY.lines_per_panel - 1),1, WHITE, BLACK, BLACK);
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
 * Returns: None
 *
 * Notes: None
 *
 **********************************************************************/
void c_entry(void)
{
  SLC_TAC_T tac;
  UNS_32 flashid;
  SLC_BLOCKPAGE_T blockpage;
  int idx;
  UNS_8 tstr[32];

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

  /* Setup DMA */
  dma_init();

  /* Enable interrupts in ARM core */
  enable_irq_fiq();

  /* Setup the LCD with SWIM for output */
  lcd_setup();

  /* Open the SLC NAND controller */
  swim_put_text(&sw, "Opening NAND SLC driver\n");
  slcnanddev = slcnand_open(SLCNAND, (INT_32) &SLCNAND_FLASH);
  if (slcnanddev == 0)
  {
    swim_put_text(&sw, "Error opening NAND SLC driver\n");
    goto testfail;
  }

  /* Setup SLC NAND controller timing */
  tac.w_rdy = 14;
  tac.w_width = 5;
  tac.w_hold = 2;
  tac.w_setup = 1;
  tac.r_rdy = 14;
  tac.r_width = 4;
  tac.r_hold = 2;
  tac.r_setup = 1;

  swim_put_text(&sw, "Setting SLC NAND timing\n");
  if (slcnand_ioctl(slcnanddev, SLC_SET_TIMING, (INT_32) &tac)
                    == _ERROR)
  {
    swim_put_text(&sw, "Error setting SLC NAND timing\n");
    goto testfail;
  }

  /* Get SLC NAND flash id */
  if (slcnand_ioctl(slcnanddev, SLC_READ_ID,
                    (INT_32) &flashid) == _ERROR)
  {
    swim_put_text(&sw, "Error reading NAND ID\n");
    goto testfail;
  }
  flashid = flashid & 0xFFFF;
  swim_put_text(&sw, "Detected FLASH ID is \n");
  str_makehex(tstr, flashid, 4);
  swim_put_ltext(&sw, (const char *) tstr);
  swim_put_newline(&sw);

  swim_put_text(&sw, "Reading NAND spare area\n");
  /* Read SLC NAND flash spare
                    dma disabled, ecc disabled */
  blockpage.dma = 0;
  blockpage.ecc = 0;
  blockpage.block_num = 0;
  blockpage.page_num = 0;
  blockpage.buffer = (UNS_8 *) buffer;

  if (slcnand_ioctl(slcnanddev, SLC_READ_SPARE,
                    (INT_32) &blockpage) == _ERROR)
  {
    swim_put_text(&sw, "Error reading block 0, page 0 spare area\n");
    goto testfail;
  }

  /* Dump data from spare area */
  dump_data("Data from spare area for block 0/page 0", buffer, 16);

#ifdef USEWRITE
  /* Erase block 100 */
  if (slcnand_ioctl(slcnanddev, SLC_ERASE_BLOCK, 100) == _ERROR)
  {
    swim_put_text(&sw, "Error erasing block 100\n");
    goto testfail;
  }

  /* Generate a test pattern for the page */
  for (idx = 0; idx < (512 / sizeof(UNS_32)); idx++)
  {
	  buffer [idx] = idx;
  }

  /* Write SLC NAND flash page
                    dma disabled, ecc disabled */
  blockpage.dma = 0;
  blockpage.ecc = 0;
  blockpage.block_num = 100;
  blockpage.page_num = 0;
  blockpage.buffer = (UNS_8 *) buffer;
  if (slcnand_ioctl(slcnanddev, SLC_WRITE_PAGE,
                    (INT_32) &blockpage) == _ERROR)
  {
    swim_put_text(&sw, "Error writing block 100/page 0\n");
    goto testfail;
  }

  /* Clear buffer for test pattern read */
  for (idx = 0; idx < (512 / sizeof(UNS_32)); idx++)
  {
	  buffer [idx] = idx;
  }
#endif

  /* Read SLC NAND flash page
                    dma disabled, ecc disabled */
  blockpage.dma = 0;
  blockpage.ecc = 0;
  blockpage.block_num = 100;
  blockpage.page_num = 0;
  blockpage.buffer = (UNS_8 *) buffer;
  if (slcnand_ioctl(slcnanddev, SLC_READ_PAGE,
                  (INT_32) &blockpage) == _ERROR)
  {
    swim_put_text(&sw, "Error reading block 100/page 0\n");
    goto testfail;
  }

  /* Dump first 32 bytes of data from page */
  dump_data("First 32 bytes of fdta from block 100/page 0", buffer, 32);

#ifdef USEWRITE
  /* Verify test pattern for the page */
  for (idx = 0; idx < (512 / sizeof(UNS_32)); idx++)
  {
	  if (buffer [idx] != idx)
	  {
	    swim_put_text(&sw, "Block 100/page 0 read failed!\n");
	    goto testfail;
	  }
  }
#endif

testfail:
  swim_put_text(&sw, "NAND SLC driver example complete\n");
  /* Disable interrupts in ARM core */
  disable_irq_fiq();

  slcnand_close(slcnanddev);
  timer_wait_ms(TIMER_CNTR0, 8000);

  /* Leave the display on so the results can still be viewed after
    the example is complete */
  while (1);
}
