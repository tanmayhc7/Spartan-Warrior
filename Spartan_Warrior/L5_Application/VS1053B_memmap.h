/*
 * VS1053B_memmap.h
 *
 *  Created on: Apr 24, 2018
 *      Author: Sneha
 */

#ifndef VS1053B_MEMMAP_H_
#define VS1053B_MEMMAP_H_

//VS10xx SCI Registers
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
#define playSpeed 0x1e04

//commands
#define READ 0x03
#define WRITE 0x02

//static uint8_t SEND_NUM_BYTES = 32;
static uint16_t READ_BYTES_FROM_FILE = 4096;
static uint16_t TRANSMIT_BYTES_TO_DECODER = 32;

#endif /* VS1053B_MEMMAP_H_ */
