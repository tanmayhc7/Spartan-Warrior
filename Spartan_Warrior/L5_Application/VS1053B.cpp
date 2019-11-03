/*
 * VS1053B.c
 *
 *  Created on: Apr 24, 2018
 *      Author: Sucheta
 */

#include "VS1053B.hpp"
#include "LPC17xx.h"
#include "LabGPIO.hpp"
#include "utilities.h"
#include "tasks.hpp"
#include "printf_lib.h"

LabGPIO chipSelect = LabGPIO(1, 22);
LabGPIO dataChipSelect = LabGPIO(1, 20);
LabGPIO chipReset = LabGPIO(1, 19);
LabGPIO dreqPin = LabGPIO(2, 7);

uint16_t MODE = 0x4800;
uint16_t CLOCKF = 0xBBE8; //was 9800, EBE8, B3E8, BBE8
volatile uint16_t VOL = 0x2222; //full vol
uint16_t BASS = 0x0076; //was 00F6
uint16_t AUDATA = 0xAC80; //for stereo decoding, AC45,AC80, BB80-check
uint16_t STATUS;


void mp3_cs()
{
    chipSelect.setAsOutput();
    chipSelect.setLow();
}

void mp3_ds()
{
    chipSelect.setHigh();
}

void mp3_data_cs()
{
    dataChipSelect.setAsOutput();
    dataChipSelect.setLow();
}

void mp3_data_ds()
{
    dataChipSelect.setHigh();
}

void mp3_reset()
{
    chipReset.setAsOutput();
    chipReset.setLow(); //enable the reset pin
    delay_ms(20);
    chipReset.setHigh(); //disable the reset pin - working mode on
    delay_ms(20);
}

void mp3_hardwareReset()
{
    /*doing a hardware reset*/
    mp3_ds();
    mp3_data_ds();
    mp3_reset(); //reset the mp3 shield

}


void mp3_dreq_init()
{
    dreqPin.setAsInput();
}

bool mp3_dreq_getLevel()
{
    return dreqPin.getLevel();
}

bool mp3_initDecoder()
{
    mp3_writeRequest(SCI_MODE, MODE);
    mp3_writeRequest(SCI_CLOCKF, CLOCKF);
    mp3_writeRequest(SCI_VOL, VOL);
    mp3_writeRequest(SCI_BASS, BASS);
    mp3_writeRequest(SCI_AUDATA, AUDATA);
    return true;
}

void refresh_params()
{
    mp3_writeRequest(SCI_MODE, MODE);
    mp3_writeRequest(SCI_CLOCKF, CLOCKF); //was 9800, EBE8, B3E8, BBE8
    mp3_writeRequest(SCI_VOL, VOL); //full vol
    mp3_writeRequest(SCI_BASS, BASS); //was 00F6
    mp3_writeRequest(SCI_AUDATA, AUDATA); //for stereo decoding, AC45,AC80, BB80-check
}


bool mp3_stop()
{
    vTaskSuspend(playSongTaskHandle);
    song_offset = 0;
    return true;
}

bool mp3_start()
{
    //local_playspeed = 1;
    vTaskResume(playSongTaskHandle);
    return true;
}

bool mp3_pause()
{
    vTaskSuspend(playSongTaskHandle);
    return true;
}

bool mp3_dec_vol()
{
    uint8_t right_ch = (VOL);
    uint8_t left_ch = right_ch;
    right_ch += 5;
    left_ch = right_ch;
    uint16_t new_vol = ((left_ch << 8) | right_ch);
    if (new_vol <= 0xFEFE && new_vol >= 0x0000) {
        VOL = new_vol;
    }
    return false;
}

bool mp3_inc_vol()
{
    uint8_t right_ch = VOL;
    uint8_t left_ch = right_ch;
    right_ch -= 5;
    left_ch = right_ch;
    uint16_t new_vol = (left_ch << 8) | right_ch;
    if (new_vol >= 0x0000 && new_vol <= 0xFEFE) {
        VOL = new_vol;
    }
    return false;
}

bool mp3_fast_forward()
{
    song_offset += 16384;
    return true;
}

