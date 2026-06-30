#ifndef __SHARED_SPI_BUS_H__
#define __SHARED_SPI_BUS_H__

#include <stdint.h>

/* CS 切换最小间隔：满足 SX1280 t9(100ns) 与 ST7735 TCHW(40ns) */
void SharedSpiCsGap( void );
void SharedSpiEnsureBusIdle( void );

uint8_t SharedSpiLcdCanAccess( void );

/* SX1280：整段 NSS 事务持锁，调用方勿嵌套 */
void SharedSpiRadioBegin( void );
void SharedSpiRadioEnd( void );

void SharedSpiTransferByte( uint8_t data );

#endif
