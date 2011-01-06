/***********************************************************************
 * $Id:: adc_example.c 3258 2010-04-12 17:28:49Z usb10132              $
 *
 * Project: NXP PHY3250 simple ADC example
 *
 * Description:	This simple ADC example uses interrupts to measure the 
 *              level on ADIN2. The two clocking schemes are used
 *              --> RTC Clock and PERIPH_CLK 
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
#include "lpc32xx_chip.h"
#include "phy3250_board.h"
#include "lpc32xx_intc_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_adc_driver.h"
#include "lpc32xx_uart_driver.h"

/* Prototype for external IRQ handler */
void lpc32xx_irq_handler(void);

/* ADC device handle */
STATIC INT_32 adcdev;

/* ADC variables*/
volatile UNS_32 adc_int;

/* UART device handles */
static INT_32 uartdev;
static UNS_8 txbuff [512], rxbuff [512];
volatile static int txsize, rxsize;
static int txfill, rxget, txget, rxfill;

#define MAX_SAMPLES 12
UNS_32 sample_check[MAX_SAMPLES] = {0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2};
UNS_32 adc_data[MAX_SAMPLES];

void start_conversion(void)
{
	int i;

	/* Select channel */
	switch (sample_check[adc_int])
	{
	case 0:
		adc_ioctl(adcdev, ADC_CH_SELECT, ADCSEL_CH_0);
		break;

	case 1:
		adc_ioctl(adcdev, ADC_CH_SELECT, ADCSEL_CH_1);
		break;

	case 2:
		adc_ioctl(adcdev, ADC_CH_SELECT, ADCSEL_CH_2);
		break;
	}

	/* start A/D convert */
	if (adc_int < MAX_SAMPLES)
		adc_ioctl(adcdev, START_CONVERSION, 0);
}

/***********************************************************************
 *
 * Function: adc_user_interrupt
 *
 * Purpose: ADC user interrupt handler
 *
 * Processing:
 *     On an interrupt read 
 *     the data from the ADC
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
void adc_user_interrupt(void)
{
	/* A read of the ADC data register will clear the ADC interrupt */

	/* Read the ADC result */
	adc_data[adc_int] = (ADC->adc_dat & TSC_ADCDAT_VALUE_MASK);

	/* Set the Flag to tell main application we got the interrupt*/
	adc_int++;	

	/* Start conversion on next channel */
	start_conversion();
}

/***********************************************************************
 *
 * Function: term_dat_send_cb
 *
 * Purpose: UART transmit data callback
 *
 * Processing:
 *     Move data from the ring buffer to the driver.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: This function is called in interrupt context.
 *
 **********************************************************************/
void term_dat_send_cb(void)
{
  INT_32 bwrite, tosend = 512 - txget;

  if (tosend > txsize)
  {
    tosend = txsize;
  }

  /* Write data */
  bwrite = uart_write(uartdev, &txbuff[txget], tosend);
  txsize = txsize - bwrite;
  txget = txget + bwrite;
  if (txget >= 512)
  {
    txget = 0;
  }
}

/***********************************************************************
 *
 * Function: term_dat_recv_cb
 *
 * Purpose: UART receive data callback
 *
 * Processing:
 *     Read data from the driver into the RX ring buffer.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Ring buffer overflow is not accounted for in this application.
 *     This function is called in interrupt context.
 *
 **********************************************************************/
void term_dat_recv_cb(void)
{
  INT_32 bread, toreadmax = 512 - rxfill;

  /* Read data */
  bread = uart_read(uartdev, &rxbuff[rxfill], toreadmax);
  rxsize = rxsize + bread;
  rxfill = rxfill + bread;
  if (rxfill >= 512)
  {
    rxfill = 0;
  }
}

/***********************************************************************
 *
 * Function: term_dat_out
 *
 * Purpose: Send some data on the terminal interface
 *
 * Processing:
 *     Place data into the TX ring buffer and start UART transmission.
 *
 * Parameters:
 *     dat   : Data to send
 *     bytes : Number of bytes to send
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Will block until all bytes are sent.
 *
 **********************************************************************/
void term_dat_out(UNS_8 *dat, int bytes)
{
  while (bytes > 0)
  {
    while ((bytes > 0) && (txsize < 512))
    {
      txbuff[txfill] = *dat;
      txfill++;
      if (txfill >= 512)
      {
        txfill = 0;
      }
      dat++;
      bytes--;
      int_disable(IRQ_UART_IIR5);
      txsize++;
      int_enable(IRQ_UART_IIR5);
    }

    int_disable(IRQ_UART_IIR5);
    term_dat_send_cb();
    int_enable(IRQ_UART_IIR5);
  }
}

/***********************************************************************
 *
 * Function: term_dat_in
 *
 * Purpose: Read some data from the terminal interface
 *
 * Processing:
 *     Move data from the ring buffer to the passed buffer.
 *
 * Parameters:
 *     buff  : Where to place the data
 *     bytes : Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read
 *
 * Notes: None
 *
 **********************************************************************/
int term_dat_in(UNS_8 *buff, int bytes)
{
  int bread = 0;

  while ((bytes > 0) && (rxsize > 0))
  {
    *buff = rxbuff[rxget];
    buff++;
    rxget++;
    if (rxget >= 512)
    {
      rxget = 0;
    }
    bytes--;
    bread++;
    int_disable(IRQ_UART_IIR5);
    rxsize--;
    int_enable(IRQ_UART_IIR5);
  }

  return bread;
}

void newline(void)
{
	term_dat_out("\n", strlen("\n"));
	term_dat_out("\r", 1);
}

void dumphex16(UNS_32 val)
{
	char str[16];
	int i;
	UNS_32 msk, val8;

	msk = 0xf;
	for (i = 0; i < 4; i++)
	{
		val8 = (val & msk) >> (i * 4);
		msk = msk << 4;
		if (val8 <= 9)
			val8 = val8 + '0';
		else
			val8 = val8 + 'a' - 10;

		str[3 - i] = (char) val8;
	}

	str[4] = ' ';
	term_dat_out(str, 5);
}

void dump_adc_values(void)
{
	int i;

	for (i = 0 ; i < MAX_SAMPLES; i++)
	{
		dumphex16(adc_data[i]);
	}

	newline();
}

void dummy_delay(UNS_32 delay)
{
	int i;

	while (delay > 0)
	{
		for (i = 0; i < 100; i++);
		delay--;
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
	UART_CBS_T cbs;
	UART_CONTROL_T ucntl;
    volatile UNS_32 i; 
	
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

	/* Setup UART for 115.2K, 8 data bits, no parity, 1 stop bit */
	ucntl.baud_rate = 115200;
	ucntl.parity = UART_PAR_NONE;
	ucntl.databits = 8;
	ucntl.stopbits = 1;
	uartdev = uart_open((void *) UART5, (INT_32) & ucntl);
	if (uartdev != 0)
	{
		/* Setup RX and TX callbacks */
		cbs.rxcb = term_dat_recv_cb;
		cbs.txcb = term_dat_send_cb;
		cbs.rxerrcb = NULL;
		uart_ioctl(uartdev, UART_INSTALL_CBS, (INT_32) &cbs);
		int_enable(IRQ_UART_IIR5);
	}

	/* Initialize TX and RX ring buffers */
	txfill = txget = rxfill = rxget = txsize = rxsize = 0;

	/* Enable interrupts */
	int_enable(IRQ_UART_IIR5);

   /* Install adc handler as a IRQ interrupts */
    int_install_irq_handler(IRQ_TS_IRQ, (PFV) adc_user_interrupt);

    /* Enable ADC interrupt in the interrupt controller */
    int_enable(IRQ_TS_IRQ);

	/* ADC conversion, RTC CLK mode */
	/* Enable 32KHz clock to ADC block */
    clkpwr_clk_en_dis(CLKPWR_ADC_CLK,1);

	/* Open the ADC device */
	adcdev = adc_open(ADC , 0);     

	/* Reset flag */
    adc_int = 0;
	
	/* enable interupts */
    enable_irq();

	/* Start an 8 sample conversion process */
	start_conversion();

	/* wait for 8 conversions to complete */
	while (adc_int < MAX_SAMPLES);

    int_disable(IRQ_TS_IRQ);
	term_dat_out("32KHz data values:", 19);
	newline();
	dump_adc_values();
	dummy_delay(100000);

	/* ADC conversion, 400KHz mode. */
	/* disable 32KHz clock to ADC block */
    clkpwr_clk_en_dis(CLKPWR_ADC_CLK, 0);

    /* Get the clock frequency for the peripheral clock*/
    i = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);
	
	/* compute the divider needed to ensure we are at or
       below 400KHz ADC Sampling rate*/
	i = (i / 400000) + 1;

	/* Set the ADC to run off the PERIPH_CLK and
       400KHz sampling rate */
	clkpwr_setup_adc_ts(CLKPWR_ADCCTRL1_PCLK_SEL, i) ;

	/* Reset flag */
    adc_int = 0;
	
	/* enable ADC interupts */
    int_enable(IRQ_TS_IRQ);

	/* Start an 8 sample conversion process */
	start_conversion();

	/* wait for 8 conversions to complete */
	while (adc_int < MAX_SAMPLES);

    int_disable(IRQ_TS_IRQ);
	term_dat_out("400KHz data values:", 20);
	newline();
	dump_adc_values();
	dummy_delay(100000);

	/* ADC conversion, 4.5MHz mode. */
	/* disable 32KHz clock to ADC block */
    clkpwr_clk_en_dis(CLKPWR_ADC_CLK, 0);

    /* Get the clock frequency for the peripheral clock*/
    i = clkpwr_get_base_clock_rate(CLKPWR_PERIPH_CLK);
	
	/* compute the divider needed to ensure we are at or
       below 4.5MHz ADC Sampling rate*/
	i = (i / 4500000);

	/* PCLK base is 13MHz */
	/* clkpwr_setup_adc_ts() function changes ADC clock rate */

	/* Set the ADC to run off the PERIPH_CLK and
       400KHz sampling rate */
	clkpwr_setup_adc_ts(CLKPWR_ADCCTRL1_PCLK_SEL, i);
	term_dat_out("ADC clock divider set to ", 25);
	dumphex16(i);
	newline();

	while (1)
	{
		/* Reset flag */
		adc_int = 0;
	
		/* enable ADC interupts */
		int_enable(IRQ_TS_IRQ);

		/* Start an 8 sample conversion process */
		start_conversion();

		/* wait for 8 conversions to complete */
		while (adc_int < MAX_SAMPLES);

	    int_disable(IRQ_TS_IRQ);
		term_dat_out("4.5MHz data values:", 20);
		newline();
		dump_adc_values();

		dummy_delay(100000);
	}

	/* close the ADC */
	adc_close(adcdev);
}
