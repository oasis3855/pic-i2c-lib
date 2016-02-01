/* 
 * File:   i2c-lcdlib-ceg003400.h
 */

#ifndef I2C_LCDLIB_CEG003400_H
#define	I2C_LCDLIB_CEG003400_H

#ifdef	__cplusplus
extern "C" {
#endif

// LCD device I2C Address
#define LCD_I2C_ADDR	0x27

// LCD Backlight
#define LCD_BACKLIGHT 0b00001000    // Backlight ON
// #define LCD_BACKLIGHT 0b00000000    // Backlight OFF

void i2c_lcd_send_common(const unsigned char data);
void i2c_lcd_send_command(unsigned char data, unsigned char cmdtype);
void i2c_lcd_send_data(unsigned char data);
void i2c_lcd_init(void);
void i2c_lcd_putch(const unsigned char c);
void i2c_lcd_puts(const unsigned char *s);
void i2c_lcd_cmd(const unsigned char cmd);
void i2c_lcd_clear(void);
void i2c_lcd_set_cursor_pos(const unsigned char pos);


#ifdef	__cplusplus
}
#endif

#endif	/* I2C_LCDLIB_CEG003400_H */

