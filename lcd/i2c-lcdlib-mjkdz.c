/*
 * File:   i2c-lcdlib-mjkdz.c   for Microchip PIC XC8
 *
 * I2C LCD 変換アダプタ mjkdz I2C lcd1602用
 */

#include <stdio.h>
#include <stdlib.h>

#include <xc.h>
#include "i2c-lib.h"
#include "i2c-lcdlib-mjkdz.h"

#ifndef _XTAL_FREQ
	/* 例：4MHzの場合、4000000 をセットする */
	#define _XTAL_FREQ 4000000
#endif

// I2C Bus Control Definition
#define I2C_WRITE_CMD 0
#define I2C_READ_CMD 1


// BL は指定するとOFFになる
#define LCD_BL_ON 0b00000000
#define LCD_BL_OFF 0b10000000
#define LCD_EN 0b00010000
#define LCD_RW 0b00100000
#define LCD_RS 0b01000000

// LCD Command
#define LCD_CLEAR 0x01
#define LCD_1CYCLE 0
#define LCD_2CYCLE 1

// send raw data to I2C LCD (I2C LCDにデータを送信する低レベル関数)
void i2c_lcd_send_common(const unsigned char data){
	i2c_start();
	i2c_send_byte((LCD_I2C_ADDR<<1) | I2C_WRITE_CMD);	// アドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte(data);
	i2c_stop();
    }

// send command to I2C LCD (I2C LCDにコマンドデータを送信する) 4 bit mode
void i2c_lcd_send_command(unsigned char data, unsigned char cmdtype){
    unsigned char lcddata;

    // Put the Upper 4 bits data
    // Write Enable Pulse E: Hi
    lcddata = (data>>4) & 0x0f | LCD_BACKLIGHT;
    i2c_lcd_send_common(lcddata | LCD_EN);
    __delay_us(2);      // PWeh=0.23 us (HD44780U), Tpw=0.14 us (SD1602V)

    // Write Enable Pulse E: Hi -> Lo
    i2c_lcd_send_common(lcddata & ~LCD_EN);
    __delay_us(2);      // Tcyce-PWeh=0.5-0.23=0.27 us (HD44780U), Tc-Tpw=1.2-0.14=1.06 us (SD1602V)

    // cmdtype = 0; One cycle write, cmdtype = 1; Two cycle writes
    if (cmdtype) {
        // Put the Lower 4 bits data
        lcddata = data & 0x0f | LCD_BACKLIGHT;
        i2c_lcd_send_common(lcddata | LCD_EN);
        __delay_us(2);

        i2c_lcd_send_common(lcddata & ~LCD_EN);
        __delay_us(2);
    }
}

// send data to I2C LCD (I2C液晶にデータを送信する) 4 bit mode
void i2c_lcd_send_data(unsigned char data){
    unsigned char lcddata;

    // Put the Upper 4 bits data
    // Write Enable Pulse E: Hi
    lcddata = (data>>4) & 0x0f | LCD_BACKLIGHT | LCD_RS;
    i2c_lcd_send_common(lcddata | LCD_EN);
    __delay_us(2);      // PWeh=0.23 us (HD44780U), Tpw=0.14 us (SD1602V)

    // Write Enable Pulse E: Hi -> Lo
    i2c_lcd_send_common(lcddata & ~LCD_EN);
    __delay_us(2);      // Tcyce-PWeh=0.5-0.23=0.27 us (HD44780U), Tc-Tpw=1.2-0.14=1.06 us (SD1602V)

    // Put the Lower 4 bit data
    lcddata = (data & 0x0f) | LCD_BACKLIGHT | LCD_RS;
    i2c_lcd_send_common(lcddata | LCD_EN);
    __delay_us(2);

    i2c_lcd_send_common(lcddata & ~LCD_EN);
    __delay_us(2);

    __delay_us(50);     // Wait 37 us (HD44780U), 43 us (SD1602V)
}

// I2C LCD initialize (I2C液晶の初期化)
void i2c_lcd_init(void){
    // Wait after Power ON : 15ms (HD44780U), 40ms (SD1602V)
    __delay_ms(40);

    // Send Command 0x30
    i2c_lcd_send_command(0x30, LCD_1CYCLE);
    // Wait 4.1ms (HD44780U), 0.037ms (SD1602V)
    __delay_ms(5);

    // Send Command 0x30
    i2c_lcd_send_command(0x30, LCD_1CYCLE);
    // Wait 0.1ms (HD44780U), 0.037ms (SD1602V)
    __delay_us(200);

    // Send Command 0x30
    i2c_lcd_send_command(0x30, LCD_1CYCLE);
    // Wait 0.037ms (HD44780U, SD1602V)
    __delay_us(100);

    // Function set: Set interface to be 4 bits long (only 1 cycle write).
    i2c_lcd_send_command(0x20, LCD_1CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)

    // Function set: DL=0;Interface is 4 bits, N=1; 2 Lines, F=0; 5x8 dots font)
    i2c_lcd_send_command(0x28, LCD_2CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)

    // Display Off: D=0; Display off, C=0; Cursor Off, B=0; Blinking Off
    i2c_lcd_send_command(0x08, LCD_2CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)

    // Display Clear
    i2c_lcd_send_command(LCD_CLEAR, LCD_2CYCLE);
    __delay_ms(2);    // Wait 1.52ms (SD1602V)

    // Entry Mode Set: I/D=1; Increament, S=0; No shift
    i2c_lcd_send_command(0x06, LCD_2CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)

    // Display On, Cursor Off
    i2c_lcd_send_command(0x0c, LCD_2CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)


}

// put one character on LCD (LCDに1文字表示する)
void i2c_lcd_putch(const unsigned char c){
	i2c_lcd_send_data(c);					//文字表示
}

// put string on LCD (LCDに文字列を表示する)
void i2c_lcd_puts(const unsigned char *s){
	while(*s)				//文字取り出し
		i2c_lcd_send_data(*s++);			//文字表示
}

// send command to I2C LCD (I2C液晶にコマンドを送信する)
void i2c_lcd_cmd(const unsigned char cmd)
{
	i2c_lcd_send_command(cmd, LCD_2CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)
}

// clear screen of I2C LCD
void i2c_lcd_clear(void)
{
	i2c_lcd_send_command(LCD_CLEAR, LCD_2CYCLE);
    __delay_ms(2);    // Wait 1.52ms (SD1602V)
}

// set cursor position of I2C LCD (I2C液晶 カーソル位置の指定)
void i2c_lcd_set_cursor_pos(const unsigned char pos)
{
	// line one : pos=0 ...
	// line two : pos=0x40 ...
	i2c_lcd_send_command(0x80 | pos, LCD_2CYCLE);
    __delay_us(100);    // Wait 0.037ms (HD44780U, SD1602V)
}
