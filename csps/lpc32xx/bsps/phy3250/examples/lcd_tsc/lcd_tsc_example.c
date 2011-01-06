/**********************************************************************
 * $Id:: lcd_tsc_example.c 1262 2008-10-28 23:21:16Z wellsk           $
 *
 * Project: NXP PHY3250 LCD with TSC example
 *
 * Description:
 *     Draws color bars on the LCD. Requires RGB565 mode.
 *     TSC running, moves LCD cusror 
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
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_gpio_driver.h"
#include "lpc32xx_clcdc_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc_swim_font.h"
#include "lpc32xx_tsc_driver.h"

#include "lpc32xx_timer_driver.h"

#define PHY_LCD_FRAME_BUF 0x81000000
#define LCD_DISPLAY       hitachi_tx09d71

/* fifo for TSC samples */
volatile UNS_32 tsc_read_fifo[16];

/* device config structure pointer */
INT_32 lcddev;
INT_32 tscdev;

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* Cursor Image definition */
const int cursorimage[] ={
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
	0xAAAAAAAA,0xAAAAAA2A,0xA8AAAAAA,0xAAAAAA0A,
	0xA8AAAAAA,0xAAAAAA0A,0xA0AAAAAA,0xAAAAAA02,
	0xA0AAAAAA,0xAAAAAA02,0x80AAAAAA,0xAAAAAA00,
	0x80AAAAAA,0xAAAAAA00,0x00AAAAAA,0xAAAA2A00,
	0x00AAAAAA,0xAAAA2A00,0x00A8AAAA,0xAAAA0A00,
	0x00A8AAAA,0xAAAA0A00,0x00A0AAAA,0xAAAA0200,
	0x00A0AAAA,0xAAAA0200,0x0080AAAA,0xAAAA0000,
	0x0080AAAA,0xAAAA0000,0x2000AAAA,0xAA2A0002,
	0xA002AAAA,0xAA2A8002,0xA0AAAAAA,0xAAAAAA02,
	0xA0AAAAAA,0xAAAAAA02,0xA0AAAAAA,0xAAAAAA02,
	0xA0AAAAAA,0xAAAAAA02,0xA0AAAAAA,0xAAAAAA02,
	0xA0AAAAAA,0xAAAAAA02,0xAAAAAAAA,0xAAAAAAAA,
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,
	0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA,0xAAAAAAAA};

/**********************************************************************
 *
 * Function: tsc_user_interrupt
 *
 * Purpose: TSC interrupt handler
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
void tsc_user_interrupt(void)
{

    volatile UNS_32 i=0, j=0, x_val[16], y_val[16];
    volatile UNS_32 x_val_av=0, y_val_av=0;

    /* Read the full FIFO until the FIFO Empty bit is set */
	while( ! tsc_ioctl(tscdev,TSC_GET_STATUS, TSC_FIFO_EMPTY_ST)){
		tsc_read_fifo[i] = TSC->tsc_fifo;

		/* Check that the TSC is pressed, sample valid */	
		if((tsc_read_fifo[i] & TSC_FIFO_TS_P_LEVEL) 
		    != TSC_FIFO_TS_P_LEVEL){
			x_val[i] = TSC_FIFO_NORMALIZE_X_VAL(tsc_read_fifo[i]);
                  y_val[i] = TSC_FIFO_NORMALIZE_Y_VAL(tsc_read_fifo[i]);
			i++;
			}
	}

	/* if we have a valid x/y sample, display it! */
	if(i>1){

		/* Average out the received samples */

		for(j=0;j<i;j++){
			x_val_av = ((x_val_av + x_val[j])/2);
			y_val_av = ((y_val_av + y_val[j])/2);
		}

		/* Calibrate the received X/Y corordinates */
		x_val_av = x_val_av * 240;
		x_val_av = x_val_av / 1023;

		y_val_av = y_val_av * 320;
		y_val_av = y_val_av / 1023;

		/* set the cursor X/Y position */
		lcd_ioctl(lcddev, LCD_CRSR_XY, 
		         ((320-y_val_av)<<16)|(240-x_val_av));
		}

	 
}

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
    UNS_16 xgs, ygs, curx, cury, curym, xidx;

    /* Disable interrupts in ARM core */
    disable_irq_fiq();

    /* Set virtual address of MMU table */
    cp15_set_vmmu_addr((void *)
		(IRAM_BASE + (256 * 1024) - (16 * 1024)));

	/* Initialize interrupt system */
    int_initialize(0xFFFFFFFF);

    /* Install standard IRQ dispatcher at ARM IRQ vector */
    int_install_arm_vec_handler(IRQ_VEC, (PFV) lpc32xx_irq_handler);

    /* Setup miscellaneous board functions */
    phy3250_board_init();

    /* enable clock to ADC block - 32KHz clock */
    clkpwr_clk_en_dis(CLKPWR_ADC_CLK,1);

    /* TSC IRQ goes active when the FIFO reaches the Interrupt level */
    int_install_irq_handler(IRQ_TS_IRQ, (PFV) tsc_user_interrupt);

    /* Enable interrupt */
    int_enable(IRQ_TS_IRQ);

    /* Open TSC, sets default timing values, fifo = 16, 
	 resolution = 10bits */
    tscdev = tsc_open(TSC, 0);

    /* TSC Auto mode enable, this also sets AUTO bit */
    tsc_ioctl(tscdev,TSC_AUTO_EN, 1);
	  
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
	
    /* write cursor image array data to cursor image RAM */
    lcd_ioctl(lcddev, LCD_CRSR_INIT_IMG, (INT_32) &cursorimage[0]);

    /* enable the default cursor 0 */
    lcd_ioctl(lcddev,LCD_CRSR_EN,1);

    /* set the cursor X/Y position */
    lcd_ioctl(lcddev, LCD_CRSR_XY, 0x0); 

    /* set the cursor pallette BGR value, col0*/
    lcd_ioctl(lcddev, LCD_CRSR_PAL0, 0x00ff0000); 

    /* set the cursor pallette BGR value, col1 */
    lcd_ioctl(lcddev, LCD_CRSR_PAL1, 0x000000ff); 

    /* Enable IRQ interrupts in the ARM core */
    enable_irq();

    /* Set frame buffer address */
    fblog = (COLOR_T *)cp15_map_physical_to_virtual(PHY_LCD_FRAME_BUF);

    /* Create a SWIM window */
    swim_window_open(&win1, LCD_DISPLAY.pixels_per_line,
        LCD_DISPLAY.lines_per_panel, fblog, 0, 0,
		(LCD_DISPLAY.pixels_per_line - 1), 
		(LCD_DISPLAY.lines_per_panel - 1),
        1, WHITE, BLACK, BLACK);

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


    /* lets stay here forever */
    while(1);

}
