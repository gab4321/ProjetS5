//Fichier fait par Louis-Philippe Bardier pour faire marcher le LCD avec notre PCB
// code de l'équipe P4, PI-anUS

//inclusions
#include "lcdSPI.h"
//MACROS

//Variables globales


//fonctions

void initLcd(){
    

    TRISDbits.TRISD4 = 0;   //SDO TO OUTPUT
    TRISDbits.TRISD3 = 0;   //SCL TO OUTPUT
    TRISDbits.TRISD2 = 0;   //SS TO OUTPUT
    
    //PPS-Lite config
    OSCCON2bits.IOLOCK = 0;
    RPOR24_25 |= 0x04;  //pin21 --> SDO1
    RPOR22_23 |= 0x05;  //pin22 --> SCK2
    RPINR12_13 |= 0x05;   //pin24 --> SDI1
    OSCCON2bits.IOLOCK = 1;
    
    //Enable and configure the PPS SPI2
    SSP2CON1bits.SSPEN2 = 1;    //SPI SERIAL START
    SSP2STATbits.CKE2   = 0;    //ClockMode 
    SSP2CON1bits.CKP2   = 1;    //ClockMode 
    
    SSP2CON1 = 0b00110010;
    
//CONFIGURATION LCD
    //Function set 8 bit
    LATDbits.LATD3 = 0;
    // Rs et Rw à 0
    WriteSPI2(0b11111000);
    //D0 D1 D2 D3    
    WriteSPI2(0b00110000);
    //D4 D5 D6 D7
    WriteSPI2(0b11000000);

    
    
    //configuration du LCD pour avoir des nombres de lignes
    // Rs et Rw à 0
    WriteSPI2(0b11111000);
    //D7 D6 D5 D4 tous à 0
    WriteSPI2(0b10010000);
    //D3 D2 D1 D0 tous à 1
    WriteSPI2(0b00000000);
    
        
    //function set RE(0)
    // Rs et Rw à 0
    WriteSPI2(0b11111000);
    //D7 D6 D5 D4 tous à 0
    WriteSPI2(0b00010000);
    //D3 D2 D1 D0 tous à 1
    WriteSPI2(0b11000000);
    
    
    //Display ON/OFF
    // Rs et Rw à 0
    WriteSPI2(0b11111000);
    //D7 D6 D5 D4 tous à 0
    WriteSPI2(0b01110000);
    //D3 D2 D1 D0 tous à 1
    WriteSPI2(0b00000000);
    
    LATDbits.LATD3 = 1;
    
    clearLcd();
}



void clearLcd(){
    LATDbits.LATD3 = 0;
    //Rs et RW à 0
    WriteSPI2(0b11111000);
    //D7 D6 D5 D4 à 0
    WriteSPI2(0b10000000);
    //D3 D2 D1 D0 -> 0 0 0 1
    WriteSPI2(0b00000000);
    LATDbits.LATD3 = 1;
}


void WriteCharLcd(char Rs, char Rw, char data){
    char TabData[8];
    char Send = 0;
    char Send2 = 0;
    char Send3 = 0;
    //envoyer le premier paquet (Rs = 1, Rw/r = 0)
    Rs = Rs<<1;
    Rw = Rw<<2;
    Send = 0b11111000 | Rs | Rw;

    
    //deuxieme paquet a envoyer, D0 D1 D2 D3 0 0 0 0
    TabData[0] = 0b00000001&data;
    TabData[1] = 0b00000010&data;
    TabData[2] = 0b00000100&data;
    TabData[3] = 0b00001000&data;
    
    TabData[0] = TabData[0]<<7;
    TabData[1] = TabData[1]<<5;
    TabData[2] = TabData[2]<<3;
    TabData[3] = TabData[3]<<1;
    
    Send2 = TabData[0] | TabData[1] | TabData[2] | TabData[3];
    Send2 = Send2 & 0b11110000;


    
    // envoie du troisieme paquet, D5 D6 D7 D8 0 0 0 0 
    TabData[4] = 0b00010000&data;
    TabData[5] = 0b00100000&data;
    TabData[6] = 0b01000000&data;
    TabData[7] = 0b10000000&data;
    
    TabData[4] = TabData[4]<<3;
    TabData[5] = TabData[5]<<1;
    TabData[6] = TabData[6]>>1;
    TabData[7] = TabData[7]>>3;
    
    TabData[6] &= 0b00100000;
    TabData[7] &= 0b00010000;
    
    Send3 = TabData[4] | TabData[5] | TabData[6] | TabData[7] &0b11110000;
   
    LATDbits.LATD3 = 0;
    WriteSPI2(Send);
    WriteSPI2(Send2);
    WriteSPI2(Send3);
    LATDbits.LATD3 = 1;
}
void WriteStrLcd(const char* str, int size){
    int i=0;
    for(i=0; i<size; i++){
        WriteCharLcd(1,0,str[i]);
    }

}
void choseLine(int line){
    switch (line){
        case 1:
            WriteCharLcd(0, 0, 0x80);
            break;
    
        case 2:
            WriteCharLcd(0, 0, 0xA0); 
            break;
        
        case 3:
            WriteCharLcd(0, 0, 0xC0);
            break;
        
        case 4:
            WriteCharLcd(0, 0, 0xE0);
            break;
            
        default:
            break;
    }
}








