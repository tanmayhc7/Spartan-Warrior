///*
// *     SocialLedge.com - Copyright (C) 2013
// *
// *     This file is part of free software framework for embedded processors.
// *     You can use it and/or distribute it as long as this copyright header
// *     remains unmodified.  The code is free for personal use and requires
// *     permission to use in a commercial product.
// *
// *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
// *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
// *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
// *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
// *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
// *
// *     You can reach the author of this software at :
// *          p r e e t . w i k i @ g m a i l . c o m
// */
//
///**
// * @file
// * @brief This is the application entry point.
// */
//
#include <stdio.h>
#include <stdlib.h>
#include <FreeRTOS.h>
#include "task.h"
#include "utilities.h"
#include "io.hpp"
#include "LabGPIO0.h"
#include "uart0_min.h"
#include <GPIOInterrupt.h>
#include "i2c2.hpp"
#include "storage.hpp"
#include "event_groups.h"
#include "tasks.hpp"
#include "handlers.hpp"
#include "lpc_pwm.hpp"
#include "Mp3Decoder.h"

struct PortPinData{
    uint8_t PIN;
    uint8_t PORT;
};

struct dataForgunShot{
    PortPinData button1;
    PortPinData button2;
    PortPinData soundBuzzer;
};

PortPinData SW_1 = {.PIN = 9, .PORT = 1};
PortPinData SW_2 = {.PIN = 10, .PORT = 1};
PortPinData Sound1 = {.PIN = 0, .PORT = 2};

dataForgunShot switches = {.button1=SW_1, .button2=SW_2, .soundBuzzer = Sound1};
dataForgunShot *switches_pntr = &switches;


void gunShot(void *p)
{
    signed int bullets = 5;
    dataForgunShot temp = *(dataForgunShot*)p;
    LabGPIO_0 trigger(temp.button1.PORT,temp.button1.PIN);
    LabGPIO_0 reload(temp.button2.PORT,temp.button2.PIN);
    LabGPIO_0 buzzer1(temp.soundBuzzer.PORT,temp.soundBuzzer.PIN);
    trigger.setAsInput();
    reload.setAsInput();
    buzzer1.setAsOutput();
    while(1)
    {
        if(reload.getLevel())
        {
            printf("Reload Pressed\n");
            bullets = 5;
        }
        if(trigger.getLevel() && bullets > 0)
        {
            PWM buzzer(PWM::pwm1, 100);
            buzzer.set(261);

            bullets--;
            if(bullets <= 0)
            {
                bullets = 0;
            }
            vTaskDelay(500);
            printf("Trigger Pressed\n");
            printf("Bullets remaining: %d\n", bullets);
        }
    vTaskDelay(100);
    }
}



Mp3Decoder MP3;

GPIO signalA(P1_23);
TaskHandle_t signalSong = NULL;
TaskHandle_t signalSong1 = NULL;

void readSong(void *p)
{
    while(1)
    {
        if(!signalA.read())
        {
            MP3.resetData();
        }
        else
        MP3.readSong();
    }
}

void readSong2(void *p)
{
    while(1)
    {
        if(signalA.read())
//        {
            MP3.readSong2();
//        }
    }
}

void playSong(void *p)
{
    MP3.resetData();
    while(1)
    {
        if(signalA.read())
            MP3.playSong();
        else
            MP3.resetData();
    }
}

void readRegisters(void *p)
{
    uint16_t temp;
    while(1)
    {
        temp = MP3.readData(SCI_CLOCKF);
        u0_dbg_printf("CLOCKF: %x\n", temp);
        temp = MP3.readData(SCI_MODE);
        u0_dbg_printf("MODE: %x\n", temp);
        temp = MP3.readData(SCI_BASS);
        u0_dbg_printf("BASS: %x\n", temp);
        temp = MP3.readData(SCI_AUDATA);
        u0_dbg_printf("AUDATA: %x\n", temp);
        temp = MP3.readData(SCI_VOL);
        u0_dbg_printf("VOL: %x\n\n", temp);
        vTaskDelay(1000);
    }
}


int main(void)
{
    ssp1_init();
    MP3.reset();
    delay_ms(100);
    MP3.initSpeaker();
    signalA.setAsInput();
//    xTaskCreate(readRegisters, (const char*) "readRegisters", 1024, NULL, 1, NULL);
    xTaskCreate(readSong, (const char*) "readSong", 4096, NULL, 1, &signalSong);
//    xTaskCreate(readSong2, (const char*) "readSong2", 4096, NULL, 1, &signalSong1);
    xTaskCreate(playSong, (const char*) "playSong", 4096, NULL, 2, NULL);
    vTaskStartScheduler();
    while(1)
    {

    }
    return 0;
}


