/*
 * File:   i2c-lib.c   for Microchip PIC XC8
 */

#include <stdio.h>
#include <stdlib.h>

#include <xc.h>
#include "i2c-lib.h"

#ifndef _XTAL_FREQ
	/* 例：4MHzの場合、4000000 をセットする */
	#define _XTAL_FREQ 4000000
#endif

// (7_bit_i2c_addr << 1) + R/W_bit
//  : Strawberry Linux LCDのアドレスは0b0111110 = 0x3e
#define LCD_I2C_ADDR	0x3e
#define CONTRAST		0x28

#define EEPROM_I2C_ADDR	0b1010001

// I2Cバスを有効化
void i2c_enable(void)
{
	SSP1STAT = 0b10000000;		// I2C 100kHz
	SSP1ADD = 9;				// I2Cバス Baud rate,  4MHz/((SSP1ADD + 1)*4) = 100kHz
	SSP1CON1 = 0b00101000;		// I2C有効, Master Mode
}

// I2Cバスを無効化
void i2c_disable(void)
{
	SSP1CON1 = 0b00001000;		// I2C無効, Master Mode
}

// I2C書き込みサイクルの開始（Start Conditionの発行）
void i2c_start(void)
{
	SSP1CON2bits.SEN = 1;	//  Start Condition Enabled bit
	i2c_wait();
}

// I2C書き込みサイクルの開始（Repeat Start Conditionの発行）
void i2c_repeat_start(void)
{
	SSP1CON2bits.RSEN = 1;	//  Start Condition Enabled bit
	i2c_wait();
}

// I2C書き込みサイクルの終了（Stop Conditionの発行）
void i2c_stop(void)
{
	SSP1CON2bits.PEN = 1;	// Stop Condition Enable bit
	i2c_wait();
}

// I2C通信がビジー状態を脱するまで待つ
void i2c_wait(void)
{
	while ( ( SSP1CON2 & 0x1F ) || ( SSP1STATbits.R_nW ) );

}

// I2Cバスにデータを送信（1バイト分）
void i2c_send_byte(unsigned char data)
{
	SSP1BUF = data;
	i2c_wait();
}

//I2Cバスからデータ受信
// ack=1 : 受信後ACKを送信し、次のデータを送るようスレーブデバイスに指示
// ack=0 : 受信後NO_ACKを送信し、これ以上受信しないことをスレーブデバイスに指示
unsigned char i2c_read_byte(int ack)
{
	SSP1CON2bits.RCEN = 1;
	i2c_wait();
	unsigned char data = SSP1BUF;
	i2c_wait();

	if(ack) SSP1CON2bits.ACKDT = 0;		// ACK
	else SSP1CON2bits.ACKDT = 1;		// NO_ACK

	SSP1CON2bits.ACKEN = 1;

	i2c_wait();
	return data;
}

// I2C液晶にコマンドを送信する
void i2c_lcd_send_command(unsigned char i2c_addr, unsigned char data)
{
	i2c_start();
	i2c_send_byte(i2c_addr<<1);	// アドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte(0);			// Control Byte : Co=0,RS=0 （Instruction Writeモード）
	i2c_send_byte(data);
	i2c_stop();

	/* ClearかHomeか */
	if((data == 0x01)||(data == 0x02))
		__delay_ms(2);			// 2msec待ち
	else
		__delay_us(30);			// 30μsec待ち
}

// I2C液晶にデータを送信する
void i2c_lcd_send_data(unsigned char i2c_addr, unsigned char data)
{
	i2c_start();
	i2c_send_byte(i2c_addr<<1);	// アドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte(0x40);		// Control Byte : Co=0,RS=1 （Data Writeモード）
	i2c_send_byte(data);
	i2c_stop();

	__delay_us(30);
}

// I2C液晶からデータを得る
unsigned char i2c_lcd_read_data(unsigned char i2c_addr, unsigned int pos)
{
	i2c_start();
	i2c_send_byte(i2c_addr<<1);	// アドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte(0);			// Control Byte : Co=0,RS=0 （Instruction Writeモード）
	i2c_send_byte(0x80 | pos);	// カーソル位置の設定
	i2c_send_byte(i2c_addr<<1 | 0x01);	// アドレスを1ビット左にシフトし、末尾にR/Wビット（Read=1）を付与
	i2c_send_byte(0x40);		// Control Byte : Co=0,RS=1 （Data Readモード）
	unsigned char data = i2c_read_byte(0);
	data = i2c_read_byte(0);
	i2c_stop();

	__delay_us(30);

	return data;
}

// I2C液晶を初期化する
void i2c_lcd_init(void)
{
	__delay_ms(40);
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x38);				// 8bit 2line Normal mode
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x39);				// 8bit 2line Extend mode
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x14);				// OSC 183Hz BIAS 1/5
	/* コントラスト設定 */
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x70 | (CONTRAST & 0x0F));
//	i2c_lcd_send_command(LCD_I2C_ADDR, 0x5C | ((CONTRAST >> 4) & 0x03));	// 3.3V Bon=1
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x58 | ((CONTRAST >> 4) & 0x03));	// 5.0V Bon=0
//	lcd_cmd(0x6A);				// Follower for 5.0V
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x6C);				// Follwer for 3.3V
	__delay_ms(300);
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x38);				// Set Normal mode
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x0C);				// Display On
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x01);				// Clear Display


}

// I2C液晶に1文字書き込む
void i2c_lcd_putch(char c)
{
	i2c_lcd_send_data(LCD_I2C_ADDR, c);					//文字表示
}

// I2C液晶に文字列を書き込む
void i2c_lcd_puts(const char *s)
{
	while(*s)				//文字取り出し
		i2c_lcd_send_data(LCD_I2C_ADDR, *s++);			//文字表示
}

// I2C液晶から文字を読み込む
unsigned char i2c_lcd_get(unsigned int pos)
{
	return i2c_lcd_read_data(LCD_I2C_ADDR, pos);
}

// I2C液晶にコマンドを送信する
void i2c_lcd_cmd(unsigned char cmd)
{
	i2c_lcd_send_command(LCD_I2C_ADDR, cmd);
}

// I2C液晶の表示をクリアする
void i2c_lcd_clear(void)
{
	i2c_lcd_send_command(LCD_I2C_ADDR, 1);
}

// I2C液晶のカーソル位置を指定する
void i2c_lcd_set_cursor_pos(unsigned int pos)
{
	// 1行目 : pos=0 ...
	// 2行目 : pos=0x40 ...
	i2c_lcd_send_command(LCD_I2C_ADDR, 0x80 | pos);
}

// EEPROMに1バイト書き込む
void i2c_eeprom_write_byte(unsigned int addr, unsigned char data)
{
	i2c_start();
	i2c_send_byte(EEPROM_I2C_ADDR << 1);	// i2cアドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte((unsigned char)(addr>>8));		// アドレスの上位バイト
	i2c_send_byte((unsigned char)(addr&0xff));		// アドレスの下位バイト
	i2c_send_byte(data);
	i2c_stop();

	__delay_us(30);

	return;
}

// EEPROMに1ページ分（32バイト）書き込む
void i2c_eeprom_write_32byte(unsigned int addr, unsigned char *data)
{
	i2c_start();
	i2c_send_byte(EEPROM_I2C_ADDR << 1);	// i2cアドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte((unsigned char)(addr>>8));		// アドレスの上位バイト
	i2c_send_byte((unsigned char)(addr&0xff));		// アドレスの下位バイト
	for(int i=0; i<32; i++) i2c_send_byte(data[i]);
	i2c_stop();

	__delay_us(30);

	return;
}

// EEPROMから1バイト読み出す
unsigned char i2c_eeprom_read_byte(unsigned int addr)
{
	i2c_start();
	i2c_send_byte(EEPROM_I2C_ADDR<<1);	// i2cアドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte((unsigned char)(addr>>8));		// アドレスの上位バイト
	i2c_send_byte((unsigned char)(addr&0xff));		// アドレスの下位バイト
	i2c_repeat_start();
	i2c_send_byte((EEPROM_I2C_ADDR<<1) | 1);	// i2cアドレスを1ビット左にシフトし、末尾にR/Wビット（Read=1）を付与
	unsigned char data = i2c_read_byte(0);	// 読み込み後NO_ACK
	i2c_stop();

	__delay_us(30);

	return(data);
}

// EEPROMから1ページ分（32バイト）読み出す
void i2c_eeprom_read_32byte(unsigned int addr, unsigned char *buf)
{
	i2c_start();
	i2c_send_byte(EEPROM_I2C_ADDR<<1);	// i2cアドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
	i2c_send_byte((unsigned char)(addr>>8));		// アドレスの上位バイト
	i2c_send_byte((unsigned char)(addr&0xff));		// アドレスの下位バイト
	i2c_repeat_start();
	i2c_send_byte((EEPROM_I2C_ADDR<<1) | 1);	// i2cアドレスを1ビット左にシフトし、末尾にR/Wビット（Read=1）を付与
	for(int i=0; i<31; i++) buf[i] = i2c_read_byte(1); 	// 読み込み後ACK
	buf[31] = i2c_read_byte(0);	// 読み込み後NO_ACK
	i2c_stop();

	__delay_us(30);

	return;
}

