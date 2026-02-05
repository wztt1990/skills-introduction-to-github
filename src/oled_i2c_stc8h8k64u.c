#include <reg51.h>
#include <intrins.h>
#include "oled_i2c_stc8h8k64u.h"

#define SSD1306_CMD_DISPLAY_OFF 0xAE
#define SSD1306_CMD_DISPLAY_ON 0xAF
#define SSD1306_CMD_SET_MEMORY_ADDR_MODE 0x20
#define SSD1306_ADDR_MODE_PAGE 0x02
#define SSD1306_CMD_SET_PAGE_START 0xB0
#define SSD1306_CMD_SET_COM_SCAN_DIR 0xC8
#define SSD1306_CMD_SET_LOW_COLUMN 0x00
#define SSD1306_CMD_SET_HIGH_COLUMN 0x10
#define SSD1306_CMD_SET_START_LINE 0x40
#define SSD1306_CMD_SET_CONTRAST 0x81
#define SSD1306_CONTRAST_DEFAULT 0x7F
#define SSD1306_CMD_SET_SEGMENT_REMAP 0xA1
#define SSD1306_CMD_NORMAL_DISPLAY 0xA6
#define SSD1306_CMD_SET_MULTIPLEX 0xA8
#define SSD1306_MULTIPLEX_DEFAULT 0x3F
#define SSD1306_CMD_DISPLAY_RAM 0xA4
#define SSD1306_CMD_SET_DISPLAY_OFFSET 0xD3
#define SSD1306_DISPLAY_OFFSET_DEFAULT 0x00
#define SSD1306_CMD_SET_DISPLAY_CLOCK 0xD5
#define SSD1306_DISPLAY_CLOCK_DEFAULT 0x80
#define SSD1306_CMD_SET_PRECHARGE 0xD9
#define SSD1306_PRECHARGE_DEFAULT 0xF1
#define SSD1306_CMD_SET_COM_PINS 0xDA
#define SSD1306_COM_PINS_DEFAULT 0x12
#define SSD1306_CMD_SET_VCOM_DETECT 0xDB
#define SSD1306_VCOM_DETECT_DEFAULT 0x40
#define SSD1306_CMD_CHARGE_PUMP 0x8D
#define SSD1306_CHARGE_PUMP_ON 0x14

#define OLED_I2C_DELAY_CYCLES 8 /* Increase value to slow SCL timing. */
#define OLED_I2C_PIN_MASK 0x03 /* P1.0/P1.1 mask for open-drain SCL/SDA configuration. */
#define SSD1306_CONTROL_BYTE_COMMAND 0x00
#define SSD1306_CONTROL_BYTE_DATA 0x40
#define SSD1306_DISPLAY_WIDTH 128
#define SSD1306_PAGE_COUNT 8

#ifndef OLED_SCL
sbit OLED_SCL = P1^0;
#endif

#ifndef OLED_SDA
sbit OLED_SDA = P1^1;
#endif

static void OLED_I2C_Delay(void)
{
    uint8_t i;

    for (i = 0; i < OLED_I2C_DELAY_CYCLES; i++) {
        _nop_();
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
    uint8_t ack_received;

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
    ack_received = (OLED_SDA == 0);
    OLED_SCL = 0;
    OLED_I2C_Delay();

    return ack_received;
}

void OLED_I2C_Init(void)
{
#if defined(P1M0) && defined(P1M1)
    P1M0 |= OLED_I2C_PIN_MASK;
    P1M1 |= OLED_I2C_PIN_MASK;
#endif
    OLED_SCL = 1;
    OLED_SDA = 1;
}

void OLED_WriteCommand(uint8_t command)
{
    OLED_I2C_Start();
    if (!OLED_I2C_Write(OLED_I2C_ADDRESS << 1)) {
        OLED_I2C_Stop();
        return;
    }
    if (!OLED_I2C_Write(SSD1306_CONTROL_BYTE_COMMAND)) {
        OLED_I2C_Stop();
        return;
    }
    if (!OLED_I2C_Write(command)) {
        OLED_I2C_Stop();
        return;
    }
    OLED_I2C_Stop();
}

void OLED_WriteData(uint8_t data)
{
    OLED_I2C_Start();
    if (!OLED_I2C_Write(OLED_I2C_ADDRESS << 1)) {
        OLED_I2C_Stop();
        return;
    }
    if (!OLED_I2C_Write(SSD1306_CONTROL_BYTE_DATA)) {
        OLED_I2C_Stop();
        return;
    }
    if (!OLED_I2C_Write(data)) {
        OLED_I2C_Stop();
        return;
    }
    OLED_I2C_Stop();
}

void OLED_SetCursor(uint8_t page, uint8_t column)
{
    OLED_WriteCommand(SSD1306_CMD_SET_PAGE_START | (page & 0x07));
    OLED_WriteCommand(SSD1306_CMD_SET_LOW_COLUMN | (column & 0x0F));
    OLED_WriteCommand(SSD1306_CMD_SET_HIGH_COLUMN | (column >> 4));
}

void OLED_DrawBuffer(const uint8_t *buffer, uint16_t length)
{
    uint16_t offset;
    uint8_t page;
    uint8_t column;

    if (!buffer || length == 0) {
        return;
    }

    offset = 0;
    for (page = 0; page < SSD1306_PAGE_COUNT && offset < length; page++) {
        OLED_SetCursor(page, 0);
        OLED_I2C_Start();
        if (!OLED_I2C_Write(OLED_I2C_ADDRESS << 1)) {
            OLED_I2C_Stop();
            return;
        }
        if (!OLED_I2C_Write(SSD1306_CONTROL_BYTE_DATA)) {
            OLED_I2C_Stop();
            return;
        }
        for (column = 0; column < SSD1306_DISPLAY_WIDTH && offset < length; column++) {
            if (!OLED_I2C_Write(buffer[offset])) {
                OLED_I2C_Stop();
                return;
            }
            offset++;
        }
        OLED_I2C_Stop();
    }
}

void OLED_Clear(void)
{
    uint8_t page;
    uint8_t column;

    for (page = 0; page < SSD1306_PAGE_COUNT; page++) {
        OLED_SetCursor(page, 0);
        OLED_I2C_Start();
        if (!OLED_I2C_Write(OLED_I2C_ADDRESS << 1)) {
            OLED_I2C_Stop();
            return;
        }
        if (!OLED_I2C_Write(SSD1306_CONTROL_BYTE_DATA)) {
            OLED_I2C_Stop();
            return;
        }
        for (column = 0; column < SSD1306_DISPLAY_WIDTH; column++) {
            if (!OLED_I2C_Write(0x00)) {
                OLED_I2C_Stop();
                return;
            }
        }
        OLED_I2C_Stop();
    }
}

void OLED_Init(void)
{
    OLED_I2C_Init();

    OLED_WriteCommand(SSD1306_CMD_DISPLAY_OFF);
    OLED_WriteCommand(SSD1306_CMD_SET_MEMORY_ADDR_MODE);
    OLED_WriteCommand(SSD1306_ADDR_MODE_PAGE);
    OLED_WriteCommand(SSD1306_CMD_SET_PAGE_START);
    OLED_WriteCommand(SSD1306_CMD_SET_COM_SCAN_DIR);
    OLED_WriteCommand(SSD1306_CMD_SET_LOW_COLUMN);
    OLED_WriteCommand(SSD1306_CMD_SET_HIGH_COLUMN);
    OLED_WriteCommand(SSD1306_CMD_SET_START_LINE);
    OLED_WriteCommand(SSD1306_CMD_SET_CONTRAST);
    OLED_WriteCommand(SSD1306_CONTRAST_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_SET_SEGMENT_REMAP);
    OLED_WriteCommand(SSD1306_CMD_NORMAL_DISPLAY);
    OLED_WriteCommand(SSD1306_CMD_SET_MULTIPLEX);
    OLED_WriteCommand(SSD1306_MULTIPLEX_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_DISPLAY_RAM);
    OLED_WriteCommand(SSD1306_CMD_SET_DISPLAY_OFFSET);
    OLED_WriteCommand(SSD1306_DISPLAY_OFFSET_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_SET_DISPLAY_CLOCK);
    OLED_WriteCommand(SSD1306_DISPLAY_CLOCK_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_SET_PRECHARGE);
    OLED_WriteCommand(SSD1306_PRECHARGE_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_SET_COM_PINS);
    OLED_WriteCommand(SSD1306_COM_PINS_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_SET_VCOM_DETECT);
    OLED_WriteCommand(SSD1306_VCOM_DETECT_DEFAULT);
    OLED_WriteCommand(SSD1306_CMD_CHARGE_PUMP);
    OLED_WriteCommand(SSD1306_CHARGE_PUMP_ON);
    OLED_WriteCommand(SSD1306_CMD_DISPLAY_ON);

    OLED_Clear();
}
