#ifndef OLED_I2C_STC8H8K64U_H
#define OLED_I2C_STC8H8K64U_H

#include <stdint.h>

#define OLED_I2C_ADDRESS 0x3C

void OLED_I2C_Init(void);
void OLED_Init(void);
void OLED_Clear(void);
void OLED_SetCursor(uint8_t page, uint8_t column);
void OLED_WriteCommand(uint8_t command);
void OLED_WriteData(uint8_t data);
void OLED_DrawBuffer(const uint8_t *buffer, uint16_t length);

#endif
