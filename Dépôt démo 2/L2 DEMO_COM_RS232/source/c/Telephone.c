/********************************************************
 **  Session 5 - APP6 - Téléphonie par DSP
 **  Fichier principal Telephone.c
 **  Auteurs : < vos noms >
 **  Date : < derniere modification >
 ********************************************************/

/***************************************************************************
	Include headers :
 ***************************************************************************/

// Used modules headers
//#include "module_example.h"
#include "SPI_driver.h"
//#include "Audio_driver.h"
//#include "C6713Helper_UdeS.h"
#include "dsk6713_led.h"
#include "dsk6713_dip.h"
#include "dsk6713.h"
#include <csl_Mcbsp.h>

// standard libraries 
#include <stdio.h>   // get standard I/O functions (as printf)
#include <stddef.h>  // get null and size_t definition
#include <stdbool.h> // get boolean, true and false definition

/****************************************************************************
	Private macros and constants :
 ****************************************************************************/

//vos  #defines ou const int blablabla
//unique à ce fichier

/****************************************************************************
	Extern content declaration :
 ****************************************************************************/

// déclaration des contenus utilisés ici mais définis ailleurs

extern void vectors();   // Vecteurs d'interruption
extern int flag_SPI;    // indique si une data SPI est prete a etre lue
extern short input_SPI;

/****************************************************************************
	Private Types :
 ****************************************************************************/

// type struct, enum , etc. definition here

// These type declaration are only valid in this .c

/****************************************************************************
	Private global variables :
 ****************************************************************************/

// Use static keyword here
int is_comp = 0;
int LastDip = 0;
short ReadValue = 0;

/****************************************************************************
	Main program private functions prototypes :
 ****************************************************************************/

void EnableGlobalInterrupts();
void EcrireNom();

/****************************************************************************
	Main Program :
 ****************************************************************************/

void main()
{
    // initialisation des modules et des périphériques
    DSK6713_init();
    DSK6713_LED_init();
    DSK6713_DIP_init();

    // init des led
    DSK6713_LED_off(0);
    DSK6713_LED_off(1);
    DSK6713_LED_off(2);
    DSK6713_LED_off(3);

    // init des modules
    SPI_init();

    // enable des interrupts
    EnableGlobalInterrupts();
    EcrireNom(); // ecrit le nom dequipe
    DSK6713_waitusec(250);

    // Boucle infinie
    while(1)
    {
        // Si donnée est prete sur MCBSP0
        if(flag_SPI)
        {
            // faire lecture SPI
            flag_SPI = 0;
            ReadValue = SPI_Read();
            printf("Valeur lue: %i\n",ReadValue);
            if(ReadValue == 0X0052) //si le caractere recu est "R"
                EcrireNom(); // ecrit le nom dequipe
            flag_SPI = 0;
        }
    }
}

/****************************************************************************
	Main program private functions :
 ****************************************************************************/

//ecrit le nom dequipe de projet
void EcrireNom()
{
    SPI_Write(0X0050);  //ecrit un "P"
    SPI_Write(0X0049);  //ecrit un "I"
    SPI_Write(0X0041);  //ecrit un "A"
    SPI_Write(0X004E);  //ecrit un "N"
    SPI_Write(0X002D);  //ecrit un "-"
    SPI_Write(0X0055);  //ecrit un "U"
    SPI_Write(0X0053);  //ecrit un "S"
    SPI_Write(0X0021);  //ecrit un "!"
}

// active les interrupts
void EnableGlobalInterrupts()
{
    IRQ_nmiEnable();
    IRQ_globalEnable();
}

/****************************************************************************
	Main program interrupt service routines (ISR) :
 ****************************************************************************/

// end of Telephone.c
