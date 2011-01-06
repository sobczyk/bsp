/***********************************************************************
 * $Id:: dram_configs.h 3436 2010-05-10 16:40:24Z usb10132             $
 *
 * Project: DRAM configuration defines
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

#ifndef DRAM_CONFIGS_H
#define DRAM_CONFIGS_H

/* SDR refresh interval in inverse time (1 / tREFI). This applies to the
   EMC DynamicRefresh register.
   Example: For a 16uS refresh cycle, this value will be
   1/16uS = 62500 */
#define SDRAM_RFSH_INTERVAL 128000

/* Number of rows, columns, and bank bits for the SDRAM device */
#define SDRAM_COLS 9
#define SDRAM_ROWS 13
#define SDRAM_BANK_BITS 2 /* 2 bits = 4 banks, 1 bit = 2 banks */

/* If using DDR or SDR with only a 16-bit configuration, set this
   value to 0. For 32-bit SDR SDRAM, set this to 1 */
#define SDRAM_32BIT_BUS 1

/* SDRAM can be configured for low power (Bank-Row-Col (BRC)) mode or
   performance (Row-Bank-Col (RBC)) mode. Performance mode uses mores
   power, but is faster. Set the following define to 0 for low power
   mode, or 1 for performance mode */
#define SDRAM_USE_PERFORMANCE_MODE 1

/* RAS and CAS clock latencies, CAS latencies are in 1/2 clocks,
   while RAS latencies are in full clocks. So a RAS latency of
   2 and a CAS latency of 2-1/2 would be RAS=2 and CAS=5 */
#define SDRAM_RAS_LATENCY 2
#define SDRAM_CAS_LATENCY 6

/* (tRP) Precharge command period - this can be defined in terms of
   clocks or inverse time (1 / t) */
#define SDRAM_TRP_DELAY 55555555 /* 18nS */

/* (tRAS) Dynamic Memory Active to Precharge Command period - this
   can be defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TRAS_DELAY 23809523 /* 42nS */

/* (tSREX) Dynamic Memory Self-refresh Exit Time - this can be defined
   in terms of clocks or inverse time (1 / t) */
#define SDRAM_TSREX_TIME 14925373

/* (tWR) Dynamic Memory Write Recovery Time - this can be defined in
   terms of clocks or inverse time (1 / t) */
#define SDRAM_TWR_TIME 83333333 /* 12nS */

/* (tRC) Dynamic Memory Active To Active Command Period - this can be
   defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TRC_TIME 16666666 /* 60nS */

/* (tRFC) Dynamic Memory Auto-refresh Period - this can be defined in
   terms of clocks or inverse time (1 / t) */
#define SDRAM_TRFC_TIME 16666666 /* 60nS */

/* (tXSNR or tXSR) Dynamic Memory Exit Self-refresh - this can be
   defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TXSNR_TIME 14925373 /* 67nS */

/* (tRRD) Dynamic Memory Active Bank A to Active Bank B Time - this
   can be defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TRRD_TIME 83333333 /* 12nS */

/* (tMRD) Dynamic Memory Load Mode Register To Active Command
   Time - this can be defined in terms of clocks or inverse time
   (1 / t) */
#define SDRAM_TMRD_TIME 2

/* (tCDLR) Dynamic Memory Last Data In to Read Command Time - this can
   be defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TCDLR_TIME 1

/* Mode word. This value defines how the SDRAM device is configured.
   See the SDRAM data sheet for info on this value. The software will
   place the mode word on the right pins.
   CAS 3, burst of 1, sequential */
#define SDRAM_MODE_WORD 0x30

/* Extended mode word. This is used for low power SDRAM. This value
   needs to be defined per the SDRAM data sheet. For the example define
   below, the following is done:
   Full array self-refresh coverage, full drive strength, normal
   operation */
#define SDRAM_EXT_MODE_WORD 0x00

/* Extended mode word write mask for banks. The extended mode word is
   usually written with the BA1 bank bit high and BA0 low. The
   following define specifies that mapping. */
#define SDRAM_EXT_MODE_BB sdram_get_bankmask(1, 0)

/* Set slew rates to fast by commenting out the following line, or
   set it to slow by uncommenting it. */
/*
#define SDRAM_USE_SLOW_SLEW
*/

/* Automatically computed SDRAM size */
#define SDRAM_SIZE (1 << (SDRAM_COLS + SDRAM_ROWS + SDRAM_BANK_BITS +\
	1 + SDRAM_32BIT_BUS))	

#endif /* DRAM_CONFIGS_H */
