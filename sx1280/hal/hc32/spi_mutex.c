#include "spi_mutex.h"
#include "ddl.h"

volatile SpiOwner_t g_spi_owner = SPI_FREE;

uint8_t SpiTryLock( SpiOwner_t owner )
{
    __disable_irq();
    if( g_spi_owner == SPI_FREE )
    {
        g_spi_owner = owner;
        __enable_irq();
        return 1u;
    }
    __enable_irq();
    return 0u;
}

void SpiUnlock( SpiOwner_t owner )
{
    __disable_irq();
    if( g_spi_owner == owner )
    {
        g_spi_owner = SPI_FREE;
    }
    __enable_irq();
}

void SpiLockBlocking( SpiOwner_t owner )
{
    while( SpiTryLock( owner ) == 0u )
    {
        __NOP();
    }
}

SpiOwner_t SpiGetOwner( void )
{
    return g_spi_owner;
}
