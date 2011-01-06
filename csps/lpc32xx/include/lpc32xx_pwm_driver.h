/***********************************************************************
 * $Id:: lpc32xx_pwm_driver.h 1114 2008-08-21 20:58:49Z stefanovicz    $
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

#ifndef LPC32XX_PWM_DRIVER_H
#define LPC32XX_PWM_DRIVER_H

#include "lpc32xx_pwm.h"

#ifdef __cplusplus
extern "C"
{
#endif

/***********************************************************************
 * PWM device configuration commands (IOCTL commands and arguments)
 **********************************************************************/

/* Structure containing callback functions for PWM management */
typedef struct
{
  /* Pointer PWM match callback, called when a match occurs */
  PFV pwmmcb;
} PWM_CBS_T;

/* PWM device commands (IOCTL commands) */
typedef enum
{
  /* PWM system control (stop/reset/sync/restart/go) */
  PWM_SYSTEM_CONTROL,
  /* PWM system setup */
  PWM_SYSTEM_SETUP,
  /* individual PWM channel setup */
  PWM_CHANNEL_SETUP,
  /* PWM output enable */
  PWM_CHANNEL_OUT_ENABLE,
  /* PWM output disanable */
  PWM_CHANNEL_OUT_DISABLE,
  /* PWM update control */
  PWM_UPDATE_CONTROL,
  /* enable PWM interrupt(s) */
  PWM_INT_ENABLE,
  /* disable PWM interrupt(s) */
  PWM_INT_DISABLE,
  /* read PWM interrupt flag(s) */
  PWM_INT_FLAGS_READ,
  /* clear PWM interrupt flag(s) */
  PWM_INT_FLAGS_CLEAR,
  /* read capture0 register */
  PWM_CAPTURE_READ,
  /* disable PCAP interrupts */
  PWM_CAPTURE_INT_DISABLE,
  /* enable PCAP interrupts */
  PWM_CAPTURE_INT_ENABLE
} PWM_IOCTL_CMD_T;

/* PWM clock source options */
typedef enum
{
  PWM_CLOCK_PERIPHERAL = 0,           /* no update needed */
  PWM_CLOCK_PCAP_RISING_EDGE,         /* PCAP rising edge */
  PWM_CLOCK_PCAP_FALLING_EDGE,        /* PCAP rising edge */
  PWM_CLOCK_PCAP_ANY_EDGE             /* PCAP rising edge */
} PWM_CLOCK_SOURCE_OPT;

/* PWM update control options */
typedef enum
{
  PWM_NO_UPDATE,  /* no update needed */
  PWM_UPDATE,     /* update after the next reset requested */
  PWM_UPDATE_NOW  /* immediate update requested => trigger a reset */
} PWM_IOCTL_UPDATE_OPT;

/* PWM timer control options */
typedef enum
{
  PWM_TIMER_STOP,     /* stop the timer */
  PWM_TIMER_RESET,    /* reset the timer = stop + reset */
  PWM_TIMER_SYNC,     /* synchronize the timer (PWM3 only) */
  PWM_TIMER_RESTART,  /* restart the timer = stop + reset + go */
  PWM_TIMER_GO        /* let the timer run */
} PWM_IOCTL_TIMER_OPT;

/* PWM waveform duty-cycle control options */
typedef enum
{
  PWM_WAV_DUTY_ABS,   /* absolute register value */
  PWM_WAV_DUTY_PER    /* duty-cycle in % of the PWM period */
} PWM_IOCTL_DUTY_OPT;

/* PWM waveform offset control options */
typedef enum
{
  PWM_WAV_OFFSET_ABS, /* absolute register value */
  PWM_WAV_OFFSET_PER  /* offset % of the PWM period */
} PWM_IOCTL_OFFSET_OPT;

/* PWM channel update/latch options */
typedef enum
{
  PWM_PERIOD_UPDATE = 1,
  PWM_CH1_UPDATE = 2,
  PWM_CH2_UPDATE = 4,
  PWM_CH3_UPDATE = 8,
  PWM_CH4_UPDATE = 16,
  PWM_CH5_UPDATE = 32,
  PWM_CH6_UPDATE = 64,
} PWM_CHANNELS_UPDATE_OPT;

/* PWM operating mode options */
typedef enum
{
  PWM_SINGLE_EDGE,
  PWM_DUAL_EDGE
} PWM_MODE_OPT;

/* PWM operating mode options */
typedef enum
{
  PWM_CAPTURE_INT_RISING = 0,
  PWM_CAPTURE_INT_FALLING,
  PWM_CAPTURE_INT_ANY
} PWM_CAPTURE_INT_ENABLE_OPT;

/* structure containing PWM period arguments */
typedef struct
{
  PWM_CLOCK_SOURCE_OPT clock;
  UNS_32 prescale;
  UNS_32 period;
  PWM_IOCTL_UPDATE_OPT update;
} PWM_SYSTEM_SETUP_T;

/* structure containing PWM period arguments */
typedef struct
{
  UNS_32 period;
  PWM_IOCTL_UPDATE_OPT update;
} PWM_SYSTEM_PERIOD_T;

/* structure containing PWM channel setup arguments */
typedef struct
{
  UNS_32 channel;
  PWM_MODE_OPT mode;
  PWM_IOCTL_DUTY_OPT duty_option;
  UNS_32 duty;
  PWM_IOCTL_OFFSET_OPT offset_option;
  UNS_32 offset;
  UNS_32 ini_state;
} PWM_CHANNEL_SETUP_T;

/* structure containing PWM update arguments */
typedef struct
{
  PWM_CHANNELS_UPDATE_OPT channels;
  PWM_IOCTL_UPDATE_OPT update;
} PWM_UPDATE_CONTROL_T;

/***********************************************************************
 * SPWM driver API functions
 **********************************************************************/

/* Open a PWM */
INT_32 pwm_open(void *ipbase, INT_32 arg);

/* Close the PWM */
STATUS pwm_close(INT_32 devid);

/* PWM configuration block */
STATUS pwm_ioctl(INT_32 devid,
                 INT_32 cmd,
                 INT_32 arg);

/* PWM read function (stub only) */
INT_32 pwm_read(INT_32 devid,
                void *buffer,
                INT_32 max_bytes);

/* PWM write function (stub only) */
INT_32 pwm_write(INT_32 devid,
                 void *buffer,
                 INT_32 n_bytes);

/***********************************************************************
 * Other PWM driver functions
 **********************************************************************/

#ifdef __cplusplus
}
#endif

#endif /* LPC32XX_PWM_DRIVER_H */
