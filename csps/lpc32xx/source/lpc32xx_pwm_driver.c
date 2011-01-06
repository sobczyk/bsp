/***********************************************************************
 * $Id:: lpc32xx_pwm_driver.c 4978 2010-09-20 22:32:52Z usb10132       $
 *
 * Project: LPC32xx PWM driver
 *
 * Description:
 *     This file contains driver support for the LPC32xx PWM.
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
 *********************************************************************/

#include "lpc32xx_pwm_driver.h"

/***********************************************************************
 * PWM driver package data
***********************************************************************/

/* PWM device configuration structure type */
typedef struct
{
  PWM_REGS_T *regptr;     /* Pointer to PWM registers */
  PWM_CBS_T cbs;          /* Call-back service */
  BOOL_32 init;           /* Device initialized flag */
} PWM_CFG_T;

/* PWM driver data for PWM3 and PWM4 */
static PWM_CFG_T pwmdat[2];

/***********************************************************************
 * PWM driver private functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: pwm_write_mr
 *
 * Purpose: write into the specified PWM match register
 *
 * Processing:
 *     Based on the passed function and option values, set appropriate
 *     timer match register parameter.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
 *     reg:   match register id
 *     arg:   argument
 *
 * Outputs: None
 *
 * Returns: Number of words actually written (always 1)
 *
 * Notes: None
 *
 **********************************************************************/
UNS_32 pwm_write_mr(PWM_REGS_T * devid,
                    INT_32 reg,
                    INT_32 arg)
{
  if (reg < 4)
  {
    *(((UNS_32 *) &(devid->pwm_mr0)) + reg) = arg;
  }
  else
  {
    *(((UNS_32 *) &(devid->pwm_mr4)) + reg - 4) = arg;
  }

  return 1;
}

/***********************************************************************
 * PWM driver public functions
 **********************************************************************/

/***********************************************************************
 *
 * Function: pwm_open
 *
 * Purpose: Open the PWM
 *
 * Processing:
 *     If the passed register base is valid and the PWM
 *     driver isn't already initialized, then setup the PWM
 *     into a default initialized state that is disabled. Return
 *     a pointer to the driver's data structure or NULL if driver
 *     initialization failed.
 *
 * Parameters:
 *     ipbase: Pointer to a PWM peripheral block
 *     arg   : Not used
 *
 * Outputs: None
 *
 * Returns: The pointer to a PWM config structure or NULL
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 pwm_open(void *ipbase, INT_32 arg)
{
  UNS_32 pwm_id = 9999;
  INT_32 tptr = (INT_32) NULL;

  if ((PWM_REGS_T *) ipbase == PWM3) pwm_id = 0;
  if ((PWM_REGS_T *) ipbase == PWM4) pwm_id = 1;
  if (pwm_id != 9999)
  {
    /* has the selected PWM been previously initialized? */
    if (pwmdat[pwm_id].init == FALSE)
    {
      /* Device not initialized and it is usable, so set it to
         used */
      pwmdat[pwm_id].init = TRUE;

      /* Save address of register block */
      if (pwm_id == 0) pwmdat[pwm_id].regptr = PWM3;
      if (pwm_id == 1) pwmdat[pwm_id].regptr = PWM4;

      /* Disable selected PWM timer and outputs */
      pwmdat[pwm_id].regptr->pwm_tcr = 0x00;
      pwmdat[pwm_id].regptr->pwm_pcr &= ~0x00007E00;

      tptr = (INT_32) & pwmdat[pwm_id];
    }
  }

  return tptr;
}

/***********************************************************************
 *
 * Function: pwm_close
 *
 * Purpose: Close the PWM
 *
 * Processing:
 *     If init is not TRUE, then return _ERROR to the caller as the
 *     device was not previously opened. Otherwise, disable the
 *     PWM, set init to FALSE, and return _NO_ERROR to the caller.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
 *
 * Outputs: None
 *
 * Returns: The status of the close operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS pwm_close(INT_32 devid)
{
  UNS_32 pwm_id = 9999;
  STATUS status = _ERROR;

  if ((PWM_CFG_T *) devid == &pwmdat[0]) pwm_id = 0;
  if ((PWM_CFG_T *) devid == &pwmdat[1]) pwm_id = 1;
  {
    if (pwmdat[pwm_id].init == TRUE)
    {
      /* Disable PWM */
      pwmdat[pwm_id].regptr->pwm_tcr = 0x00;

      /* Set timer as uninitialized */
      pwmdat[pwm_id].init = FALSE;

      /* Successful operation */
      status = _NO_ERROR;
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: pwm_ioctl
 *
 * Purpose: PWM configuration block
 *
 * Processing:
 *     This function is a large case block. Based on the passed function
 *     and option values, set or get the appropriate timer parameter.
 *
 * Parameters:
 *     devid: Pointer to PWM config structure
 *     cmd:   ioctl command
 *     arg:   ioctl argument
 *
 * Outputs: None
 *
 * Returns: The status of the ioctl operation
 *
 * Notes: None
 *
 **********************************************************************/
STATUS pwm_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg)
{
  UNS_32 pwm_id = 9999;
  INT_32 status = _ERROR;
  UNS_32 channel, pointf, pointr, pulse;
  BOOL_32 solved;

  if ((PWM_CFG_T *) devid == &pwmdat[0]) pwm_id = 0;
  if ((PWM_CFG_T *) devid == &pwmdat[1]) pwm_id = 1;

  if (pwm_id != 9999)
  {
    if (pwmdat[pwm_id].init == TRUE)
    {
      status = _NO_ERROR;

      switch (cmd)
      {
        case PWM_SYSTEM_CONTROL:
          switch (arg)
          {
            case PWM_TIMER_STOP:
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_ENABLE;
              break;

            case PWM_TIMER_RESET:
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_RESET;
              break;

            case PWM_TIMER_SYNC:
              if (pwm_id == 1)
              {
                status = LPC_BAD_PARAMS;
              }
              else
              {
                pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_ENABLE;
                pwmdat[pwm_id].regptr->pwm_tcr |= PWM_ENABLE;
                pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_RESET;
                pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_RESET;
                pwmdat[pwm_id].regptr->pwm_tcr |= PWM_MASTER_DISABLE;
              }
              break;

            case PWM_TIMER_RESTART:
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_ENABLE;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_ENABLE;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_ENABLE;
              break;

            case PWM_TIMER_GO:
              pwmdat[pwm_id].regptr->pwm_tcr =
                ((pwmdat[pwm_id].regptr->pwm_tcr) & 
                 (~PWM_MASTER_DISABLE)) | 
                 (PWM_ENABLE | PWM_COUNTER_ENABLE);
              break;

            default:
              status = LPC_BAD_PARAMS;
              break;
          }
          break;

        case PWM_SYSTEM_SETUP:
          if (((PWM_SYSTEM_SETUP_T *) arg)->clock > 
                                            PWM_CLOCK_PCAP_ANY_EDGE)
          {
            status = LPC_BAD_PARAMS;
            break;
          }
          else
          {
            pwmdat[pwm_id].regptr->pwm_ctcr = 
                                   ((PWM_SYSTEM_SETUP_T *) arg)->clock;
          }
          pwmdat[pwm_id].regptr->pwm_pr = 
                              ((PWM_SYSTEM_SETUP_T*) arg)->prescale - 1;
          pwmdat[pwm_id].regptr->pwm_mr0 = 
                                 ((PWM_SYSTEM_SETUP_T *) arg)->period;
          switch (((PWM_SYSTEM_SETUP_T *) arg)->update)
          {
            case PWM_NO_UPDATE:
              break;

            case PWM_UPDATE:
              pwmdat[pwm_id].regptr->pwm_ler |= PWM_EN_MATCHL(0);
              break;

            case PWM_UPDATE_NOW:
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_ENABLE;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_ENABLE;
              pwmdat[pwm_id].regptr->pwm_ler |= PWM_EN_MATCHL(0);
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_ENABLE;
              break;

            default:
              status = LPC_BAD_PARAMS;
              break;
          }
          break;

        case PWM_CHANNEL_SETUP:
          channel = ((PWM_CHANNEL_SETUP_T *) arg)->channel;
          if ((channel == 0) || (channel > 6))
          {
            status = LPC_BAD_PARAMS;
          }
          else
          {
            switch (((PWM_CHANNEL_SETUP_T *) arg)->mode)
            {
              case PWM_SINGLE_EDGE:
                if (((PWM_CHANNEL_SETUP_T *) arg)->duty_option == 
                                                       PWM_WAV_DUTY_ABS)
                {
                  pointf = ((PWM_CHANNEL_SETUP_T *) arg)->duty;
                }
                else if (((PWM_CHANNEL_SETUP_T *) arg)->duty == 0)
                {
                  pointf = 0;
                }
                else if (((PWM_CHANNEL_SETUP_T *) arg)->duty > 99)
                {
                  pointf = 0xFFFFFFFF;
                }
                else
                {
                  pointf = ((((PWM_CHANNEL_SETUP_T *) arg)->duty) * 
                                (pwmdat[pwm_id].regptr->pwm_mr0)) / 100;
                }

                pwm_write_mr(pwmdat[pwm_id].regptr, channel, pointf);
                if (channel > 1)
                {
                  pwmdat[pwm_id].regptr->pwm_pcr &= 
                                              ~PWM_DOUBLE_EDGE(channel);
                }
                break;

              case PWM_DUAL_EDGE:
                solved = FALSE;
                if (((PWM_CHANNEL_SETUP_T *) arg)->duty_option == 
                                                       PWM_WAV_DUTY_ABS)
                {
                  pulse = ((PWM_CHANNEL_SETUP_T *) arg)->duty;
                }
                else if (((PWM_CHANNEL_SETUP_T *) arg)->duty > 100)
                {
                  status = LPC_BAD_PARAMS;
                  break;
                }
                else if (((PWM_CHANNEL_SETUP_T *) arg)->duty == 0)
                {
                  pointf = 0;
                  pointr = 0xFFFFFFFF;
                  solved = TRUE;
                }
                else if (((PWM_CHANNEL_SETUP_T *) arg)->duty == 100)
                {
                  pointr = 0;
                  pointf = 0xFFFFFFFF;
                  solved = TRUE;
                }
                else
                {
                  pulse = ((((PWM_CHANNEL_SETUP_T *) arg)->duty) * 
                                (pwmdat[pwm_id].regptr->pwm_mr0)) / 100;
                }


                if (solved == FALSE)
                {
                  if (((PWM_CHANNEL_SETUP_T *) arg)->offset_option == 
                                                     PWM_WAV_OFFSET_ABS)
                  {
                    pointr = ((PWM_CHANNEL_SETUP_T *) arg)->offset;
                  }
                  else if (((PWM_CHANNEL_SETUP_T *) arg)->offset > 100)
                  {
                    status = LPC_BAD_PARAMS;
                    break;
                  }
                  else if (((PWM_CHANNEL_SETUP_T *) arg)->offset == 0)
                  {
                    pointr = 0;
                  }
                  else if (((PWM_CHANNEL_SETUP_T *) arg)->offset == 100)
                  {
                    pointr = pwmdat[pwm_id].regptr->pwm_mr0;
                  }
                  else
                  {
                    pointr = ((((PWM_CHANNEL_SETUP_T *) arg)->offset) * 
                                (pwmdat[pwm_id].regptr->pwm_mr0)) / 100;
                  }
                  pointf = pointr + pulse;
                  if (((PWM_CHANNEL_SETUP_T *) arg)->ini_state != 0)
                  {
                    pulse = pwmdat[pwm_id].regptr->pwm_mr0 - 
                                                      (pointf - pointr);
                    pointf = pointr;
                    pointr = pointf + pulse;
                  }
                }

                pwm_write_mr(pwmdat[pwm_id].regptr,channel - 1,pointr);
                pwm_write_mr(pwmdat[pwm_id].regptr, channel, pointf);
                pwmdat[pwm_id].regptr->pwm_pcr |= 
                                               PWM_DOUBLE_EDGE(channel);
                break;

              default:
                status = LPC_BAD_PARAMS;
                break;
            }
          }
          break;

        case PWM_CHANNEL_OUT_ENABLE:
          pwmdat[pwm_id].regptr->pwm_pcr |= PWM_ENABLE_OUT(arg);
          break;

        case PWM_CHANNEL_OUT_DISABLE:
          pwmdat[pwm_id].regptr->pwm_pcr &= ~PWM_ENABLE_OUT(arg);
          break;

        case PWM_UPDATE_CONTROL:
          switch (((PWM_UPDATE_CONTROL_T *) arg)->update)
          {
            case PWM_UPDATE:
              pwmdat[pwm_id].regptr->pwm_ler |= 
                               ((PWM_UPDATE_CONTROL_T *) arg)->channels;
              break;

            case PWM_UPDATE_NOW:
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_ENABLE;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_ENABLE;
              pwmdat[pwm_id].regptr->pwm_ler |= 
                               ((PWM_UPDATE_CONTROL_T *) arg)->channels;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr &= ~PWM_COUNTER_RESET;
              pwmdat[pwm_id].regptr->pwm_tcr |= PWM_COUNTER_ENABLE;
              break;

            default:
              status = LPC_BAD_PARAMS;
              break;
          }

          break;

        case PWM_INT_ENABLE:
          if (arg > 6)
          {
            status = LPC_BAD_PARAMS;
            break;
          }
          else
          {
            pwmdat[pwm_id].regptr->pwm_mcr |= PWM_INT_ON_MATCH(arg);
          }
          break;

        case PWM_INT_DISABLE:
          pwmdat[pwm_id].regptr->pwm_mcr &= ~PWM_INT_ON_MATCH(arg);
          pwmdat[pwm_id].regptr->pwm_ir = PWM_INT_MATCH(arg);
          break;

        case PWM_INT_FLAGS_READ:
          status = pwmdat[pwm_id].regptr->pwm_ir;
          break;

        case PWM_INT_FLAGS_CLEAR:
          pwmdat[pwm_id].regptr->pwm_ir = arg;
          break;

        case PWM_CAPTURE_READ:
          status = pwmdat[pwm_id].regptr->pwm_cr0;
          break;

        case PWM_CAPTURE_INT_DISABLE:
          pwmdat[pwm_id].regptr->pwm_ccr &=
            ~(PWM_INT_ON_PCAP(0) | PWM_PCAP_RISING(0) | 
              PWM_PCAP_FALLING(0));
          pwmdat[pwm_id].regptr->pwm_ir = 0x10;
          break;

        case PWM_CAPTURE_INT_ENABLE:
          if (arg == PWM_CAPTURE_INT_RISING)
          {
            pwmdat[pwm_id].regptr->pwm_ccr |= PWM_PCAP_RISING(0);
            pwmdat[pwm_id].regptr->pwm_ccr &= ~PWM_PCAP_FALLING(0);
          }
          else if (arg == PWM_CAPTURE_INT_FALLING)
          {
            pwmdat[pwm_id].regptr->pwm_ccr &= ~PWM_PCAP_RISING(0);
            pwmdat[pwm_id].regptr->pwm_ccr |= PWM_PCAP_FALLING(0);
          }
          else
          {
            pwmdat[pwm_id].regptr->pwm_ccr |= PWM_PCAP_RISING(0);
            pwmdat[pwm_id].regptr->pwm_ccr |= PWM_PCAP_FALLING(0);
          }
          pwmdat[pwm_id].regptr->pwm_ccr |= PWM_INT_ON_PCAP(0);
          break;

        default:
          break;
      }
    }
  }

  return status;
}

/***********************************************************************
 *
 * Function: pwm_read
 *
 * Purpose: PWM read function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:     Pointer to millisecond timer descriptor
 *     buffer:    Pointer to data buffer to copy to
 *     max_bytes: Number of bytes to read
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually read (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 pwm_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes)
{
  return 0;
}

/***********************************************************************
 *
 * Function: pwm_write
 *
 * Purpose: PWM write function (stub only)
 *
 * Processing:
 *     Return 0 to the caller.
 *
 * Parameters:
 *     devid:   Pointer to millisecond timer descriptor
 *     buffer:  Pointer to data buffer to copy from
 *     n_bytes: Number of bytes to write
 *
 * Outputs: None
 *
 * Returns: Number of bytes actually written (always 0)
 *
 * Notes: None
 *
 **********************************************************************/
INT_32 pwm_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes)
{
  return 0;
}
