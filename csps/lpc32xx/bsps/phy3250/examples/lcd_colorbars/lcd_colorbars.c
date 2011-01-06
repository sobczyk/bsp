/**********************************************************************
 * $Id:: lcd_colorbars.c 1260 2008-10-28 23:20:43Z wellsk             $
 *
 * Project: NXP PHY3250 LCD example
 *
 * Description:
 *     Draws color bars on the LCD. Requires RGB565 mode.
 *
 **********************************************************************
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

#include "lpc_types.h"
#include "lpc_irq_fiq.h"
#include "lpc_swim.h"
#include "lpc_lcd_params.h"
#include "lpc_arm922t_cp15_driver.h"
#include "phy3250_board.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clcdc_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_swim_font.h"
#include "lpc_swim.h"
#include "lpc_lcd_params.h"

#define PHY_LCD_FRAME_BUF 0x81000000
#define LCD_DISPLAY       hitachi_tx09d71

/**********************************************************************
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
 *********************************************************************/
void c_entry(void)
{
    SWIM_WINDOW_T win1;
    COLOR_T clr, *fblog;
    int idx;
    INT_32 lcddev;
    UNS_16 xgs, ygs, curx, cury, curym, xidx;

    /* Disable interrupts in ARM core */
    disable_irq_fiq();

    /* Set virtual address of MMU table */
    cp15_set_vmmu_addr((void *)
		(IRAM_BASE + (256 * 1024) - (16 * 1024)));

    /* Setup miscellaneous board functions */
    phy3250_board_init();
  
    /* Setup LCD muxing for STN Color 16BPP */
    clkpwr_setup_lcd(CLKPWR_LCDMUX_TFT16, 1);

    /* Enable clock to LCD block (HCLK_EN)*/
    clkpwr_clk_en_dis(CLKPWR_LCD_CLK, 1);
	
    /* Setup LCD paramaters in the LCD controller */
    lcddev = lcd_open(CLCDC, (INT_32) &LCD_DISPLAY);

    /* Upper Panel Frame Base Address register */
    lcd_ioctl(lcddev, LCD_SET_UP_FB, PHY_LCD_FRAME_BUF); 

    /* Enable LCD controller and power signals */
    lcd_ioctl(lcddev, LCD_PWENABLE, 1);

    /* Enable LCD backlight */
    phy3250_lcd_backlight_enable(TRUE);

    /* Enable LCD power */
    phy3250_lcd_power_enable(TRUE);

    /* Set frame buffer address */
    fblog = (COLOR_T *) 
            cp15_map_physical_to_virtual(PHY_LCD_FRAME_BUF);

    /* Create a SWIM window */
    swim_window_open(&win1, LCD_DISPLAY.pixels_per_line,
        LCD_DISPLAY.lines_per_panel, fblog, 0, 0,
		(LCD_DISPLAY.pixels_per_line - 1), 
		(LCD_DISPLAY.lines_per_panel - 1),1, WHITE, BLACK, BLACK);

    /* Compute vertical size for bars */
    ygs = LCD_DISPLAY.lines_per_panel / 3;

    /* Draw Red bars */
    cury = 0;
    curx = 0;
    curym = ygs - 1;
    xgs = LCD_DISPLAY.pixels_per_line / RED_COLORS;
    clr = BLACK;
    for (xidx = 0; xidx < RED_COLORS; xidx++)
    {
        swim_set_pen_color(&win1, clr);
        for (idx = 0; idx <= xgs; idx++)
        {
            swim_put_line(&win1, curx, cury, curx, curym);
            curx++;
        }
        clr = clr + 0x0800;
        }

    /* Draw green bars */
    cury = cury + ygs;
    curx = 0;
    curym = cury + (ygs - 1);
    xgs = LCD_DISPLAY.pixels_per_line / GREEN_COLORS;
    clr = BLACK;
    for (xidx = 0; xidx < GREEN_COLORS; xidx++)
    {
        swim_set_pen_color(&win1, clr);
        for (idx = 0; idx <= xgs; idx++)
        {
            swim_put_line(&win1, curx, cury, curx, curym);
            curx++;
        }
        clr = clr + 0x0020;
    }

    /* Draw blue bars */
    cury = cury + ygs;
    curx = 0;
    curym = cury + (ygs - 1);
    xgs = LCD_DISPLAY.pixels_per_line / BLUE_COLORS;
    clr = BLACK;
    for (xidx = 0; xidx < BLUE_COLORS; xidx++)
    {
        swim_set_pen_color(&win1, clr);
        for (idx = 0; idx <= xgs; idx++)
        {
            swim_put_line(&win1, curx, cury, curx, curym);
            curx++;
        }
        clr = clr + 0x0001;
    }
}
