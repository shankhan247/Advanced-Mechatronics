#include<xc.h>           // processor SFR definitions
#include<sys/attribs.h>  // __ISR macro
#include<stdio.h> 
#include "i2c_master_noint.h"
#include "ssd1306.h"
#include "font.h"
#include "imu.h"

// DEVCFG0
#pragma config DEBUG = OFF // disable debugging
#pragma config JTAGEN = OFF // disable jtag
#pragma config ICESEL = ICS_PGx1 // use PGED1 and PGEC1
#pragma config PWP = OFF // disable flash write protect
#pragma config BWP = OFF // disable boot write protect
#pragma config CP = OFF // disable code protect

// DEVCFG1
#pragma config FNOSC = PRIPLL // use primary oscillator with pll
#pragma config FSOSCEN = OFF // disable secondary oscillator
#pragma config IESO = OFF // disable switching clocks
#pragma config POSCMOD = HS // high speed crystal mode
#pragma config OSCIOFNC = OFF // disable clock output
#pragma config FPBDIV = DIV_1 // divide sysclk freq by 1 for peripheral bus clock
#pragma config FCKSM = CSDCMD // disable clock switch and FSCM
#pragma config WDTPS = PS1048576 // use largest wdt
#pragma config WINDIS = OFF // use non-window mode wdt
#pragma config FWDTEN = OFF // wdt disabled
#pragma config FWDTWINSZ = WINSZ_25 // wdt window at 25%

// DEVCFG2 - get the sysclk clock to 48MHz from the 8MHz crystal
#pragma config FPLLIDIV = DIV_2 // divide input clock to be in range 4-5MHz
#pragma config FPLLMUL = MUL_24 // multiply clock after FPLLIDIV
#pragma config FPLLODIV = DIV_2 // divide clock after FPLLMUL to get 48MHz

// DEVCFG3
#pragma config USERID = 0 // some 16bit userid, doesn't matter what
#pragma config PMDL1WAY = OFF // allow multiple reconfigurations
#pragma config IOL1WAY = OFF // allow multiple reconfigurations

void write(unsigned char address, unsigned char rgstr, unsigned char data);
unsigned char read(unsigned char write_address, unsigned char read_address, unsigned char rgstr);
double drawLetter(unsigned char letter, unsigned char x, unsigned char y);
void bar(unsigned char x_inc,unsigned char y_inc);

int main() {

    __builtin_disable_interrupts(); // disable interrupts while initializing things

    // set the CP0 CONFIG register to indicate that kseg0 is cacheable (0x3)
    __builtin_mtc0(_CP0_CONFIG, _CP0_CONFIG_SELECT, 0xa4210583);

    // 0 data RAM access wait states
    BMXCONbits.BMXWSDRM = 0x0;

    // enable multi vector interrupts
    INTCONbits.MVEC = 0x1;

    // disable JTAG to get pins back
    DDPCONbits.JTAGEN = 0;

    // do your TRIS and LAT commands here
    TRISAbits.TRISA4 = 0;
    LATAbits.LATA4 = 0;

    __builtin_enable_interrupts();
    
    unsigned char write_address = 0xD6;
    unsigned char read_address = 0xD7;
    unsigned char reg = 0x00;
    signed short s_data[7];
    
    i2c_master_setup();
    imu_setup();
    ssd1306_setup();
    
    ssd1306_clear();
    ssd1306_update();
    
    while (1) {
        // use _CP0_SET_COUNT(0) and _CP0_GET_COUNT() to test the PIC timing
        // remember the core timer runs at half the sysclk
        _CP0_SET_COUNT(0);
        //ssd1306_drawPixel(0,0,1);
        //ssd1306_update();
        if (PORTAbits.RA4 == 0) {
            while(_CP0_GET_COUNT() < 24000000/2) {
                LATAbits.LATA4 = 1;
            }
        }
        
        else {
            //ssd1306_drawPixel(0,0,0);
            //ssd1306_update();
            while(_CP0_GET_COUNT() < 24000000/2) {
                LATAbits.LATA4 = 0;
            }
        }
        
        imu_read(0x20, s_data, 7);
        
        unsigned char x_inc, y_inc;
        x_inc = -1*(s_data[4]/500);
        y_inc = -1*(s_data[5]/500);
        
        
        //unsigned char let;
        unsigned char let[50];
        unsigned char m = 0;
        unsigned char x,y;
        double t;
        unsigned char t_print[50];
        sprintf(let,"g: %d %d %d", s_data[1], s_data[2], s_data[3]);
        x = 0;
        y = 0;
        while(let[m] != 0) {
            t = drawLetter(let[m],x,y);
            m++;
            x = x+5;
            if (x == 125) {
                x = 0;
                y = y+8;
            }
            //y = y+8;
        }
        sprintf(t_print,"a: %d %d %d", s_data[4], s_data[5], s_data[6]);
        m = 0;
        y = y+8;
        x = 0;
        while(t_print[m] != 0) {
            t = drawLetter(t_print[m],x,y);
            m++;
            x = x+5;
            if (x == 125) {
                x = 0;
                y = y+8;
            }
            //y = y+8;
        }
        
        sprintf(t_print,"t: %d", s_data[0]);
        m = 0;
        y = y+8;
        x = 0;
        while(t_print[m] != 0) {
            t = drawLetter(t_print[m],x,y);
            m++;
            x = x+5;
            if (x == 125) {
                x = 0;
                y = y+8;
            }
            //y = y+8;
        }
        
         
        
        
        //bar(x_inc, y_inc);
        
        
    }
}

void bar(unsigned char x_inc,unsigned char y_inc) {
    unsigned char i;
    unsigned char x = 62;
    unsigned char y = 12;
    unsigned char color = 1;
    
    for (i = 0; i <= x_inc; ++i) {
        x = x + i;
        ssd1306_drawPixel(x,y,color);
        ssd1306_update();
    }
}

double drawLetter(unsigned char letter, unsigned char x, unsigned char y) {
    unsigned char j = 0;
    unsigned char k = 0;
    unsigned char color;
    unsigned char y_init = y;
    double time;
    double fps;
    _CP0_SET_COUNT(0);   
    for (j = 0; j < 5; ++j) { 
        for (k = 0; k < 8; ++k) {   
            _CP0_SET_COUNT(0);  
            color = (ASCII[letter - 0x20][j] >> k) & 1;
             ssd1306_drawPixel(x,y,color);
             ssd1306_update();
             time = _CP0_GET_COUNT();
             ++y;
        }
        ++x;
        y = y_init;
    }
    fps = 48000000/time;
    return fps;
}
