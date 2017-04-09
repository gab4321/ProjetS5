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

// variable de test de fiabilité de la détection des singulieres
extern int Buff_Do;
extern int Buff_Do_dies;
extern int Buff_Re;
extern int Buff_Re_dies;
extern int Buff_Mi;
extern int Buff_Fa;
extern int Buff_Fa_dies;
extern int Buff_Sol;
extern int Buff_Sol_dies;
extern int Buff_La;
extern int Buff_La_dies;
extern int Buff_Si;

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

    for(i = 0; i<ntrame; i++)
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
        //printf("signal periodique\n "); //a remplacer par des lumieres
        FlagLed0 = 0;
        *CPLD_USER_REG &=~0x01;      //éteindre led0
        *CPLD_USER_REG |=0x02;      //allumer led1
    }
    else
    {
        //printf("signal non periodique \n"); //a remplacer par des lumieres
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
    gen_w_r2(w, ntrame);
    bit_rev(w, ntrame>>1);

    // FFT sur la trame
    DSPF_sp_cfftr2_dit(TableInputPadder, w, ntrame);

    // Remise dans lordre non-bit-reversed
    bitrev_index(index,ntrame);
    DSPF_sp_bitrev_cplx((double*)TableInputPadder, index, ntrame);

    // Absolu du résultat pour observer le contenue en amplitude
    for(i = 0; i<ntrame; i++ )
    {
        Sortie_FFT[i] = sqrt(pow((TableInputPadder[2*i]),2) + pow((TableInputPadder[2*i+1]),2));
    }
}

/********************************************************************************************
fonction qui place le vecteur de la trame dans un vecteur avec Im = 0
********************************************************************************************/
void padderFFT(int *TableFFT, float *TableInputPadder)
{
    int n;

    for (n=0; n<ntrame*2; n++)
    {
        if(n%2 == 0)
            TableInputPadder[n] = (float)TableFFT[n/2];
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
    for(i = 0; i < ntrame; i++)
    {
        FenHamming[i] = 0.54 - 0.46*cos(2*PI*i/ntrame);
    }
}

/********************************************************************************************
fonction qui applique une fenetre
********************************************************************************************/
void ApplicationFentre(float *Fenetre, float *TableFFT)
{
    int i;
    for(i = 0; i < ntrame; i++)
    {
        TableFFT[i] = TableFFT[i]*Fenetre[i];
    }
}

/********************************************************************************************
Description : analyse le resultat de la FFT (POUR NOTES SINGULIERES)
********************************************************************************************/
int AnalyseFFT_Singuliere(float *Sortie_FFT)
{
    static int note;
    int ind1 = 0, ind2 = 0, ind3 = 0, ind4 = 0, i = 0;
    int max1 = 0, max2  = 0, max3 = 0, max4 = 0;
    float freq1;
    float freq2;
    float freq3;
    float freq4;

    float erreur = 6; //erreur sur la frequence en Hz pour la fondamentale
    float erreur2 = 12; //erreur sur la frequence en Hz pour la premiere harmonique
    float erreur3 = 25;

    int MargeElim = 10;

    // recherche la fondamentale (OCTAVE 4)
    for(i = 31; i < 66; i++ )   // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max1)
        {
            max1 = Sortie_FFT[i];
            ind1 = i;
        }
    }

    // recherche la premiere harmonique (OCTAVE 5)
    for(i = 66; i <= 130 ; i++ )  // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max2)
        {
            max2 = Sortie_FFT[i];
            ind2 = i;
        }
    }

    ///////////////////// OCTAVE 6 -> CAS BIZARR ///////////////////
    // recherche la troisieme harmonique (OCTAVE 6)
    for(i = 131; i <= 255 ; i++ )  // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max3)
        {
            max3 = Sortie_FFT[i];
            ind3 = i;
        }
    }

    for(i = (ind3-MargeElim); i < (ind3 + MargeElim); i++)
    {
        Sortie_FFT[i] = 0;
    }

    for(i = 131; i <= 255 ; i++ )  // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max4)
        {
            max4 = Sortie_FFT[i];
            ind4 = i;
        }
    }

    ///////////////////// OCTAVE 6 -> CAS BIZARR ///////////////////

    // frequence de la fondamentale (OCTAVE 4)
    freq1 = (float)(ind1*(float)Fs/(float)ntrame);

    // frequence de la premiere harmonique (OCTAVE 5)
    freq2 = (float)(ind2*(float)Fs/(float)ntrame);

    // frequence de la troisieme harmonique (OCTAVE 6) -> possible premier peak
    freq3 = (float)(ind3*(float)Fs/(float)ntrame);

    // frequence de la troisieme harmonique (OCTAVE 6) -> possible deuxieme peak
    freq4 = (float)(ind4*(float)Fs/(float)ntrame);

    // boucle de comparaison aux frequences singulieres (fondamentales et 1ere harmoniques)

    note = TrouveNote_W_Harm(freq1, freq2, freq3, freq4, erreur, erreur2, erreur3);

    return note;

}

/********************************************************************************************
Description : analyse le resultat de la FFT (POUR ACCORDS)
********************************************************************************************/
void AnalyseFFT_Accord(float *Sortie_FFT)
{
    int ind1 = 0, ind2 = 0, ind3 = 0, i = 0;
    int max1 = 0, max2  = 0, max3 = 0;
    float freq1;
    float freq2;
    float freq3;

    int note1, note2, note3;

    int MargeElim = 5; // marge delimination des peaks en nb dindice de FFT

    float erreur =11; //erreur sur la frequence en Hz pour la fondamentale
    //float erreur2 = 10; //erreur sur la frequence en Hz pour la premiere harmonique

    /////////////////// PREMIERE NOTE ///////////////////
    // recherche la premiere note
    for(i =30; i < 65; i++ )   // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max1)
        {
            max1 = Sortie_FFT[i];
            ind1 = i;
        }
    }

    // elimine le peaks de la premiere note
    for(i = (ind1-MargeElim); i < (ind1 + MargeElim); i++)
    {
        Sortie_FFT[i] = 0;
    }

    /////////////////// DEUXIEME NOTE ///////////////////

    for(i = 30; i < 65; i++ )   // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max2)
        {
            max2 = Sortie_FFT[i];
            ind2 = i;
        }
    }

    // elimine le peaks de la premiere note
    for(i = (ind2-MargeElim); i < (ind2 + MargeElim); i++)
    {
        Sortie_FFT[i] = 0;
    }

    /////////////////// TROISIEME NOTE ///////////////////

    for(i = 30; i < 65; i++ )   // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max3)
        {
            max3 = Sortie_FFT[i];
            ind3 = i;
        }
    }

    /////////////////// DETECTION ACCORD ///////////////////


    freq1 = (float)(ind1*(float)Fs/(float)ntrame);
    freq2 = (float)(ind2*(float)Fs/(float)ntrame);
    freq3 = (float)(ind3*(float)Fs/(float)ntrame);

    note1 = TrouveNote_WO_Harm(freq1, erreur);
    note2 = TrouveNote_WO_Harm(freq2, erreur);
    note3 = TrouveNote_WO_Harm(freq3, erreur);

    // Re mineur
    if(note1 == 3 || note2 == 3 || note3 == 3)
    {
        if(note1 == 6 || note2 == 6 || note3 == 6)
        {
            if(note1 == 10 || note2 == 10 || note3 == 10)
            {
                printf("Re mineur\n ");
            }
        }
    }

    // Re majeur
    if(note1 == 3 || note2 == 3 || note3 == 3)
    {
        if(note1 == 7 || note2 == 7 || note3 == 7)
        {
            if(note1 == 10 || note2 == 10 || note3 == 10)
            {
                printf("Re majeur\n ");
            }
        }
    }

    // C
    if(note1 == 1 || note2 == 1 || note3 == 1)
    {
        if(note1 == 5 || note2 == 5 || note3 == 5)
        {
            if(note1 == 8 || note2 == 8 || note3 == 8)
            {
                printf("C\n ");
            }
        }
    }

    // Sol inv
    if(note1 == 3 || note2 == 3 || note3 == 3)
    {
        if(note1 == 8 || note2 == 8 || note3 == 8)
        {
            if(note1 == 12 || note2 == 12 || note3 == 12)
            {
                printf("Sol inv\n ");
            }
        }
    }

    /*
    if(note1 == -1 || note2 == -1 || note3 == -1)
    {
        //printf("rien\n ");
    }
    */
}

/********************************************************************************************
Description : trouver si une frequence correspond a une note (AVEC 1ere HARMONIQUE -> SING)
********************************************************************************************/
int TrouveNote_W_Harm(float freq1, float freq2, float freq3, float freq4, float erreur, float erreur2, float erreur3)
{
    int notes = -1;

    // Do
    if(freq1 > (Do-erreur) && freq1 < (Do+erreur))
    {
        if(freq2 > 2*Do-erreur2 && freq2 < 2*Do+erreur2)
        {
            if((freq3 >4*Do-erreur3 && freq3 < 4*Do+erreur3) || (freq4 >4*Do-erreur3 && freq4 < 4*Do+erreur3))
            {
                //printf("note: Do\n ");
                //Buff_Do++;
                notes = 1;
            }
        }
    }

    // Do_dies
    if(freq1 > (Do_dies-erreur) && freq1 < (Do_dies+erreur))
    {
        if(freq2 > 2*Do_dies-erreur2 && freq2 < 2*Do_dies+erreur2)
        {
            if((freq3 > 4*Do_dies-erreur3 && freq3 < 4*Do_dies+erreur3) || (freq4 > 4*Do_dies-erreur3 && freq4 < 4*Do_dies+erreur3))
            {
                //printf("note: Do dies\n ");
                //Buff_Do_dies++;
                notes = 2;
            }
        }
    }

    // Re
    if(freq1 > (Re-erreur) && freq1 < (Re+erreur))
    {
        if(freq2 > 2*Re-erreur2 && freq2 < 2*Re+erreur2)
        {
            if((freq3 > 4*Re-erreur3 && freq3 < 4*Re+erreur3) || (freq4 > 4*Re-erreur3 && freq4 < 4*Re+erreur3))
            {
                //printf("note: Re\n ");
                //Buff_Re++;
                notes = 3;
            }
        }
    }

    // Re_dies
    if(freq1 > (Re_dies-erreur) && freq1 < (Re_dies+erreur))
    {
        if(freq2 > 2*Re_dies-erreur2 && freq2 < 2*Re_dies+erreur2)
        {
            if((freq3 > 4*Re_dies-erreur3 && freq3 < 4*Re_dies+erreur3) || (freq4 > 4*Re_dies-erreur3 && freq4 < 4*Re_dies+erreur3))
            {
                //printf("note: Re dies\n ");
                //Buff_Re_dies++;
                notes = 4;
            }
        }
    }

    // Mi
    if(freq1 > (Mi-erreur) && freq1 < (Mi+erreur))
    {
        if(freq2 > 2*Mi-erreur2 && freq2 < 2*Mi+erreur2)
        {
            if((freq3 > 4*Mi-erreur3 && freq3 < 4*Mi+erreur3) || (freq4 > 4*Mi-erreur3 && freq4 < 4*Mi+erreur3))
            {
                //printf("note: Mi\n ");
                //Buff_Mi++;
                notes = 5;
            }
        }
    }

    // Fa
    if(freq1 > (Fa-erreur) && freq1 < (Fa+erreur))
    {
        if(freq2 > 2*Fa-erreur2 && freq2 < 2*Fa+erreur2)
        {
            if((freq3 > 4*Fa-erreur3 && freq3 < 4*Fa+erreur3) || (freq4 > 4*Fa-erreur3 && freq4 < 4*Fa+erreur3))
            {
                //printf("note: Fa\n ");
                //Buff_Fa++;
                notes = 6;
            }
        }
    }

    // Fa_dies
    if(freq1 > (Fa_dies-erreur) && freq1 < (Fa_dies+erreur))
    {
        if(freq2 > 2*Fa_dies-erreur2 && freq2 < 2*Fa_dies+erreur2)
        {
            if((freq3 > 4*Fa_dies-erreur3 && freq3 < 4*Fa_dies+erreur3) || (freq4 > 4*Fa_dies-erreur3 && freq4 < 4*Fa_dies+erreur3))
            {
                //printf("note: Fa dies\n ");
                //Buff_Fa_dies++;
                notes = 7;
            }
        }
    }

    // Sol
    if(freq1 > (Sol-erreur) && freq1 < (Sol+erreur))
    {
        if(freq2 > 2*Sol-erreur2 && freq2 < 2*Sol+erreur2)
        {
            if((freq3 > 4*Sol-erreur3 && freq3 < 4*Sol+erreur3) || (freq4 > 4*Sol-erreur3 && freq4 < 4*Sol+erreur3))
            {
                //printf("note: Sol\n ");
                //Buff_Sol++;
                notes = 8;
            }
        }
    }

    // Sol_dies
    if(freq1 > (Sol_dies-erreur) && freq1 < (Sol_dies+erreur))
    {
        if(freq2 > 2*Sol_dies-erreur2 && freq2 < 2*Sol_dies+erreur2)
        {
            if((freq3 > 4*Sol_dies-erreur3 && freq3 < 4*Sol_dies+erreur3) || (freq4 > 4*Sol_dies-erreur3 && freq4 < 4*Sol_dies+erreur3))
            {
                //printf("note: Sol dies\n ");
                //Buff_Sol_dies++;
                notes = 9;
            }
        }
    }

    // La
    if(freq1 > (La-erreur) && freq1 < (La+erreur))
    {
        if(freq2 > 2*La-erreur2 && freq2 < 2*La+erreur2)
        {
            if((freq3 > 4*La-erreur3 && freq3 < 4*La+erreur3) || (freq4 > 4*La-erreur3 && freq4 < 4*La+erreur3))
            {
                //printf("note: La\n ");
               // Buff_La++;
                notes = 10;
            }
        }
    }

    // La_dies
    if(freq1 > (La_dies-erreur) && freq1 < (La_dies+erreur))
    {
        if(freq2 > 2*La_dies-erreur2 && freq2 < 2*La_dies+erreur2)
        {
            if((freq3 > 4*La_dies-erreur3 && freq3 < 4*La_dies+erreur3) || (freq4 > 4*La_dies-erreur3 && freq4 < 4*La_dies+erreur3))
            {
                //printf("note: La dies\n ");
                //Buff_La_dies++;
                notes = 11;
            }
        }
    }

    // Si
    if(freq1 > (Si-erreur) && freq1 < (Si+erreur))
    {
        if(freq2 > 2*Si-erreur2 && freq2 < 2*Si+erreur2)
        {
            if((freq3 > 4*Si-erreur3 && freq3 < 4*Si+erreur3) || (freq4 > 4*Si-erreur3 && freq4 < 4*Si+erreur3))
            {
                //printf("note: Si\n ");
                //Buff_Si++;
                notes = 12;
            }
        }
    }

    return notes;
}

/********************************************************************************************
Description : trouver si une frequence correspond a une note (SANS HARMONIQUE -> ACCORD)
********************************************************************************************/
int TrouveNote_WO_Harm(float freq, float erreur)
{
    // Do
    if(freq > (Do-erreur) && freq < (Do+erreur))
    {
        // 1 = Do
        return 1;
    }

    // Do_dies
    if(freq > (Do_dies-erreur) && freq < (Do_dies+erreur))
    {
        // 2 = Do_dies
        return 2;
    }

    // Re
    if(freq > (Re-erreur) && freq < (Re+erreur))
    {
        // 3 = Re
        return 3;
    }

    // Re_dies
    if(freq > (Re_dies-erreur) && freq < (Re_dies+erreur))
    {
        // 4 = Re_dies
        return 4;
    }

    // Mi
    if(freq > (Mi-erreur) && freq < (Mi+erreur))
    {
        // 5 = Mi
        return 5;
    }

    // Fa
    if(freq > (Fa-erreur) && freq < (Fa+erreur))
    {
        // 6 = Fa
        return 6;
    }

    // Fa_dies
    if(freq > (Fa_dies-erreur) && freq < (Fa_dies+erreur))
    {
        // 7 = Fa_dies
        return 7;
    }

    // Sol
    if(freq > (Sol-erreur) && freq < (Sol+erreur))
    {
        // 8 = Sol
        return 8;
    }

    // Sol_dies
    if(freq > (Sol_dies-erreur) && freq < (Sol_dies+erreur))
    {
        // 9 = Sol_dies
        return 9;
    }

    // La
    if(freq > (La-erreur) && freq < (La+erreur))
    {
        // 10 = La
        return 10;
    }

    // La_dies
    if(freq > (La_dies-erreur) && freq < (La_dies+erreur))
    {
        // 11 = La_dies
        return 11;
    }

    // Si
    if(freq > (Si-erreur) && freq < (Si+erreur))
    {
        // 12 = Si
        return 12;
    }

    return -1;
}

/********************************************************************************************
Description : genere le prochain echantillon de metronome a jouer
********************************************************************************************/
short GenererMetronome(short table[], int nbexecution, int nbcompteur, int nbValSinus, int *compteurTemps, int *compteurNbFois, int *compteurSinus, int *compteurmesure)
{
    //static int compteurmesure = 0;
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

        if (*compteurmesure==3)
        {
            *compteurSinus = *compteurSinus + 2;
        }
        else
        {
            *compteurSinus = *compteurSinus + 1;
        }

        if(*compteurSinus >= nbValSinus)
        {
            *compteurSinus = 0;
            *compteurNbFois = *compteurNbFois + 1;
        }

        if (*compteurmesure==3)
        {
            if(*compteurNbFois >= nbexecution*4)
            {
                start = 0;
                *compteurSinus = 0;
                *compteurNbFois = 0;

            }
        }
        else
        {
            if(*compteurNbFois >= nbexecution)
            {
                start = 0;
                *compteurSinus = 0;
                *compteurNbFois = 0;

            }
        }
    }

    *compteurTemps = *compteurTemps + 1;

    if(*compteurTemps >= nbcompteur)
    {
        *compteurTemps = 0;
        if (*compteurmesure >= 3)
            *compteurmesure = 0;
        else
            *compteurmesure = *compteurmesure + 1;
    }

    if(start == 0)
    {
        valactuelle = 0;
    }

    return valactuelle;
}

/********************************************************************************************
Description : genere les parametre de la fonction de metronome selon les parametre selectionnés
********************************************************************************************/
void InitialiseMetronome(short table[], int *nbexecution, int *nbcompteur, int nbValSinus,int volumeMet, int freqMet,int tempsMet, int BPM)
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






