/*
 * File:   uart.c
 * Author: barl2407
 *
 * Created on 19 avril 2017, 13:48
 */


#include <xc.h>
#include "uart.h"

void initUART()
{
    TRISDbits.TRISD6 = 0; // TX pin as output
    TRISDbits.TRISD7 = 1; // RX pin as input
    
    
    //PPS-Lite config
    OSCCON2bits.IOLOCK = 0;
    RPOR26_27 = 0x11;  
    RPINR0_1 = 0x66;  // Map signal U1TX/RX to pin RP26/RP27
    OSCCON2bits.IOLOCK = 1;

    
    //baud rate et global   
    RCSTAbits.SPEN = 1; // Disable serial port (reset)
    TXSTAbits.SYNC = 0;
    SPBRG1 = BAUD_RATE_GEN;
    TXSTAbits.BRGH = 1;
    BAUDCONbits.BRG16 = 0;
    
   
    // Set up transmission
    TXSTAbits.TXEN = 1;
    TXSTAbits.TX9 = 0;
    
    // Set up reception
    RCSTAbits.CREN = 1;
    RCSTAbits.RC9 = 0;
    
}

void WriteCharUART(const char carac) {
    while(TXSTAbits.TRMT != 1);  //On attne que le buffer d'envoi soit vide
    TXREG = carac;              //On le print
}

void WriteArrayUART(const char* carac, int size) {
    for (int i = 0 ; i<size ; i++) {    //On fait une boucle pour 
        WriteCharUART(carac[i]);            //chacun des caracteres du string
    }
}