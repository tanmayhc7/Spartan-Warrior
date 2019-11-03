/*
 *     SocialLedge.com - Copyright (C) 2013
 *
 *     This file is part of free software framework for embedded processors.
 *     You can use it and/or distribute it as long as this copyright header
 *     remains unmodified.  The code is free for personal use and requires
 *     permission to use in a commercial product.
 *
 *      THIS SOFTWARE IS PROVIDED "AS IS".  NO WARRANTIES, WHETHER EXPRESS, IMPLIED
 *      OR STATUTORY, INCLUDING, BUT NOT LIMITED TO, IMPLIED WARRANTIES OF
 *      MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE APPLY TO THIS SOFTWARE.
 *      I SHALL NOT, IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL, OR
 *      CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.
 *
 *     You can reach the author of this software at :
 *          p r e e t . w i k i @ g m a i l . c o m
 */

/**
 * @file
 * @brief This is the application entry point.
 *          FreeRTOS and stdio printf is pre-configured to use uart0_min.h before main() enters.
 *          @see L0_LowLevel/lpc_sys.h if you wish to override printf/scanf functions.
 *
 */
#include "tasks.hpp"
#include "LabSPI.hpp"
#include "printf_lib.h"
#include "semphr.h"
#include "VS1053B_memmap.h"
#include "VS1053B.hpp"
#include "song.hpp"
#include "storage.hpp"
#include "string.h"
#include "uart2.hpp"
#include "BT_message.hpp"
#include "ffconf.h"
#include "utilities.h"

uint8_t COUNT = READ_BYTES_FROM_FILE / TRANSMIT_BYTES_TO_DECODER;  //Divided by 128 because the decoder decodes only if maximum bytes are 128.
//MODE RELATED VARIABLES
extern uint16_t MODE;
extern uint16_t CLOCKF;
extern uint16_t VOL;
extern uint16_t BASS;
extern uint16_t AUDATA;
char SONG_NAME[50] = "1:Track.mp3";

Uart2& BT = Uart2::getInstance();
LabSPI decoderSPI;
QueueHandle_t musicQueue;
TaskHandle_t bufferTaskHandle;

TaskHandle_t playSongTaskHandle = NULL;
unsigned long song_offset = NULL;

void BTSendSongList(char msg[]);


//Read the 16-bit value of a VS10xx register
extern uint16_t mp3_readRequest(uint8_t address)
{
    uint16_t result = 0;
    while (!mp3_dreq_getLevel())
        ;
    mp3_cs(); //do a chip select
    decoderSPI.transfer(READ);
    decoderSPI.transfer(address);

    uint8_t highbyte = decoderSPI.transfer(0xFF);
    uint8_t lowbyte = decoderSPI.transfer(0xFF);
    result = (highbyte << 8) | lowbyte;
    while (!mp3_dreq_getLevel())
        ; // dreq should go high indicating transfer complete
    mp3_ds(); // Assert a HIGH signal to de-assert the CS

    return result;
}

extern void mp3_writeRequest(uint8_t address, uint16_t data)
{
    while (!mp3_dreq_getLevel())
        ;
    mp3_cs(); //do a chip select
    decoderSPI.transfer(WRITE);
    decoderSPI.transfer(address);
    uint8_t dataUpper = (data >> 8 & 0xFF);
    uint8_t dataLower = (data & 0xFF);
    decoderSPI.transfer(dataUpper);
    decoderSPI.transfer(dataLower);
    while (!mp3_dreq_getLevel())
        ; // dreq should go high indicating transfer complete
    mp3_ds(); // Assert a HIGH signal to de-assert the CS

}

void vinitCheck(void * pvParameters)
{
    static int count;
    while (1) {
        if (xSemaphoreTake(LabSPI::spi_mutex, 1000)) {
            if (mp3_dreq_getLevel()) {
                mp3_cs(); //do a chip select
                count++;
                if (count % 2 == 0) {
                    uint16_t res = mp3_readRequest(SCI_MODE);
                    u0_dbg_printf("MODE is %x\n", res);
                }
                mp3_ds(); // Assert a HIGH signal to de-assert the CS
            }
            xSemaphoreGive(LabSPI::spi_mutex);
        }

        vTaskDelay(1000);

    }
}

void vplayHello(void * pvParameters)
{
    unsigned char *p;

    for (int x = 0; x < 4; x++) {
        if (xSemaphoreTake(LabSPI::spi_mutex, 1000)) {
            p = &HelloMP3[0]; // Point "p" to the beginning of array
            while (p <= &HelloMP3[sizeof(HelloMP3) - 1]) {
                while (!mp3_dreq_getLevel()) {
                    //DREQ is low while the receive buffer is full
                    //You can do something else here, the bus is free...
                    //Maybe set the volume or whatever...
                }

                //Once DREQ is released (high) we can now send 32 bytes of data
                mp3_data_cs(); //Select Data
                decoderSPI.transfer(*p++); // Send SPI byte
                mp3_data_ds(); //Deselect Data
            }

            //End of file - send 2048 zeros before next file
            mp3_data_cs(); //Select Data//Select Data
            for (int i = 0; i < 2048; i++) {
                while (!mp3_dreq_getLevel())
                    ; //If we ever see DREQ low, then we wait here
                decoderSPI.transfer(0);
            }
            while (!mp3_dreq_getLevel())
                ; //Wait for DREQ to go high indicating transfer is complete
            mp3_data_ds(); //Deselect Data
            xSemaphoreGive(LabSPI::spi_mutex);
        }
    }

    u0_dbg_printf("Done!");
    while (1)
        ;

}

unsigned char song[5000];

void vgetSong(void * pvParameters)
{
    unsigned char current_byte[READ_BYTES_FROM_FILE];  //static unsigned char *p; p = &HelloMP3[0];
    // int i;
    unsigned long filesize = Storage::getfileinfo(SONG_NAME);
    //u0_dbg_printf("%lu ", filesize);
    while (1) {

        if ((filesize - song_offset) > READ_BYTES_FROM_FILE) {
            if (xSemaphoreTake(LabSPI::spi_mutex, portMAX_DELAY)) {
                Storage::read(SONG_NAME, &current_byte[0], READ_BYTES_FROM_FILE, song_offset);
                xSemaphoreGive(LabSPI::spi_mutex);
                bool sent = xQueueSend(musicQueue, &current_byte, portMAX_DELAY);
                if (!sent) u0_dbg_printf("sf");
                song_offset += READ_BYTES_FROM_FILE;
            }
        }
        else {
            if (xSemaphoreTake(LabSPI::spi_mutex, portMAX_DELAY)) {
                Storage::read(SONG_NAME, &current_byte[0], (filesize - song_offset), song_offset);
                xSemaphoreGive(LabSPI::spi_mutex);
                bool sent = xQueueSend(musicQueue, &current_byte, portMAX_DELAY);
                if (!sent) u0_dbg_printf("sf");
                song_offset = 0;
            }
        }
    }
}

void vplaySong(void * pvParameters)
{
    static uint8_t current_count = 0;
    uint8_t receivedByte[READ_BYTES_FROM_FILE];
    uint32_t j = 0;
    while (1) {
        if (current_count == 0) {
            xQueueReceive(musicQueue, &receivedByte[0], portMAX_DELAY);
        }
        uint8_t i_plus_one = current_count + 1;
        uint32_t j_init = (current_count * TRANSMIT_BYTES_TO_DECODER);
        uint32_t j_max = (i_plus_one * TRANSMIT_BYTES_TO_DECODER);

        while (!mp3_dreq_getLevel()) {
            /*DREQ is low while the receive buffer is full,
             * we can do something else here, the bus is free.
             * */
            refresh_params();
        }

        //Once DREQ is released (high) we can now send 32 bytes of data

        if (xSemaphoreTake(LabSPI::spi_mutex, portMAX_DELAY)) {
            mp3_data_cs();                //Select Data

            for (j = j_init; j < j_max; j++) {
                decoderSPI.transfer(receivedByte[j]); // Send SPI byte
            }

            mp3_data_ds(); //Deselect Data
            xSemaphoreGive(LabSPI::spi_mutex);

            if (current_count == (COUNT - 1)) {
                current_count = 0;
                j = 0;
            }
            else {
                current_count += 1;
            }
        }
    }
}

/*
 * This task receives the commands given from the App via Bluetooth.
 *      Valid Commands Received from BT :
 *      *0          ->STOP              Returns 10
 *      *1          ->START             Returns 1
 *      *2          ->PAUSE             Returns 2
 *      *3          ->Volume ++         Returns 3
 *      *4          ->Volume --         Returns 4
 *      *5\n        ->Fast Forward      Returns 5
 *      SONG_NAME   ->Play that song    Returns 9
 *      #song_name\n (format)
 *
 *      ERROR                           Returns -1
 */

void vBTRecCommand(void * pvParameters)
{
    bool recd = false;
    uint8_t index = 0;
    while (1) {
        char *c = new char[1];
        char BTMsg[5] = "";
        recd = false;
        index = 0;
        //Keep receiving till we get the \n message
        while (BT.getRxQueueSize() > 0) {
            BT.getChar(c, 5);

            if (*c != '\n') {
                BTMsg[index] = c[0];
                recd = true;
                index++;
            }
            else {
                BTMsg[index] = c[0];
                break;
            }
        }

        //Once we receive the data, now process it
        if (recd) {

            int check = validate_BT_message(BTMsg);

            if (check == 10) {
                mp3_stop();
            }
            else if (check == 1) {
                mp3_start();
            }
            else if (check == 2) {
                mp3_pause();
            }
            else if (check == 3) {
                mp3_inc_vol();
            }
            else if (check == 4) {
                mp3_dec_vol();
            }
            else if (check == 5) {
                mp3_fast_forward();
            }
            else if (check == 9) {
                change_song();
            }
        }
        delete c; //free the memory
        vTaskDelay(100);
    }
}

/*
 * This function lists the names of all the songs on the memory card
 * The limit on the name is 13 characters (as defined in ff.h file in FILINFO definition)
 * This function runs only once when the music player starts
 *
 */
void getSongListFromMemCard()
{
    DIR Dir;
    FILINFO Finfo;
    FATFS *fs;
    FRESULT returnCode = FR_OK;
    const char *dirPath = "1:";
    char buffer[100];
    buffer[0] = '#';
    unsigned int fileBytesTotal = 0, numFiles = 0, numDirs = 0;

#if _USE_LFN
    char Lfname[_MAX_LFN];
    Finfo.lfname = Lfname;
    Finfo.lfsize = sizeof(Lfname);
#endif

    f_opendir(&Dir, dirPath);

    for (;;) {
#if _USE_LFN
        Finfo.lfname = Lfname;
        Finfo.lfsize = sizeof(Lfname);
#endif

        returnCode = f_readdir(&Dir, &Finfo);
        if ((FR_OK != returnCode) || !Finfo.fname[0]) {
            break;
        }

        if (Finfo.fattrib & AM_DIR) {
            numDirs++;
        }
        else {
            numFiles++;
            fileBytesTotal += Finfo.fsize;
            sprintf(buffer, "%s%s@", buffer, &(Finfo.fname[0]));
        }
    }
    sprintf(buffer, "%s\n", buffer);
    //Now that we have the list os songs as string of characters send them to the Android App via Bluetooth
    BTSendSongList(buffer);
}


void BTSendSongList(char songlist[])
{
    // while (1) {
    u0_dbg_printf("Sent song list to app!\n");
    char *ptr = songlist;
    while (*ptr != '\n') {
        BT.putChar(*ptr);
        ptr++;
    }
    BT.putChar('\n');
}

int main(void)
{

    //scheduler_add_task(new terminalTask(PRIORITY_HIGH));

    // Initialize MP3 decoder's SPI
    decoderSPI.init(decoderSPI.SSP1, 8, decoderSPI.Mode_SPI, 96);

    musicQueue = xQueueCreate(1, sizeof(char[READ_BYTES_FROM_FILE]));

    // Initialization of the mp3 decoder
    mp3_dreq_init();
    mp3_hardwareReset();
    mp3_initDecoder();


    // Initialization of the Bluetooth
    BT.init(9600, 1250, 350);   //param: baud rate, RXQ sz, TXQ sz)


    //Following function gives the list of songs on the memory card
    getSongListFromMemCard();

#if 0
    //Tasks that we ran initially to check if the decoder is working
    xTaskCreate(vinitCheck, "initCheckTask", 1024, (void *) 1, 1, NULL);
    xTaskCreate(vplayHello, "playHelloTask", 5120, (void *) 1, 2, NULL);
#endif

    xTaskCreate(vgetSong, "getSongTask", 3072, (void *) 1, 1, NULL);
    xTaskCreate(vplaySong, "playSongTask", 3072, (void *) 1, 2, &playSongTaskHandle);
    xTaskCreate(vBTRecCommand, "BTRecCommand", 1024, (void *) 1, 3, NULL);

    /* Start Scheduler - This will not return, and your tasks will start to run their while(1) loop */
    vTaskStartScheduler();
    //scheduler_start();
    return 0;
}
