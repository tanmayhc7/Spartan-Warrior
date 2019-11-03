/*
 * LabSPI.cpp
 *
 *  Created on: Feb 25, 2018
 *      Author: Sneha
 */
#include "LabSPI.hpp"
#include "LPC17xx.h"
#include "tasks.hpp"
#include "semphr.h"
const LPC_SSP_TypeDef * LabSPI::SSP[2] = { LPC_SSP0, LPC_SSP1 };
SemaphoreHandle_t LabSPI::spi_mutex = 0;
/**
 * 1) Powers on SPPn peripheral
 * 2) Set peripheral clock
 * 3) Sets pins for specified peripheral to MOSI, MISO, and SCK
 *
 * @param peripheral which peripheral SSP0 or SSP1 you want to select.
 * @param data_size_select transfer size data width; To optimize the code, look for a pattern in the datasheet
 * @param format is the code format for which synchronous serial protocol you want to use.
 * @param divide is the how much to divide the clock for SSP; take care of error cases such as the value of 0, 1, and odd numbers
 *
 * @return true if initialization was successful
 */
bool LabSPI::init(Peripheral peripheral, uint8_t data_size_select, FrameModes format, uint8_t divide)
{
    spi_mutex = xSemaphoreCreateMutex();
    if (peripheral == SSP1) {
        //power on SSP1 using the PCONP register
        LPC_SC->PCONP |= (1 << 10); //setting bit 10 of PCONP register as 1 to enable SSP1

        //select the clock for SSP1 using PCLKSEL0 register
        //setting bits 20 and 21 in PCLKSEL0 register to 0x01 to select PCLK/1
        //this will be scaled down by the prescale register
        LPC_SC->PCLKSEL0 &= ~(0x3 << 20); //set bit 20 and 21 to 0
        LPC_SC->PCLKSEL0 |= (1 << 20);  //set bit 20 and 21 to 0x01 to keep it at PCLK

        //enabling SSP1
        LPC_PINCON->PINSEL0 &= ~(0x3F << 14);
        LPC_PINCON->PINSEL0 |= (0x2A << 14); //setting bits 15,17 and 19

        //setting the prescale register for SSP1
        //making sure that 'divide' is a value between 2 and 254
        if (divide == 0 || divide == 1) divide = 2;
        if (divide % 2 != 0) divide -= 1;

        LPC_SSP1->CPSR |= divide; //setting the value of divide in the CPSR

        //setting the format in SSP1CR0
        LPC_SSP1->CR0 &= ~(3 << 4); //setting bits 4 and 5 to 0
        LPC_SSP1->CR0 |= (format << 4); //setting the format

        //enabling 8 bit data mode
        LPC_SSP1->CR0 &= ~(0xFF);
        LPC_SSP1->CR0 |= (data_size_select - 1);
        //enabling the SSP controller
        LPC_SSP1->CR1 &= ~(0xF);
        LPC_SSP1->CR1 |= (1 << 1);

        return true;
    }
    else if (peripheral == SSP0) {
        //power on SSP0 using the PCONP register
        LPC_SC->PCONP |= (1 << 21); //setting bit 10 of PCONP register as 1 to enable SSP0

        //select the clock for SSP0 using PCLKSEL1 register
        //setting bits 10 and 11 in PCLKSEL0 register to 0x01 to select PCLK/1
        //this will be scaled down by the prescale register
        LPC_SC->PCLKSEL1 &= ~(0x3 << 10); //set bit 10 and 11 to 0
        LPC_SC->PCLKSEL1 |= (1 << 10);  //set bit 10 and 11 to 0x01 to keep it at PCLK

        //enabling SSP0
        LPC_PINCON->PINSEL0 &= ~(0x3 << 30); //clearing bits 31 and 30
        LPC_PINCON->PINSEL0 |= (0x2 << 30); //setting bits 31,30 to 10
        LPC_PINCON->PINSEL1 &= ~(0xF << 2); //clearing bits 31 and 30
        LPC_PINCON->PINSEL1 |= (0xA << 2); //setting bits 3 and 5

        //setting the prescale register for SSP1
        //making sure that 'divide' is a value between 2 and 254
        if (divide == 0 || divide == 1) divide = 2;
        if (divide % 2 != 0) divide -= 1;

        LPC_SSP0->CPSR |= divide; //setting the value of divide in the CPSR

        //setting the format in SSP1CR0
        LPC_SSP0->CR0 &= ~(3 << 4); //setting bits 4 and 5 to 0
        LPC_SSP0->CR0 |= (format << 4); //setting the format

        //enabling 8 bit data mode
        LPC_SSP0->CR0 &= ~(0xFF);
        LPC_SSP0->CR0 |= (data_size_select - 1);
        //enabling the SSP controller
        LPC_SSP0->CR1 &= ~(0xF);
        LPC_SSP0->CR1 |= (1 << 1);

        return true;
    }
    return false;
}

/**
 * Transfers a byte via SSP to an external device using the SSP data register.
 * This region must be protected by a mutex static to this class.
 *
 * @return received byte from external device via SSP data register.
 */
uint8_t LabSPI::transfer(uint8_t to_send)
{

    LPC_SSP1->DR = to_send;
    while (LPC_SSP1->SR & (1 << 4)); // Wait if the SPI is transmitting/receiving for it to be idle

    return LPC_SSP1->DR;

}
