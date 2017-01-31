/* 
 main.c 
 Projet S5, équipe P4
*/

#include <stdio.h>
#include <stdlib.h>
#include <xc.h>        /* XC8 General Include File */
#include <stdint.h>        /* For uint8_t definition */
#include <stdbool.h>       /* For true/false definition */
#include "config_bits.c"

#define _XTAL_FREQ 20000000   // oscillator frequency for _delay() function

// Interruptions
void interrupt high_priority myITR(void);
void interrupt low_priority myITR2(void);


void interrupt high_priority myITR(void)
{
    // L'interruption est causée par une réception UART
    if(PIE1bits.RCIE && PIR1bits.RCIF)
    {
    }
}

void interrupt low_priority myITR2(void)
{
    // L'interruption est causée par une conversion terminée de l'ADC
    if(PIE1bits.ADIE && PIR1bits.ADIF)
    {
    }
}

int main() {
    
   
}

