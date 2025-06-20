//*****************************************************************************
//
// project0.c - Example to demonstrate minimal TivaWare setup
//
// Copyright (c) 2012-2020 Texas Instruments Incorporated.  All rights reserved.
// Software License Agreement
// 
// Texas Instruments (TI) is supplying this software for use solely and
// exclusively on TI's microcontroller products. The software is owned by
// TI and/or its suppliers, and is protected under applicable copyright
// laws. You may not combine this software with "viral" open-source
// software in order to form a larger program.
// 
// THIS SOFTWARE IS PROVIDED "AS IS" AND WITH ALL FAULTS.
// NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT
// NOT LIMITED TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. TI SHALL NOT, UNDER ANY
// CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR CONSEQUENTIAL
// DAMAGES, FOR ANY REASON WHATSOEVER.
// 
// This is part of revision 2.2.0.295 of the EK-TM4C123GXL Firmware Package.
//
//*****************************************************************************

#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_types.h"
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
////bibliotecas adicionales para proyecto
#include "inc/hw_ints.h"
#include "driverlib/interrupt.h"
///////mis bibliotecas
#include "ucs.h"
#include "spi.h"
#include "frames.h"
#include "TLC698x_LP589x_APIs.h"

    //
    // Setup the system clock to run at 50 Mhz from PLL with crystal reference
    //
    //SysCtlClockSet(SYSCTL_SYSDIV_4|SYSCTL_USE_PLL|SYSCTL_XTAL_16MHZ|
                    //SYSCTL_OSC_MAIN);

    //
    // Enable and wait for the port to be ready for access
    //
//

//*****************************************************************************
//
// Define pin to LED color mapping.
//
//*****************************************************************************

//*****************************************************************************
//
//! \addtogroup example_list
//! <h1>Project Zero (project0)</h1>
//!
//! This example demonstrates the use of TivaWare to setup the clocks and
//! toggle GPIO pins to make the LED's blink. This is a good place to start
//! understanding your launchpad and the tools that can be used to program it.
//
//*****************************************************************************

#define RED_LED   GPIO_PIN_1
#define BLUE_LED  GPIO_PIN_2
#define GREEN_LED GPIO_PIN_3

//*****************************************************************************
//
// The error routine that is called if the driver library encounters an error.
//
//*****************************************************************************
#ifdef DEBUG
void
__error__(char *pcFilename, uint32_t ui32Line)
{
}
#endif

// Globals
//
uint16_t cpuTimer0IntCount = 0;

void Device_init(){
    //WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer
    // 1. Configurar Watchdog Timer (deshabilitarlo)
    SysCtlPeripheralDisable(SYSCTL_PERIPH_WDOG0);  // Deshabilitar WDT
    // Habilitar interrupciones globales
    IntMasterEnable();
    ucsInitial(); //system clock initial
    timerInitial();
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA))
    {
    }
    spiInitial();
    dmaInitial();
    //*/

    // Set P1.0 and P4.7 for LED blinking
    //P1DIR |= BIT0;                  // configure P1.0 as output
    //P4DIR |= BIT7;                  // configure P4.7 as output
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);

    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOF))
    {
    }

    //
    // Configure the GPIO port for the LED operation.
    //
    //GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED);
    GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, RED_LED|GREEN_LED);

    //GPIOPinTypeGPIOOutput(GPIO_PORTF_BASE, BLUE_LED);

}
//*****************************************************************************
//

void fadeFrame(int fade_step, int compare_val, uint16_t bus){
    int y_idx = 0;
    int x_idx = 0;
    int x_loop_start = 0;
    int x_loop_idx = 0;
    int x_loop_end = 0;
    int y_loop_end = 0;
    int y_loop_length = 0;
    int x_loop_length = 0;
    int temp_val = 0;

    x_loop_start = 0;
    x_loop_end = SCREEN_SIZE_X;
    x_loop_length = x_loop_end - x_loop_start;
    y_idx = 0;
    y_loop_end = SCREEN_SIZE_Y;
    y_loop_length = y_loop_end - y_idx;

    while(y_loop_length > 0){
        x_idx = x_loop_start;
        x_loop_idx = x_loop_length;
        while(x_loop_idx > 0){
#if MONOCHROMATIC
            if(img_mono[y_idx][x_idx][bus] >= (uint16_t) compare_val) {
                temp_val = (int) img_mono[y_idx][x_idx][bus] + fade_step;
                if(temp_val < 0x00) {
                    img_mono[y_idx][x_idx][bus] = 0x00;
                }
                else {
                    if(temp_val > 0xFF) {
                        img_mono[y_idx][x_idx][bus] = 0xFF;
                    }
                    else {
                        img_mono[y_idx][x_idx][bus] = temp_val & 0xFF;
                    }
                }
            }
#else
            if(img_R_RGB[y_idx][x_idx][bus] >= (uint16_t) compare_val) {
                temp_val = (int) img_R_RGB[y_idx][x_idx][bus] + fade_step;
                if(temp_val < 0x00) {
                    img_R_RGB[y_idx][x_idx][bus] = 0x00;
                }
                else {
                    if(temp_val > 0xFF) {
                        img_R_RGB[y_idx][x_idx][bus] = 0xFF;
                    }
                    else {
                        img_R_RGB[y_idx][x_idx][bus] = temp_val & 0xFF;
                    }
                }
            }
            if(img_G_RGB[y_idx][x_idx][bus] >= (uint16_t) compare_val) {
                temp_val = (int) img_G_RGB[y_idx][x_idx][bus] + fade_step;
                if(temp_val < 0x00) {
                    img_G_RGB[y_idx][x_idx][bus] = 0x00;
                }
                else {
                    if(temp_val > 0xFF) {
                        img_G_RGB[y_idx][x_idx][bus] = 0xFF;
                    }
                    else {
                        img_G_RGB[y_idx][x_idx][bus] = temp_val & 0xFF;
                    }
                }
            }
            if(img_B_RGB[y_idx][x_idx][bus] >= (uint16_t) compare_val) {
                temp_val = (int) img_B_RGB[y_idx][x_idx][bus] + fade_step;
                if(temp_val < 0x00) {
                    img_B_RGB[y_idx][x_idx][bus] = 0x00;
                }
                else {
                    if(temp_val > 0xFF) {
                        img_B_RGB[y_idx][x_idx][bus] = 0xFF;
                    }
                    else {
                        img_B_RGB[y_idx][x_idx][bus] = temp_val & 0xFF;
                    }
                }
            }
#endif
            x_idx ++;
            x_loop_idx--;
        }
        y_idx ++;
        y_loop_length--;
    }
}
// Main 'C' Language entry point.  Toggle an LED using TivaWare.
//
//***********************************************************

//*****************************************************************************
int
main(void)
{

    // Initialize device clock and peripherals
    Device_init();
    //IntRegister(INT_TIMER0A, Timer0_ISR);
    
    //
    // Loop Forever
    //
    while(1)
    {
        //
        // Turn on the LED
        //
        //GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, RED_LED);
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|GREEN_LED, RED_LED|GREEN_LED);

        //
        // Delay for a bit
        //
        SysCtlDelay(2000000);

        //
        // Turn on the LED
        //
        //GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, BLUE_LED);
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|GREEN_LED, RED_LED);

        //
        // Delay for a bit
        //
        SysCtlDelay(2000000);
        //GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|BLUE_LED|GREEN_LED, GREEN_LED);
        GPIOPinWrite(GPIO_PORTF_BASE, RED_LED|GREEN_LED, GREEN_LED);

        //
        // Delay for a bit
        //
        SysCtlDelay(2000000);

    }
}
/*
void Timer0_ISR(void)
{
    // Tu c�digo aqu� (lo que deseas hacer en cada interrupci�n)
    // Por ejemplo, cambiar el estado de un LED o realizar alguna tarea.
    //vsyncDone = 1;
    timer_IntClear_user();
    //TimerIntClear(TIMER0_BASE, TIMER_TIMA_TIMEOUT);
}
*/
