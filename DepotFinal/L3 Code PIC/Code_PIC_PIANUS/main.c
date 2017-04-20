/*
 * File:   main.c
 * Author: chrc1601
 * LPB
 *
 * Created on 8 avril 2017, 09:37
 */

#include "config_bits.c"
#include <xc.h>
#include "lcdSPI.h"
#include "uart.h"

// États
#define ETAT_ASYNCHRONE 1
#define ETAT_SYNCHRONE 2
#define ETAT_MENU 3
#define ETAT_ACCORD 4
#define ETAT_NOTE 5
#define ETAT_ORDI 6
#define BOUTON_PLUS 1
#define BOUTON_MOINS 2
#define BOUTON_E 3
#define BOUTON_C 4
#define TX_WAIT 1
#define TX_SEND 2

//Variables Gloables
static char flag_lireBouton = 0;
static char flag_UARTrecu = 0;
static char RXUART = 0;
char commandBouton = 0;
char Etat = ETAT_NOTE;
char commandEtat = 0;
char changementMode = 0;

static char sync = 0;
static int bpm = 100;
static char* bpm_str = (char*)"000";

//Macros
#define _XTAL_FREQ 8000000 //The speed of your internal(or)external oscillator
#define USE_AND_MASKS


//prototype de fonctions
void initGpio();
char lireBouton();
void initUART();
void setupTimer();
void initInterrupt();
void afficherNote();
void Timer1s(int secondes);
void interrupt ITR(void);
void BPMtoString();


void interrupt ITR(void)
{
    LATFbits.LATF6 = 1;
    // If timer interrupt
    if(INTCONbits.TMR0IE && INTCONbits.TMR0IF)
    {
        
        flag_lireBouton = 1;
        INTCONbits.TMR0IF = 0;
    }
    // If UART reception interrupt
    if(PIR1bits.RC1IF && PIE1bits.RC1IE)
    {
        RXUART = RCREG;
        flag_UARTrecu = 1;
    }
    
    LATFbits.LATF6 = 0;
}

//code principal
void main(void) {
    char previousBouton = 0;
    char Bouton = 0;
    char commandSent = 0;
    
    // Initialisations
    initLcd();
    initGpio();
    setupTimer();
    initUART();
    initInterrupt();
   
    // Splash screen
    clearLcd();
    choseLine(2);
    WriteStrLcd("       ",7);
    WriteCharLcd(1,0,0b10110110);
    WriteStrLcd("an-US",5);
    choseLine(4);
    WriteStrLcd("Press =", 7);
    
    while(1){
       
        
        //Réinitialisation du RX USART suivant une erreur d'overrun
        if (!RCSTAbits.CREN)
        {
            RCSTAbits.CREN = 1;
        }
        
        // Lecture du clavier
        if(flag_lireBouton)
        {
            Bouton = lireBouton();
            // Debouncing
            if(Bouton == previousBouton)
            {
                if(!commandSent && Bouton!=0)
                {
                    switch(Bouton)
                    {
                        case '+':
                            commandBouton = BOUTON_PLUS;
                            break;
                        case '-':
                            commandBouton = BOUTON_MOINS;
                            break;
                        case 'C':
                            commandBouton = BOUTON_C;
                            break;
                        case 'E':
                            commandBouton = BOUTON_E;
                            break;
                        default:
                            break;
                    }
                    commandSent = 1;
                }
            }
            else
            {
                commandSent = 0;
            }
            previousBouton = Bouton;
            flag_lireBouton = 0;
        }
        
        
        
        // USART framing error management
        if (RCSTAbits.FERR)
        {
            PORTDbits.RD0 = 1;
        }
        
        // USART overrun error management
        if (RCSTAbits.OERR)
        {
            PORTDbits.RD0 = 1;
            
            // We need to disable receiver to clear error. Receiver is enabled later.
            RCSTAbits.CREN = 0;
        }
            
        
        // Gestion du menu
        if(commandBouton || flag_UARTrecu || changementMode)
        {
            changementMode = 0;
            switch(Etat){
                case ETAT_ASYNCHRONE:
                    clearLcd();
                    choseLine(1);
                    WriteStrLcd("     ASYNCHRONE", 15);
                    choseLine(2);
                    WriteStrLcd("Press + for note", 16);
                    choseLine(3);
                    WriteStrLcd("Press - for chords", 18);
                    choseLine(4);
                    WriteStrLcd("Press = to exit", 15);
                    if(commandBouton == BOUTON_PLUS){
                        Etat = ETAT_NOTE;
                        WriteCharUART(0X78);
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_MOINS){
                        Etat = ETAT_ACCORD;
                        WriteCharUART(0X7A);
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_E){
                        Etat = ETAT_MENU;
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    break;

                case ETAT_SYNCHRONE:
                    clearLcd();
                    choseLine(1);
                    WriteStrLcd("+ : for BPM+5    BPM", 20);
                    choseLine(2);
                    WriteStrLcd("C : to confirm   ", 17);
                    BPMtoString();
                    WriteStrLcd(bpm_str,3);
                    choseLine(3);
                    WriteStrLcd("- : for BPM-5", 13);
                    choseLine(4);
                    WriteStrLcd("= : to exit", 11);
                    if(commandBouton==BOUTON_PLUS){
                        if(bpm<120)
                        {
                            bpm = bpm+5;
                            WriteCharUART(0X77);
                        }
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_MOINS){
                        if(bpm>80)
                        {
                            bpm = bpm-5;
                            WriteCharUART(0X73);
                        }
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_E){
                        Etat = ETAT_MENU;
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_C){
                        Etat = ETAT_ORDI;
                        WriteCharUART(0X61);
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    break;

                case ETAT_ACCORD:
                    afficherNote();
                    if (commandBouton == BOUTON_E){
                        Etat = ETAT_ASYNCHRONE;
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    break;

                case ETAT_NOTE:
                    afficherNote();
                    if (commandBouton == BOUTON_E){
                        Etat = ETAT_ASYNCHRONE;
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    break;

                case ETAT_MENU:
                    clearLcd();
                    choseLine(1);
                    WriteStrLcd("       MENU", 11);
                    choseLine(3);
                    WriteStrLcd("Press + for ASYNC", 16);
                    choseLine(4);
                    WriteStrLcd("Press - for SYNC", 16);
                    if(commandBouton==BOUTON_PLUS){
                        Etat = ETAT_ASYNCHRONE; 
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_MOINS){
                        Etat = ETAT_SYNCHRONE;
                        changementMode = 1;
                        commandBouton = 0;
                    }
                    if(commandBouton==BOUTON_E){
                        choseLine(1);
                        WriteStrLcd("       MENU", 11);
                        choseLine(3);
                        WriteStrLcd("Press + for ASYNC", 16);
                        choseLine(4);
                        WriteStrLcd("Press - for SYNC", 16);
                        changementMode = 0;
                        commandBouton = 0;
                    }
                    break;

                case ETAT_ORDI :
                    clearLcd();
                    choseLine(1);
                    WriteStrLcd("      SYNCHRONE", 15);
                    choseLine(2);
                    WriteStrLcd("Play music", 10);
                    choseLine(3);
                    WriteStrLcd("Look at the console", 19);
                    choseLine(4);
                    WriteStrLcd("Press Q to exit", 15);
                    if(RXUART==0x83){
                        changementMode = 1;
                        Etat = ETAT_MENU;
                        RXUART = 0;
                    }
                    commandBouton = 0;
                    break;
                default :
                    break;
            }
        }
        if(flag_UARTrecu)
        {
            flag_UARTrecu = 0;
        }
        
    }
    
}



void initGpio(){
    
    //colonne (y) du clavier
    TRISEbits.TRISE0 = 0;
    TRISEbits.TRISE1 = 0;
    TRISEbits.TRISE2 = 0;
    TRISEbits.TRISE3 = 0;
    
    LATEbits.LATE0 = 1; // État initial à 1
    LATEbits.LATE1 = 1; // État initial à 1
    LATEbits.LATE2 = 1; // État initial à 1
    LATEbits.LATE3 = 1; // État initial à 1
    
    //ligne (x) du clavier
    TRISEbits.TRISE4 = 1;
    TRISEbits.TRISE5 = 1;
    TRISEbits.TRISE6 = 1;
    TRISEbits.TRISE7 = 1;

    
    
    // Sortie GPIO interupt
    TRISFbits.TRISF6 = 0;
       
     }


char lireBouton(){
    char lecture = 0;
    
    LATEbits.LATE3 = 0; // État initial à 1
    
    if (!PORTEbits.RE4){
        lecture = '+';
    }
    
    if(!PORTEbits.RE5){
        if(lecture != 0){ 
            return 0;
        }
        else{
            lecture = '-';
        }
    }
    //bouton 0
    if(!PORTEbits.RE6){
        if(lecture != 0){ 
            return 0;
        }
        else{
            lecture = 'C';
        }
    }
    //bouton A
    if(!PORTEbits.RE7){
        if(lecture != 0){ 
            return 0;
        }
        else{
            lecture = 'E';
        }
    }

    LATEbits.LATE3 = 1; // État initial à 1
        
    return lecture;
}


void setupTimer(){
    // Stop timer0
    T0CONbits.TMR0ON = 0;
    
    // Setup T0CON
    T0CONbits.T08BIT = 0;
    T0CONbits.T0CS1 = 0;
    T0CONbits.T0CS0 = 1;
    T0CONbits.PSA = 0;
    T0CONbits.T0PS2 = 0;
    T0CONbits.T0PS1 = 0;
    T0CONbits.T0PS0 = 1;
    TMR0L = 0x9F;
    TMR0H = 0x24;
    
    INTCONbits.TMR0IE = 1;
    INTCONbits.TMR0IF = 0;
    
    // Start timer0
    T0CONbits.TMR0ON = 1;
}

void initInterrupt(){
    PIE1bits.RC1IE = 1;
    PIR1bits.RC1IF = 0;
    
    INTCONbits.GIE = 1;
    INTCONbits.PEIE = 1;
}



void afficherNote()
{
    switch (RXUART){
        case 0x01:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : DO", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x02:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : DO#", 10);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x03:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : RE", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x04:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : RE#", 10);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x05:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : MI", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x06:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : FA", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x07:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : FA#", 10);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x08:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : SOL", 10);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x09:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : SOL#", 11);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x0A:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : LA", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x0B:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : LA#", 10);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x0C:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Note : SI", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x0D:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Chord : RE m", 12);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x0E:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Chord : RE maj", 14);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x0F:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Chord : C", 9);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x10:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(2);
            WriteStrLcd("Chord : SOL inv", 15);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        case 0x4E:
            clearLcd();
            choseLine(1);
            WriteStrLcd("     ASYNCHRONE", 15);
            choseLine(4);
            WriteStrLcd("Press = to exit", 15);
            break;
        default :
            break;
    }
}

void BPMtoString()
{
    switch(bpm)
    {
        case 80:
            bpm_str = (char*)"080";
            break;
        case 85:
            bpm_str = (char*)"085";
            break;
        case 90:
            bpm_str = (char*)"090";
            break;
        case 95:
            bpm_str = (char*)"095";
            break;
        case 100:
            bpm_str = (char*)"100";
            break;
        case 105:
            bpm_str = (char*)"105";
            break;
        case 110:
            bpm_str = (char*)"110";
            break;
        case 115:
            bpm_str = (char*)"115";
            break;
        case 120:
            bpm_str = (char*)"120";
            break;
        default:
            bpm_str = (char*)"000";
            break;
    }
}












