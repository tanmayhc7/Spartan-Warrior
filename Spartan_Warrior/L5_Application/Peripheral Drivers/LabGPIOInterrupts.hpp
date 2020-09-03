/*
 * LabGPIOInterrupts.hpp
 *
 *  Created on: Feb 19, 2018
 *      Author: Sucheta
 */

#ifndef LABGPIOINTERRUPTS_HPP_
#define LABGPIOINTERRUPTS_HPP_

#include <stdint.h>
#include "lpc_sys.h"
#include "singleton_template.hpp"  // Singleton Template
typedef enum
{
    risingEdge,
    fallingEdge,
    bothEdges
} InterruptCondition_E;


class LabGPIOInterrupts : public SingletonTemplate<LabGPIOInterrupts>
{
private:
    /**
     * Your job here is to construct a lookup table matrix that correlates a pin
     * and port to a registered ISR. You may want to make additional probably need
     * more than one. Be clever here. How can you do this such that you and the
     * cpu do the least amount of work.
     */
    LabGPIOInterrupts(){ };

public:
    /**
     * LabGPIOInterrupts should be a singleton, meaning, only one instance can exist at a time.
     * Look up how to implement this.
     */

    /**
     * 1) Should setup register "externalIRQHandler" as the EINT3 ISR.
     * 2) Should configure NVIC to notice EINT3 IRQs.
     */
    void init();
    /**
     * This handler should place a function pointer within the lookup table for the externalIRQHandler to find.
     *
     * @param[in] port         specify the GPIO port
     * @param[in] pin          specify the GPIO pin to assign an ISR to
     * @param[in] pin_isr      function to run when the interrupt event occurs
     * @param[in] condition    condition for the interrupt to occur on. RISING, FALLING or BOTH edges.
     * @return should return true if valid ports, pins, isrs were supplied and pin isr insertion was sucessful
     */
    bool attachInterruptHandler(uint8_t port, uint32_t pin, void (*pin_isr)(void), InterruptCondition_E condition);
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

    friend class SingletonTemplate<LabGPIOInterrupts>;  ///< Friend class used for Singleton Template
    ~LabGPIOInterrupts();
};



#endif /* LABGPIOINTERRUPTS_HPP_ */
