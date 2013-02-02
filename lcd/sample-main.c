/* 
 * File:   main.c
 *
 * Sample main file for LCD Lib
 */

#include <stdio.h>
#include <stdlib.h>

#include <xc.h>
#include "i2c-lib.h"
#include "i2c-lcdlib-mjkdz.h"


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


/*
 * 
 */

/* printf関数の出力先のスタブ関数を定義する */
void putch(unsigned char ch){
	i2c_lcd_putch(ch);
//	if(flag_serial_enable) rs232c_putch(ch);
	return;
}


int main(int argc, char** argv) {
    // 基本機能の設定
    OSCCON = 0b01101010;        // 内部オシレーター 4MHz
    TRISA = 0b00101111;         // IOポートRA0(AN0),RA1(SCL),RA2(SDA),RA5(RX)を入力モード（RA3は入力専用）、RA4(TX)を出力モード
    ANSELA = 0;                 // A/D変換を無効
    ADCON0 = 0;                 // AN0選択, A/D機能停止
    PORTA = 0;

    INTCONbits.PEIE = 0;        // 割り込み機能をOFF （RX受信割り込みを使う場合）
    INTCONbits.GIE = 0;         // 割り込み機能をOFF （RX受信割り込みを使う場合）

    i2c_enable();
    OPTION_REGbits.nWPUEN = 0;  // I2C プルアップ抵抗 有効
    WPUA = 0b00000110;          // pull-up (RA1=SCL, RA2=SDA pull-up enable)

    // 0.5秒待つ
    __delay_ms(500);


    i2c_lcd_init();

    i2c_lcd_set_cursor_pos(0);
    printf("LCD Test");
        
    for(unsigned int i=0; i<0xfff0; i++){

        i2c_lcd_set_cursor_pos(0x40);
        printf("count %04X", i);

    }

    return (EXIT_SUCCESS);
}

