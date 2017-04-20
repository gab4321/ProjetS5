
/* 
 * File:   
 * Author: 
 * Comments:
 * Revision history: 
 */

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef XC_HEADER_TEMPLATE_H
#define	XC_HEADER_TEMPLATE_H

#include <xc.h> // include processor files - each processor file is guarded.  
#include <spi.h>

//prototypes de fonctions
void initLcd();
void clearLcd();
void WriteCharLcd(char Rs, char Rw, char data);
void WriteStrLcd(const char* str, int size);
void choseLine(int line);

#endif	/* XC_HEADER_TEMPLATE_H */

