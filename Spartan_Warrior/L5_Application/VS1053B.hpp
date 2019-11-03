/*
 * VS1053B.h
 *
 *  Created on: Apr 24, 2018
 *      Author: Sneha
 */

#ifndef VS1053B_HPP_
#define VS1053B_HPP_
#include "stdio.h"
#include "VS1053B_memmap.h"
#include "tasks.hpp"

//enabling Chip select by writing a low to pin P1.22
void mp3_cs();

//disabling Chip select by writing a high to pin P1.22
void mp3_ds();

//enabling Chip select by writing a low to pin P1.20
void mp3_data_cs();

//disabling Chip select by writing a low to pin P1.20
void mp3_data_ds();

//reset functions
void mp3_reset();
void mp3_hardwareReset();


//enabling Chip select by writing a low to pin P2.7
void mp3_dreq_init();

//Read the current level of DREQ pin
bool mp3_dreq_getLevel();

//Initialize the decoder with default values
bool mp3_initDecoder();


extern void mp3_writeRequest(uint8_t address, uint16_t data);

extern uint16_t mp3_readRequest(uint8_t address);

//Refresh the new parameters based on the selection done by the app
void refresh_params();

/**
 * Start/Stop/Pause/Fast Forward based on Commands received from Bluetooth (via App)
 *
 * When a FAST FORWARD command is received from the app, this function is called.
 * The offset value chosen is a tried and tested value, based on how much the song
 * skips when we fast forward it. The song should not have skipped a large chunk so
 * as to let the listener feel that they have missed partial song. Neither should
 * it be so small that the listener should not notice the effect of fast forward at all.
 */
bool mp3_stop();
bool mp3_pause();
bool mp3_start();
bool mp3_fast_forward();

/**
 * Volume max = 0000
 * Volume min = FEFE
 * XXYY where XX is left channel and YY is right channel
 */
bool mp3_dec_vol();
bool mp3_inc_vol();

extern TaskHandle_t playSongTaskHandle;
extern unsigned long song_offset;
#endif /* VS1053B_HPP_ */
