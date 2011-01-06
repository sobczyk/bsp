/***********************************************************************
 * $Id:: board_spi_flash_driver.c 3380 2010-05-05 23:54:23Z usb10132   $
 *
 * Project: Simple I2C functions for I2C EEPROM access
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
#include "board_i2c_eeprom_driver.h"
#include "lpc32xx_clkpwr_driver.h"
#include "lpc32xx_timer_driver.h"
#include "lpc32xx_i2c.h"
#include "board_config.h"

/* Holds I2C block base address */
I2C_REGS_T  *pi2cbase = NULL;

/***********************************************************************
 *
 * Function: board_i2c_reset
 *
 * Purpose: Reset I2C Controller
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
void board_i2c_reset(void)
{
	UNS_32 ctrl = 0;

	/* Issue soft reset of I2C block */
	pi2cbase->i2c_ctrl = I2C_RESET;
	/* Wait for reset to complete */
	do
	{
		ctrl = pi2cbase->i2c_ctrl;
	} while (ctrl & I2C_RESET);
}

/***********************************************************************
 *
 * Function: board_i2c_init
 *
 * Purpose: Initialize I2C Controller
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *		id:	I2C Block identifier
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void board_i2c_init(UNS_32 id)
{
	UNS_32 i2cclk;
	CLKPWR_CLK_T clkid;

	if (id == 0)
	{
		clkid = CLKPWR_I2C1_CLK;
		pi2cbase = I2C1;	
	}
	else
	{
		clkid = CLKPWR_I2C2_CLK;
		pi2cbase = I2C2;	
	}

	/* Enable I2C clock */
	clkpwr_clk_en_dis(clkid, 1);

	/* Issue soft reset of I2C block */
	board_i2c_reset();
	
	/* Get the I2C Controller base clock rate */
	i2cclk = clkpwr_get_clock_rate(clkid);

	/* Program CLK_HIGH and CLK_LOW to generate 100KHz SCL */
	pi2cbase->i2c_clk_hi = ((i2cclk / 100000) / 2) - 2;
	pi2cbase->i2c_clk_lo = ((i2cclk / 100000) / 2 ) - 2;

	/* Configure I2C pins in High drive mode */
	clkpwr_set_i2c_driver(id + 1, 1);
}

/***********************************************************************
 *
 * Function: board_i2c_write
 *
 * Purpose: Write data to EEPROM starting from given address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr :	Address from where to start write data (I2C)
 *	   buffer:	Write data buffer
 *	   length:	Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: On Success, number of data write
 *			On Failure, -1;
 *
 * Notes: Will handle up to 32KBytes devices.
 *
 **********************************************************************/
int board_i2c_write(UNS_32 addr, UNS_8 *buffer, UNS_32 length)
{
	UNS_32 status = 0;
	UNS_32 idx;

	/* For Page write, start address must be page aligned */
	if (addr % EEPROM_FIFO_DEPTH)
	{
		return -1;
	}

	/* Length must not be zero */
	if ((length <= 0) || (length > EEPROM_FIFO_DEPTH))
	{
		return -1;
	}

	/* Clear TX/RX FIFO, Status register, state machines of I2C
	   block */
	board_i2c_reset();

	/* Write Device (Slave) Address with START condition */
	pi2cbase->i2c_txrx = (EEPROM_SLAVE_ADDR | I2C_START);	

	/* Write 16-bit EEPROM address */
	pi2cbase->i2c_txrx = ((addr >> 8) & 0xFF);
	pi2cbase->i2c_txrx = (addr & 0xFF);

	/* 
	 * Write a length amount of data. Also generate STOP
	 * Condition for last trasmit byte.
	 */
	for (idx = 0; idx < length; idx++)
	{
		/* Wait untill TX FIFO gets empty */
		while ((pi2cbase->i2c_stat & I2C_TFF) == I2C_TFF);

		if (idx == (length - 1))
		{
			pi2cbase->i2c_txrx = (buffer[idx] | I2C_STOP); 
		}
		else
		{
			pi2cbase->i2c_txrx = buffer[idx]; 
		}
	}

	/* Wait untill NAK or X'fer Done is generated */
	while ((status & (I2C_TDI | I2C_NAI)) == 0x0)
	{
		status = pi2cbase->i2c_stat;
	}

	/*
	 * Check If X'fer is successfully complete or not.
	 * If Complet successfully, Clear TDI bit.
	 */
	if (status & I2C_TDI)
	{
		pi2cbase->i2c_stat = I2C_TDI;
	}
	else
	{
		return -1;
	}

	/* Allow some time for programming */
	timer_wait_ms(TIMER_CNTR0, 500);

	return length;
}

/***********************************************************************
 *
 * Function: board_i2c_write_byte
 *
 * Purpose: Write data to EEPROM starting from given address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr :	Address from where to start write data (I2C)
 *	   buffer:	Write data buffer
 *	   length:	Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: On Success, number of data write
 *			On Failure, -1;
 *
 * Notes: Will handle up to 32KBytes devices.
 *
 **********************************************************************/
int board_i2c_write_byte(UNS_32 addr, UNS_8 data)
{
	UNS_32 status = 0;

	/* Clear TX/RX FIFO, Status register, state machines of I2C
	   block */
	board_i2c_reset();

	/* Write Device (Slave) Address with START condition */
	pi2cbase->i2c_txrx = (EEPROM_SLAVE_ADDR | I2C_START);	

	/* Write 16-bit EEPROM address */
	pi2cbase->i2c_txrx = ((addr >> 8) & 0xFF);
	pi2cbase->i2c_txrx = (addr & 0xFF);

	/* Wait for TX FIFO to empty */
	while (pi2cbase->i2c_stat & I2C_TFF);

	/* 
	 * Write a length amount of data. Also generate STOP
	 * Condition for last trasmit byte.
	 */
	pi2cbase->i2c_txrx = (data | I2C_STOP); 

	/* Wait untill NAK or X'fer Done is generated */
	while ((status & (I2C_TDI | I2C_NAI)) == 0x0)
	{
		status = pi2cbase->i2c_stat;
	}

	/*
	 * Check If X'fer is successfully complete or not.
	 * If Complet successfully, Clear TDI bit.
	 */
	if (status & I2C_TDI)
	{
		pi2cbase->i2c_stat = I2C_TDI;
	}
	else
	{
		return -1;
	}

	/* Wait for EEPROM Write cycle to complete */
	timer_wait_ms(TIMER_CNTR0, 10);

	return 1;
}

/***********************************************************************
 *
 * Function: board_i2c_read
 *
 * Purpose:
 *     Read data from the serial EEPROM starting from given address
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     addr :	Address from where to start read data
 *	   buffer:	Read data buffer
 *	   length:	Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: On Success, number of data read
 *			On Failure, -1;
 *
 * Notes: Will handle up to 32KBytes devices.
 *
 **********************************************************************/
int board_i2c_read(UNS_32 addr, UNS_8 *buffer, UNS_32 length)
{
	UNS_32 status = 0;
	UNS_32 readlen = 0;

	/* Clear TX/RX FIFO, Status register, state machines of I2C
	   block */
	board_i2c_reset();

	/* Write Device (Slave) Address with START condition */
	pi2cbase->i2c_txrx = (EEPROM_SLAVE_ADDR | I2C_START);	

	/* Write 16-bit EEPROM address */
	pi2cbase->i2c_txrx = ((addr >> 8) & 0xFF);
	pi2cbase->i2c_txrx = (addr & 0xFF);

	/* Write Device (Slave) Address with Re-START condition & Read
	   mode */
	pi2cbase->i2c_txrx = ((EEPROM_SLAVE_ADDR | 0x1) | I2C_START);	

	/* 
	 * Write a length amount of data. Also generate STOP
	 * Condition for last trasmit byte.
	 */
	while (readlen < length)
	{
		/* Wait for TX FIFO to empty */
		while (pi2cbase->i2c_stat & I2C_TFF);
		
		/* Write Dummy Data to recive data from Slave */
		if (readlen == (length - 1))
		{
			pi2cbase->i2c_txrx = (0xFF | I2C_STOP);
		}
		else
		{
			pi2cbase->i2c_txrx = 0xFF;
		}

		/* Wait untill RX FIFO gets data */
		while ((pi2cbase->i2c_stat & I2C_RFE) &&
			(!(pi2cbase->i2c_stat & I2C_DRMI)));
		
		if ((pi2cbase->i2c_stat & I2C_RFE) != I2C_RFE)
		{
			/* Read data from RX FIFO */
			buffer[readlen] = pi2cbase->i2c_txrx;
			readlen++;
		}
	}

	/* Wait untill NAK or X'fer Done is generated */
	while ((status & (I2C_TDI | I2C_NAI)) == 0x0)
	{
		status = pi2cbase->i2c_stat;
	}

	/* Check If X'fer is successfully complete or not */
	if (status & I2C_TDI)
	{	
		pi2cbase->i2c_stat = I2C_TDI;
		return length;
	}
	else
	{
		return -1;
	}
}
