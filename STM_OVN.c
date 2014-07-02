/***********************************************************************
    2011-14 (C) Alex Dobrianski OVEN CONTROL MODULE
    works with OpenLog https://github.com/nseidle/OpenLog/wiki

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>

    Design and development by Team "Plan B" is licensed under 
    a Creative Commons Attribution-ShareAlike 3.0 Unported License.
    http://creativecommons.org/licenses/by-sa/3.0/ 
************************************************************************/
/***********************************************************************
************************************************************************/
#define MY_UNIT '6' 
#define NOT_USE_COM1 1


//#define ALLOW_RELAY_TO_NEW
#ifdef __18CXX
#ifdef __16F88
#define _16F88 1
#endif
#ifdef __16F784
#define _16F884 1
#endif

#ifdef __16F884
#define _16F884 1
#endif
#ifdef __16F724 // the same as 16F884
#define _16F884 1
#endif
#ifdef __18F2321
#define _18F2321 1
#endif
#endif

// will be responce on command "=<unit><cmd>"
#define RESPONCE_ON_EQ

// CMD switch processed in interrupt
#define NEW_CMD_PROC 1

// sync clock / timeral  support
//#define SYNC_CLOCK_TIMER  



////////////////////////////////////////////////////////////////////////
// disable I2C proc
////////////////////////////////////////////////////////////////////////
#define NO_I2C_PROC 1




#include "commc0.h"


//
// additional code:

#pragma rambank 2

#include "commc1.h"
unsigned char CallBkComm(void); // return 1 == process queue; 0 == do not process; 
                                // 2 = do not process and finish process 3 == process and finish internal process
                                // in case 0 fucntion needs to pop queue byte by itself

unsigned char CallBkI2C(void); // return 1 == process queue; 0 == do not process; 
                               // 2 = do not process and finish process 3 == process and finish internal process
                               // in case 0 fucntion needs to pop queue byte by itself
unsigned char CallBkMain(void); // 0 = do continue; 1 = process queues
void Reset_device(void);
void ShowMessage(void);
void ProcessCMD(unsigned char bByte);
unsigned char getchI2C(void);
#ifndef NOT_USE_COM1
void putch(unsigned char simbol);
void putchWithESC(unsigned char simbol);
unsigned char getch(void);
void putch_main(void);
#endif
void main()
{
    unsigned char bWork;
    /*if (POR_) // this is can be sync of a timer from MCLR
    {
        if (SetSyncTime)
        {
            TMR1L = 0; // must be delay in 2MHz clock
            TMR1H = 0;
            TMR130 = setTMR130;
            TMR1SEC = setTMR1SEC;
            TMR1MIN = setTMR1MIN;
            TMR1HOUR = setTMR1HOUR;
            TMR1DAY = setTMR1DAY;
        }
    }*/
    
    Reset_device();
    // needs to check what is it:

    //if (TO) // Power up or MCLR
    {
       // Unit == 1 (one) is ADC and unit == 2 (two) is DAC 
        // Unit == 3 Gyro
        //UnitADR = '1';
        UnitADR = '2'; // mem/ camera/ backup comm/ unit 2
        //UnitADR = '4';
#include "commc6.h"

    }
    PEIE = 1;    // bit 6 PEIE: Peripheral Interrupt Enable bit
                 // 1 = Enables all unmasked peripheral interrupts
                 // 0 = Disables all peripheral interrupts
    GIE = 1;     // bit 7 GIE: Global Interrupt Enable bit
                 // 1 = Enables all unmasked interrupts
                 // 0 = Disables all interrupts
    RBIF = 0;
    ShowMessage();
    bitset(PORTA,3);
    //bitset(SSPCON,4);  // set clock high;
#include "commc7.h"
///////////////////////////////////////////////////////////////////////

} // at the end will be Sleep which then continue to main


#define SPBRG_SPEED SPBRG_57600

#include "commc2.h"
// additional code:

void ProcessCMD(unsigned char bByte)
{
#ifndef NOT_USE_COM1
    unsigned char bWork;
    long wWork;
    long *FileLen;
    if (!Main.getCMD) // CMD not receved et.
    {

#include "commc3.h"
       
// additional code:

#include "commc4.h"
// additional code:
        //else if (bByte == 'F') // set file name
        
SKIP_BYTE:
    } // do not confuse: this is a else from Main.getCMD == 1
#endif // #ifndef NOT_USE_COM1
}

unsigned char CallBkComm(void) // return 1 == process queue; 0 == do not process; 
                               // 2 = do not process and finish process 3 == process and finish internal process
{                              // in case 0 fucntion needs to pop queue byte by itself
    unsigned char bBy;
    return 1; // this will process next byte 
}
unsigned char CallBkI2C(void)
{
    return 1;
}
unsigned char CallBkMain(void) // 0 = do continue; 1 = process queues
{
    //if (Timer0Waiting)
    //{
    //    if (Timer0Fired)
    //        Timer0Waiting = 0;
    //    else
    //        return 0;
    //}
    return 1;
}
#pragma codepage 1
void Reset_device(void)
{
#ifdef _16F88
//                      ===================
//   RA2/AN2/CVREF/   1 ||4|           |2|| 18  RA1/AN1
//   RA3/AN3/VREF+/   2 ||0|        |hold|| 17  RA0/AN0
//   RA4/AN4/T0CKI/   3 ||ENTER|    \1min\| 16  RA7/OSC1/CLKI
//   RA5/MCLR/VPP     4 | mclr       |hit|| 15  RA6/OSC2/CLKO
//   VSS              5 | ground      +5V | 14  VDD
//   RB0/INT/CCP1(1)  6 | \32min\   PGD   | 13  RB7/AN6/PGD/
//   RB1/SDI/SDA      7 | \16min\   PGC   | 12  RB6/AN5/PGC/
//   RB2/SDO/RX/DT    8 | \8min\    \2min\| 11  RB5/SS/TX/CK
//   RB3/PGM/CCP1(1)  9 | PGR(LVP)  \4min\| 10  RB4/SCK/SCL
//                      ===================
    ANSEL =   0b00000000; //Turn pins to Digital instead of Analog

    TRISA = 0b10100000;  //0 = Output, 1 = Input 
    PORTA = 0b00000000;  // SSCS set high

    TRISB = 0b11111111;  //0 = Output, 1 = Input 
    PORTB = 0b00000000;  

//      REGISTER 4-2: OSCCON: OSCILLATOR CONTROL REGISTER (ADDRESS 8Fh) 
//      Note: The OST is only used when switching to
//      XT, HS and LP Oscillator modes.
//      U-0 R/W-0 R/W-0 R/W-0 R-0 R/W-0 R/W-0 R/W-0 
//      — IRCF2 IRCF1 IRCF0 OSTS(1) IOFS SCS1 SCS0
//       0                              bit 7 bit 0
//       0                              bit 7 Unimplemented: Read as ‘0’
//       0                              bit 6-4 IRCF<2:0>: Internal RC Oscillator Frequency Select bits
//        000                           = 31.25 kHz
//        001                           = 125 kHz
//        010                           = 250 kHz
//        011                           = 500 kHz
//        100                           = 1 MHz
//        101                           = 2 MHz
//        110                           = 4 MHz
//        111                           = 8 MHz
//                                      bit 3 OSTS: Oscillator Start-up Time-out Status bit(1)
//            1                         = Device is running from the primary system clock
//            0                         = Device is running from T1OSC or INTRC as a secondary system clock
//                                      Note 1: Bit resets to ‘0’ with Two-Speed Start-up mode and LP, XT or HS selected as the
//                                      oscillator mode.
//                                      bit 2 IOFS: INTOSC Frequency Stable bit
//             1                        = Frequency is stable
//             0                        = Frequency is not stable
//                                      bit 1-0 SCS<1:0>: Oscillator Mode Select bits
//              00                      = Oscillator mode defined by FOSC<2:0>
//              01                      = T1OSC is used for system clock
//              10                      = Internal RC is used for system clock
//              11                      = Reserved
//       0111 1100
    OSCCON =  0b01111100;

//      REGISTER 4-1: OSCTUNE: OSCILLATOR TUNING REGISTER (ADDRESS 90h) 
//      U-0 U-0 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0 R/W-0 
//      — — TUN5 TUN4 TUN3 TUN2 TUN1 TUN0
//      bit 7 bit 0
//      bit 7-6 Unimplemented: Read as ‘0’
//      bit 5-0 TUN<5:0>: Frequency Tuning bits
//      011111 = Maximum frequency
//      011110 = 
//      •
//      •
//      •
//      000001 = 
//      000000 = Center frequency. Oscillator module is running at the calibrated frequency.
//      111111 = 
//      •
//      •
//      •
//      100000 = Minimum frequency
#endif 

    //RBIF = 0;
    //RBIE = 1;
#ifndef NOT_USE_COM1    
    enable_uart(); //Setup the hardware UART for 20MHz at 9600bps
#endif
    // next two bits has to be set after all intialization done
    //PEIE = 1;    // bit 6 PEIE: Peripheral Interrupt Enable bit
                 // 1 = Enables all unmasked peripheral interrupts
                 // 0 = Disables all peripheral interrupts
    //GIE = 1;     // bit 7 GIE: Global Interrupt Enable bit
                 // 1 = Enables all unmasked interrupts
                 // 0 = Disables all interrupts
#ifndef NO_I2C_PROC
    enable_I2C();
#endif
    TIMER0_INT_FLG = 0; // clean timer0 interrupt
    TIMER0_INT_ENBL = 0; // diasable timer0 interrupt
    TMR1IF = 0; // clean timer0 interrupt
    TMR1IE = 0; // diasable timer0 interrupt
}


void ShowMessage(void)
{
#ifndef NOT_USE_COM1
    // if message initiated by unit needs to check then it is possible to do:
    while(!Main.prepStream) // this will wait untill no relay message
    {
    }
    // in a case of a CMD replay it is safly to skip that check - unit allow to send message in CMD mode
    putch(UnitADR);  // this message will circle over com and will be supressed by unit
    Puts("~");
    putch(UnitADR);
#endif
}

#include "commc8.h"
