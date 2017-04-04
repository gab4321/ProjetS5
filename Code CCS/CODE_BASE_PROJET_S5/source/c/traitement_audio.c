/*
 * traitement_audio.c
 */

#include "traitement_audio.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "twiddle.h"

#include "DSPF_sp_cfftr2_dit.h"
#include "DSPF_sp_cfftr4_dif.h"
#include "bitrev_index.h"
#include "DSPF_sp_bitrev_cplx.h"
#include "gen_w_r2.h"
#include "bit_rev.c"

/********************************************************************************************
Variables pour les fonctions de traitement audio
********************************************************************************************/
// adresse du USER_REG du CPLD pour LED et Switch
#define CPLD_USER_REG (unsigned int *) 0x90080000

extern const double PI;
extern const int Fs;

// variables du main pour la correlation
extern int ndecalage;
extern int ncorr;
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

/********************************************************************************************
fonction de preparation du vecteur de correlation
********************************************************************************************/
void CorrManip(int *VectCorr, int *VectPadder)
{
    int i;

    for(i = 0; i<ndecalage; i++)
    {
        VectPadder[i] = 0;
    }

    for(i = 0; i<ncorr; i++)
    {
        VectPadder[i+ndecalage] = VectCorr[i];
    }

    for(i = npadd-1 ; i>(npadd-1-ndecalage);i--)
    {
        VectPadder[i] = 0;
    }
}

/********************************************************************************************
fonction qui test lintensite du son avant de lancer lautocorrelation
********************************************************************************************/
void TestIntensite(int *TestAmp, int ntestson)
{
    int i;
    int intensite = 0;

    for(i = 0; i < ntestson; i++)
    {
        if(TestAmp[i] <0)
        {TestAmp[i] = -1*TestAmp[i];}
        intensite = intensite + TestAmp[i];
    }

    intensite = intensite/ntestson;


    if(intensite > SeuilSon)
    {
        IsSound = 1;
    }
    else
    {
        // printf("pas de son\n");  //a remplacer par des lumieres
        if(FlagLed0==0)
        {

            *CPLD_USER_REG &=~0x06;     //éteindre led1 & 2
            *CPLD_USER_REG |=0x01;      //allumer led0
            FlagLed0 =1;
        }
    }
}

/********************************************************************************************
fonction qui test la periodicite dun vecteur de sortie de lautocorrelation
********************************************************************************************/
void TestPeriodicite(int *VectRep, int *TabPeaks, int nrep)
{
    int i;
    int maxcorr = 0;
    int npeaks = 0;
    double interIni;
    int detect = 1;
    double interK;
    double a;

    //detection de lamplitude maximale de la correlation
    for(i = 0; i < nrep; i++)
    {
        if ( abs(VectRep[i]) > maxcorr )
            maxcorr = abs(VectRep[i]);
    }

    //scaling du maximum
    maxcorr = maxcorr * scale;

    //detection des indices i des peaks de la correlation
    for(i = 2; i < nrep-2; i++)
    {
        if(VectRep[i] > VectRep[i-1] && VectRep[i] > VectRep[i-2])
        {
            if(VectRep[i] > VectRep[i+1] && VectRep[i] > VectRep[i+2])
            {
                if(VectRep[i] > maxcorr)
                {
                    TabPeaks[npeaks] = i;
                    npeaks+=1;
                }
            }
        }
    }

    // code d'erreur si seulement un peak est trouvé (signal apériodique)
    if(npeaks < 8)
    {
        detect = 0;
    }

    //detection des intervalles K entre les peaks
    //(le nb de verifications necessaires est a valider min = 2)
    interIni = (double)(TabPeaks[1] - TabPeaks[0]);
    for(i = 1; i < nVerif; i++ )
    {
        interK = (double)(TabPeaks[i+1] - TabPeaks[i]);
        a = ((interK-interIni)/interIni)*100.0;
        if( a > 6) //si ecart relative sup a 5 %
        {
            detect = 0;
            break;
        }
    }

    if(detect == 1)
    {
        printf("signal periodique\n "); //a remplacer par des lumieres
        FlagLed0 = 0;
        *CPLD_USER_REG &=~0x01;      //éteindre led0
        *CPLD_USER_REG |=0x02;      //allumer led1
    }
    else
    {
        printf("signal non periodique \n"); //a remplacer par des lumieres
        FlagLed0 = 0;
        *CPLD_USER_REG &=~0x01;      //éteindre led0
        *CPLD_USER_REG |=0x04;      //allumer led2
    }
}

/********************************************************************************************
fonction qui effectue la FFT de la trame
********************************************************************************************/
void calculFFT(float *TableInputPadder, float *Sortie_FFT,float *w, short *index)
{
    int i = 0;

    // Génération des twiddles factor
    gen_w_r2(w, ncorr);
    bit_rev(w, ncorr>>1);

    // FFT sur la trame
    DSPF_sp_cfftr2_dit(TableInputPadder, w, ncorr);

    // Remise dans lordre non-bit-reversed
    bitrev_index(index,ncorr);
    DSPF_sp_bitrev_cplx((double*)TableInputPadder, index, ncorr);

    // Absolu du résultat pour observer le contenue en amplitude
    for(i = 0; i<ncorr; i++ )
    {
        Sortie_FFT[i] = sqrt(pow((TableInputPadder[2*i]),2) + pow((TableInputPadder[2*i+1]),2));
    }
}

/********************************************************************************************
fonction qui place le vecteur de la trame dans un vecteur avec Im = 0
********************************************************************************************/
void padderFFT(float *TableFFT, float *TableInputPadder)
{
    int n;

    for (n=0; n<ncorr*2; n++)
    {
        if(n%2 == 0)
            TableInputPadder[n] = TableFFT[n/2];
        else
            TableInputPadder[n] = 0;
    }
}

/********************************************************************************************
fonction qui genre une fenetre de hamming (PAS NECESSAIREMENT LA FENETRE QUE LON CHOISIRA)
********************************************************************************************/
void GenHamming(float* FenHamming)
{
    int i;
    for(i = 0; i < ncorr; i++)
    {
        FenHamming[i] = 0.54 - 0.46*cos(2*PI*i/ncorr);
    }
}

/********************************************************************************************
fonction qui applique une fenetre
********************************************************************************************/
void ApplicationFentre(float *Fenetre, float *TableFFT)
{
    int i;
    for(i = 0; i < ncorr; i++)
    {
        TableFFT[i] = TableFFT[i]*Fenetre[i];
    }
}

/********************************************************************************************
Description : analyse le resultat de la FFT
********************************************************************************************/
void AnalyseFFT(float *Sortie_FFT)
{
    // A FAIRE !!!!!!!
}

/********************************************************************************************
Description : genere le prochain echantillon de metronome a jouer
********************************************************************************************/
short GenererMetronome(short *table, int nbexecution, int nbcompteur, int nbValSinus, int *compteurTemps, int *compteurNbFois, int *compteurSinus)
{
    static int start;
    static short valactuelle;

    if(*compteurTemps == 0)
    {
        *compteurNbFois = 0;
        *compteurSinus = 0;

        start = 1;
    }

    if(start == 1)
    {

        valactuelle = table[*compteurSinus];

        if(*compteurSinus++ >= nbValSinus)
        {
            *compteurSinus = 0;
            *compteurNbFois++;
        }

        if(*compteurNbFois >= nbexecution)
        {
            start = 0;
            *compteurSinus = 0;
            *compteurNbFois = 0;

        }
    }

    if(*compteurTemps++ >= nbcompteur)
        *compteurTemps = 0;


    if(start == 0)
    {
        valactuelle = 0;
    }

    return valactuelle;
}

/********************************************************************************************
Description : genere les parametre de la fonction de metronome selon les parametre selectionnés
********************************************************************************************/
void InitialiseMetronome(short *table, int *nbexecution, int *nbcompteur, int nbValSinus,int volumeMet, int freqMet,int tempsMet, int BPM)
{
    int n;

    // genere la table de sinus
    for (n = 0; n < nbValSinus; n++)
    {
        table[n] = (short)(volumeMet*cos(2.0 * PI * (float)freqMet * (float)n / (float)Fs));
    }

    // nb de fois quil faut executer le tableau de sinus pour faire le temps desirer par coup de metronome
    *nbexecution = ((float)tempsMet/1000.0)/((float)nbValSinus*1.0/((float)Fs));

    // nb de coup de frequence 1/fs a attendre avant doutputter le prochain coup de metronome
    *nbcompteur = (1.0/((float)BPM/60.0))/(1.0/((float)Fs));
}






