/*
 * LabGPIO_0.cpp
 *
 *  Created on: Feb 15, 2018
 *      Author: Sucheta
 */

#include "LPC17xx.h"
#include "LabGPIO.hpp"
/*Constructor
 *
 */
LabGPIO::LabGPIO(uint8_t port_number, uint8_t pin_number)
{
    /*Set all the private variables
     *
     */
    _port_number = port_number;
    _pin_number = pin_number;

    switch(port_number)
    {
        case 0: GPIO_register_map = (LPC_GPIO_TypeDef*)LPC_GPIO0_BASE; //Reference to the GPIO Port 0
                break;
        case 1: GPIO_register_map = (LPC_GPIO_TypeDef*)LPC_GPIO1_BASE; //Reference to the GPIO Port 1
                break;
        case 2: GPIO_register_map = (LPC_GPIO_TypeDef*)LPC_GPIO2_BASE; //Reference to the GPIO Port 2
                break;
        case 3: GPIO_register_map = (LPC_GPIO_TypeDef*)LPC_GPIO3_BASE; //Reference to the GPIO Port 3
                break;
        case 4: GPIO_register_map = (LPC_GPIO_TypeDef*)LPC_GPIO4_BASE; //Reference to the GPIO Port 4
                break;
        default:GPIO_register_map = (LPC_GPIO_TypeDef*)LPC_GPIO0_BASE; //Should never come here
                break;
    }

    /*Setting the port_number.pin_number as GPIO pin
     *
     */

    volatile uint32_t *base_address_of_PINSEL = nullptr;
    if (pin_number <=15 && pin_number >=0)
    {
        switch(port_number)
        {
            case 0: base_address_of_PINSEL = &(LPC_PINCON->PINSEL0); break;
            case 1: base_address_of_PINSEL = &(LPC_PINCON->PINSEL2); break;
            case 2: base_address_of_PINSEL = &(LPC_PINCON->PINSEL4); break;
            case 3: base_address_of_PINSEL = &(LPC_PINCON->PINSEL6); break;
            case 4: base_address_of_PINSEL = &(LPC_PINCON->PINSEL8); break;
        }
        *base_address_of_PINSEL &= ~(1 << (2 * pin_number));
        *base_address_of_PINSEL &= ~(1 << ((2*pin_number)+1));
    }
    if (pin_number >=15 && pin_number <=31)
    {
        pin_number = pin_number - 31;
        switch(port_number)
        {
            case 0: base_address_of_PINSEL = &(LPC_PINCON->PINSEL1); break;
            case 1: base_address_of_PINSEL = &(LPC_PINCON->PINSEL3); break;
            case 2: base_address_of_PINSEL = &(LPC_PINCON->PINSEL5); break;
            case 3: base_address_of_PINSEL = &(LPC_PINCON->PINSEL7); break;
            case 4: base_address_of_PINSEL = &(LPC_PINCON->PINSEL9); break;
        }
        *base_address_of_PINSEL &= ~(1 << (2*pin_number));
        *base_address_of_PINSEL &= ~(1 << ((2*pin_number)+1));
    }
}

/*Destructor
 *
 */
LabGPIO::~LabGPIO()
{
    setAsOutput();   //Always safe to let the pin be set as output/
}


void LabGPIO::setAsInput()
{
    /*Set the pin as an input pin
     *
     */
    GPIO_register_map->FIODIR &= ~(1 << _pin_number);
}

void LabGPIO::setAsOutput()
{
    /*Set the pin as output pin
     *
     */
    GPIO_register_map->FIODIR |= (1 << _pin_number);
}

void LabGPIO::setDirection(bool output)
{
    /*output true(1)? set as output
     * output false(0)? set as input
     */
    if (output == false)
    {
        setAsInput();
    }
    else
    {
        setAsOutput();
    }
}

void LabGPIO::setHigh()
{
    GPIO_register_map->FIOSET = 1 << _pin_number;
}

void LabGPIO::setLow()
{
    GPIO_register_map->FIOCLR = 1 << _pin_number;
}

void LabGPIO::set(bool high)
{
    /**
     * If high = true(1), set pin as high
     * If high = false(0), set pin as low
     */
    if (high == true)
    {
        setHigh();
    }
    else
    {
        setLow();
    }
}

bool LabGPIO::getLevel()
{
    return (GPIO_register_map->FIOPIN & 1<<_pin_number);
}
