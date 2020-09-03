/*
 * Mp3Decoder.h
 *
 *  Created on: Dec 4, 2018
 *      Author: akwon_000
 */

#ifndef MP3DECODER_H_
#define MP3DECODER_H_

#include <stdio.h>
#include <stdlib.h>
#include "lpc_sys.h"
#include "gpio.hpp"
#include <FreeRTOS.h>
#include "semphr.h"
#include "ssp1.h"
#include "printf_lib.h"
#include "storage.hpp"

class Mp3Decoder {
public:
    enum mp3_register{
        mode_,
        clockf_,
        vol,
        bass,
        audata,
    };

    Mp3Decoder();
    ~Mp3Decoder();

    uint16_t readData(uint8_t address);
    void writeData(uint8_t address, uint16_t data);
    void initSpeaker();
    void playSong();
    void readSong();
    void readSong2();
    void sineTest();
    void playTest();
    void resetData();
    void reset();
    uint16_t getRegisterContents(mp3_register name);
private:
    QueueHandle_t musicQueue;
    SemaphoreHandle_t spi_bus_lock;
    uint16_t MODE;
    uint16_t CLOCKF;
    uint16_t VOL;
    uint16_t BASS;
    uint16_t AUDATA;
    uint16_t STATUS;

    uint8_t current_count;
    unsigned long songoffset;
    uint32_t j;

};


#define SCI_MODE 0x00
#define SCI_STATUS 0x01
#define SCI_BASS 0x02
#define SCI_CLOCKF 0x03
#define SCI_DECODE_TIME 0x04
#define SCI_AUDATA 0x05
#define SCI_WRAM 0x06
#define SCI_WRAMADDR 0x07
#define SCI_HDAT0 0x08
#define SCI_HDAT1 0x09
#define SCI_AIADDR 0x0A
#define SCI_VOL 0x0B
#define SCI_AICTRL0 0x0C
#define SCI_AICTRL1 0x0D
#define SCI_AICTRL2 0x0E
#define SCI_AICTRL3 0x0F

#define READ 0x03
#define WRITE 0x02


#endif /* MP3DECODER_H_ */
