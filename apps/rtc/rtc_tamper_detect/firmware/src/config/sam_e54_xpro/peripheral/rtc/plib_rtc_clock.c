/*******************************************************************************
  Real Time Counter (RTC) PLIB

  Company:
    Microchip Technology Inc.

  File Name:
    plib_rtc_clock.c

  Summary:
    RTC PLIB Implementation file

  Description:
    This file defines the interface to the RTC peripheral library. This
    library provides access to and control of the associated peripheral
    instance in clock/calendar mode.

*******************************************************************************/
// DOM-IGNORE-BEGIN
/*******************************************************************************
* Copyright (C) 2018 Microchip Technology Inc. and its subsidiaries.
*
* Subject to your compliance with these terms, you may use Microchip software
* and any derivatives exclusively with Microchip products. It is your
* responsibility to comply with third party license terms applicable to your
* use of third party software (including open source software) that may
* accompany Microchip software.
*
* THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
* EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
* WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
* PARTICULAR PURPOSE.
*
* IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
* INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
* WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
* BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
* FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
* ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
* THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.
*******************************************************************************/
// DOM-IGNORE-END

// *****************************************************************************
// *****************************************************************************
// Section: Included Files
// *****************************************************************************
// *****************************************************************************
/* This section lists the other files that are included in this file.
*/

#include "interrupts.h"
#include "plib_rtc.h"
#include <stdlib.h>


/* Reference Year */
#define REFERENCE_YEAR              (2016U)

/* Refernce Year in tm structure year (C standard) */
#define TM_STRUCT_REFERENCE_YEAR    (1900U)

/* Adjust user year with respect to tm structure year (C Standard) */
#define ADJUST_TM_YEAR(year)        (year + TM_STRUCT_REFERENCE_YEAR)

/* Adjust user month */
#define ADJUST_MONTH(month)         (month + 1U)

/* Adjust to tm structure month */
#define ADJUST_TM_STRUCT_MONTH(mon) (mon - 1U)

static RTC_OBJECT rtcObj;

void RTC_Initialize(void)
{
    RTC_REGS->MODE2.RTC_CTRLA |= (uint16_t)RTC_MODE2_CTRLA_SWRST_Msk;

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_SWRST_Msk) == RTC_MODE2_SYNCBUSY_SWRST_Msk)
    {
        /* Wait for synchronization after Software Reset */
    }

    RTC_REGS->MODE2.RTC_TAMPCTRL = (uint16_t)(RTC_TAMPCTRL_IN0ACT(0x2UL)  | RTC_TAMPCTRL_DEBNC0_Msk | \
                                   RTC_TAMPCTRL_IN1ACT(0x0UL)  | \
                                   RTC_TAMPCTRL_IN2ACT(0x0UL)  | \
                                   RTC_TAMPCTRL_IN3ACT(0x0UL)  | \
                                   RTC_TAMPCTRL_IN4ACT(0x0UL) );

    RTC_REGS->MODE2.RTC_CTRLA = (uint16_t)(RTC_MODE2_CTRLA_MODE(2UL) | RTC_MODE2_CTRLA_PRESCALER(0xBUL) | RTC_MODE2_CTRLA_CLOCKSYNC_Msk | RTC_MODE2_CTRLA_ENABLE_Msk | RTC_MODE2_CTRLA_BKTRST_Msk );


    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_ENABLE_Msk) == RTC_MODE2_SYNCBUSY_ENABLE_Msk)
    {
        /* Wait for Synchronization after Enabling RTC */
    }

    RTC_REGS->MODE2.RTC_INTENSET = 0x4000U;

}


bool RTC_RTCCTimeSet (struct tm * initialTime )
{
    /*
     * Add 1900 to the tm_year member and the adjust for the RTC reference year
     * Set YEAR(according to Reference Year), MONTH and DAY
     *set Hour Minute and Second
     */
    RTC_REGS->MODE2.RTC_CLOCK = (uint32_t)(((TM_STRUCT_REFERENCE_YEAR + (uint32_t)initialTime->tm_year) - REFERENCE_YEAR) << RTC_MODE2_CLOCK_YEAR_Pos |
                    ((ADJUST_MONTH((uint32_t)initialTime->tm_mon)) << RTC_MODE2_CLOCK_MONTH_Pos) |
                    ((uint32_t)initialTime->tm_mday << RTC_MODE2_CLOCK_DAY_Pos) |
                    ((uint32_t)initialTime->tm_hour << RTC_MODE2_CLOCK_HOUR_Pos) |
                    ((uint32_t)initialTime->tm_min << RTC_MODE2_CLOCK_MINUTE_Pos) |
                    ((uint32_t)initialTime->tm_sec << RTC_MODE2_CLOCK_SECOND_Pos));

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_CLOCK_Msk) == RTC_MODE2_SYNCBUSY_CLOCK_Msk)
    {
        /* Synchronization after writing value to CLOCK Register */
    }
    return true;
}

void RTC_RTCCTimeGet ( struct tm * currentTime )
{
    uint32_t dataClockCalendar = 0U;
    uint32_t timeMask = 0U;

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_CLOCKSYNC_Msk) == RTC_MODE2_SYNCBUSY_CLOCKSYNC_Msk)
    {
        /* Synchronization before reading value from CLOCK Register */
    }

    dataClockCalendar = RTC_REGS->MODE2.RTC_CLOCK;

    timeMask = (dataClockCalendar & RTC_MODE2_CLOCK_HOUR_Msk) >> RTC_MODE2_CLOCK_HOUR_Pos;
    currentTime->tm_hour = (int)timeMask;
    timeMask = (dataClockCalendar & RTC_MODE2_CLOCK_MINUTE_Msk) >> RTC_MODE2_CLOCK_MINUTE_Pos;
    currentTime->tm_min = (int)timeMask;
    timeMask = (dataClockCalendar & RTC_MODE2_CLOCK_SECOND_Msk) >> RTC_MODE2_CLOCK_SECOND_Pos;
    currentTime->tm_sec = (int)timeMask;

    timeMask = ADJUST_TM_STRUCT_MONTH(((dataClockCalendar & RTC_MODE2_CLOCK_MONTH_Msk) >> RTC_MODE2_CLOCK_MONTH_Pos));
    currentTime->tm_mon  = (int)timeMask; 
    timeMask = (((dataClockCalendar & RTC_MODE2_CLOCK_YEAR_Msk)>> RTC_MODE2_CLOCK_YEAR_Pos) + REFERENCE_YEAR) - TM_STRUCT_REFERENCE_YEAR;
    currentTime->tm_year = (int)timeMask; 
    timeMask = (dataClockCalendar & RTC_MODE2_CLOCK_DAY_Msk) >> RTC_MODE2_CLOCK_DAY_Pos;
    currentTime->tm_mday = (int)timeMask; 
}
void RTC_BackupRegisterSet( BACKUP_REGISTER reg, uint32_t value )
{
    RTC_REGS->MODE2.RTC_BKUP[reg] = value;
}

uint32_t RTC_BackupRegisterGet( BACKUP_REGISTER reg )
{
    return(RTC_REGS->MODE2.RTC_BKUP[reg]);
}
 TAMPER_CHANNEL RTC_TamperSourceGet( void )
{
    return((TAMPER_CHANNEL) (RTC_REGS->MODE2.RTC_TAMPID) & (0xFFU));
}

void RTC_RTCCTimeStampGet(  struct tm * timeStamp  )
{
   uint32_t dataClockCalendar = 0U;
   uint32_t timeMask = 0U;

   dataClockCalendar = RTC_REGS->MODE2.RTC_TIMESTAMP;
   timeMask = (dataClockCalendar & RTC_MODE2_TIMESTAMP_HOUR_Msk) >> RTC_MODE2_TIMESTAMP_HOUR_Pos;
   timeStamp->tm_hour = (int)timeMask;
   timeMask = (dataClockCalendar & RTC_MODE2_TIMESTAMP_MINUTE_Msk) >> RTC_MODE2_TIMESTAMP_MINUTE_Pos;
   timeStamp->tm_min = (int)timeMask;
   timeMask = (dataClockCalendar & RTC_MODE2_TIMESTAMP_SECOND_Msk) >> RTC_MODE2_TIMESTAMP_SECOND_Pos;
   timeStamp->tm_sec = (int)timeMask;

   timeMask = ADJUST_TM_STRUCT_MONTH(((dataClockCalendar & RTC_MODE2_TIMESTAMP_MONTH_Msk) >> RTC_MODE2_TIMESTAMP_MONTH_Pos));
   timeStamp->tm_mon  = (int)timeMask;
   timeMask = (((dataClockCalendar & RTC_MODE2_TIMESTAMP_YEAR_Msk)>> RTC_MODE2_TIMESTAMP_YEAR_Pos) + REFERENCE_YEAR) - TM_STRUCT_REFERENCE_YEAR;
   timeStamp->tm_year = (int)timeMask; 
   timeMask = (dataClockCalendar & RTC_MODE2_TIMESTAMP_DAY_Msk) >> RTC_MODE2_TIMESTAMP_DAY_Pos;
   timeStamp->tm_mday = (int)timeMask; 
}

bool RTC_RTCCAlarm0Set (struct tm * alarmTime, RTC_ALARM_MASK mask)
{
    /*
     * Add 1900 to the tm_year member and the adjust for the RTC reference year
     * Set YEAR(according to Reference Year), MONTH and DAY
     * Set Hour, Minute and second
     */
    RTC_REGS->MODE2.RTC_ALARM0 = (uint32_t)(((TM_STRUCT_REFERENCE_YEAR + (uint32_t)alarmTime->tm_year) - REFERENCE_YEAR) << RTC_MODE2_CLOCK_YEAR_Pos |
                    (ADJUST_MONTH((uint32_t)alarmTime->tm_mon) << RTC_MODE2_CLOCK_MONTH_Pos) |
                    ((uint32_t)alarmTime->tm_mday << RTC_MODE2_CLOCK_DAY_Pos) |
                    ((uint32_t)alarmTime->tm_hour << RTC_MODE2_CLOCK_HOUR_Pos) |
                    ((uint32_t)alarmTime->tm_min << RTC_MODE2_CLOCK_MINUTE_Pos) |
                    ((uint32_t)alarmTime->tm_sec << RTC_MODE2_CLOCK_SECOND_Pos));

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_ALARM0_Msk) == RTC_MODE2_SYNCBUSY_ALARM0_Msk)
    {
        /* Synchronization after writing to ALARM register */
    }

    RTC_REGS->MODE2.RTC_MASK0 = (uint8_t)mask;

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_MASK0_Msk) == RTC_MODE2_SYNCBUSY_MASK0_Msk)
    {
        /* Synchronization after writing value to MASK Register */
    }

    RTC_REGS->MODE2.RTC_INTENSET = (uint16_t)RTC_MODE2_INTENSET_ALARM0_Msk;

    return true;
}
bool RTC_RTCCAlarm1Set (struct tm * alarmTime, RTC_ALARM_MASK mask)
{
    /*
     * Add 1900 to the tm_year member and the adjust for the RTC reference year
     * Set YEAR(according to Reference Year), MONTH and DAY
     * Set Hour, Minute and second
     */
    RTC_REGS->MODE2.RTC_ALARM1 = (uint32_t)(((TM_STRUCT_REFERENCE_YEAR + (uint32_t)alarmTime->tm_year) - REFERENCE_YEAR) << RTC_MODE2_CLOCK_YEAR_Pos |
                    (ADJUST_MONTH((uint32_t)alarmTime->tm_mon) << RTC_MODE2_CLOCK_MONTH_Pos) |
                    ((uint32_t)alarmTime->tm_mday << RTC_MODE2_CLOCK_DAY_Pos) |
                    ((uint32_t)alarmTime->tm_hour << RTC_MODE2_CLOCK_HOUR_Pos) |
                    ((uint32_t)alarmTime->tm_min << RTC_MODE2_CLOCK_MINUTE_Pos) |
                    ((uint32_t)alarmTime->tm_sec << RTC_MODE2_CLOCK_SECOND_Pos));

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_ALARM1_Msk) == RTC_MODE2_SYNCBUSY_ALARM1_Msk)
    {
        /* Synchronization after writing to ALARM register */
    }

    RTC_REGS->MODE2.RTC_MASK1 = (uint8_t)mask;

    while((RTC_REGS->MODE2.RTC_SYNCBUSY & RTC_MODE2_SYNCBUSY_MASK1_Msk) == RTC_MODE2_SYNCBUSY_MASK1_Msk)
    {
        /* Synchronization after writing value to MASK Register */
    }

    RTC_REGS->MODE2.RTC_INTENSET = (uint16_t)RTC_MODE2_INTENSET_ALARM1_Msk;

    return true;
}

void RTC_RTCCCallbackRegister ( RTC_CALLBACK callback, uintptr_t context)
{
    rtcObj.alarmCallback = callback;
    rtcObj.context       = context;
}

void RTC_InterruptHandler(void)
{
    rtcObj.intCause = (RTC_CLOCK_INT_MASK) RTC_REGS->MODE2.RTC_INTFLAG;

    /* Clear All Interrupts */
    RTC_REGS->MODE2.RTC_INTFLAG = (uint16_t)RTC_MODE2_INTFLAG_Msk;

    if(rtcObj.alarmCallback != NULL)
    {
        rtcObj.alarmCallback(rtcObj.intCause, rtcObj.context);
    }
}
