// lcd.h

#ifndef LCD_H
#define LCD_H

#include <stdint.h>

// Definir los pines I2C
#define I2C_MASTER_SCL_IO 22
#define I2C_MASTER_SDA_IO 21
#define I2C_MASTER_NUM I2C_NUM_0
#define I2C_MASTER_FREQ_HZ 100000

// Definir la dirección I2C del LCD
#define LCD_ADDR 0x27

// Declaración de funciones
void i2c_master_init(void);
void lcd_init(void);
void lcd_clear(void);
void lcd_return_home(void);
void lcd_set_cursor(uint8_t col, uint8_t row);
void lcd_display_text(const char *text);
void lcd_showData(float latitude, float longitude,char* time);
void i2c_scanner(void);

#endif // LCD_H
