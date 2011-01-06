/***********************************************************************
 * $Id:: board_nor_flash_driver.c 3380 2010-05-05 23:54:23Z usb10132   $
 *
 * Project: Simple functions for NOR FLASH program
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

#include "board_nor_flash_driver.h"

#define M8(adr)  (*((volatile UNS_8  *)		(adr)))
#define M16(adr) (*((volatile UNS16 *)		(adr)))
#define M32(adr) (*((volatile UNS_32  *)	(adr)))

#define BOARD_NOR_FLASH_BASE_ADDR			EMC_CS0_BASE

/*
 * Flash Status Register
 */
union fsreg {
  struct b  {
    unsigned int q0l:1;
    unsigned int q1l:1;
    unsigned int q2l:1;
    unsigned int q3l:1;
    unsigned int q4l:1;
    unsigned int q5l:1;
    unsigned int q6l:1;
    unsigned int q7l:1;
    unsigned int rl:8;
    unsigned int q0h:1;
    unsigned int q1h:1;
    unsigned int q2h:1;
    unsigned int q3h:1;
    unsigned int q4h:1;
    unsigned int q5h:1;
    unsigned int q6h:1;
    unsigned int q7h:1;
    unsigned int rh:8;
  } b;
  unsigned int v;
} fsr;
UNS_32 base_adr = BOARD_NOR_FLASH_BASE_ADDR;

/***********************************************************************
 *
 * Function: board_not_polling
 *
 * Purpose: Check if Program/Erase completed.
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     adr  : Block Start Address
 *
 * Outputs: None
 *
 * Returns: 0 - OK, 1 - Failed
 *
 * Notes: None
 *
 **********************************************************************/
static INT_32 board_not_polling(UNS_32 adr)
{
  UNS_32 q6l, q6h;

  fsr.v = M32(adr);
  q6l = fsr.b.q6l;
  q6h = fsr.b.q6h;
  do
  {
    fsr.v = M32(adr);
    if ((fsr.b.q6l == q6l) && (fsr.b.q6h == q6h))
	{
      return (0);
    }
    q6l = fsr.b.q6l;
    q6h = fsr.b.q6h;
  } while ((fsr.b.q5l == 0) || (fsr.b.q5h == 0));
  fsr.v = M32(adr);
  q6l = fsr.b.q6l;
  q6h = fsr.b.q6h;
  fsr.v = M32(adr);
  if ((fsr.b.q6l == q6l) && (fsr.b.q6h == q6h))
  {
    return (0);
  }
  
  /* Send RESET command to reset device */
  M32(adr) = 0x00F000F0;
  return (1);
}

/***********************************************************************
 *
 * Function: board_nor_erase_chip
 *
 * Purpose: Erase whole NOR chip.
 *
 * Processing:
 *     See function.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: 0 - OK, 1 - Failed
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 board_nor_erase_chip(void)
{
  /* Start Chip Erase Command */
  M32(base_adr + (0x555 << 2)) = 0x00AA00AA;
  M32(base_adr + (0x2AA << 2)) = 0x00550055;
  M32(base_adr + (0x555 << 2)) = 0x00800080;
  M32(base_adr + (0x555 << 2)) = 0x00AA00AA;
  M32(base_adr + (0x2AA << 2)) = 0x00550055;
  M32(base_adr + (0x555 << 2)) = 0x00100010;

  /* Wait until Erase completed */
  return (board_not_polling(base_adr));           
}

/***********************************************************************
 *
 * Function: board_nor_erase_sector
 *
 * Purpose:  Erase Sector in Flash Memory
 *
 * Processing:
 *     See function.
 * 
 * Parameters:
 *     adr:  Sector Address
 *
 * Outputs: None
 *
 * Returns: 0 - OK, 1 - Failed
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 board_nor_erase_sector(UNS_32 adr)
{
  /* Start Erase Sector Command */
  M32(base_adr + (0x555 << 2)) = 0x00AA00AA;
  M32(base_adr + (0x2AA << 2)) = 0x00550055;
  M32(base_adr + (0x555 << 2)) = 0x00800080;
  M32(base_adr + (0x555 << 2)) = 0x00AA00AA;
  M32(base_adr + (0x2AA << 2)) = 0x00550055;
  M32(adr) = 0x00300030;

  /* Wait for Sector Erase Timeout */
  do {					          
    fsr.v = M32(adr);
  } while ((fsr.b.q3l == 0) || (fsr.b.q3h == 0));

  /* Wait until Erase completed */
  return (board_not_polling(adr));
}

/***********************************************************************
 *
 * Function: board_nor_prg
 *
 * Purpose:  Program NOR Flash Memory
 *
 * Processing:
 *     See function.
 * 
 * Parameters:
 *     adr:  Start Address
 *     buf:  Write Buffer Address
 *     sz:   Write Size
 *
 * Outputs: None
 *
 * Returns: 0 - OK, 1 - Failed
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 board_nor_prg(UNS_32 adr, UNS_8 *buf, UNS_32 sz)
{
  int  i;

  for (i = 0; i < ((sz+3)/4); i++)  {
    /* Start Program Command */
    M32(base_adr + (0x555 << 2)) = 0x00AA00AA;
    M32(base_adr + (0x2AA << 2)) = 0x00550055;
    M32(base_adr + (0x555 << 2)) = 0x00A000A0;
    M32(adr) = *((UNS_32 *) buf);

	/* Wait until Programming completed */
    if (board_not_polling(adr))
		return (1);       
    buf += 4;
    adr += 4;
  }
  return (0);
}

/***********************************************************************
 *
 * Function: board_nor_verify
 *
 * Purpose:  Verify Flash Contents
 *
 * Processing:
 *     See function.
 * 
 * Parameters:
 *     adr:  Start Address
 *     buf:  Write Buffer Address
 *     sz:   Write Size
 *
 * Outputs: None
 *
 * Returns: 0 - OK, 1 - Failed
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 board_nor_verify(UNS_32 adr, UNS_8 *buf, UNS_32 sz )
{
  UNS_32 i;

  for (i = 0; i < ((sz+3)/4); i++)
  {
    if (M32(adr) != *((UNS_32 *)buf))
		return (1);
    buf      += 4;
    adr      += 4;
  }

  return (0);
}

/***********************************************************************
 *
 * Function: board_nor_init
 *
 * Purpose:  Initialize NOR flash Timing & Memory Width
 *
 * Processing:
 *     See function.
 * 
 * Parameters:
 *     memory_width:  Memory Width of Externel Memory
 *	   			      00 - 8bit
 *				      01 - 16bit
 *				      02 - 32bit 
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: Timing configuration needs to be done elsewhere.
 *
 **********************************************************************/
void board_nor_init(UNS_8 memory_width)
{
	EMC->emcstatic_regs[0].emcstaticconfig = (EMC_STC_BLS_EN_BIT |
		memory_width);
}
