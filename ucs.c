/*
 * UCS.c
 *
 *  Created on: Jan 21, 2020
 *      Author: a0222913
 */

//#include <msp430.h>
#include "ucs.h"
#include <stdint.h>
#include <stdbool.h>
//#include "inc/hw_types.h"
//#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "inc/hw_memmap.h"
#include "driverlib/timer.h"

void ucsInitial()
{
    //ucs setting
    SysCtlClockSet(SYSCTL_SYSDIV_2_5 | SYSCTL_USE_PLL | SYSCTL_XTAL_16MHZ | SYSCTL_OSC_MAIN);
    //Reloj configurado a 80MHz con cristal de 16

    //Output ACLK
   //P1DIR |= BIT0;
   //P1SEL |= BIT0;
   //Output SMCLK
   //P2DIR |= BIT2;
   //P2SEL |= BIT2;
}

void timerInitial()
{
    SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
    TimerConfigure(TIMER0_BASE, TIMER_CFG_PERIODIC);
    TimerLoadSet(TIMER0_BASE, TIMER_A, SysCtlClockGet() / 1000); // 1 ms
    TimerIntEnable(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
    TimerEnable(TIMER0_BASE, TIMER_A);
}

void timer_IntClear_user()
{
    TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}


