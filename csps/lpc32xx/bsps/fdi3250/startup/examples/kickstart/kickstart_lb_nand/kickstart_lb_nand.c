/***********************************************************************
 * $Id:: kickstart_lb_nand.c 3418 2010-05-06 19:59:34Z usb10132        $
 *
 * Project: Kickstart loader from large block NAND FLASH
 *
 * Description:
 *     This file contains a sample kickstart loader that can be used
 *     to load a larger stage 1 application from large block NAND
 *     FLASH.
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

#include "board_slc_nand_lb_driver.h"
#include "startup.h"
#include "misc_config.h"
#include "dram_configs.h"
#include "common_funcs.h"

static UNS_8 tmpbuff [2048+64];

#define OPTION_MEMORY_TEST      0
#define OPTION_FULL_MEMORY_TEST OPTION_MEMORY_TEST


void putc(void *p, char ch)
{
    UNS_8 chars[2];
    if (ch == '\n')
        uart_output((UNS_8 *)"\r");
    chars[0] = ch;
    chars[1] = 0;
    uart_output(chars);
}

#if OPTION_MEMORY_TEST
void MemoryTestFail(void)
{
    uart_output((UNS_8 *)"Memory test fail!");
#if OPTION_TESTER
    G_finalResult = EFalse;
#else
    while (1)  {}
#endif
}

void MemoryTestWait(void)
{
//    msecs = 0;
//    while (msecs < (3 * 1000));
}

#define REPORT_SIZE     0x40000 // every 256K

typedef unsigned char TUInt8;
typedef unsigned long TUInt32;
#define SDRAM_BASE (0x80000000)

void MemoryTest(void)
{
    TUInt8 *sdram = (TUInt8 *)SDRAM_BASE;
    TUInt32 *sdram32 = (TUInt32 *)SDRAM_BASE;
    TUInt32 count = 0;
    TUInt8 prime;
    TUInt32 next;
//    int j;

    uart_output((UNS_8 *)"\r\nMemory Test: \r\n");
#if OPTION_FULL_MEMORY_TEST
    // Clear with zeros
    uart_output((UNS_8 *)"Phase 1\r\n");
    for (count=next=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
//            printf("  Phase 1: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
         sdram32[count>>2] = 0x00000000;
    }

    MemoryTestWait();
    // Now confirm
    uart_output((UNS_8 *)"Phase 2\r\n");
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
//            printf("  Phase 2: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        if (sdram32[count>>2] != 0x00000000) {
            MemoryTestFail();
            return;
        }
    }

    // Fill with 0xFF's
    uart_output((UNS_8 *)"Phase 3\r\n");
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
    //        printf("  Phase 3: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        sdram32[count>>2] = 0xFFFFFFFF;
    }

    // Now confirm
    MemoryTestWait();
    uart_output((UNS_8 *)"Phase 4\r\n");
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
            //printf("  Phase 4: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        if (sdram32[count>>2] != 0xFFFFFFFF) {
            MemoryTestFail();
            return;
        }
    }

    // Fill with 0xAA
    uart_output((UNS_8 *)"Phase 5\r\n");
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
            //printf("  Phase 5: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        sdram32[count>>2] = 0xAAAAAAAA;
    }

    MemoryTestWait();
    uart_output((UNS_8 *)"Phase 6\r\n");
// Now confirm
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
            //printf("  Phase 6: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        if (sdram32[count>>2] != 0xAAAAAAAA) {
            MemoryTestFail();
            return;
        }
    }

    uart_output((UNS_8 *)"Phase 7\r\n");
    // Fill with 0x55
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
            //printf("  Phase 7: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        sdram32[count>>2] = 0x55555555;
    }
    MemoryTestWait();
    uart_output((UNS_8 *)"Phase 8\r\n");
    // Now confirm
    for (next=count=0; count<SDRAM_SIZE; count+=4) {
        if (count >= next) {
            //printf("  Phase 8: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        if (sdram32[count] != 0x55555555) {
            MemoryTestFail();
            return;
        }
    }
#endif

    // Now use a semi-random pattern (minimal test)
    uart_output((UNS_8 *)"Phase 9\r\n");
    for (next=count=0, prime=0; count<SDRAM_SIZE; count++, prime+=31) {
        if (count >= next) {
            //printf("  Phase 9: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
            next += REPORT_SIZE;
        }
        sdram[count^0x111371] = prime;
    }

    MemoryTestWait();
    uart_output((UNS_8 *)"Phase 10\r\n");
    // Now confirm
//    for (j=0; j<5; j++)  {
        for (next=count=0, prime=0; count<SDRAM_SIZE; count++, prime+=31) {
            if (count >= next) {
                //printf("  Phase 10: 0x%08X of 0x%08X\r", next, SDRAM_SIZE);
                next += REPORT_SIZE;
            }
            if (sdram[count^0x111371] != prime) {
                MemoryTestFail();
                return;
            }
        }
//    }
}
#endif

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
void c_entry(void) {
	UNS_8 *p8, ret;
	INT_32 toread, idx, blk, page, sector;
	PFV execa = (PFV) STAGE1_LOAD_ADDR;

	uart_output_init();

	//while (1) // for (idx=0; idx<9999; idx++)
    uart_output((UNS_8 *)"FDI3250 Kickstart v1.00\r\n");

#if OPTION_MEMORY_TEST
    uart_output((UNS_8 *)"DDR:   Doing memory test ... \n");
    MemoryTest();
    uart_output((UNS_8 *)"Done\n");
#endif

    /* Initialize NAND FLASH */
	if (nand_lb_slc_init() != 1) 
	{
	    uart_output((UNS_8 *)"NAND Flash Init failure!\r\n");
		while (1);
	} else {
	    uart_output((UNS_8 *)"NAND Flash Initialized\r\n");
	}

	/* Read data into memory */
	toread = STAGE1_LOAD_SIZE;
	blk = 1;
	page = 0;
	p8 = (UNS_8 *) STAGE1_LOAD_ADDR;
	while (toread > 0) 
	{
		ret = nand_lb_slc_is_block_bad(blk);
		if (ret == 0)
		{
			while(page < nandgeom.pages_per_block)
			{
				sector = nand_bp_to_sector(blk, page);
				nand_lb_slc_read_sector(sector, tmpbuff,NULL);
				for (idx = 0; idx < 2048; idx++) 
				{
					*p8 = tmpbuff [idx];
					p8++;
				}
				page++;
				toread = toread - 2048;
			}
			blk++;
			page = 0;
		}
		else
		{
			blk++;	
		}
	}
	
#ifdef USE_MMU
	dcache_flush();
	dcache_inval();
	icache_inval();
#endif

    uart_output((UNS_8 *)"Running Stage 1 Loader ...\r\n");
	execa();
}
