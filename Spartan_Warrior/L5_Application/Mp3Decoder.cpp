

#include <Mp3Decoder.h>
uint8_t COUNT = 128;
Mp3Decoder::Mp3Decoder()
{
    // TODO Auto-generated constructor stub
    mp3_CS.setAsOutput();
    mp3_RST.setAsOutput();
    mp3_XDCS.setAsOutput();
    mp3_SDCS.setAsOutput();
    mp3_DREQ.setAsInput();

    AUDATA = 0x2B10;
    BASS = 0x21F0;
    CLOCKF = 0x2BE8; //24Mhz clock
    MODE = 0x4800;
    STATUS = 0;
    VOL = 0x2424;

    musicQueue = xQueueCreate(1,sizeof(uint8_t)*4); //create queue
    spi_bus_lock = xSemaphoreCreateMutex(); // create mutex to prevent multiple exchanges over spi

    ssp1_init();
    ssp1_set_max_clock(24);
}


Mp3Decoder::~Mp3Decoder()
{
    // TODO Auto-generated destructor stub
}

uint16_t Mp3Decoder::readData(uint8_t address)
{
    uint16_t temp;
    if(xSemaphoreTake(spi_bus_lock,1000))
    {
        while(!mp3_DREQ.read()); //don't do anything if DREQ is low
        mp3_CS.setLow();
        ssp1_exchange_byte(READ);
        ssp1_exchange_byte(address);
        temp = ssp1_exchange_byte(0x00);
        temp = (temp << 8) | ssp1_exchange_byte(0x00);
        while(!mp3_DREQ.read()); // don't do anything if DREQ is low
        mp3_CS.setHigh();
        xSemaphoreGive(spi_bus_lock);

        return temp;
    }
    else
    {
        return 0;
    }
}

void Mp3Decoder::writeData(uint8_t address, uint16_t data)
{
    if(xSemaphoreTake(spi_bus_lock,1000))
    {
        while(!mp3_DREQ.read()); //don't do anything if DREQ is low
        mp3_CS.setLow();
        ssp1_exchange_byte(WRITE);
        ssp1_exchange_byte(address);
        ssp1_exchange_byte((data >> 8)); //send upper byte
        ssp1_exchange_byte((data & 0x00FF)); //send lower byte
        while(!mp3_DREQ.read()); // don't do anything if DREQ is low
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
    writeData(SCI_MODE,MODE);
    writeData(SCI_AUDATA,AUDATA);
}


void Mp3Decoder::playSong()
{
    static uint8_t current_count = 0;
            uint8_t receivedByte[4096];
            uint32_t j = 0;
            while (1)
            {
              if (current_count == 0)
              {
                xQueueReceive(musicQueue, &receivedByte[0], portMAX_DELAY);
              }
            uint8_t i_plus_one = current_count + 1;
            uint32_t j_init = (current_count * 32);
            uint32_t j_max = (i_plus_one * 32);
            while (!!mp3_DREQ.read());
    if(xSemaphoreTake(spi_bus_lock,1000))
    {
      mp3_CS.setLow();
      for (j = j_init; j < j_max; j++)
      {
          ssp1_exchange_byte(receivedByte[j]); // Send SPI byte
      }
      mp3_CS.setHigh();
      xSemaphoreGive(spi_bus_lock);
      if (current_count == (COUNT - 1))
      {
          current_count = 0;
          j = 0;
      }
      else
      {
       current_count += 1;
      }
    }
    }
}
