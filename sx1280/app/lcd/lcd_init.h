#ifndef __LCD_INIT_H
#define __LCD_INIT_H

#include <stdint.h>
#include "gpio.h"
#include "spi.h"
#include "sx1280_hal_hc32.h"

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;

/* 竖屏 80x160，与 Initial_ST7735 / address() 一致 */
#define USE_HORIZONTAL      0
#define LCD_COL_OFFSET      24

#define LCD_W               80
#define LCD_H               160

#define LCD_CS_Clr()   GPIO_PA04_RESET()
#define LCD_CS_Set()   GPIO_PA04_SET()

#define LCD_DC_Clr()   GPIO_PA07_RESET()
#define LCD_DC_Set()   GPIO_PA07_SET()

#define LCD_RES_Clr()  GPIO_PA10_RESET()
#define LCD_RES_Set()  GPIO_PA10_SET()

#define LCD_BLK_Clr()  GPIO_PA11_RESET()
#define LCD_BLK_Set()  GPIO_PA11_SET()

void LCD_GPIO_Init(void);
void LCD_SpiBegin(void);
uint8_t LCD_SpiReady(void);
void LCD_Writ_Bus(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_REG(u8 dat);
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_Init(void);
void LCD_SetInitMode( uint8_t enable );

#endif
