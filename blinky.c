#include <msp430.h>

void main(void)
{
    WDTCTL = WDTPW | WDTHOLD;                   // Stop Watchdog timer
    PM5CTL0 &= ~LOCKLPM5;                       // Disable the GPIO power-on default high-impedance mode

    P1DIR |= BIT0;                              // Set P1.0 to output direction, Red   LED
    P9DIR |= BIT7;                              // Set P9.7 to output direction, Green LED

    P1REN |= BIT1;                              // Enable Resistor on P1.1, Button 1
    P1REN |= BIT2;                              // Enable Resistor on P1.2, Button 2

    P1OUT |= BIT1;                              // Enable Pull-up on P1.1, Button 1
    P1OUT |= BIT2;                              // Enable Pull-up on P1.2, Button 2

    const unsigned long int delay = 200000; // Constant can be defined for __delay_cycles but not a variable

    unsigned short int ledsequence1, ledsequence2 = 0;
    P1OUT &= ~BIT0;
    P9OUT &= ~BIT7; // initialize both led's to off
    while(1)
    {
        unsigned char button1 = !(P1IN & BIT1); // char because it takes only 1 byte and can be used for boolean
        unsigned char button2 = !(P1IN & BIT2);
            if ( button1 == 1 )               // If Button 1 is pressed
                  {

                       ledsequence1 = 1;
                  }

             if ( button2 == 1 )              // If Button 2 is pressed
                  {

                       ledsequence2 = 1;
                  }
             if (ledsequence1 && ledsequence2 == 1 )
             {
                 ledsequence1 = 0;
                 ledsequence2 = 0;
             }
             while (ledsequence1 >= 1 && ledsequence1 <= 4) // red led sequence
             {
                 P1OUT |= BIT0;           // Turn LED1 On
                 __delay_cycles(delay);
                 P1OUT &= ~BIT0;
                 __delay_cycles(delay);
                 ledsequence1 ++;
             }
             while (ledsequence2 >= 1 && ledsequence2 <=4) // green led sequence
             {
                 P9OUT |= BIT7;           // Turn LED1 On
                 __delay_cycles(delay);
                 P9OUT &= ~BIT7;
                 __delay_cycles(delay);
                 ledsequence2 ++;
              }

    }

}
