######################################################################## 
# $Id:: makefile 2620 2009-12-17 21:05:28Z usb10132                    $
# 
# Project:     
# 
# Description: 
#  Makefile
# 
######################################################################## 
# Software that is described herein is for illustrative purposes only  
# which provides customers with programming information regarding the  
# products. This software is supplied "AS IS" without any warranties.  
# NXP Semiconductors assumes no responsibility or liability for the 
# use of the software, conveys no license or title under any patent, 
# copyright, or mask work right to the product. NXP Semiconductors 
# reserves the right to make changes in the software without 
# notification. NXP Semiconductors also make no representation or 
# warranty that such application will be suitable for the specified 
# use without further testing or modification. 
########################################################################

########################################################################
#
# Pick up the default build rules 
#
########################################################################

include $(NXPMCU_SOFTWARE)\makerule\$(CSP)\make.$(CSP).$(TOOL)

SUBDIRS = adc dram_self_refresh hstimer hsuart i2c kscan lcd_colorbars
SUBDIRS += lcd_tsc mi2c mstimer pwm pwm_simple rtc sdcard sdcard_dma
SUBDIRS += slc spi ssp sysclock timer uart wdt
	
########################################################################
#
# Build all the examples
#
########################################################################
subdirs: $(SUBDIRS)

$(SUBDIRS): common
	@$(ECHO) "Building" $@;
	@$(MAKE) -C $@

common:
	@$(ECHO) "Building" $@;
	@$(MAKE) -C $@ 

########################################################################
#
# Clean the BSP library
#
########################################################################

clean: 
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir clean ; done

realclean: 
	@for dir in $(SUBDIRS) ; do $(MAKE) -C $$dir realclean ; done

.PHONY: subdirs $(SUBDIRS) common clean realclean
