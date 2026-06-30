#include "lcd_init.h"
#include "shared_spi_bus.h"
#include "spi_mutex.h"
#include "sx1280_hal_hc32.h"
#include "ddl.h"
#include "sysctrl.h"

static uint8_t lcd_spi_depth = 0u;
static uint8_t lcd_init_mode = 0u;

void LCD_SetInitMode( uint8_t enable )
{
    if( ( enable == 0u ) && ( lcd_spi_depth != 0u ) )
    {
        LCD_CS_Set();
        lcd_spi_depth = 0u;
        SpiUnlock( SPI_TFT );
    }
    lcd_init_mode = enable;
}

static uint8_t LcdSpiEnter( void )
{
    if( lcd_spi_depth == 0u )
    {
        if( lcd_init_mode != 0u )
        {
            SpiLockBlocking( SPI_TFT );
        }
        else if( SpiTryLock( SPI_TFT ) == 0u )
        {
            return 0u;
        }

        RADIO_CS_SET();
        SharedSpiCsGap();
        LCD_CS_Clr();
    }

    lcd_spi_depth++;
    return 1u;
}

static void LcdSpiLeave( void )
{
    if( lcd_spi_depth == 0u )
    {
        return;
    }

    lcd_spi_depth--;
    if( lcd_spi_depth == 0u )
    {
        LCD_CS_Set();
        SharedSpiCsGap();
        SpiUnlock( SPI_TFT );
    }
}

void LCD_GPIO_Init(void)
{
    stc_gpio_init_t stcGpioInit;

    SYSCTRL_PeriphClockEnable(PeriphClockGpio);

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_04;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    stcGpioInit.bOutputValue = TRUE;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA04_AF_GPIO_SET();

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_07;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    stcGpioInit.bOutputValue = TRUE;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA07_AF_GPIO_SET();

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_10;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    stcGpioInit.bOutputValue = TRUE;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA10_AF_GPIO_SET();

    GPIO_StcInit(&stcGpioInit);
    stcGpioInit.u32Pin = GPIO_PIN_11;
    stcGpioInit.u32Mode = GPIO_MD_OUTPUT_PP;
    stcGpioInit.bOutputValue = TRUE;
    GPIOA_Init(&stcGpioInit);
    GPIO_PA11_AF_GPIO_SET();
}

uint8_t LCD_SpiReady(void)
{
    return SharedSpiLcdCanAccess();
}

void LCD_SpiBegin(void)
{
    (void)LCD_SpiReady();
}

void LCD_Writ_Bus(u8 dat)
{
    if( LcdSpiEnter() == 0u )
    {
        return;
    }

    SharedSpiTransferByte( dat );
    LcdSpiLeave();
}

void LCD_WR_DATA8(u8 dat)
{
    LCD_Writ_Bus(dat);
}

void LCD_WR_DATA(u16 dat)
{
    LCD_Writ_Bus((u8)(dat >> 8));
    LCD_Writ_Bus((u8)dat);
}

void LCD_WR_REG(u8 dat)
{
    LCD_DC_Clr();
    LCD_Writ_Bus(dat);
    LCD_DC_Set();
}

void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8((u8)(x1 + LCD_COL_OFFSET));
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8((u8)(x2 + LCD_COL_OFFSET));

    LCD_WR_REG(0x2B);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8((u8)y1);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8((u8)y2);

    LCD_WR_REG(0x2C);
}

void LCD_Init(void)
{
    LCD_GPIO_Init();
    LCD_SetInitMode( 1u );

    LCD_BLK_Set();

    LCD_RES_Set();
    DDL_Delay1ms(10);
    LCD_RES_Clr();
    DDL_Delay1ms(10);
    LCD_RES_Set();
    DDL_Delay1ms(10);

    LCD_WR_REG(0x11);
    DDL_Delay1ms(120);

    LCD_WR_REG(0xB1);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB3);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x3C);
    LCD_WR_DATA8(0x3C);

    LCD_WR_REG(0xB4);
    LCD_WR_DATA8(0x03);

    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x0E);
    LCD_WR_DATA8(0x04);

    LCD_WR_REG(0xC1);
    LCD_WR_DATA8(0xC0);

    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x0D);
    LCD_WR_DATA8(0x00);

    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0x2A);

    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x8D);
    LCD_WR_DATA8(0xEE);

    LCD_WR_REG(0xC5);
    LCD_WR_DATA8(0x04);

    LCD_WR_REG(0x36);
    LCD_WR_DATA8(0x08);

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x1A);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x3D);
    LCD_WR_DATA8(0x38);
    LCD_WR_DATA8(0x2E);
    LCD_WR_DATA8(0x30);
    LCD_WR_DATA8(0x2D);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x30);
    LCD_WR_DATA8(0x3B);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x01);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x10);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0x05);
    LCD_WR_DATA8(0x1A);
    LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x15);
    LCD_WR_DATA8(0x36);
    LCD_WR_DATA8(0x2E);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x2B);
    LCD_WR_DATA8(0x28);
    LCD_WR_DATA8(0x30);
    LCD_WR_DATA8(0x3B);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x01);
    LCD_WR_DATA8(0x02);
    LCD_WR_DATA8(0x10);

    LCD_WR_REG(0x29);

    LCD_SetInitMode( 0u );
}
