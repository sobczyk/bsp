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
#if (SOMDIMM_LPC3250_REV==1)
// MT46H16M16LFBF-6  32MB Low Power (Mobile) DDR
//                   256M: 16Mx16, 4 Banks, 13 Rows, 9 Columns
// AAAAA
// 11111AAAAAAAAAA
// 432109876543210 LPC3250 Pins
//
// BBAAA
// AA111AAAAAAAAAA
// 102109876543210 DDR SDRAM pins
//
// 00                         Standard Mode Register
//         ???                  CAS Latency {-,-,2,3,-}
//            ?                 Burst Type {Sequential,Interleaved}
//             ???              Burst Length {-,2,4,8,16,-}
//
// 10                         Extended Mode Register
//        ???                   Drive Strength {Full,1/2,1/4,3/4,3/4,-}
//             ???              Partial Array Self Refresh Coverage {Full,1/2,1/4,-,-,1/8,1/16,-}
//
// 222221111111111
// 4321098765432109876543210  LPC3250 Internal Address Space (relative)
//                         x    16 bits wide
//                ccccccccc     Column
//   rrrrrrrrrrrrr              Row
// bb                           Bank
//
// -+---+---+---+---+---+---
// 1000000000000000000000000  // Extended Mode Register(Full Drive Strength, Full Self-Refresh coverage)
// 0000000001100010000000000  // Standard Mode Register(CAS Latency 3, Burst Type Sequential, Burst Length 2)
// -+---+---+---+---+---+---

//#define DDR_LP_MODE_EXTENDED    0x01000000
//#define DDR_LP_MODE_NORMAL      0x0000C400

/* Number of rows, columns, and bank bits for the SDRAM device */
#define SDRAM_BANK_BITS 2 /* 2 bits = 4 banks, 1 bit = 2 banks */
#define SDRAM_COLS 9
#define SDRAM_ROWS 13
#endif

#if ((SOMDIMM_LPC3250_REV==2)||(SOMDIMM_LPC3250_REV==3))
// MT46H32M16LFBF-6  64MB Low Power (Mobile) DDR
//                   512M: 32Mx16, 4 Banks, 13 Rows, 10 Columns
// AAAAA
// 11111AAAAAAAAAA
// 432109876543210 LPC3250 Pins
//
// BBAAA
// AA111AAAAAAAAAA
// 102109876543210 DDR SDRAM pins
//
// 00                         Standard Mode Register
//         ???                  CAS Latency {-,-,2,3,-}
//            ?                 Burst Type {Sequential,Interleaved}
//             ???              Burst Length {-,2,4,8,16,-}
//
// 10                         Extended Mode Register
//        ???                   Drive Strength {Full,1/2,1/4,3/4,3/4,-}
//             ???              Partial Array Self Refresh Coverage {Full,1/2,1/4,-,-,1/8,1/16,-}
//
// 2222221111111111
// 54321098765432109876543210  LPC3250 Internal Address Space (relative)
//                          x    16 bits wide
//                cccccccccc     Column
//   rrrrrrrrrrrrr               Row
// bb                            Bank
//
// --+---+---+---+---+---+---
// 10000000000000000000000000  // Extended Mode Register(Full Drive Strength, Full Self-Refresh coverage)
// 00000000011000100000000000  // Standard Mode Register(CAS Latency 3, Burst Type Sequential, Burst Length 2)
// --+---+---+---+---+---+---

//#define DDR_LP_MODE_EXTENDED    0x02000000
//#define DDR_LP_MODE_NORMAL      0x00018800

/* Number of rows, columns, and bank bits for the SDRAM device */
#define SDRAM_BANK_BITS 2 /* 2 bits = 4 banks, 1 bit = 2 banks */
#define SDRAM_COLS 10
#define SDRAM_ROWS 13
#endif

#define SDRAM_RFSH_INTERVAL 128205 // 7.8 uS, or ~64 ms per 8192 rows

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
#define SDRAM_CAS_LATENCY 6 // 6 half cycles = 3

#define SDRAM_NS(x)       (1000000000/(x))
#define SDRAM_CLKS(x)     (x)

// (tRP) Precharge command period (1 / t)
#define SDRAM_TRP_DELAY SDRAM_NS(45) // 45.0 nS

// (tRAS) Dynamic Memory Active to Precharge Command period (1 / t)
#define SDRAM_TRAS_DELAY SDRAM_NS(90) // 42nS

// (tSREX, tXSR) Dynamic Memory Self-refresh Exit Time (clocks)
#define SDRAM_TSREX_TIME SDRAM_NS(240) // 120nS

// (tWR) Dynamic Memory Write Recovery Time (1 / t)
#define SDRAM_TWR_TIME SDRAM_NS(30) // 12nS

// (tRC) Dynamic Memory Active To Active Command Period (1 / t)
#define SDRAM_TRC_TIME SDRAM_NS(120) // 60nS

// (tRFC) Dynamic Memory Auto-refresh Period (1 / t)
#define SDRAM_TRFC_TIME SDRAM_NS(140) // 70nS

// (tXSNR, tXSR) Dynamic Memory Exit Self-refresh (1 / t)
#define SDRAM_TXSNR_TIME SDRAM_NS(240) // 120nS

// (tRRD) Dynamic Memory Active Bank A to Active Bank B Time (1 / t)
#define SDRAM_TRRD_TIME SDRAM_NS(30) // 12nS

// (tMRD) Dynamic Memory Load Mode Register To Active Command
// Time (1 / t)
#define SDRAM_TMRD_TIME SDRAM_CLKS(4) // 2 clocks

// (tCDLR) Dynamic Memory Last Data In to Read Command Time (1 / t)
//#define SDRAM_LP_TCDLR_TIME 75000000 // 13.3 nS?
// NOTE, TBD: This is a guess, using tWR or 12 nS and adding 3 nS for safety
#define SDRAM_TCDLR_TIME SDRAM_NS(30) // 15.0 nS?

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
