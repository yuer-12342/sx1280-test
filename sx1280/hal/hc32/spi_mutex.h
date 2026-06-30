#ifndef __SPI_MUTEX_H__
#define __SPI_MUTEX_H__

#include <stdint.h>

typedef enum
{
    SPI_FREE = 0,
    SPI_SX1280 = 1,
    SPI_TFT = 2
} SpiOwner_t;

extern volatile SpiOwner_t g_spi_owner;

uint8_t SpiTryLock( SpiOwner_t owner );
void SpiUnlock( SpiOwner_t owner );
void SpiLockBlocking( SpiOwner_t owner );
SpiOwner_t SpiGetOwner( void );

#endif
