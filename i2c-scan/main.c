/*
 * I2C address scanner for Microchip PIC 12F1822
 * File:   main.c
 */

#include <stdio.h>
#include <stdlib.h>

#include <xc.h>
#include "i2c-lib.h"
#include "serial-lib.h"

/* PIC Configuration 1 */
__CONFIG(FOSC_INTOSC &	// INTOSC oscillator: I/O function on CLKIN pin
		WDTE_OFF &		// WDT(Watchdog Timer) disabled
		PWRTE_ON &		// PWRT(Power-up Timer) disabled
		MCLRE_OFF &		// MCLR pin function is digital input
		CP_OFF &		// Program memory code protection is disabled
		CPD_OFF &		// Data memory code protection is disabled
		BOREN_OFF &		// BOR(Brown-out Reset) disabled
		CLKOUTEN_OFF &	// CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin
		IESO_OFF &		// Internal/External Switchover mode is disabled
		FCMEN_OFF);		// Fail-Safe Clock Monitor is disabled

/* PIC Configuration 2 */
__CONFIG(WRT_OFF &		// Flash Memory Self-Write Protection : OFF
		VCAPEN_OFF &	// VDDCORE pin functionality is disabled
		PLLEN_OFF &		// 4x PLL disabled
		STVREN_ON &		// Stack Overflow or Underflow will not cause a Reset
		BORV_HI &		// Brown-out Reset Voltage Selection : High Voltage
		DEBUG_OFF &		// In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins
		LVP_OFF);		// Low-voltage programming : disable


#ifndef _XTAL_FREQ
	/* 例：4MHzの場合、4000000 をセットする */
	#define _XTAL_FREQ 4000000
#endif

// スキャンするI2Cアドレスの範囲
#define I2C_ADDR_FIRST	0b0000111	// 0～0b111はシステムで予約されている
#define I2C_ADDR_LAST	0b1111000	// 0b11110000以降はシステムで予約されている

/*
 * 
 */
/* printf関数の出力先のスタブ関数を定義する */
void putch(unsigned char ch){
	i2c_lcd_putch(ch);
//	rs232c_putch(ch);
	return;
}

int main(int argc, char** argv) {
	// 基本機能の設定
	OSCCON = 0b01101010;		// 内部オシレーター 4MHz
	TRISA = 0b00101111;			// IOポートRA0(AN0),RA1(SCL),RA2(SDA),RA5(RX)を入力モード（RA3は入力専用）、RA4(TX)を出力モード
	APFCONbits.RXDTSEL = 1;		// シリアルポート RXをRA5ピンに割付
	APFCONbits.TXCKSEL = 1;		// シリアルポート TXをRA4ピンに割付
	ANSELA = 0b00000001;		// A/D変換をAN0を有効、AN1,AN2,AN4を無効
	PORTA = 0;
	INTCONbits.PEIE = 1;		// 割り込み機能をON （RX受信割り込みを使う場合）
	INTCONbits.GIE = 1;			// 割り込み機能をON （RX受信割り込みを使う場合）

	i2c_enable();
	OPTION_REGbits.nWPUEN = 0;	// I2C プルアップ抵抗 有効
	WPUA = 0b00000110;			// pull-up (RA1=SCL, RA2=SDA pull-up enable)

	ADCON0 = 0;					// AN0選択, A/D機能停止
	ADCON1 = 0b10010000;		// 変換結果右詰, クロックFOSC/4, 比較対象VDD

	// 0.5秒待つ
	__delay_ms(500);

	rs232c_init(9600);

	while(1)
	{
		int i,j;
		char temp_str[6];	// 表示用に数値をフォーマットするために利用
		unsigned int found_addr[10];	// 見つかったi2cアドレス
		for(j=0; j<10; j++) found_addr[j] = 0;
		int found_addr_count = 0;		// 見つかったアドレスの個数

		i2c_lcd_init();
		i2c_lcd_puts("I2C addr scan");
		rs232c_puts("\r\nI2C addr scan\r\n");

		for(i=0; i<=0xff; i+=0x10)
		{

			for(j=0; j<=0xf; j++)
			{
				unsigned int i2c_addr = i+j;
				i2c_lcd_set_cursor_pos(0x40);		// 2行目の1文字目に移動
				printf("scan %02X-%02X", i, i+0xf);
				// システムで予約された範囲のアドレスはスキャンしない
				if(i2c_addr <= I2C_ADDR_FIRST || i2c_addr >= I2C_ADDR_LAST)
				{
					rs232c_puts(" -- ");
				}
				else
				{
					i2c_start();
					i2c_send_byte(i2c_addr<<1);	// アドレスを1ビット左にシフトし、末尾にR/Wビット（Write=0）を付与
					if(SSP1CON2bits.ACKSTAT){
						// ACKが検出されなかった場合
						sprintf(temp_str, " %02X ", i2c_addr);
						rs232c_puts(temp_str);
					}
					else{
						// ACKが検出されなかった場合
						sprintf(temp_str, "[%02X]", i2c_addr);
						rs232c_puts(temp_str);
						found_addr[found_addr_count++] = i2c_addr;
					}
					i2c_stop();

					// I2Cバスを切・入して通信途中で放置した動作を一旦リセットする
					i2c_disable();
					__delay_ms(20);
					i2c_enable();
				}
			}
			rs232c_puts("\r\n");
		}

		// 最終行に、見つかったI2Cアドレス一覧を表示
		i2c_lcd_set_cursor_pos(0x40);		// 2行目の1文字目に移動
		for(j=0;j<16;j++) i2c_lcd_putch(' ');	// 2行目クリア
		i2c_lcd_set_cursor_pos(0x40);		// 2行目の1文字目に移動
		rs232c_puts("found:");
		for(j=0; j<found_addr_count; j++)
		{
			printf("%02X,", found_addr[j]);	// 見つかったアドレスをLCDに表示
			sprintf(temp_str, "%02X, ", found_addr[j]);
			rs232c_puts(temp_str);
		}
		rs232c_puts("\r\n");

		// RS232C入力またはRA3ボタンが押されるまで待つ
		while(1)
		{
			// 0.5秒待つ
			__delay_ms(500);

			// RS232Cで改行を伴う文字列入力を検知した場合
			if(flag_rs232c_received == 1)
			{
				rs232c_buffer[0] = (char)0;
				flag_rs232c_received = 0;
				break;
			}

			// RA3ボタンが押された場合
			if(!PORTAbits.RA3)
			{
				break;
			}
		}
	}

	return (EXIT_SUCCESS);

}

