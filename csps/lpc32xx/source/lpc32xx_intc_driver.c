/***********************************************************************
 * $Id:: lpc32xx_intc_driver.c 4984 2010-09-21 09:16:58Z ing03005      $
 *
 * Project: LPC32XX interrupt driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx
 *     interrupt driver.
 *
 * Notes:
 *     This driver requires that the CP15 MMU driver is correctly
 *     working.
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
***********************************************************************/

#include "lpc_arm_arch.h"
#include "lpc_arm922t_cp15_driver.h"
#include "lpc32xx_intc_driver.h"

/***********************************************************************
 * Interrupt driver package data
***********************************************************************/

/* External vector jump addresses - setting one of these addresses with
   a new address of a function will cause the new function to be called
   when the interrupt or exception occurs */
extern UNS_32 lpc32xx_reset_vector;
extern UNS_32 vec_reset_handler;
extern UNS_32 vec_undefined_handler;
extern UNS_32 vec_swi_handler;
extern UNS_32 vec_prefetch_handler;
extern UNS_32 vec_abort_handler;
extern UNS_32 vec_irq_handler;
extern UNS_32 vec_fiq_handler;

/* Array of Interrupt handlers */
PFV irq_func_ptrs[IRQ_END_OF_INTERRUPTS];

/* Pointer to logical interrupt vector area (writable) */
UNS_32 *vecarea;

/***********************************************************************
 * Vectored Interrupt driver private functions
***********************************************************************/


/***********************************************************************
 *
 * Function: int_write_arm_vec_table
 *
 * Purpose: Writes the vector table and jump addresses to vector area
 *
 * Processing:
 *     Copy the shadowed image of the interrupt and exception vector
 *     table from memory to the vector jump area (usually at location
 *     0x00000000). Force out any cached values to external memory.
 *
 * Parameters: None
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes:
 *     Ideally, we would check the state of the V bit in the CP15
 *     coprocessor register 1 to determine the address of the where
 *     the vector area is located. If that bit was set, the vectors
 *     would be located at address 0xFFFF0000 instead of 0x00000000.
 *     This function assumes that the vector area is at 0x00000000.
 *
 **********************************************************************/
static void int_write_arm_vec_table(void)
{
  UNS_32 *dst, *dstsave, *src;
  INT_32 vecsize;
  UNS_32 high_vector;

  /* If vector address is automatic address, compute address */
  dst = vecarea;
  if ((UNS_32) dst == 0xFFFFFFFF)
  {
    /* Assume that vector table is located at low vector
       (0x00000000) address */
    dst = (UNS_32 *) ARM_RESET_VEC;

    /* Check status of high vector bit in MMU control register and
       set destination address of vector table to high vector
       address if bit is set */
    high_vector = cp15_get_mmu_control_reg();

    /* If high bit is set, use high vector addresses instead */
    if ((high_vector & ARM922T_MMU_CONTROL_V) != 0)
    {
      dst = (UNS_32 *) 0xFFFF0000;
    }
  }

  /* Copy vector block to interrupt vector area */
  dstsave = dst;
  for (src = (UNS_32 *) & lpc32xx_reset_vector;
       src <= (UNS_32 *) &vec_fiq_handler; src++)
  {
    *dst = *src;
    dst++;
  }

  /* Write out cached vector table to memory */
  vecsize = ((INT_32) & vec_fiq_handler -
             (INT_32) & lpc32xx_reset_vector) / 4;
  cp15_force_cache_coherence(dstsave, (dstsave + vecsize));
}

/***********************************************************************
 *
 * Function: int_get_controller
 *
 * Purpose: Determines the interrupt controller based on interrupt
 *   source
 *
 * Processing:
 *     If the interrupt source is in the range of the MIC controller
 *	   then return MIC controller base or else if interrupt source is
 *	   in the range of the SIC1 controller return MIC controller base
 *	   or else if interrupt source is in the range of the SIC2
 *	   controller return SIC2 controller base.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs:
 *	   pIntc_base: Contains the base address of controller
 *	   pBit_pos  : Contains the bit position of interrpt source within
 *                 controller regs
 *
 * Returns:
 *	  Interrupt controller base.
 *
 *
 **********************************************************************/
static BOOL_32 int_get_controller(INTERRUPT_SOURCE_T source,
                                  INTC_REGS_T** pIntc_base,
                                  UNS_32* pBit_pos)
{
  BOOL_32 ret_value = TRUE;

  /* Determine the interrupt controller */
  if (source < IRQ_SIC1_BASE)
  {
    *pIntc_base = MIC;
    *pBit_pos = (UNS_32)source;
  }
  else if ((source >= IRQ_SIC1_BASE) && (source < IRQ_SIC2_BASE))
  {
    *pIntc_base = SIC1;
    *pBit_pos = ((UNS_32)source - IRQ_SIC1_BASE);
  }
  else if (source < IRQ_END_OF_INTERRUPTS)
  {
    *pIntc_base = SIC2;
    *pBit_pos = ((UNS_32)source - IRQ_SIC2_BASE);
  }
  else
  {
    *pIntc_base = 0;
    *pBit_pos = 0;
    ret_value = FALSE;
  }
  return ret_value;
}


/***********************************************************************
 * Interrupt driver public functions
***********************************************************************/

/***********************************************************************
 *
 * Function: int_initialize
 *
 * Purpose: Initialize the interrupt controller
 *
 * Processing:
 *     For all IRQ interrupt sources, clear the dispatcher jump address
 *     and disable the interrupt in the interrupt controller. Copy the
 *     vector table and vector branch instructions to the interrupt
 *     and exception area with a call to int_write_table.
 *
 * Parameters:
 *     vectbladdr: Pointer to interrupt vector area, or 0xFFFFFFFF to
 *                 have driver determine address
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void int_initialize(UNS_32 vectbladdr)
{
  UNS_32 source;

  /* Initialize main interrupt controller*/
  MIC->er = 0x00000000; /*disable all interrupt sources*/
  MIC->rsr = 0xFFFFFFFF;/*clear all edge triggered interrupts*/
  MIC->apr = MIC_APR_DEFAULT;/*set polarity for all internal irqs*/
  MIC->atr = MIC_ATR_DEFAULT;/*set act. types for all internal irqs*/
  MIC->itr = (_BIT(IRQ_SUB1FIQ) | _BIT(IRQ_SUB2FIQ));

  /* Initialize sub interrupt controller 1 */
  SIC1->er = 0x00000000; /*disable all interrupt sources*/
  SIC1->rsr = 0xFFFFFFFF;/*clear all edge triggered interrupts*/
  SIC1->apr = SIC1_APR_DEFAULT;/*set polarity for internal irqs*/
  SIC1->atr = SIC1_ATR_DEFAULT;/*set act-types for internal irqs*/
  SIC1->itr = 0x00000000;/*set all interrupts as irqs*/

  /* Initialize sub interrupt controller 2 */
  SIC2->er = 0x00000000; /*disable all interrupt sources*/
  SIC2->rsr = 0xFFFFFFFF;/*clear all edge triggered interrupts*/
  SIC2->apr = SIC2_APR_DEFAULT;/*set polarity for internal irqs*/
  SIC2->atr = SIC2_ATR_DEFAULT;/*set act. types for internal irqs*/
  SIC2->itr = 0x00000000;/*set all interrupts as irqs*/

  /* Enable sub-IRQ/FIQ handlers on main interrupt controller */
  MIC->er = (_BIT(IRQ_SUB1IRQ) | _BIT(IRQ_SUB2IRQ) |
             _BIT(IRQ_SUB1FIQ) | _BIT(IRQ_SUB2FIQ));

  /* Clear the IRQ vector table and disable all interrupts */
  for (source = 0; source < IRQ_END_OF_INTERRUPTS; source++)
  {
    irq_func_ptrs[source] = (PFV) NULL;
  }

  /* Save user passed vector area pointer */
  vecarea = (UNS_32 *) vectbladdr;

  /* write ARM vector table */
  int_write_arm_vec_table();
}

/***********************************************************************
 *
 * Function: int_install_arm_vec_handler
 *
 * Purpose: Install an new ARM exception vector handler
 *
 * Processing:
 *     If the passed fiq_handler_ptr pointer is not NULL, then
 *     set the handler jump address for the specific interrupt or
 *     exception to handler_ptr.  Recopy the vector table and vector
 *     branch instructions to the interrupt and exception area with a
 *     call to int_write_table.
 *
 * Parameters:
 *     handler_id  : Must be an enumeration of type VECTOR_T
 *     handler_ptr : Pointer to new interrupt or exception handler
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void int_install_arm_vec_handler(VECTOR_T handler_id,
                                 PFV handler_ptr)
{
  /* Update address only if it is not NULL */
  if (handler_ptr != (PFV) NULL)
  {
    switch (handler_id)
    {
      case RESET_VEC:
        vec_reset_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_reset_handler,
          (UNS_32 *) &vec_reset_handler);
        break;

      case UNDEFINED_INST_VEC:
        vec_undefined_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_undefined_handler,
          (UNS_32 *) &vec_undefined_handler);
        break;

      case SWI_VEC:
        vec_swi_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_swi_handler,
          (UNS_32 *) &vec_swi_handler);
        break;

      case PREFETCH_ABORT_VEC:
        vec_prefetch_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_prefetch_handler,
          (UNS_32 *) &vec_prefetch_handler);
        break;

      case DATA_ABORT_VEC:
        vec_abort_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_abort_handler,
          (UNS_32 *) &vec_abort_handler);
        break;

      case IRQ_VEC:
        vec_irq_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_irq_handler,
          (UNS_32 *) &vec_irq_handler);
        break;

      case FIQ_VEC:
        vec_fiq_handler = (UNS_32) handler_ptr;
        cp15_force_cache_coherence(
          (UNS_32 *) &vec_fiq_handler,
          (UNS_32 *) &vec_fiq_handler);
        break;

      default:
        break;
    }

    /* Update ARM vector table */
    int_write_arm_vec_table();
  }
}

/***********************************************************************
 *
 * Function: int_install_irq_handler
 *
 * Purpose: Install an IRQ interrupt handler for an internal interrupt
 *
 * Processing:
 *     For the selected interrupt, sets the function called as the
 *     passed value.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *     func_ptr : Pointer to a void function
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_install_irq_handler(INTERRUPT_SOURCE_T source,
                                PFV func_ptr)
{
  BOOL_32 ret_value = FALSE;

  if (source < IRQ_END_OF_INTERRUPTS)
  {
    irq_func_ptrs[source] = func_ptr;
    ret_value = TRUE;
  }
  return ret_value;
}

/***********************************************************************
 *
 * Function: int_install_ext_irq_handler
 *
 * Purpose: Install an IRQ interrupt handler for an external interrupt
 *
 * Processing:
 *     For the selected interrupt, sets the function called as the
 *     passed value.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *     func_ptr : Pointer to a void function
 *     type	    : Interrupt activation type (INTERRUPT_TYPE_T)
 *     high     : Interrupt on high level or edge
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_install_ext_irq_handler(INTERRUPT_SOURCE_T source,
                                    PFV func_ptr,
                                    INTERRUPT_TYPE_T type,
                                    int high)
{
  BOOL_32 ret_value = TRUE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    switch (type)
    {
      case ACTIVE_LOW:
        pIntc->apr &= ~_BIT(bit_pos);
        pIntc->atr &= ~_BIT(bit_pos);
        break;
      case ACTIVE_HIGH:
        pIntc->apr |= _BIT(bit_pos);
        pIntc->atr &= ~_BIT(bit_pos);
        break;
      case FALLING_EDGE:
        pIntc->apr &= ~_BIT(bit_pos);
        pIntc->atr |= _BIT(bit_pos);
        break;
      case RISING_EDGE:
        pIntc->apr |= _BIT(bit_pos);
        pIntc->atr |= _BIT(bit_pos);
        break;
      default:
        ret_value = FALSE;
        break;
    }
    if (TRUE == ret_value)
      irq_func_ptrs[source] = func_ptr;
  }

  return ret_value;
}

/***********************************************************************
 *
 * Function: int_enable
 *
 * Purpose: Enable an interrupt
 *
 * Processing:
 *     Enables the interrupt in the controller for the selected source.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void int_enable(INTERRUPT_SOURCE_T source)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    /* cast the interrupt controller pointer*/
    pIntc->er |= _BIT(bit_pos);
  }
}

/***********************************************************************
 *
 * Function: int_disable
 *
 * Purpose: Disable an interrupt
 *
 * Processing:
 *     Masks the interrupt in the controller for the selected source.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Nothing
 *
 * Notes: None
 *
 **********************************************************************/
void int_disable(INTERRUPT_SOURCE_T source)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    /* cast the interrupt controller pointer*/
    pIntc->er &= ~_BIT(bit_pos);
  }
}

/***********************************************************************
 *
 * Function: int_pending
 *
 * Purpose: Check to see if a unmasked interrupt is pending
 *
 * Processing:
 *     If the status for the selected interrupt source is set,
 *     a TRUE is returned. Otherwise, FALSE is returned.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_pending(INTERRUPT_SOURCE_T source)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    /* cast the interrupt controller pointer*/
    ret_value = ((pIntc->sr & _BIT(bit_pos)) != 0);
  }

  return ret_value;
}

/***********************************************************************
 *
 * Function: int_raw_pending
 *
 * Purpose: Check to see if a raw interrupt is pending
 *
 * Processing:
 *     If the raw status for the selected interrupt source is set,
 *     a TRUE is returned. Otherwise, FALSE is returned.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_raw_pending(INTERRUPT_SOURCE_T source)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    /* cast the interrupt controller pointer*/
    ret_value = ((pIntc->rsr & _BIT(bit_pos)) != 0);
  }

  return ret_value;
}

/***********************************************************************
 *
 * Function: int_enabled
 *
 * Purpose: Check to see if an interrupt is enabled
 *
 * Processing:
 *     If the selected interrupt source is enabled, a TRUE is returned.
 *     Otherwise, FALSE is returned.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_enabled(INTERRUPT_SOURCE_T source)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    /* cast the interrupt controller pointer*/
    ret_value = ((pIntc->er & _BIT(bit_pos)) != 0);
  }

  return ret_value;
}

/***********************************************************************
 *
 * Function: int_clear
 *
 * Purpose: Clear a pending (latched) interrupt
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     source   : Interrupt source of type INTERRUPT_SOURCE_T
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_clear(INTERRUPT_SOURCE_T source)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (TRUE == ret_value)
  {
    /* Clear pending interrupt */
    pIntc->rsr = _BIT(bit_pos);
  }

  return ret_value;
}

/***********************************************************************
 *
 * Function: int_setup_irq_fiq
 *
 * Purpose: Setup an interrupt as an IRQ (FALSE) or and FIQ (TRUE)
 *
 * Processing:
 *     See function.
 *
 * Parameters:
 *     source  : Interrupt source of type INTERRUPT_SOURCE_T
 *     use_fiq : TRUE to use an FIQ interrupt type, FALSE for IRQ
 *
 * Outputs: None
 *
 * Returns: Returns TRUE or FALSE.
 *
 * Notes: None
 *
 **********************************************************************/
BOOL_32 int_setup_irq_fiq(INTERRUPT_SOURCE_T source,
                          BOOL_32 use_fiq)
{
  BOOL_32 ret_value = FALSE;
  INTC_REGS_T *pIntc;
  UNS_32 bit_pos = 0;

  /* get the interrupt controller for the give interrupt source */
  ret_value = int_get_controller(source, &pIntc, &bit_pos);
  if (use_fiq == TRUE)
  {
    /* FIQ interrupt type */
    pIntc->itr |= _BIT(bit_pos);
  }
  else
  {
    /* IRQ interrupt type */
    pIntc->itr &= ~_BIT(bit_pos);
  }

  return ret_value;
}
