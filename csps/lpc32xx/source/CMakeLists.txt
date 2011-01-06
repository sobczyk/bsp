set (src_lpc32xxlib
	lpc32xx_adc_driver.c
	lpc32xx_dma_driver.c
	lpc32xx_hsuart_driver.c
	lpc32xx_intc_driver.c
	lpc32xx_mstimer_driver.c
	lpc32xx_rtc_driver.c
	lpc32xx_spi_driver.c
	lpc32xx_tsc_driver.c
	lpc3xxx_spwm_driver.c
	lpc32xx_clcdc_driver.c
	lpc32xx_gpio_driver.c
	lpc32xx_i2c_driver.c
	lpc32xx_kscan_driver.c
	lpc32xx_mstr_i2c_driver.c
	lpc32xx_sdcard_driver.c
	lpc32xx_ssp_driver.c
	lpc32xx_uart_driver.c
	lpc32xx_clkpwr_driver.c
	lpc32xx_hstimer_driver.c
	lpc32xx_i2s_driver.c
	lpc32xx_mlcnand_driver.c
	lpc32xx_pwm_driver.c
	lpc32xx_slcnand_driver.c
	lpc32xx_timer_driver.c
	lpc32xx_wdt_driver.c
	lpc32xx_vectors.asm
)

ADD_LIBRARY(lpc32xxLIB STATIC ${src_lpc32xxlib})