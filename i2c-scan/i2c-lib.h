/* 
 * File:   i2c-lib.h   for Microchip PIC XC8
 */

#ifndef LCD_I2C_LIB_H
#define	LCD_I2C_LIB_H

#ifdef	__cplusplus
extern "C" {
#endif

void i2c_enable(void);
void i2c_disable(void);
void i2c_start(void);
void i2c_repeat_start(void);
void i2c_stop(void);
void i2c_send_byte(unsigned char data);
unsigned char i2c_read_byte(int ack);
void i2c_lcd_send_command(unsigned char i2c_addr, unsigned char data);
void i2c_lcd_send_data(unsigned char i2c_addr, unsigned char data);
unsigned char i2c_lcd_read_data(unsigned char i2c_addr, unsigned int pos);
void i2c_wait(void);
void i2c_lcd_init(void);
void i2c_lcd_putch(char c);
void i2c_lcd_puts(const char *s);
unsigned char i2c_lcd_get(unsigned int pos);
void i2c_lcd_cmd(unsigned char cmd);
void i2c_lcd_clear(void);
void i2c_lcd_set_cursor_pos(unsigned int pos);

void i2c_eeprom_write_byte(unsigned int addr, unsigned char data);
void i2c_eeprom_write_32byte(unsigned int addr, unsigned char *data);
unsigned char i2c_eeprom_read_byte(unsigned int addr);
void i2c_eeprom_read_32byte(unsigned int addr, unsigned char *buf);


#ifdef	__cplusplus
}
#endif

#endif	/* LCD_I2C_LIB_H */

