/*
 * dma.c
 *
 *  Created on: Jun 24, 2019
 *      Author: a0222913
 */
#include <stdint.h>
#include "dma.h"
#include "frames.h"
#include "led_driver.h"
////////
#include <stdbool.h>
#include "inc/hw_memmap.h"
//#include "driverlib/pin_map.h"
#include "driverlib/sysctl.h"
#include "driverlib/udma.h"
#include "driverlib/ssi.h"
/*
 *
#include "spi.h"
#include "driverlib/gpio.h"
 */

uint16_t ledXmtBuffer[CCSI_BUS_NUM][MAX_DATA_LENGTH]; // Stores the data bytes to be sent
uint16_t ledRcvBuffer[CCSI_BUS_NUM][MAX_DATA_LENGTH]; // Processed received bytes
uint16_t RxRingBuff[CCSI_BUS_NUM][MAX_DATA_LENGTH]; //Stores the data bytes to be received

// Tabla de control DMA (debe estar alineada a 1024 bytes)
#if defined(__TI_ARM__)
#pragma DATA_ALIGN(udmaControlTable, 1024)
#else
__attribute__ ((aligned(1024)))
#endif
static tDMAControlTable udmaControlTable[64];

unsigned int firstTransfer = TRUE;


void dmaInitial() //DMA0 -> Tx, DMA1 -> Rx
{

    // Configuración del canal DMA0 para transmisión SPI
    //uDMAEnable();                                   // Habilitar el módulo DMA

    // Deshabilitar el canal SSI0
    //SSIDisable(SSI0_BASE);
    //SSIEnable(SSI0_BASE);

    // Configurar la dirección de origen (ledXmtBuffer)
    //uDMASrcEndAddrSet(DMA_CHANNEL_0, (uint32_t)&ledXmtBuffer[0]);

    /*
    // Configuración del canal DMA0 para transmisión SPI
    SYSCTL_RCGCDMA |= 0x01;  // Habilitar el módulo DMA0
    DMA_ENA = 0;             // Deshabilitar el canal DMA0

    // Configurar la dirección de origen (ledXmtBuffer)
    DMA_SRCENDP(DMA_CHANNEL_0) = (uint32_t)&ledXmtBuffer[0];

    // Configurar la dirección de destino (SPI TX FIFO)
    DMA_DSTENDP(DMA_CHANNEL_0) = (uint32_t)&SSI0_DR;  // Ajusta según la interfaz SPI que estés utilizando

    // Opciones de control (incremento de dirección, etc.)
    DMA_CTL(DMA_CHANNEL_0) = DMA_CTL_SRC_INC | DMA_CTL_DST_INC | DMA_CTL_MODE_BASIC;

    // Habilitar el canal DMA0
    DMA_ENA = 1;
    */
    /////////////ddddd
    /*
    DMACTL0 = DMA1TSEL_18+DMA0TSEL_19;        // DMA0 - UCB0TXIFG
                                               // DMA1 - UCB0RXIFG
     // Setup DMA0
     __data20_write_long((uintptr_t) &DMA0SA,(uintptr_t) &(ledXmtBuffer[0]));
                                               // Source block address
     __data20_write_long((uintptr_t) &DMA0DA,(uintptr_t) &UCB0TXBUF);
                                               // Destination single address

     DMA0CTL |= DMASRCINCR_3 + DMASBDB + DMALEVEL;  // increase source address, source byte to destination byte, level sensitive

     // Setup DMA1
     __data20_write_long((uintptr_t) &DMA1SA,(uintptr_t) &UCB0RXBUF);
                                               // Source single address
     __data20_write_long((uintptr_t) &DMA1DA,(uintptr_t) &(RxRingBuff[0]));
                                               // Destination block address

                                                    // Destination block address
     DMA1CTL |= DMADSTINCR_3 + DMASBDB + DMALEVEL;//;  // increase destination address, source byte to destination byte, level sensitive*/
    // 1. Habilitar periférico uDMA
    SysCtlPeripheralEnable(SYSCTL_PERIPH_UDMA);
    while(!SysCtlPeripheralReady(SYSCTL_PERIPH_UDMA)) {}

    // 2. Habilitar el controlador DMA
    uDMAEnable();

    // 3. Configurar estructura de control DMA para TX (SPI)
    uDMAControlBaseSet(udmaControlTable);

    // 4. Configurar canal de transmisión (SPI TX)
    uDMAChannelAssign(UDMA_CH11_SSI0TX);  // Canal 11 para SSI0 TX

    // 5. Configurar canal de recepción (SPI RX)
    uDMAChannelAssign(UDMA_CH10_SSI0RX);  // Canal 10 para SSI0 RX

    // 6. Configurar estructuras de transferencia
    // Configuración para TX (modo básico)
    uDMAChannelControlSet(UDMA_CH11_SSI0TX | UDMA_PRI_SELECT,
                            UDMA_SIZE_8 | UDMA_SRC_INC_8 | UDMA_DST_INC_NONE | UDMA_ARB_4);
    // Configuración para RX (modo básico)
    uDMAChannelControlSet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT,
                            UDMA_SIZE_8 | UDMA_SRC_INC_NONE | UDMA_DST_INC_8 | UDMA_ARB_4);
}

void dmaTransfer(uint8_t bus_index, uint16_t dmaTxSize, uint16_t dmaRxSize, bool checkResponse)
{
    // 1. Configurar transferencia de TX (16 bits)
    uDMAChannelTransferSet(UDMA_CH11_SSI0TX | UDMA_PRI_SELECT,
                         UDMA_MODE_BASIC,
                         ledXmtBuffer[bus_index],      // Origen específico del bus
                         (void*)(SSI0_BASE + 0x008),  // Destino (SSI0_DR)
                         dmaTxSize);                   // Número de elementos uint16_t

    // Configurar control para TX (16 bits)
    uDMAChannelControlSet(UDMA_CH11_SSI0TX | UDMA_PRI_SELECT,
                         UDMA_SIZE_16 | UDMA_SRC_INC_16 |
                         UDMA_DST_INC_NONE | UDMA_ARB_4);

    if(checkResponse)
    {
        // 2. Configurar transferencia de RX (8 bits)
        uDMAChannelTransferSet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT,
                             UDMA_MODE_BASIC,
                             (void*)(SSI0_BASE + 0x008),  // Origen (SSI0_DR)
                             RxRingBuff[bus_index],       // Destino específico del bus
                             dmaRxSize);                  // Número de bytes a recibir

        // Configurar control para RX (8 bits)
        uDMAChannelControlSet(UDMA_CH10_SSI0RX | UDMA_PRI_SELECT,
                             UDMA_SIZE_8 | UDMA_SRC_INC_NONE |
                             UDMA_DST_INC_8 | UDMA_ARB_4);

        // 3. Habilitar ambos canales
        uDMAChannelEnable(UDMA_CH11_SSI0TX); // TX primero
        uDMAChannelEnable(UDMA_CH10_SSI0RX); // Luego RX

        // 4. Esperar completar ambas transferencias
        while(uDMAChannelIsEnabled(UDMA_CH11_SSI0TX) ||
              uDMAChannelIsEnabled(UDMA_CH10_SSI0RX));
    }
    else
    {
        // Solo transferencia de TX
        uDMAChannelEnable(UDMA_CH11_SSI0TX);
        while(uDMAChannelIsEnabled(UDMA_CH11_SSI0TX));
    }
}
/*
void dmaTransfer(unsigned int dmaTxSize, unsigned int dmaRxSize, unsigned int checkResponse)
{
    /*
    uint16_t idx = 0;
    uint16_t startLoop = 0;
    uint16_t endLoop = 0;
    volatile uint16_t timer_val = 0;
    volatile uint16_t dma0ctl_val = 0;

    if(firstTransfer == FALSE){
        while(!(DMA0CTL&DMAIFG)); //Wait until send all command bytes
        DMA0CTL &= ~(DMAEN + DMAIFG);//disable DMA0
    }
    else
        firstTransfer = FALSE;
    
    if(checkResponse==TRUE) {
        // Ensure that rest of transmit data is all 0xFFFF
        if(dmaTxSize < dmaRxSize) {
            startLoop = (dmaTxSize >> 1) + (dmaTxSize & 0x1);
            endLoop = (dmaRxSize >> 1) + (dmaRxSize & 0x1);
            for(idx = startLoop; idx <= endLoop; idx++){
                ledXmtBuffer[0][idx] = 0xFFFF;
            }
        }
        DMA0SZ = dmaRxSize;
        DMA1SZ = dmaRxSize;
    }
    else {
        DMA0SZ = dmaTxSize;
    }

    dma0ctl_val = DMA0CTL;
    dma0ctl_val |= DMAEN;
    timer_val = TA2R;

    while(timer_val > 0){
        timer_val--;
    }

#ifdef LP589X
    __no_operation();
#endif

    DMA0CTL = dma0ctl_val;

    if(checkResponse==TRUE)
    {
        DMA1CTL |= DMAEN; //enable Rx DMA
        while(!(DMA0CTL&DMAIFG)); //Wait until send all command bytes
        while(!(DMA1CTL&DMAIFG)); //Wait until receive all response byte
        DMA1CTL &= ~(DMAEN + DMAIFG); //disable DMA1
        DMA0CTL &= ~(DMAEN); //disable DMA0
    }

}

*/
