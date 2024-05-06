#include <msp430.h>
#include <stdbool.h>
extern void LCDini(void);                           // Initialize the LCD Display
extern void showChar(char c, int position);         // Define Show Character Routine
extern char Mess[18] = {32};                        // RFID Data Buffer
//extern char password[10]={"3A00419545"};            // Code to open Lock (Hard Coded)
extern char password[10]={};
extern unsigned int Point   = 0;                    // Pointer to Buffer Position
extern char  Match          = 0;                    // No Code Match
extern char  BFull          = 0;                    // Define "Buffer full" Flag
extern char learn = 'L';
extern short i  = 0;                                   // Define & Initialize Loop counter
extern short j  = 0;                                   // Define & Initialize Loop counter
// ******************************************************************************************
// *************    Final Project 5/3/2024  *************
// ******************************************************************************************

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                       // Stop the Watch-dog timer
    PM5CTL0 &= ~LOCKLPM5;                           // Disable the GPIO power-on default high-impedance mode

    for(i = 0; i < 18; i++) Mess[i] = ' ';          // Clear Buffer; fill with Space Characters

    LCDini();                                       // Initialize the LCD Display

//  Configure LEDs
    P1DIR |= BIT0;                                  // Set P1.0 to output direction, Red   LED
    P9DIR |= BIT7;                                  // Set P9.7 to output direction, Green LED
    P1OUT &= ~BIT0;                                 // Turn Red   LED Off
    P9OUT &= ~BIT7;                                 // Turn Green LED Off

//  Configure GPIO for UART
    P4SEL0 |=   BIT2 | BIT3;                        // USCI_A0 UART operation, TX = MCU_OUT = P4.2
    P4SEL1 &= ~(BIT2 | BIT3);                       // USCI_A0 UART operation, RX = MCU_IN  = P4.3

//  Configure USCI_A0 for UART mode
    UCA0CTLW0 = UCSWRST;                            // Put eUSCI in reset
    UCA0CTL1 |= UCSSEL__SMCLK;                      // CLK = 1.0 MHz SMCLK
    UCA0BR0 = 104;                                  // 1000000/9600 = 104.1666
    UCA0MCTLW = 0x1100;                             // 1000000/9600 - INT(1000000/9600)=0.1666
    UCA0BR1 = 0;                                    // UCA0BR1 value
    UCA0CTL1 &= ~UCSWRST;                           // Release from Reset
    UCA0IE |= UCRXIE;                               // Enable USCI_A0 RX interrupt
    __enable_interrupt();                           // Enable interrupts

    short length     =   12;                      // Initialize the length of the Message


  while(1)
{
  while( learn == 'L' ) //Learn routine
   {
      char  display[6]= "*LEARN";
          for ( i = 0; i < 6; i++ )
              {
              showChar(display[i], i );
              }
      P9OUT |= BIT7;                         // Turn Green LED On
      __delay_cycles(250000);         // Wait 250 ms
      P9OUT &= ~BIT7;                         // Turn Green LED Off
      __delay_cycles(250000);         // Wait 250 ms
      if (BFull == 1)
      {

          for( j = 0; j <= length; j++ )          // Loop through the Data Buffer
              {
              for ( i = 0; i < 6; i++ )
                  {
                      showChar(Mess[i+j], i ); // Show Characters on LCD
                  }
                  __delay_cycles(500000);         // Wait 500 ms
              }

          for(i=0; i<10; i++) //Fill password with spaces
             {
                 password[i] = Mess[i+1];
             }

          for(j = 0; j < 18; j++) Mess[j] = 32;    // Clear Buffer; fill with Space Characters
          BFull = 0; Point = 0;
          learn = 'S';
      }
   }
  while( learn == 'S' )    //Standby
     {
      P1OUT &= ~BIT0;                                 // Turn Red   LED Off
      char  display[6]= "INPUT ";
            for ( i = 0; i < 6; i++ )
                {
                showChar(display[i], i );
                }
      P9OUT |= BIT7;                         // Turn Green LED On
     }

  while( learn == 'C' ) //Check
   {
   P1OUT &= ~BIT0;                                 // Turn Red   LED Off
   P9OUT &= ~BIT7;                                // Turn Green LED off
       if( BFull == 1 )
        {

          Match = 1;                                // Assume Codes are a match
          for(i = 0; i < 10; i++)
              {
               if(password[i] != Mess[i+1]) Match = 0;// Codes do not match
              }

     if( Match == 1 )                               // If codes match
          {
             __delay_cycles(250000);                // .25 second delay to make it blink
             P9OUT |= BIT7;                         // Turn Green LED On
             char  display[6]= "OPEN  ";            // Define 6 Character Message
             int i = 0;                             // Define and initialize loop counter
                 for( i = 0; i < 6; i++ )           // Read 6 characters from the message in a loop
                  {
                    showChar(display[i], i );       // Show OPEN Message
                  }
             __delay_cycles(4000000);                // wait 4 seconds
             for(j = 0; j < 18; j++) Mess[j] = 32;    // Clear Buffer; fill with Space Characters
             BFull = 0; Point = 0;
             learn = 'S';
           }
     else                                           // Codes don't match
          {
             __delay_cycles(250000);                // .25 second delay to make it blink
             P1OUT |= BIT0;                         // Turn Red LED On
             char  display[6]= "LOCKED";            // Define 6 Character Message
             int i = 0;                             // Define and initialize loop counter
                 for( i = 0; i < 6; i++ )           // Read 6 characters from the message in a loop
                  {
                    showChar(display[i], i );       // Show LOCKED Message
                  }
             __delay_cycles(4000000);                // wait 4 seconds
             for(j = 0; j < 18; j++) Mess[j] = 32;    // Clear Buffer; fill with Space Characters
             BFull = 0; Point = 0;
             learn = 'S';
           }
         }                                          // End of BFull loop

   }                                                // End of while(1) loop
   }                                                // End of Unlocking loop
}                                                   // End of Main
// ******************************************************************************************
// *****  UART Interrupt Routine receiving RFID Data @ 9,600 Baud with MSP @ 1MHz Clock *****
// *****                 RX = MCU_IN = P4.3, TX = MCU_OUT = P4.2                        *****
// ******************************************************************************************
#pragma vector = USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
        volatile char RXData = 0;                   // Define RXData variable
        RXData = UCA0RXBUF;                         // Read Receive buffer
        if ( RXData == 0x02 ) RXData = '$';         // Detect & Substitute STX Byte
        if ( RXData == 0x0D ) RXData = '(';         // Detect & Substitute CR  Byte
        if ( RXData == 0x0A ) RXData = ')';         // Detect & Substitute LF  Byte
        if ( RXData == 0x03 ) RXData = '*';         // Detect & Substitute ETX Byte

        if( (Point < 16) && (BFull == 0) )          // If Buffer not full
           {
            P9OUT |= BIT7;                          // Turn Green LED On
            Mess[Point] = RXData;                   // Put Data in Buffer
            Point++;                                // Increment Pointer
            if( Point == 16 )                       // Detect End of Message
               {
                BFull = 1;                          // Set Buffer full Flag
                P9OUT &= ~BIT7;                     // Turn Green LED off
               }
           }
        if (learn == 'S') learn = 'C';
        UCA0IFG &= ~UCRXIFG;                        // Clear interrupt Flag
}

// *************************************************************************************************
// **  Show Character at LCD Position, works with Space, Numbers, Upper and Lower case Characters **
// *************************************************************************************************
void showChar(char c, int position)
{
// Space/Number Characters:  [  SP   ]  [   !   ]  [   "   ]  [   #   ]  [   $   ]  [   %   ]  [   &   ]  [   '   ]  [   (   ]  [   )   ]  [   *   ]  [   +   ]  [   ,   ]
const char sp_num[26][2]  = {0x00,0x00, 0x60,0x01, 0x40,0x40, 0x61,0x50, 0xB7,0x50, 0x00,0x29, 0x10,0xCA, 0x00,0x20, 0x00,0x22, 0x00,0x88, 0x03,0xAA, 0x03,0x50, 0x00,0x08,
                             0x03,0x00, 0x00,0x01, 0x00,0x28, 0xFC,0x28, 0x60,0x20, 0xDB,0x00, 0xF3,0x00, 0x67,0x00, 0xB7,0x00, 0xBF,0x00, 0xE4,0x00, 0xFF,0x00, 0xF7,0x00};
// Space/Number Characters:  [   -   ]  [   .   ]  [   /   ]  [   0   ]  [   1   ]  [   2   ]  [   3   ]  [   4   ]  [   5   ]  [   6   ]  [   7   ]  [   8   ]  [   9   ]

// Upper Case Characters:    [   A   ]  [   B   ]  [   C   ]  [   D   ]  [   E   ]  [   F   ]  [   G   ]  [   H   ]  [   I   ]  [   J   ]  [   K   ]  [   L   ]  [   M   ]
const char  upper[26][2]  = {0xEF,0x00, 0xF1,0x50, 0x9C,0x00, 0xF0,0x50, 0x9F,0x00, 0x8F,0x00, 0xBD,0x00, 0x6F,0x00, 0x90,0x50, 0x78,0x00, 0x0E,0x22, 0x1C,0x00, 0x6C,0xA0,
                             0x6C,0x82, 0xFC,0x00, 0xCF,0x00, 0xFC,0x02, 0xCF,0x02, 0xB7,0x00, 0x80,0x50, 0x7C,0x00, 0x0C,0x28, 0x6C,0x0A, 0x00,0xAA, 0x00,0xB0, 0x90,0x28};
// Upper Case Characters:    [   N   ]  [   O   ]  [   P   ]  [   Q   ]  [   R   ]  [   S   ]  [   T   ]  [   U   ]  [   V   ]  [   W   ]  [   X   ]  [   Y   ]  [   Z   ]

// Lower Case Characters:    [   a   ]  [   b   ]  [   c   ]  [   d   ]  [   e   ]  [   f   ]  [   g   ]  [   h   ]  [   i   ]  [   J   ]  [   k   ]  [   l   ]  [   m   ]
const char  lower[26][2]  = {0x21,0x12, 0x3F,0x00, 0x1B,0x00, 0x7B,0x00, 0x1A,0x08, 0x0E,0x00, 0xF7,0x00, 0x2F,0x00, 0x10,0x10, 0x78,0x00, 0x00,0x72, 0x0C,0x00, 0x2B,0x10,
                             0x21,0x10, 0x0A,0x18, 0xCF,0x00, 0xE7,0x00, 0x0A,0x00, 0x11,0x02, 0x03,0x10, 0x38,0x00, 0x08,0x08, 0x28,0x0A, 0x00,0xAA, 0x00,0xA8, 0x12,0x08};
// Lower Case Characters:    [   n   ]  [   o   ]  [   p   ]  [   q   ]  [   r   ]  [   s   ]  [   t   ]  [   u   ]  [   v   ]  [   w   ]  [   x   ]  [   y   ]  [   z   ]

// The Position accounts for Offset of Alphanumeric character positions in the LCD Memory Map
short  Pos[6]  = { 9, 5, 3, 18, 14, 7 };        // Define and populate Position, Range 0 - 5

    if (c >= ' ' && c <= '9')
      {
        // Display special printable characters & Numbers
        LCDMEM[Pos[position]]   =  sp_num[c-32][0];
        LCDMEM[Pos[position]+1] =  sp_num[c-32][1];
      }
    else if (c >= 'A' && c <= 'Z')
      {
        // Display Upper-case Letters
        LCDMEM[Pos[position]]   = upper[c-65][0];
        LCDMEM[Pos[position]+1] = upper[c-65][1];
      }
    else if (c >= 'a' && c <= 'z')
      {
        // Display lower-case letters
        LCDMEM[Pos[position]]   = lower[c-97][0];
        LCDMEM[Pos[position]+1] = lower[c-97][1];
      }
    else
      {
        // Show underline for other characters
        LCDMEM[Pos[position]]   = 0x10;
        LCDMEM[Pos[position]+1] = 0x00;
      }
}

void LCDini(void)
{
// ******************************************************************************************
// *****  Basic LCD Initialization Copyright (c) 2014, Texas Instruments Incorporated   *****
// ******************************************************************************************
//  This examples configures the LCD in 4-Mux mode.
//  The internal voltage is sourced to V2 through V4 and V5
//  is connected to ground. Charge pump is enabled.
//  It uses LCD pin L0~L21 and L26~L43 as segment pins.
//  f(LCD) = 32768Hz/((1+1)*2^4) = 1024Hz, ACLK = 32768Hz,
//  MCLK = SMCLK = default DCODIV 1MHz.
//
//      MSP430FR6989 / MSP-EXP430FR6989 Launchpad
//              -----------------
//          /|\|                 |
//           | |              XIN|--
//  GND      --|RST              |  32768Hz
//   |         |             XOUT|--
//   |         |                 |
//   |         |             COM3|----------------|
//   |         |             COM2|---------------||
//   |--4.7uF -|LCDCAP       COM1|--------------|||
//             |             COM0|-------------||||
//             |                 |    -------------
//             |           Sx~Sxx|---| 1 2 3 4 5 6 |
//             |                 |    -------------
//             |                 |       TI LCD
//                                 (See MSP-EXP430FR6989 Schematic)
//
//*****************************************************************************
// Initialize LCD segments 0 - 21; 26 - 43
   LCDCPCTL0 = 0xFFFF;
   LCDCPCTL1 = 0xFC3F;
   LCDCPCTL2 = 0x00FF;

    PJSEL0 = BIT4 | BIT5;                      // Turn on LFXT (Low Frequency Crystal Oscillator)
// Configure LFXT 32kHz CrystalOscillator
   CSCTL0_H = CSKEY >> 8;                      // Unlock CS registers
   CSCTL4 &= ~LFXTOFF;                         // Enable LFXT  (Low Frequency Crystal Oscillator)
         do
           {
            CSCTL5 &= ~LFXTOFFG;               // Clear LFXT fault flag
            SFRIFG1 &= ~OFIFG;
            }
            while (SFRIFG1 & OFIFG);           // Test oscillator fault flag
            CSCTL0_H = 0;                      // Lock CS registers

// Initialize LCD_Clock
// ACLK, Divider = 1, Pre-divider = 16; 4-pin MUX
            LCDCCTL0 = LCDDIV__1 | LCDPRE__16 | LCD4MUX | LCDLP;

// VLCD generated internally,
// V2-V4 generated internally, V5 to ground
// Set VLCD voltage to 2.60V
// Enable charge pump and select internal reference for it
           LCDCVCTL     =   VLCD_1 | VLCDREF_0 | LCDCPEN;
           LCDCCPCTL    = LCDCPCLKSYNC;         // Clock synchronization enabled
           LCDCMEMCTL   = LCDCLRM;              // Clear LCD memory
           LCDCCTL0    |= LCDON;                // Turn LCD on
}
