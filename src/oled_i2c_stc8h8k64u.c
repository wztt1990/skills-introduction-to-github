#include <reg51.h>
#include "oled_i2c_stc8h8k64u.h"

#ifndef OLED_SCL
sbit OLED_SCL = P1^0;
#endif

#ifndef OLED_SDA
sbit OLED_SDA = P1^1;
#endif

static void OLED_I2C_Delay(void)
{
    volatile uint8_t i;

    for (i = 0; i < 8; i++) {
        ;
    }
}

static void OLED_I2C_Start(void)
{
    OLED_SDA = 1;
    OLED_SCL = 1;
    OLED_I2C_Delay();
    OLED_SDA = 0;
    OLED_I2C_Delay();
    OLED_SCL = 0;
}

static void OLED_I2C_Stop(void)
{
    OLED_SDA = 0;
    OLED_SCL = 1;
    OLED_I2C_Delay();
    OLED_SDA = 1;
    OLED_I2C_Delay();
}

static uint8_t OLED_I2C_Write(uint8_t data)
{
    uint8_t i;
    uint8_t ack;

    for (i = 0; i < 8; i++) {
        OLED_SDA = (data & 0x80) ? 1 : 0;
        OLED_I2C_Delay();
        OLED_SCL = 1;
        OLED_I2C_Delay();
        OLED_SCL = 0;
        data <<= 1;
    }

    OLED_SDA = 1;
    OLED_I2C_Delay();
    OLED_SCL = 1;
    OLED_I2C_Delay();
    ack = OLED_SDA;
    OLED_SCL = 0;
    OLED_I2C_Delay();

    return ack == 0;
}

void OLED_I2C_Init(void)
{
#if defined(P1M0) && defined(P1M1)
    P1M0 |= 0x03;
    P1M1 |= 0x03;
#endif
    OLED_SCL = 1;
    OLED_SDA = 1;
}

void OLED_WriteCommand(uint8_t command)
{
    OLED_I2C_Start();
    OLED_I2C_Write(OLED_I2C_ADDRESS << 1);
    OLED_I2C_Write(0x00);
    OLED_I2C_Write(command);
    OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t data)
{
    OLED_I2C_Start();
    OLED_I2C_Write(OLED_I2C_ADDRESS << 1);
    OLED_I2C_Write(0x40);
    OLED_I2C_Write(data);
    OLED_I2C_Stop();
}

void OLED_SetCursor(uint8_t page, uint8_t column)
{
    OLED_WriteCommand(0xB0 | (page & 0x07));
    OLED_WriteCommand(0x00 | (column & 0x0F));
    OLED_WriteCommand(0x10 | (column >> 4));
}

void OLED_DrawBuffer(const uint8_t *buffer, uint16_t length)
{
    uint16_t i;

    if (!buffer || length == 0) {
        return;
    }

    OLED_I2C_Start();
    OLED_I2C_Write(OLED_I2C_ADDRESS << 1);
    OLED_I2C_Write(0x40);
    for (i = 0; i < length; i++) {
        OLED_I2C_Write(buffer[i]);
    }
    OLED_I2C_Stop();
}

void OLED_Clear(void)
{
    uint8_t page;
    uint8_t column;

    for (page = 0; page < 8; page++) {
        OLED_SetCursor(page, 0);
        OLED_I2C_Start();
        OLED_I2C_Write(OLED_I2C_ADDRESS << 1);
        OLED_I2C_Write(0x40);
        for (column = 0; column < 128; column++) {
            OLED_I2C_Write(0x00);
        }
        OLED_I2C_Stop();
    }
}

void OLED_Init(void)
{
    OLED_I2C_Init();

    OLED_WriteCommand(0xAE);
    OLED_WriteCommand(0x20);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0xB0);
    OLED_WriteCommand(0xC8);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0x10);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0x81);
    OLED_WriteCommand(0x7F);
    OLED_WriteCommand(0xA1);
    OLED_WriteCommand(0xA6);
    OLED_WriteCommand(0xA8);
    OLED_WriteCommand(0x3F);
    OLED_WriteCommand(0xA4);
    OLED_WriteCommand(0xD3);
    OLED_WriteCommand(0x00);
    OLED_WriteCommand(0xD5);
    OLED_WriteCommand(0x80);
    OLED_WriteCommand(0xD9);
    OLED_WriteCommand(0xF1);
    OLED_WriteCommand(0xDA);
    OLED_WriteCommand(0x12);
    OLED_WriteCommand(0xDB);
    OLED_WriteCommand(0x40);
    OLED_WriteCommand(0x8D);
    OLED_WriteCommand(0x14);
    OLED_WriteCommand(0xAF);

    OLED_Clear();
}
