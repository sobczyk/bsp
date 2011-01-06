/***********************************************************************
 * $Id:: mmu_setup.c 3376 2010-05-05 22:28:09Z usb10132                $
 *
 * Project: MMU setup code
 *
 * Description:
 *     Provides MMU and cache setup for the board
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

#include "lpc_arm922t_cp15_driver.h"
#include "startup.h"

/***********************************************************************
 * Startup code private data
 **********************************************************************/

/* MMU virtual mapping table */
static const TT_SECTION_BLOCK_T tt_init_basic[] = {
	/* 0x00000000  0x08000000	0x04000000	u	IRAM uncached */
    {64, 0x00000000, 0x00000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x04000000	0x04000000	0x04000000	u	Unused */
    {64, 0x04000000, 0x04000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x08000000	0x08000000	0x04000000	cb	IRAM cached */
    {64, 0x08000000, 0x08000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x0C000000	0x0C000000	0x04000000	u	IROM uncached */
    {64, 0x0C000000, 0x0C000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x10000000	0x10000000	0x10000000	u	Reserved */
    {256, 0x10000000, 0x10000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x20000000	0x20000000	0x10000000	u	Registers */
    {256, 0x20000000, 0x20000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x30000000	0x230000000	0x10000000	u	Registers */
    {256, 0x30000000, 0x30000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x40000000	0x40000000	0x10000000	u	Registers */
    {256, 0x40000000, 0x40000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x50000000	0x50000000	0x30000000	u	Reserved */
    {256, 0x50000000, 0x50000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
    {256, 0x60000000, 0x60000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
    {256, 0x70000000, 0x70000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0x80000000	0x80000000	0x10000000	cb	DDR#0 cached */
    {256, 0x80000000, 0x80000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* DDR 0x90000000	0x80000000	0x10000000	u	DDR#0 uncached */
	{256, 0x90000000, 0x80000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xA0000000	0xA0000000	0x10000000	u	DDR#1 uncached */
	{256, 0xA0000000, 0xA0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xB0000000	0xA0000000	0x10000000	cb	DDR#1 cached */
	{256, 0xB0000000, 0xA0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xC0000000	0xC0000000	0x10000000	u */
    {256, 0xC0000000, 0xC0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xD0000000	0xD0000000	0x10000000	u	 */
    {256, 0xD0000000, 0xD0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE0000000	0xE0000000	0x01000000	u	ERAM#0 uncached */
    {16, 0xE0000000, 0xE0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE1000000	0xE1000000	0x01000000	u	ERAM#1 uncached */
    {16, 0xE1000000, 0xE1000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE2000000	0xE2000000	0x01000000	u	ERAM#2 uncached */
    {16, 0xE2000000, 0xE2000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE3000000	0xE3000000	0x01000000	u	ERAM#3 uncached */
    {16, 0xE3000000, 0xE3000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE4000000	0xE0000000	0x01000000	cb	ERAM#0 cached */
    {16, 0xE4000000, 0xE0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE5000000	0xE1000000	0x01000000	cb	ERAM#1 cached */
    {16, 0xE5000000, 0xE1000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE6000000	0xE2000000	0x01000000	cb	ERAM#2 cached */
    {16, 0xE6000000, 0xE2000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE7000000	0xE3000000	0x01000000	cb	ERAM#3 cached */
    {16, 0xE7000000, 0xE3000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_CACHEABLE | ARM922T_L1D_BUFFERABLE |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xE8000000	0xE8000000	0x08000000	u	 */
    {128, 0xE8000000, 0xE0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
	/* 0xF0000000	0xF0000000	0x10000000	u */
    {256, 0xF0000000, 0xF0000000, 
        (ARM922T_L1D_AP_ALL | ARM922T_L1D_DOMAIN(0) |
        ARM922T_L1D_TYPE_SECTION)},
    {0, 0, 0, 0}  /* Marks end of initialization array.  Required! */
};

/***********************************************************************
 * Public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: mmu_setup
 *
 * Purpose: MMU page table initialization
 *
 * Processing:
 *     Calls the cp15_init_mmu_trans_table() function with the addresses
 *     of the MMU translation table and the page mapping table.
 *
 * Parameters:
 *     mmu_base_aadr : Physical address base for page table
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void mmu_setup(TRANSTABLE_T *mmu_base_aadr)
{
    cp15_init_mmu_trans_table(mmu_base_aadr, tt_init_basic);
}
