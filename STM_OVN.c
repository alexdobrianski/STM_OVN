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
unsigned long Timer1Count; 

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
unsigned char Timer0Id;
void SetTimer0(unsigned char iTime)
{
     // with 8MHz it is CLKO = FOSC/ 4 = 2Mhz
     // in case of the prescaller = 111 (1:256) it is 1 interrupt in 0.032768 sec
     // in 1 second it is = 30.517578125 interrupts
    // 20 minutes == 1200 sec = 36621.09375 interrupts
  
    TMR0 = iTime; // time0 can not be stopped - just set it run
     Timer0Id = iTime;
     I2C.Timer0Fired = 0;
     

     TIMER0_INT_FLG = 0; // clean timer0 interrupt
     TIMER0_INT_ENBL = 1;  // enable timer0 interrupt

//    already set:
//     OPTION_REG: OPTION CONTROL REGISTER (ADDRESS 81h, 181h) 
//
//       0                 bit 7 RBPU: PORTB Pull-up Enable bit
//        0                bit 6 INTEDG: Interrupt Edge Select bit
//         0               bit 5 T0CS: TMR0 Clock Source Select bit 
//                            1 = Transition on T0CKI pin 
//                            0 = Internal instruction cycle clock (CLKO) 
//          0              bit 4 T0SE: TMR0 Source Edge Select bit 
//                            1 = Increment on high-to-low transition on T0CKI pin 
//                            0 = Increment on low-to-high transition on T0CKI pin 
//           0             bit 3 PSA: Prescaler Assignment bit 
//                            1 = Prescaler is assigned to the WDT 
//                            0 = Prescaler is assigned to the Timer0 module 
//            111          bit 2-0 PS<2:0>: Prescaler Rate Select bits 
//                            000 1:2
//                            001 1:4
//                            010 1:8
//                            011 1:16
//                            100 1:32
//                            101 1:64
//                            110 1:128
//                            111 1:256
//     OPTION_REG = 0b00000111;
}
void SetTimer1(unsigned long iTime)
{
     // with 8MHz it is CLKO = FOSC/ 4 = 2Mhz
     // in case of the prescaller = 11 (1:8) it is 1 interrupt in 0.262144 sec
     // in 1 second it is = 3.814697265625 interrupts
     // 20 minutes == 4577.63671875 interrupts
     TMR1ON = 0;
     TMR1H = (unsigned char)(iTime>>8);
     TMR1L = (unsigned char)(iTime&0xff);
     TMR1IF = 0; // clean timer1 interrupt
     TMR1IE = 1;  // enable timer1 interrupt
// T1CON: TIMER1 CONTROL REGISTER (ADDRESS 10h)
//      0             bit 7 Unimplemented: Read as ‘0’
//       0            bit 6 T1RUN: Timer1 System Clock Status bit
//                       1 = System clock is derived from Timer1 oscillator
//                       0 = System clock is derived from another source
//        11          bit 5-4 T1CKPS<1:0>: Timer1 Input Clock Prescale Select bits
//                       11 =1:8 Prescale value
//                       10 =1:4 Prescale value
//                       01 =1:2 Prescale value
//                       00 =1:1 Prescale value
//          1         bit 3 T1OSCEN: Timer1 Oscillator Enable Control bit
//                       1 = Oscillator is enabled
//                       0 = Oscillator is shut off (the oscillator inverter is turned off to eliminate power drain)
//           0        bit 2 T1SYNC: Timer1 External Clock Input Synchronization Control bit
//                       if TMR1CS = 1:
//                            1 = Do not synchronize external clock input
//                            0 = Synchronize external clock input
//                       if TMR1CS = 0:
//                           This bit is ignored. Timer1 uses the internal clock when TMR1CS = 0.
//            0       bit 1 TMR1CS: Timer1 Clock Source Select bit
//                       1 = External clock from pin RB6/AN5(1)/PGC/T1OSO/T1CKI (on the rising edge)
//                       0 = Internal clock (FOSC/4) 
//                     Note 1: Available on PIC16F88 devices only.
//             1      bit 0 TMR1ON: Timer1 On bit
//                       1 = Enables Timer1
//                       0 = Stops Timer1
     T1CON = 0b00111001;
}

unsigned long DelaySeconds;
unsigned char StatusTimers;
unsigned long CountSeconds;
unsigned char Status;
//#define MINUTES_20 4577
#define MINUTES_20 5

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
    StatusTimers = 0;
    PEIE = 1;    // bit 6 PEIE: Peripheral Interrupt Enable bit
                 // 1 = Enables all unmasked peripheral interrupts
                 // 0 = Disables all peripheral interrupts
    GIE = 1;     // bit 7 GIE: Global Interrupt Enable bit
                 // 1 = Enables all unmasked interrupts
                 // 0 = Disables all interrupts
    RBIF = 0;
    ShowMessage();
    //bitset(PORTA,3);
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
    unsigned char portA;
    unsigned char portB;
    portA = PORTA;
    portB = PORTB;
    //  RB0  = \32min\
    //  RB1  = \16min\
    //  RB2  = \8min\
    //  RB4  = \4min\
    //  RB5  = \2min\
    //  RA7  = \1min\ & start

    if (portB.0)          // sw1   100000
        DelaySeconds = 32;
    if (portB.1)          // sw2   010000
        DelaySeconds += 16;
    if (portB.2)          // sw3   001000
        DelaySeconds += 8;
    if (portB.4)          // sw4   000100
        DelaySeconds += 4;
    if (portB.5)          // sw5   000010
        DelaySeconds += 2;

    if (StatusTimers ==0) // intializing of the controller
    {
        if (portA.7) //  start timer1   sw 6      000001
        {
            StatusTimers =1;
            Timer1Count = 0;
            SetTimer1(0);
            Status =0;
        }
    }
    else
    {
        if (!portA.7) //  stop timer1
        {
            TMR1ON = 0;
            StatusTimers =0;
            Timer1Count = 0;
        }
        else
        {
            if (StatusTimers == 1)
            { 
                DelaySeconds = DelaySeconds * 218;
                if (Timer1Count > DelaySeconds )
                {
                    StatusTimers = 2;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = MINUTES_20; // 20 minutes
                } 
            }
            else if (StatusTimers == 2)
            {
                if ( Timer1Count > CountSeconds) // press HOLD
                {
                    StatusTimers = 3;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.0 = 1;
                } 
            }
            else if (StatusTimers == 3)
            {
                if ( Timer1Count > CountSeconds) // relese HOLD
                {
                    StatusTimers = 4;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.0 = 0;
                } 
            }
            else if (StatusTimers == 4)
            {
                if ( Timer1Count > CountSeconds) // press '2'
                {
                    StatusTimers = 5;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.1 = 1;
                } 
            }
            else if (StatusTimers == 5)
            {
                if ( Timer1Count > CountSeconds) // relese '2'
                {
                    StatusTimers = 6;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.1 = 0;
                } 
            }
            else if (StatusTimers == 6)
            {
                if ( Timer1Count > CountSeconds) // press '4'
                {
                    StatusTimers = 7;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.2 = 1;
                } 
            }
            else if (StatusTimers == 7)
            {
                if ( Timer1Count > CountSeconds) // relese '4'
                {
                    StatusTimers = 8;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.2 = 0;
                } 
            }
            else if (StatusTimers == 8)
            {
                if ( Timer1Count > CountSeconds) // press '0'
                {
                    StatusTimers = 9;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.3 = 1;
                } 
            }
            else if (StatusTimers == 9)
            {
                if ( Timer1Count > CountSeconds) // relese '0'
                {
                    StatusTimers = 10;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.3 = 0;
                } 
            }
            else if (StatusTimers == 10)
            {
                if ( Timer1Count > CountSeconds) // press 'Enter'
                {
                    StatusTimers = 11;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.4 = 1;
                } 
            }
            else if (StatusTimers == 11)
            {
                if ( Timer1Count > CountSeconds) // relese 'Enter'
                {
                    StatusTimers = 1;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.4 = 0;
                    Status++; // 1 == set next 20 minute; 2== set next 20 minutes; 3 == set next 20 minute and set hit 
                    if (Status == 3) // 1 hour passed
                    {
                        StatusTimers = 20;
                    }
                             // 4 == set next 20 minutes 
                    else if (Status == 4)
                    {
                        StatusTimers = 1;
                    }
                             // 5 = set next 20 minutes (40 minutes); 6 == set next 20 minutes (60 minutes); 7 == set next 20 minutes (80 minutes) 
                    else if (Status == 7)
                    {
                        StatusTimers = 99;
                    }
                } 
            }
            else if (StatusTimers == 20)
            {
                if ( Timer1Count > CountSeconds) // press 'HIT'
                {
                    StatusTimers = 21;
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.6 = 1;
                } 
            }
            else if (StatusTimers == 21)
            {
                if ( Timer1Count > CountSeconds) // release 'HIT'
                {
                    StatusTimers = 4;  // back to '2' '4' '0'
                    Timer1Count = 0;
                    SetTimer1(0);
                    CountSeconds = 1;
                    PORTA.6 = 0;
                } 
            }
            else if (StatusTimers == 99)
            {
            }
        }
    } 


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
   OSCTUNE = 0b00000000;

//     OPTION_REG: OPTION CONTROL REGISTER (ADDRESS 81h, 181h) 
//
//       0                 bit 7 RBPU: PORTB Pull-up Enable bit
//        0                bit 6 INTEDG: Interrupt Edge Select bit
//         0               bit 5 T0CS: TMR0 Clock Source Select bit 
//                            1 = Transition on T0CKI pin 
//                            0 = Internal instruction cycle clock (CLKO) 
//          0              bit 4 T0SE: TMR0 Source Edge Select bit 
//                            1 = Increment on high-to-low transition on T0CKI pin 
//                            0 = Increment on low-to-high transition on T0CKI pin 
//           0             bit 3 PSA: Prescaler Assignment bit 
//                            1 = Prescaler is assigned to the WDT 
//                            0 = Prescaler is assigned to the Timer0 module 
//            111          bit 2-0 PS<2:0>: Prescaler Rate Select bits 
//                            000 1:2
//                            001 1:4
//                            010 1:8
//                            011 1:16
//                            100 1:32
//                            101 1:64
//                            110 1:128
//                            111 1:256
    OPTION_REG = 0b00000111;
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
