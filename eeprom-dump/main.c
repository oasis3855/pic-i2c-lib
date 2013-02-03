/*
 * I2C EEPROM dump/write utility for Microchip PIC 12F1822
 * File:   main.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <xc.h>
#include "common-lib.h"
#include "i2c-lib.h"
#include "i2c-eepromlib.h"
#include "serial-lib.h"

/* PIC Configuration 1 */
__CONFIG(FOSC_INTOSC &  // INTOSC oscillator: I/O function on CLKIN pin
        WDTE_OFF &      // WDT(Watchdog Timer) disabled
        PWRTE_ON &      // PWRT(Power-up Timer) disabled
        MCLRE_OFF &     // MCLR pin function is digital input
        CP_OFF &        // Program memory code protection is disabled
        CPD_OFF &       // Data memory code protection is disabled
        BOREN_OFF &     // BOR(Brown-out Reset) disabled
        CLKOUTEN_OFF &  // CLKOUT function is disabled. I/O or oscillator function on the CLKOUT pin
        IESO_OFF &      // Internal/External Switchover mode is disabled
        FCMEN_OFF);     // Fail-Safe Clock Monitor is disabled

/* PIC Configuration 2 */
__CONFIG(WRT_OFF &      // Flash Memory Self-Write Protection : OFF
        VCAPEN_OFF &    // VDDCORE pin functionality is disabled
        PLLEN_OFF &     // 4x PLL disabled
        STVREN_ON &     // Stack Overflow or Underflow will not cause a Reset
        BORV_HI &       // Brown-out Reset Voltage Selection : High Voltage
        DEBUG_OFF &     // In-Circuit Debugger disabled, ICSPCLK and ICSPDAT are general purpose I/O pins
        LVP_OFF);       // Low-voltage programming : disable


#ifndef _XTAL_FREQ
    /* 例：4MHzの場合、4000000 をセットする */
    #define _XTAL_FREQ 4000000
#endif

/*
 * 
 */

void parse_hex_param(unsigned char *buf, unsigned int *val_1, unsigned int *val_2, unsigned int *val_3)
{
    char *ptr;
    const char *sep = ",";
    unsigned char i = 0;

    ptr = strtok(buf, sep);
    while(ptr){
        switch(i++){
            case 0: *val_1 = xtoi(ptr); break;
            case 1: *val_2 = xtoi(ptr); break;
            case 2: *val_3 = xtoi(ptr); break;
        }
        ptr = strtok(NULL, sep);
    }

}

void print_crlf(void){
    rs232c_puts("\r\n");
}

int main(int argc, char** argv) {
    // 基本機能の設定
    OSCCON = 0b01101010;        // 内部オシレーター 4MHz
    TRISA = 0b00101111;         // IOポートRA0(AN0),RA1(SCL),RA2(SDA),RA5(RX)を入力モード（RA3は入力専用）、RA4(TX)を出力モード
    APFCONbits.RXDTSEL = 1;     // シリアルポート RXをRA5ピンに割付
    APFCONbits.TXCKSEL = 1;     // シリアルポート TXをRA4ピンに割付
    ANSELA = 0b00000001;        // A/D変換をAN0を有効、AN1,AN2,AN4を無効
    PORTA = 0;
    INTCONbits.PEIE = 1;        // 割り込み機能をON （RX受信割り込みを使う場合）
    INTCONbits.GIE = 1;         // 割り込み機能をON （RX受信割り込みを使う場合）

    i2c_enable();
    OPTION_REGbits.nWPUEN = 0;  // I2C プルアップ抵抗 有効
    WPUA = 0b00000110;          // pull-up (RA1=SCL, RA2=SDA pull-up enable)

    ADCON0 = 0;                 // AN0選択, A/D機能停止

    // 0.5秒待つ
    __delay_ms(500);

    rs232c_init(9);     // init 9600bps

    unsigned int start_addr = 0;
    unsigned int page_count = 2;
    unsigned int temp_num = 0;
    unsigned char write_data = (char)0;
    unsigned char buf_strfmt[5];   // 数値の文字列変換用

    // 動作モード
    // R: dump with addr, H: dump without addr, P: write page, B: write byte
    unsigned char mode_sw = 'R';

    while(1)
    {

        print_crlf();
        rs232c_puts("EEPROM util");
        print_crlf();
        rs232c_puts("[R|H][adr],[pg], W[adr],[dat], M[adr],[dat],[cnt]");
        print_crlf();

        switch(mode_sw)
        {
            case 'R':
            case 'H':
                rs232c_puts("dmp"); break;
            case 'W':
            case 'M':
                rs232c_puts("wrt"); break;
        }
        print_crlf();

        INTCONbits.PEIE = 0;        // 割り込み機能を一旦OFF
        INTCONbits.GIE = 0;         // 割り込み機能を一旦OFF


        unsigned char i;

        if(mode_sw == 'R' || mode_sw == 'H')
        {
            rs232c_puts("adr ");
            rs232c_puts(uint_to_hex4(start_addr, buf_strfmt));
            rs232c_puts(",pg ");
            rs232c_puts(uint_to_hex4(page_count, buf_strfmt));
            print_crlf();
            for(int j=0; j<page_count; j++)
            {
                i2c_eeprom_read_32byte(start_addr + (j<<5), rs232c_buffer);
                if(mode_sw == 'R')
                {
                    rs232c_puts(uint_to_hex4(start_addr + (j<<5), buf_strfmt));
                    rs232c_puts(": ");
                }
                for(i=0; i<32; i++)
                {
                    rs232c_puts(uchar_to_hex2(rs232c_buffer[i], buf_strfmt));
                    rs232c_puts(" ");
                    if(i==15)
                    {
                        print_crlf();
                        if(mode_sw == 'R')
                        {
                            rs232c_puts(uint_to_hex4(start_addr + (j<<5) + 16, buf_strfmt));
                            rs232c_puts(": ");
                        }
                    }
                }
                print_crlf();
            }
        }
        else if(mode_sw == 'W' || mode_sw == 'M')
        {
            rs232c_puts("adr ");
            rs232c_puts(uint_to_hex4(start_addr, buf_strfmt));
            if(mode_sw == 'M'){
                rs232c_puts(",cnt ");
                rs232c_puts(uint_to_hex4(page_count, buf_strfmt));
            }
            rs232c_puts(",dat ");
            rs232c_puts(uchar_to_hex2(write_data, buf_strfmt));
            print_crlf();
            if(mode_sw == 'W'){
                i2c_eeprom_write_byte(start_addr, write_data);
            }
            else{
                for(i=0; i<page_count; i++){
                    i2c_eeprom_write_byte(start_addr + i, write_data);
                }
            }
            rs232c_puts("done");
            print_crlf();
        }

        // RS232Cでの受信待機モードに切り替える
        rs232c_buffer[0] = (char)0;
        flag_rs232c_received = 0;
        INTCONbits.PEIE = 1;        // 割り込み機能を再度ON
        INTCONbits.GIE = 1;         // 割り込み機能を再度ON

        start_addr = 0;
        page_count = 0;
        temp_num = 0;
        mode_sw = 'R';

        // RS232C入力またはRA3ボタンが押されるまで待つ
        while(1)
        {
            // 0.5秒待つ
            __delay_ms(500);

            // RS232Cで改行を伴う文字列入力を検知した場合
            if(flag_rs232c_received)
            {
                if(rs232c_buffer[0] == 'R' || rs232c_buffer[0] == 'H')
                {
                    parse_hex_param(rs232c_buffer+1, &start_addr, &page_count, &temp_num);
                    mode_sw = rs232c_buffer[0];
                }
                else if(rs232c_buffer[0] == 'W' || rs232c_buffer[0] == 'M')
                {
                    parse_hex_param(rs232c_buffer+1, &start_addr, &temp_num, &page_count);
                    write_data = (char)(temp_num & 0xff);
                    mode_sw = rs232c_buffer[0];
                }

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

