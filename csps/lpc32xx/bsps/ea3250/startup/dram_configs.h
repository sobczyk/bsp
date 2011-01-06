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

/***********************************************************************
 * BEFORE USING THIS CODE, MAKE SURE THE FOLLOWING PARAMETERS ARE
 * CORRECTLY SETUP FOR YOUR SYSTEM! ONLY THE CONFIGURATION SPECIFIC TO
 * YOUR SYSTEM NEEDS TO BE CHANGED.
 ***********************************************************************
 * Note some timing values can be defined in clocks instead of inverse
 * time. For these timing values, the static clock count will always be
 * used for configuration instead of the automatically computed time
 * based on the system clock speed.
 **********************************************************************/

/***********************************************************************
 ***********************************************************************
 * Low power DDR SDRAM configuration
 ***********************************************************************
 **********************************************************************/
/* DDR refresh interval in inverse time (1 / tREFI). This applies to the
   EMC DynamicRefresh register.
   Example: For a 16uS refresh cycle, this value will be
   1/16uS = 62500 */
#define SDRAM_RFSH_INTERVAL 128205

/* Number of rows, columns, and bank bits for the SDRAM device */
#define SDRAM_COLS 10
#define SDRAM_ROWS 13
#define SDRAM_BANK_BITS 2 /* 2 bits = 4 banks, 1 bit = 2 banks */

/* If using DDR or SDR with only a 16-bit configuration, set this
   value to 0. For 32-bit SDR SDRAM, set this to 1 */
#define SDRAM_32BIT_BUS 0

/* SDRAM can be configured for low power (Bank-Row-Col (BRC)) mode or
   performance (Row-Bank-Col (RBC)) mode. Performance mode uses mores
   power, but is faster. Set the following define to 0 for low power
   mode, or 1 for performance mode */
#define SDRAM_USE_PERFORMANCE_MODE 0

/* RAS and CAS clock latencies, CAS latencies are in 1/2 clocks,
   while RAS latencies are in full clocks. So a RAS latency of
   2 and a CAS latency of 2-1/2 would be RAS=2 and CAS=5 */
#define SDRAM_RAS_LATENCY 2
#define SDRAM_CAS_LATENCY 6

/* (tRP) Precharge command period - this can be defined in terms of
   clocks or inverse time (1 / t) */
#define SDRAM_TRP_DELAY 44444444 /* 22.5nS */

/* (tRAS) Dynamic Memory Active to Precharge Command period - this
   can be defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TRAS_DELAY 22222222 /* 45nS */

/* (tSREX) Dynamic Memory Self-refresh Exit Time - this can be defined
   in terms of clocks or inverse time (1 / t) */
#define SDRAM_TSREX_TIME 8333333 /* 120nS */

/* (tWR) Dynamic Memory Write Recovery Time - this can be defined in
   terms of clocks or inverse time (1 / t) */
#define SDRAM_TWR_TIME 66666666 /* 15nS */

/* (tRC) Dynamic Memory Active To Active Command Period - this can be
   defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TRC_TIME 13333333 /* 75nS */

/* (tRFC) Dynamic Memory Auto-refresh Period - this can be defined in
   terms of clocks or inverse time (1 / t) */
#define SDRAM_TRFC_TIME 14285714 /* 70nS */

/* (tXSNR or tXSR) Dynamic Memory Exit Self-refresh - this can be
   defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TXSNR_TIME 8333333 /* 120nS */

/* (tRRD) Dynamic Memory Active Bank A to Active Bank B Time - this
   can be defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TRRD_TIME 66666666 /* 15nS */

/* (tMRD) Dynamic Memory Load Mode Register To Active Command
   Time - this can be defined in terms of clocks or inverse time
   (1 / t) */
#define SDRAM_TMRD_TIME 3

/* (tCDLR) Dynamic Memory Last Data In to Read Command Time - this can
   be defined in terms of clocks or inverse time (1 / t) */
#define SDRAM_TCDLR_TIME 75000000

/* DDR mode word. This value defines how the SDRAM device is configured.
   See the SDRAM data sheet for info on this value. The software will
   place the mode word on the right pins.
   CAS 3, burst of 2, sequential */
#define SDRAM_MODE_WORD 0x31

/* DDR extended mode word. This value defines how the SDRAM device is
   configured. See the SDRAM data sheet for info on this value. The
   software will place the mode word on the right pins. When defining
   this value, do not define it with the BA0,1 signal states.
   full array self-refresh coverage, full strength driver */
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
