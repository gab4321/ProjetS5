/*
 * Communication_232.h
 */

#include "Communication_232.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "SPI_driver.h"

/********************************************************************************************
Variables pour les fonctions de traitement audio
********************************************************************************************/
// adresse du USER_REG du CPLD pour LED et Switch
#define CPLD_USER_REG (unsigned int *) 0x90080000

extern const double PI;
extern const int Fs;

// variables du main pour la correlation
extern int ndecalage;
extern int ntrame;
extern int npadd;

// variable du main pour la detection de periodicité
extern int FlagLed0;
extern float scale;
extern int nVerif;

// variables du main pour lenregistrement des trames
extern int SeuilSon;
extern int IsSound;

// variables du main pour la FFT
extern short index[];

// flag pour la gestion de laffichage LCD
extern int FLAGPASNOTE;
extern int FLAGNOTE;
extern int CompPasNote;

// define des frequences de notes
#define Do 261.63
#define Do_dies 277.18
#define Re 293.66
#define Re_dies 311.13
#define Mi 329.63
#define Fa 349.23
#define Fa_dies 369.99
#define Sol 392.00
#define Sol_dies 415.30
#define La 440.00
#define La_dies 466.16
#define Si 493.88

/********************************************************************************************
Description: fonction envoyant les notes et accords au LCD en mode ASYNCHRONE
********************************************************************************************/
void NotesToPIC(int ismodeaccord, int note)
{
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // FONCTION A COMPLETER!!! *****************************************************
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    if(ismodeaccord == 1)
    {
        if(note == 1)           // Re mineur
        {
            SPI_Write(0X000D);  // pour pic
            //SPI_Write(0X0031);
        }
        else if(note == 2)      // Re majeur
        {
            SPI_Write(0X000E);  // pour pic
            //SPI_Write(0X0032);
        }
        else if(note == 3)      // C
        {
            SPI_Write(0X000F);  // pour pic
            //SPI_Write(0X0033);
        }
        else if(note == 4)      // Sol inv
        {
            SPI_Write(0X0010);   // pour pic
            //SPI_Write(0X0034);
        }
        else      // rien
        {
            //SPI_Write(0X000F);    // pour pic
            //SPI_Write(0X0030);
        }
    }

    if(ismodeaccord == 0)
    {
        if(note == 1)           // DO
        {
            SPI_Write(0X0001);  // pour pic
            //SPI_Write(0X0031);
        }
        else if(note == 2)      // DO #
        {
            SPI_Write(0X0002);  // pour pic
            //SPI_Write(0X0032);
        }
        else if(note == 3)      // RE
        {
            SPI_Write(0X0003);  // pour pic
            //SPI_Write(0X0033);
        }
        else if(note == 4)      // RE #
        {
            SPI_Write(0X0004);   // pour pic
            //SPI_Write(0X0034);
        }
        else if(note == 5)      // MI
        {
            SPI_Write(0X0005);    //pour pic
            //SPI_Write(0X0035);
        }
        else if(note == 6)      // FA
        {
            SPI_Write(0X0006);    //pour pic
            //SPI_Write(0X0036);
        }
        else if(note == 7)      // FA #
        {
            SPI_Write(0X0007);    // pour pic
            //SPI_Write(0X0037);
        }
        else if(note == 8)      // SOL
        {
            SPI_Write(0X0008);    // pour pic
            //SPI_Write(0X0038);
        }
        else if(note == 9)      // SOL #
        {
            SPI_Write(0X0009);    // pour pic
            //SPI_Write(0X0039);
        }
        else if(note == 10)     // LA
        {
            SPI_Write(0X000A);    // pour pic
            //SPI_Write(0X0031);
            //SPI_Write(0X0030);
        }
        else if(note == 11)     // LA #
        {
            SPI_Write(0X000B);    // pour pic
            //SPI_Write(0X0031);
            //SPI_Write(0X0031);
        }
        else if(note == 12)     // SI
        {
            SPI_Write(0X000C);    // pour pic
            //SPI_Write(0X0031);
            //SPI_Write(0X0032);
        }
        else      // rien
        {
            //SPI_Write(0X000F);    // pour pic
            //SPI_Write(0X0030);
        }
    }

    // test pour dire denvoyer linformation: IL NY A PLUS DE NOTE !!!!!
    FLAGNOTE = 1;
    CompPasNote = 0;
}

/********************************************************************************************
Description : fait un print dune partition de 4 mesures dans la console en mode SYNCHRONE
********************************************************************************************/
void printpartitionConsole(int *buffernote, int *buffertiming)
{
    int i = 0;

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // CHANGER LES PRINTF POUR DES SPI_WRITE (CARACTERES ASCII) !!!!!!
    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////

    //printf("XX|----------------|----------------|----------------|----------------|\n");
    //printf("XX|                |                |                |                |\n");

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note SILENCE

    //printf("XX|");
    SPI_Write(0X0058);
    SPI_Write(0X0058);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        if(buffertiming[i] != 5) // croche de silence
        {
            //printf("  "); // aucune note
            SPI_Write(0X002D);
            SPI_Write(0X002D);
        }
        else                     // autre
        {
            //printf("ss");
            SPI_Write(0X0073);
            SPI_Write(0X0073);
        }

        if((i+1)%8 == 0 && i != 0)
        {
            //printf("|"); // fin dune mesure
            SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note SI

    //printf("SI|");
    SPI_Write(0X0053);
    SPI_Write(0X0049);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        if(buffertiming[i] == 1 && buffernote[i] == 12) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 12) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 12) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 12) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else                                    // autre
        {
            //printf("--"); // aucune note
            SPI_Write(0X002D);
            SPI_Write(0X002D);
        }

        if((i+1)%8 == 0 && i != 0)
        {
            //printf("|"); // fin dune mesure
            SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note LA ou LA #

    //printf("LA|");
    SPI_Write(0X004C);
    SPI_Write(0X0041);
    SPI_Write(0X007C);
    for(i = 0; i < 32; i++)
    {
        // LA #
        if(buffertiming[i] == 1 && buffernote[i] == 11) // croche
        {
            //printf("c#");
            SPI_Write(0X0063);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 11) // noire
        {
            //printf("n#");
            SPI_Write(0X006E);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 11) // blanche
        {
            //printf("b#");
            SPI_Write(0X0062);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 11) // ronde
        {
            //printf("r#");
            SPI_Write(0X0072);
            SPI_Write(0X0023);
        }

        // LA
        else if(buffertiming[i] == 1 && buffernote[i] == 10) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 10) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 10) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 10) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else                            // autre
        {
            //printf("  "); // aucune note
            SPI_Write(0X0020);
            SPI_Write(0X0020);
        }

        if((i+1)%8 == 0 && i != 0)
        {
            //printf("|"); // fin dune mesure
            SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note SOL ou SOL #

    //printf("SO|");
    SPI_Write(0X0053);
    SPI_Write(0X004F);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        // SOL #
        if(buffertiming[i] == 1 && buffernote[i] == 9) // croche
        {
            //printf("c#");
            SPI_Write(0X0063);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 9) // noire
        {
            //printf("n#");
            SPI_Write(0X006E);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 9) // blanche
        {
            //printf("b#");
            SPI_Write(0X0062);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 9) // ronde
        {
            //printf("r#");
            SPI_Write(0X0072);
            SPI_Write(0X0023);
        }

        // SOL
        else if(buffertiming[i] == 1 && buffernote[i] == 8) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 8) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 8) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 8) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else                    // autre
        {
            //printf("--"); // aucune note
            SPI_Write(0X002D);
            SPI_Write(0X002D);
        }

        if((i+1)%8 == 0 && i != 0)
        {
             //printf("|"); // fin dune mesure
             SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note FA ou FA #

    //printf("FA|");
    SPI_Write(0X0046);
    SPI_Write(0X0041);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        // FA #
        if(buffertiming[i] == 1 && buffernote[i] == 7) // croche
        {
            //printf("c#");
            SPI_Write(0X0063);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 7) // noire
        {
            //printf("n#");
            SPI_Write(0X006E);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 7) // blanche
        {
            //printf("b#");
            SPI_Write(0X0062);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 7) // ronde
        {
            //printf("r#");
            SPI_Write(0X0072);
            SPI_Write(0X0023);
        }

        // FA
        else if(buffertiming[i] == 1 && buffernote[i] == 6) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 6) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 6) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 6) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else                // autre
        {
            //printf("  "); // aucune note
            SPI_Write(0X0020);
            SPI_Write(0X0020);
        }

        if((i+1)%8 == 0 && i != 0)
        {
             //printf("|"); // fin dune mesure
             SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note MI

    //printf("MI|");
    SPI_Write(0X004D);
    SPI_Write(0X0049);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        if(buffertiming[i] == 1 && buffernote[i] == 5) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 5) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 5) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 5) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else           // autre
        {
            //printf("--"); // aucune note
            SPI_Write(0X002D);
            SPI_Write(0X002D);
        }

        if((i+1)%8 == 0 && i != 0)
        {
             //printf("|"); // fin dune mesure
             SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note RE ou RE #

    //printf("RE|");
    SPI_Write(0X0052);
    SPI_Write(0X0045);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        // RE #
        if(buffertiming[i] == 1 && buffernote[i] == 4) // croche
        {
            //printf("c#");
            SPI_Write(0X0063);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 4) // noire
        {
            //printf("n#");
            SPI_Write(0X006E);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 4) // blanche
        {
            //printf("b#");
            SPI_Write(0X0062);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 4) // ronde
        {
            //printf("r#");
            SPI_Write(0X0072);
            SPI_Write(0X0023);
        }

        // RE
        else if(buffertiming[i] == 1 && buffernote[i] == 3) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 3) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 3) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 3) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else                        // autre
        {
            //printf("  "); // aucune note
            SPI_Write(0X0020);
            SPI_Write(0X0020);
        }

        if((i+1)%8 == 0 && i != 0)
        {
             //printf("|"); // fin dune mesure
             SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
    // ecrire la ligne de la note DO ou DO #

    //printf("DO|");
    SPI_Write(0X0044);
    SPI_Write(0X004F);
    SPI_Write(0X007C);

    for(i = 0; i < 32; i++)
    {
        // DO #
        if(buffertiming[i] == 1 && buffernote[i] == 2) // croche
        {
            //printf("c#");
            SPI_Write(0X0063);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 2) // noire
        {
            //printf("n#");
            SPI_Write(0X006E);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 2) // blanche
        {
            //printf("b#");
            SPI_Write(0X0062);
            SPI_Write(0X0023);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 2) // ronde
        {
            //printf("r#");
            SPI_Write(0X0072);
            SPI_Write(0X0023);
        }

        // DO
        else if(buffertiming[i] == 1 && buffernote[i] == 1) // croche
        {
            //printf("c-");
            SPI_Write(0X0063);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 2 && buffernote[i] == 1) // noire
        {
            //printf("n-");
            SPI_Write(0X006E);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 3 && buffernote[i] == 1) // blanche
        {
            //printf("b-");
            SPI_Write(0X0062);
            SPI_Write(0X002D);
        }
        else if(buffertiming[i] == 4 && buffernote[i] == 1) // ronde
        {
            //printf("r-");
            SPI_Write(0X0072);
            SPI_Write(0X002D);
        }
        else            // autre
        {
            //printf("--"); // aucune note
            SPI_Write(0X002D);
            SPI_Write(0X002D);
        }

        if((i+1)%8 == 0 && i != 0)
        {
             //printf("|"); // fin dune mesure
             SPI_Write(0X007C);
        }
    }
    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    //printf("\n");
    SPI_Write(0X000A);
    SPI_Write(0X000D);

    ////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////
}
