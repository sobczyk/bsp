/***********************************************************************
 * $Id:: sys.h 3396 2010-05-06 18:03:32Z usb10132                      $
 *
 * Project: System structures and functions
 *
 * Description:
 *     Various system specific items
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

#include "lpc_types.h"
#include "s1l_cfg.h"

#ifndef SYS_H
#define SYS_H

/* 10mS timer tick used for some timeout checks */
extern volatile UNS_32 tick_10ms;

/* Initialize MMU command group */
void mmu_cmd_group_init(void);

/* Initialize hardware command group */
void hw_cmd_group_init(void);

/* Flag used to indicate bad blocks can be erased */
extern int erasebadblocks;

#endif /* SYS_H */
