/*
 * SPI.c
 *
 *  Created on: Jan 21, 2020
 *      Author: a0222913
 */

#include "spi.h"
#include <stdint.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/ssi.h"
/*
void spiInitial()
{
    // Habilitar el módulo SSI0 (equivalente al USCI en MSP430)
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // Configurar el SSI0 como maestro
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0, SSI_MODE_MASTER, 12000000, 8);

    // Habilitar el SSI0
    SSIEnable(SSI0_BASE);

    // Configurar los pines GPIO para SPI (SCK, MOSI, MISO)
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_5 | GPIO_PIN_4);
}
*/
void spiInitial(void)
{
    // 1. Habilitar periféricos
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);  // Puerto A para SSI0
    SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI0);

    // Esperar a que estén listos
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_GPIOA)) {}
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_SSI0)) {}

    // 2. Configurar pines SPI
    // PA2 - SSI0CLK (SCLK)
    // PA4 - SSI0RX (MISO/SOMI)
    // PA5 - SSI0TX (MOSI/SIMO)
    GPIOPinConfigure(GPIO_PA2_SSI0CLK);
    GPIOPinConfigure(GPIO_PA4_SSI0RX);
    GPIOPinConfigure(GPIO_PA5_SSI0TX);

    GPIOPinTypeSSI(GPIO_PORTA_BASE, GPIO_PIN_2 | GPIO_PIN_4 | GPIO_PIN_5);

    // 3. Configurar módulo SSI (equivalente a USCI en MSP430)
    SSIConfigSetExpClk(SSI0_BASE, SysCtlClockGet(),
                      SSI_FRF_MOTO_MODE_1,  // CPOL=1, CPHA=0 (Mode 1)
                      SSI_MODE_MASTER,
                      12000000,             // 12 MHz (igual que en MSP430)
                      8);                   // 8 bits

    // 4. Habilitar SSI
    SSIEnable(SSI0_BASE);
}

