///*
// * LabGPIOInterrupts.cpp
// *
// *  Created on: Feb 19, 2018
// *      Author: Sucheta
// */
//
//#include "LabGPIOInterrupts.hpp"
//
//
//
//
//

#include "LabGPIOInterrupts.hpp"
#include "LPC17xx.h"
#include "lpc_isr.h"
#include "printf_lib.h"
/**
 * LPC datasheet indicates that all of Port0 and Port2 interrupts use
 * a single interrupt vector called EINT3
 */
void_func_t Port0_InterruptCB_Rising[31]; //RISING interrupt CBs for Port 0.0 to 0.31
void_func_t Port0_InterruptCB_Falling[31]; //FALLING interrupt CBs for Port 0.0 to 0.31
void_func_t Port2_InterruptCB_Rising[14]; //RISING interrupt CBs for Port 2.0 to 2.13
void_func_t Port2_InterruptCB_Falling[14]; //FALLING interrupt CBs for Port 2.0 to 2.13

/**
 * This handler should place a function pointer within the lookup table for the externalIRQHandler to find.
 *
 * @param[in] port         specify the GPIO port
 * @param[in] pin          specify the GPIO pin to assign an ISR to
 * @param[in] pin_isr      function to run when the interrupt event occurs
 * @param[in] condition    condition for the interrupt to occur on. RISING, FALLING or BOTH edges.
 * @return should return true if valid ports, pins, isrs were supplied and pin isr insertion was sucessful
 */
bool LabGPIOInterrupts::attachInterruptHandler(uint8_t port, uint32_t pin, void (*pin_isr)(void), InterruptCondition_E condition)
{
    if (port == 0 && (pin >= 0 && pin <= 30) && (condition == risingEdge)) {
        u0_dbg_printf("%d_%dR\n",port,pin);
        Port0_InterruptCB_Rising[pin] = pin_isr; //save the ISR for that pin
        LPC_GPIOINT->IO0IntEnR |= (1 << pin); //enable the rising edge intr for that pin
        return true;
    }

    else if (port == 0 && (pin >= 0 && pin <= 30) && (condition == fallingEdge)) {
        u0_dbg_printf("%d_%dF\n",port,pin);
        Port0_InterruptCB_Falling[pin] = pin_isr;
        LPC_GPIOINT->IO0IntEnF |= (1 << pin); //enable the falling edge intr for that pin
        return true;
    }
    else if (port == 0 && (pin >= 0 && pin <= 30) && (condition == bothEdges)) {
        u0_dbg_printf("%d_%dB\n",port,pin);
        Port0_InterruptCB_Falling[pin] = pin_isr;
        Port0_InterruptCB_Rising[pin] = pin_isr;
        LPC_GPIOINT->IO0IntEnF |= (1 << pin); //enable the falling edge intr for that pin
        LPC_GPIOINT->IO0IntEnR |= (1 << pin); //enable the rising edge intr for that pin
        return true;
    }

    else if (port == 2 && (pin >= 0 && pin <= 13) && (condition == risingEdge)) {
        u0_dbg_printf("%d_%dR\n",port,pin);
        Port2_InterruptCB_Rising[pin] = pin_isr;
        LPC_GPIOINT->IO2IntEnR |= (1 << pin); //enable the rising edge intr for that pin
        return true;
    }
    else if (port == 2 && (pin >= 0 && pin <= 13) && (condition == fallingEdge)) {
        u0_dbg_printf("%d_%dF\n",port,pin);
        Port2_InterruptCB_Falling[pin] = pin_isr;
        LPC_GPIOINT->IO2IntEnF |= (1 << pin); //enable the falling edge intr for that pin
        return true;
    }
    else if (port == 2 && (pin >= 0 && pin <= 13) && (condition == bothEdges)) {
        u0_dbg_printf("%d_%dB\n",port,pin);
        Port2_InterruptCB_Falling[pin] = pin_isr;
        Port2_InterruptCB_Rising[pin] = pin_isr;
        LPC_GPIOINT->IO2IntEnF |= (1 << pin); //enable the falling edge intr for that pin
        LPC_GPIOINT->IO2IntEnR |= (1 << pin); //enable the rising edge intr for that pin
        return true;
    }

    else
        return false;
}
/**
 * After the init function has run, this will be executed whenever a proper
 * EINT3 external GPIO interrupt occurs. This function figure out which pin
 * has been interrupted and run the ccorrespondingISR for it using the lookup table.
 *
 * VERY IMPORTANT! Be sure to clear the interrupt flag that caused this
 * interrupt, or this function will be called again and again and again, ad infinitum.
 *
 * Also, NOTE that your code needs to be able to handle two GPIO interrupts occurring
 * at the same time.
 */
static void externalIRQHandler(void)
{
    //check which pin has been interrupted
    uint32_t port0_risingStatusReg = LPC_GPIOINT->IO0IntStatR;
    uint32_t port0_fallingStatusReg = LPC_GPIOINT->IO0IntStatF;
    uint32_t port2_risingStatusReg = LPC_GPIOINT->IO2IntStatR;
    uint32_t port2_fallingStatusReg = LPC_GPIOINT->IO2IntStatF;

    //check for port 0 rising or falling interrupts and execute the CB from lookup
    for (int pinNum = 0; pinNum <= 30; pinNum++) {
        if (port0_risingStatusReg & (1 << pinNum)) {
            Port0_InterruptCB_Rising[pinNum](); //execute CB rising

        }

        if (port0_fallingStatusReg & (1 << pinNum)) {
            Port0_InterruptCB_Falling[pinNum](); //execute CB falling

        }
        LPC_GPIOINT->IO0IntClr |= (1 << pinNum); //clear all intrs for that pin

    }

    //check for port 2 rising or falling interrupts and execute the CB from lookup
    for (int pinNum = 0; pinNum <= 13; pinNum++)
    {
        if (port2_risingStatusReg & (1 << pinNum))
        {
            Port2_InterruptCB_Rising[pinNum](); //execute CB rising
        }

        if (port2_fallingStatusReg & (1 << pinNum))
        {
            Port2_InterruptCB_Falling[pinNum](); //execute CB falling
        }
        LPC_GPIOINT->IO2IntClr |= (1 << pinNum); //clear all intrs for that pin
    }

}

void LabGPIOInterrupts::init(void)
{
    // Register your ISR using lpc_isr.h
    isr_register(EINT3_IRQn, externalIRQHandler);
    // Enable interrupt for the EINT3
    NVIC_EnableIRQ(EINT3_IRQn);
}


LabGPIOInterrupts::~LabGPIOInterrupts()
{
    ;
}
