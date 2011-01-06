/***********************************************************************/
/*  This file is part of the ARM Toolchain package                     */
/*  Copyright KEIL ELEKTRONIK GmbH 2003 - 2008                         */
/***********************************************************************/
/*                                                                     */
/*  FlashDev.C:  Device Description for flash algorithm using NXP      */
/*               LPC32x0 Device Series and Dual Spansion S29AL008D     */
/*               External NOR Flash (32-bit Bus)                       */
/*                                                                     */
/***********************************************************************/

#include "FlashOS.H"        // FlashOS Structures

struct FlashDevice const FlashDevice  =  {
   FLASH_DRV_VERS,             // Driver Version, do not modify!
   "LPC32x0 S29AL008D Dual Flash",  // Device Name
   EXT32BIT,                   // Device Type
   0x80000000,                 // Device Start Address
   0x1F8000,                   // Device Size in Bytes (2016kB)
   1024,                       // Programming Page Size
   0,                          // Reserved, must be 0
   0xFF,                       // Initial Content of Erased Memory
   100,                        // Program Page Timeout 100 mSec
   3000,                       // Erase Sector Timeout 3000 mSec

// Specify Size and Address of Sectors
// 2*0x04000, 0x000000,        // Sector Size Dual 16kB =  32kB ( 1 Sector)
//                             // Sector 0 Reserved for Bootloader
   2*0x02000, 0x000000,        // Sector Size Dual  8kB =  16kB ( 2 Sectors)
   2*0x08000, 0x008000,        // Sector Size Dual 32kB =  64kB ( 1 Sector)
   2*0x10000, 0x018000,        // Sector Size Dual 64kB = 128kB (15 Sectors)
   SECTOR_END
};
