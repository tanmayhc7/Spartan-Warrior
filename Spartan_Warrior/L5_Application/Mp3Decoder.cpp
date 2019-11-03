/*
 * Mp3Decoder.cpp
 *
 *  Created on: Dec 4, 2018
 *      Author: akwon_000
 */

#include <Mp3Decoder.h>
#include "hellomp3.hpp"
#include "utilities.h"


GPIO mp3_CS(P0_29);
GPIO mp3_RST(P0_30);
GPIO mp3_XDCS(P1_19);
GPIO mp3_DREQ(P1_22);


Mp3Decoder::Mp3Decoder()
{
    // TODO Auto-generated constructor stub
    mp3_CS.setAsOutput();
    mp3_RST.setAsOutput();
    mp3_XDCS.setAsOutput();
    mp3_DREQ.setAsInput();
    mp3_CS.setHigh();
    mp3_XDCS.setHigh();

    AUDATA = 0xAC45;//0xAC45;
    BASS = 0x21F0;
    CLOCKF = 0xA800;//0xA800; //24Mhz clock 0x03E8
    MODE = 0x4800; //0x4820
    STATUS = 0;
    VOL = 0x2020;//0x2424;

    musicQueue = xQueueCreate(1,sizeof(char[4096])); //create queue
    spi_bus_lock = xSemaphoreCreateMutex(); // create mutex to prevent multiple exchanges over spi

    songoffset = 0;
    j = 0;
    current_count = 0;
}


Mp3Decoder::~Mp3Decoder()
{
    // TODO Auto-generated destructor stub
}

uint16_t Mp3Decoder::readData(uint8_t address)
{
    uint16_t temp = 0;
    uint8_t temp2 = 0;
    if(xSemaphoreTake(spi_bus_lock,1000))
    {
        while(!mp3_DREQ.read()); //don't do anything if DREQ is low
        mp3_CS.setLow();
        ssp1_exchange_byte(READ);
        ssp1_exchange_byte(address);
        temp = ssp1_exchange_byte(0x00);
        temp2 = ssp1_exchange_byte(0x00);
        temp = temp << 8;
        temp = temp | temp2;
        while(!mp3_DREQ.read()); // don't do anything if DREQ is low
        mp3_CS.setHigh();
        xSemaphoreGive(spi_bus_lock);

    }
    return temp;
}

void Mp3Decoder::writeData(uint8_t address, uint16_t data)
{
    if(xSemaphoreTake(spi_bus_lock,1000))
    {
        while(!mp3_DREQ.read())
            ; //don't do anything if DREQ is low
        mp3_CS.setLow();
        ssp1_exchange_byte(WRITE);
        ssp1_exchange_byte(address);
        ssp1_exchange_byte((data >> 8)); //send upper byte
        ssp1_exchange_byte((data & 0x00FF)); //send lower byte
        while(!mp3_DREQ.read())
            ; // don't do anything if DREQ is low
        mp3_CS.setHigh();
        xSemaphoreGive(spi_bus_lock);
    }
}

void Mp3Decoder::initSpeaker()
{
    writeData(SCI_MODE,MODE);
    writeData(SCI_BASS,BASS);
    writeData(SCI_VOL,VOL);
    writeData(SCI_CLOCKF,CLOCKF);
    writeData(SCI_AUDATA,AUDATA);
}



void Mp3Decoder::readSong()
{
    char dataChunk[4096];
    unsigned long filesize = Storage::getfileinfo("1:track1.mp3");
    if((filesize - songoffset) > 4096)
    {
        if(xSemaphoreTake(spi_bus_lock, portMAX_DELAY))
        {
            Storage::read("1:track1.mp3", &dataChunk[0], 4096, songoffset);
            xSemaphoreGive(spi_bus_lock);
            xQueueSend(musicQueue, &dataChunk, portMAX_DELAY);
            songoffset = songoffset + 4096;
        }
    }
    else
        if(xSemaphoreTake(spi_bus_lock, portMAX_DELAY))
        {
            Storage::read("1:track1.mp3", &dataChunk[0], filesize - songoffset, songoffset);
            xSemaphoreGive(spi_bus_lock);
            xQueueSend(musicQueue, &dataChunk, portMAX_DELAY);
            songoffset = 0;
        }

}

void Mp3Decoder::resetData()
{
    current_count = 0;
    songoffset = 0;
    j = 0;

}

void Mp3Decoder::readSong2()
{
    char dataChunk[4096];
    unsigned long filesize = Storage::getfileinfo("1:track2.mp3");
    if((filesize - songoffset) > 4096)
    {
        if(xSemaphoreTake(spi_bus_lock, portMAX_DELAY))
        {
            Storage::read("1:track2.mp3", &dataChunk[0], 4096, songoffset);
            xSemaphoreGive(spi_bus_lock);
            xQueueSend(musicQueue, &dataChunk, portMAX_DELAY);
            songoffset = songoffset + 4096;
        }
    }
    else
        if(xSemaphoreTake(spi_bus_lock, portMAX_DELAY))
        {
            Storage::read("1:track2.mp3", &dataChunk[0], filesize - songoffset, songoffset);
            xSemaphoreGive(spi_bus_lock);
            xQueueSend(musicQueue, &dataChunk, portMAX_DELAY);
            songoffset = 0;
        }

}

void Mp3Decoder::playSong()
{
    char receivedByte[4096];
    if (current_count == 0) {
        xQueueReceive(musicQueue, &receivedByte[0], 1000);
    }
    uint8_t i_plus_one = current_count + 1;
    uint32_t start_offset = (current_count * 32);
    uint32_t end_offset = (i_plus_one * 32);
    while (!mp3_DREQ.read())
    {

    }
    if (xSemaphoreTake(spi_bus_lock, 1000)) {
        mp3_XDCS.setLow();
        for (j = start_offset; j < end_offset; j++) {
            ssp1_exchange_byte(receivedByte[j]); // Send SPI byte
        }
        mp3_XDCS.setHigh();

        xSemaphoreGive(spi_bus_lock);
        if (current_count == 127) { // if 4k bytes sent, grab next 4k bytes (128*32)
            current_count = 0;
            j = 0;
        }
        else {
            current_count += 1;
        }
    }
}

void Mp3Decoder::playTest()
{
    unsigned char *p;
    if(xSemaphoreTake(spi_bus_lock, 1000))
    {
        p = &HelloMP3[0];
        while(p <= &HelloMP3[sizeof(HelloMP3) - 1])
        {
            while(!mp3_DREQ.read());
            mp3_XDCS.setLow();
            ssp1_exchange_byte(*p++);
            mp3_XDCS.setHigh();
            while(!mp3_DREQ.read());
        }
        xSemaphoreGive(spi_bus_lock);
    }
    delay_ms(5000);
}

uint16_t Mp3Decoder::getRegisterContents(mp3_register name)
{
    switch(name)
    {
        case 0:
            return MODE;
        case 1:
            return CLOCKF;
        case 2:
            return VOL;
        case 3:
            return BASS;
        case 4:
            return AUDATA;
        default:
            return 0;
    }
}

void Mp3Decoder::sineTest(void)
{
    if(xSemaphoreTake(spi_bus_lock, 1000))
        {
                while(!mp3_DREQ.read());

                mp3_XDCS.setLow();
                ssp1_exchange_byte(0x53); //0x53 0xEF 0x6E n 0 0 0 0
                ssp1_exchange_byte(0xEF);
                ssp1_exchange_byte(0x6E);
                ssp1_exchange_byte(3);
                ssp1_exchange_byte(0x00);
                ssp1_exchange_byte(0x00);
                ssp1_exchange_byte(0x00);
                ssp1_exchange_byte(0x00);
                mp3_XDCS.setHigh();

                delay_ms(100);

                while(!mp3_DREQ.read());
                mp3_XDCS.setLow();
                ssp1_exchange_byte(0x45);  //0x45 0x78 0x69 0x74 0 0 0 0
                ssp1_exchange_byte(0x78);
                ssp1_exchange_byte(0x69);
                ssp1_exchange_byte(0x74);
                ssp1_exchange_byte(0x00);
                ssp1_exchange_byte(0x00);
                ssp1_exchange_byte(0x00);
                ssp1_exchange_byte(0x00);
                mp3_XDCS.setHigh();

            xSemaphoreGive(spi_bus_lock);
        }
}

void Mp3Decoder::reset(void)
{
    mp3_RST.setLow();
    delay_ms(100);
    mp3_RST.setHigh();
    delay_ms(100);
}
