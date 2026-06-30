#include "sx1280_hal_hc32.h"
#include "shared_spi_bus.h"
#include "ddl.h"
#include "sysctrl.h"
#include <string.h>

#define MAX_HAL_BUFFER_SIZE  128
static uint8_t halTxBuffer[MAX_HAL_BUFFER_SIZE];
static uint8_t halRxBuffer[MAX_HAL_BUFFER_SIZE];
static DioIrqHandler *dioIrqCallbacks[3] = {NULL, NULL, NULL};

en_result_t SPI_TransmitReceive8(SPI_TypeDef *SPIx, uint8_t pu8SendBuf[], uint8_t pu8RecBuf[], uint32_t u32Len)
{
    for (uint32_t u32Index = 0; u32Index < u32Len; u32Index++)
    {
        while (FALSE == SPIx->SR_f.TXE) {}
        SPIx->DR = pu8SendBuf[u32Index];
        while (FALSE == SPIx->SR_f.RXNE) {}
        pu8RecBuf[u32Index] = (uint8_t)SPIx->DR;
    }
    while (FALSE == SPIx->SR_f.TXE) {}
    while (TRUE == SPIx->SR_f.BUSY) {}
    return Ok;
}

static uint8_t SpiReadWriteByte(uint8_t tx)
{
    uint8_t tx8 = tx;
    uint8_t rx8;
    SPI_TransmitReceive8(SX1280_SPI, &tx8, &rx8, 1);
    return rx8;
}

static void RadioSpiTransfer(const uint8_t *txBuf, uint8_t *rxBuf, uint16_t len)
{
    uint16_t i;

    SX1280HalWaitOnBusy();
    SharedSpiRadioBegin();
    for( i = 0; i < len; i++ )
    {
        uint8_t rx = SpiReadWriteByte( txBuf[i] );
        if( rxBuf != NULL )
        {
            rxBuf[i] = rx;
        }
    }
    SharedSpiRadioEnd();
}

void SX1280HalWaitOnBusy(void)
{
    while (BUSY_IS_HIGH()) {}
}

void SX1280HalInit(DioIrqHandler **irqHandlers)
{
    int i;
    stc_gpio_init_t stcGpioInit;
    stc_spi_init_t  stcSpiInit;

    for (i = 0; i < 3; i++)
    {
        dioIrqCallbacks[i] = irqHandlers[i];
    }

    SYSCTRL_PeriphClockEnable(PeriphClockGpio);
    SYSCTRL_PeriphClockEnable(PeriphClockSpi);
    SYSCTRL_PeriphReset(PeriphResetSpi);

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_01;
    stcGpioInit.u32Mode = GPIO_MD_INPUT;
    stcGpioInit.u32PullUp = GPIO_PULL_UP;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA01_AF_SPI_MISO();

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_02;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA02_AF_SPI_MOSI();

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_03;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA03_AF_SPI_SCK();

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_00;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    stcGpioInit.bOutputValue = TRUE;
    GPIOA_Init(&stcGpioInit);

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin    = GPIO_PIN_05;
    stcGpioInit.u32Mode   = GPIO_MD_INPUT;
    stcGpioInit.u32PullUp = GPIO_PULL_NONE;
    GPIOA_Init(&stcGpioInit);

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_08;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    stcGpioInit.bOutputValue = TRUE;
    GPIOA_Init(&stcGpioInit);

#if RADIO_DIO1_ENABLE
    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Mode = GPIO_MD_INT_INPUT;
    stcGpioInit.u32Pin = RADIO_DIO1_PIN;
    stcGpioInit.u32PullUp = GPIO_PULL_UP;
    stcGpioInit.u32ExternInt = GPIO_EXTI_RISING;
    GPIOA_Init(&stcGpioInit);
    NVIC_SetPriority(PORTA_IRQn, 1);
    NVIC_EnableIRQ(PORTA_IRQn);
#endif

    SPI_StcInit(&stcSpiInit);
    stcSpiInit.u32BaudRate    = SPI_BAUDRATE_PCLK_DIV8;
    stcSpiInit.u32Mode        = SPI_MD_MASTER;
    stcSpiInit.u32CPHA        = SPI_CLK_PHASE_1EDGE;
    stcSpiInit.u32CPOL        = SPI_CLK_POLARITY_LOW;
    stcSpiInit.u32BitOrder    = SPI_MSB_FIRST;
    stcSpiInit.u32DataWidth   = SPI_DATA_WIDTH_8BIT;
    stcSpiInit.u32NSS         = SPI_NSS_SOFT;
    stcSpiInit.u32TransDir    = SPI_FULL_DUPLEX;
    stcSpiInit.u32SampleDelay = SPI_SAMPLE_NORMAL;
    SPI_Init(SPI, &stcSpiInit);
    SPI_Enable(SPI);
}

void SX1280HalIoIrqInit(DioIrqHandler **irqHandlers) { (void)irqHandlers; }

void SX1280HalReset(void)
{
    SX1280HalWaitOnBusy();
    DDL_Delay1ms(20);
    RADIO_RST_RESET();
    DDL_Delay1ms(50);
    RADIO_RST_SET();
    DDL_Delay1ms(20);
}

void SX1280HalClearInstructionRam(void)
{
    uint16_t halSize = 3 + IRAM_SIZE;
    halTxBuffer[0] = RADIO_WRITE_REGISTER;
    halTxBuffer[1] = (IRAM_START_ADDRESS >> 8) & 0xFF;
    halTxBuffer[2] = IRAM_START_ADDRESS & 0xFF;
    for (uint16_t i = 0; i < IRAM_SIZE; i++)
        halTxBuffer[3 + i] = 0x00;
    RadioSpiTransfer(halTxBuffer, NULL, halSize);
    SX1280HalWaitOnBusy();
}

void SX1280HalWakeup(void)
{
    halTxBuffer[0] = RADIO_GET_STATUS;
    halTxBuffer[1] = 0x00;
    RadioSpiTransfer(halTxBuffer, NULL, 2);
    SX1280HalWaitOnBusy();
}

void SX1280HalWriteCommand(RadioCommands_t opcode, uint8_t *buffer, uint16_t size)
{
    uint16_t halSize = size + 1;
    halTxBuffer[0] = (uint8_t)opcode;
    memcpy(halTxBuffer + 1, buffer, size);
    RadioSpiTransfer(halTxBuffer, NULL, halSize);
    if (opcode != RADIO_SET_SLEEP)
        SX1280HalWaitOnBusy();
}

void SX1280HalReadCommand(RadioCommands_t opcode, uint8_t *buffer, uint16_t size)
{
    uint16_t halSize = 2 + size;
    halTxBuffer[0] = (uint8_t)opcode;
    halTxBuffer[1] = 0x00;
    for (uint16_t i = 0; i < size; i++)
        halTxBuffer[2 + i] = 0x00;
    RadioSpiTransfer(halTxBuffer, halRxBuffer, halSize);
    memcpy(buffer, halRxBuffer + 2, size);
    SX1280HalWaitOnBusy();
}

void SX1280HalWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
    uint16_t halSize = 3 + size;
    halTxBuffer[0] = RADIO_WRITE_REGISTER;
    halTxBuffer[1] = (address >> 8) & 0xFF;
    halTxBuffer[2] = address & 0xFF;
    memcpy(halTxBuffer + 3, buffer, size);
    RadioSpiTransfer(halTxBuffer, NULL, halSize);
    SX1280HalWaitOnBusy();
}

void SX1280HalWriteRegister(uint16_t address, uint8_t value)
{
    SX1280HalWriteRegisters(address, &value, 1);
}

void SX1280HalReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size)
{
    uint16_t halSize = 4 + size;
    halTxBuffer[0] = RADIO_READ_REGISTER;
    halTxBuffer[1] = (address >> 8) & 0xFF;
    halTxBuffer[2] = address & 0xFF;
    halTxBuffer[3] = 0x00;
    for (uint16_t i = 0; i < size; i++)
        halTxBuffer[4 + i] = 0x00;
    RadioSpiTransfer(halTxBuffer, halRxBuffer, halSize);
    memcpy(buffer, halRxBuffer + 4, size);
    SX1280HalWaitOnBusy();
}

uint8_t SX1280HalReadRegister(uint16_t address)
{
    uint8_t data;
    SX1280HalReadRegisters(address, &data, 1);
    return data;
}

void SX1280HalWriteBuffer(uint8_t offset, uint8_t *buffer, uint8_t size)
{
    uint16_t halSize = 2 + size;
    halTxBuffer[0] = RADIO_WRITE_BUFFER;
    halTxBuffer[1] = offset;
    memcpy(halTxBuffer + 2, buffer, size);
    RadioSpiTransfer(halTxBuffer, NULL, halSize);
    SX1280HalWaitOnBusy();
}

void SX1280HalReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size)
{
    uint16_t halSize = 3 + size;
    halTxBuffer[0] = RADIO_READ_BUFFER;
    halTxBuffer[1] = offset;
    halTxBuffer[2] = 0x00;
    for (uint16_t i = 0; i < size; i++)
        halTxBuffer[3 + i] = 0x00;
    RadioSpiTransfer(halTxBuffer, halRxBuffer, halSize);
    memcpy(buffer, halRxBuffer + 3, size);
    SX1280HalWaitOnBusy();
}

uint8_t SX1280HalGetDioStatus(void)
{
    return GPIO_PA15_READ();
}
