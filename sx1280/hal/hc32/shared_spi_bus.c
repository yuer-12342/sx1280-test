#include "shared_spi_bus.h"
#include "spi_mutex.h"
#include "sx1280_hal_hc32.h"
#include "gpio.h"
#include "ddl.h"

#define LCD_CS_SET()    GPIO_PA04_SET()
#define LCD_CS_RESET()  GPIO_PA04_RESET()

void SharedSpiCsGap( void )
{
    volatile uint32_t n = 48u;

    while( n-- )
    {
        __NOP();
    }
}

void SharedSpiEnsureBusIdle( void )
{
    RADIO_CS_SET();
    LCD_CS_SET();
    SharedSpiCsGap();
}


uint8_t SharedSpiLcdCanAccess( void )
{
    if( BUSY_IS_HIGH() )
    {
        return 0u;
    }
    if( SpiGetOwner() != SPI_FREE )
    {
        return 0u;
    }
    return 1u;
}

void SharedSpiTransferByte( uint8_t data )
{
    uint8_t tx = data;
    uint8_t rx;

    (void)SPI_TransmitReceive8( SX1280_SPI, &tx, &rx, 1u );
}

void SharedSpiRadioBegin( void )
{
    SpiLockBlocking( SPI_SX1280 );
    LCD_CS_SET();
    SharedSpiCsGap();
    RADIO_CS_RESET();
}

void SharedSpiRadioEnd( void )
{
    RADIO_CS_SET();
    SharedSpiCsGap();
    SpiUnlock( SPI_SX1280 );
}
