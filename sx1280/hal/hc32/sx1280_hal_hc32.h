#ifndef __SX1280_HAL_HC32_H__
#define __SX1280_HAL_HC32_H__

#include "sx1280.h"
#include "gpio.h"
#include "spi.h"


#define RADIO_CS_PORT       GPIOA
#define RADIO_CS_PIN        GPIO_PIN_00
#define RADIO_CS_SET()      GPIO_PA00_SET()
#define RADIO_CS_RESET()    GPIO_PA00_RESET()

#define RADIO_BUSY_PORT     GPIOA
#define RADIO_BUSY_PIN      GPIO_PIN_05
#define BUSY_IS_HIGH()      (GPIO_PA05_READ() == 1)

#define RADIO_DIO1_PORT     GPIOA
#define RADIO_DIO1_PIN      GPIO_PIN_15      /* DIO1 = PA15 */

#define RADIO_RST_PORT      GPIOA
#define RADIO_RST_PIN       GPIO_PIN_08      /* NRESET = PA08 */
#define RADIO_RST_SET()     GPIO_PA08_SET()
#define RADIO_RST_RESET()   GPIO_PA08_RESET()


/* LED ���� PA06 */
#define LED_PORT    GPIOA
#define LED_PIN     GPIO_PIN_06
#define LED_ON()    GPIO_PA06_SET()
#define LED_OFF()   GPIO_PA06_RESET()


#define RADIO_DIO1_ENABLE   1
#define RADIO_DIO2_ENABLE   0
#define RADIO_DIO3_ENABLE   0

#define SX1280_SPI          SPI

en_result_t SPI_TransmitReceive8(SPI_TypeDef *SPIx, uint8_t pu8SendBuf[], uint8_t pu8RecBuf[], uint32_t u32Len);

/* HAL 接口 */
void SX1280HalInit(DioIrqHandler **irqHandlers);
void SX1280HalWaitOnBusy(void);
void SX1280HalReset(void);
void SX1280HalClearInstructionRam(void);
void SX1280HalWakeup(void);
void SX1280HalWriteCommand(RadioCommands_t opcode, uint8_t *buffer, uint16_t size);
void SX1280HalReadCommand(RadioCommands_t opcode, uint8_t *buffer, uint16_t size);
void SX1280HalWriteRegisters(uint16_t address, uint8_t *buffer, uint16_t size);
void SX1280HalWriteRegister(uint16_t address, uint8_t value);
void SX1280HalReadRegisters(uint16_t address, uint8_t *buffer, uint16_t size);
uint8_t SX1280HalReadRegister(uint16_t address);
void SX1280HalWriteBuffer(uint8_t offset, uint8_t *buffer, uint8_t size);
void SX1280HalReadBuffer(uint8_t offset, uint8_t *buffer, uint8_t size);
uint8_t SX1280HalGetDioStatus(void);
void SX1280HalIoIrqInit(DioIrqHandler **irqHandlers);
void EXTI15_IRQHandler(void);
#endif