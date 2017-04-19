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
extern int ncorr;
extern int npadd;

// variable du main pour la detection de periodicit�
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
fonction qui fait la moyenne des intensit� dune trame en mode synchrone
********************************************************************************************/
int intensitesynchrone(int *Vectacq)
{

    int i;
    int intensite = 0;

    for(i = 0; i < ntrame; i++)
    {
        if(Vectacq[i] < 0)
        {
            Vectacq[i] = -1*Vectacq[i];
        }
        intensite = intensite + Vectacq[i];
    }

    intensite = intensite/ntrame;

    return intensite;
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
        /*
        // printf("pas de son\n");  //a remplacer par des lumieres
        if(FlagLed0==0)
        {

            *CPLD_USER_REG &=~0x06;     //�teindre led1 & 2
            *CPLD_USER_REG |=0x01;      //allumer led0
            FlagLed0 =1;
        }
        */
    }
}

/********************************************************************************************
fonction qui test la periodicite dun vecteur de sortie de lautocorrelation
********************************************************************************************/
int TestPeriodicite(int *VectRep, int *VectRep2, int *TabPeaks, int *TabPeaks2, int nrep)
{

    int i;
    int maxcorr = 0;
    int maxcorr2 =0;
    int npeaks = 0;
    int npeaks2 = 0;
    int detect = 1;
    double a;

    //detection de lamplitude maximale de la correlation
    for(i = 0; i < nrep; i++)
    {
        if ( VectRep[i] > maxcorr )
            maxcorr = VectRep[i];

        if ( VectRep2[i] > maxcorr2 )
            maxcorr2 = VectRep[i];
    }

    //scaling du maximum
    maxcorr = maxcorr * scale;
    maxcorr2 = maxcorr2 * scale;

    //detection des indices i des peaks de la correlation
    for(i = 4; i < nrep-4; i++)
    {
        if(VectRep[i] > VectRep[i-1] && VectRep[i] > VectRep[i-2] && VectRep[i] > VectRep[i-3] && VectRep[i] > VectRep[i-4])
        {
            if(VectRep[i] > VectRep[i+1] && VectRep[i] > VectRep[i+2] && VectRep[i] > VectRep[i+3] && VectRep[i] > VectRep[i+4])
            {
                if(VectRep[i] > maxcorr)
                {
                    TabPeaks[npeaks] = i;
                    npeaks+=1;
                }
            }
        }

        if(VectRep2[i] > VectRep2[i-1] && VectRep2[i] > VectRep2[i-2] && VectRep2[i] > VectRep2[i-3] && VectRep2[i] > VectRep2[i-4])
        {
            if(VectRep2[i] > VectRep2[i+1] && VectRep2[i] > VectRep2[i+2] && VectRep2[i] > VectRep2[i+3] && VectRep2[i] > VectRep2[i+4])
            {
                if(VectRep2[i] > maxcorr2)
                {
                    TabPeaks2[npeaks2] = i;
                    npeaks2+=1;
                }
            }
        }
    }

    //Comparaison peak entre les 2 trames
    if(npeaks != npeaks2)                           /// PAS SUR
        detect = 0;

    for(i = 0; i < npeaks; i++)
    {
        a = abs(TabPeaks[i] - TabPeaks2[i]);
        if(a > 5)
            detect = 0;
    }

    if(detect == 1)
    {
        //printf("signal periodique \n"); //a remplacer par des lumieres
        *CPLD_USER_REG &=~0x04;      //�teindre led2
        *CPLD_USER_REG |=0x02;      //allumer led1
    }
    else
    {
        //printf("signal non periodique \n"); //a remplacer par des lumieres
        *CPLD_USER_REG &=~0x02;      //�teindre led1
        *CPLD_USER_REG |=0x04;      //allumer led2
    }

    return detect;
}

/********************************************************************************************
fonction qui effectue la FFT de la trame
********************************************************************************************/
void calculFFT(float *TableInputPadder, float *Sortie_FFT,float *w, short *index)
{
    int i = 0;

    // G�n�ration des twiddles factor
    gen_w_r2(w, ntrame);
    bit_rev(w, ntrame>>1);

    // FFT sur la trame
    DSPF_sp_cfftr2_dit(TableInputPadder, w, ntrame);

    // Remise dans lordre non-bit-reversed
    bitrev_index(index,ntrame);
    DSPF_sp_bitrev_cplx((double*)TableInputPadder, index, ntrame);

    // Absolu du r�sultat pour observer le contenue en amplitude
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
    int maxverif = 0;
    int indverif = 0;
    float freq1 = 0;
    float freq2 = 0;
    float freq3 = 0;
    float freq4 = 0;

    float erreur = 6; //erreur sur la frequence en Hz pour la fondamentale
    float erreur2 = 15; //erreur sur la frequence en Hz pour la premiere harmonique
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

    for(i = (ind1-MargeElim); i < (ind1 + MargeElim); i++)
    {
        Sortie_FFT[i] = 0;
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // VERIF DAMPLITUDE relatif a max1
    for(i = 31; i < 66; i++ )   // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > maxverif)
        {
            maxverif = Sortie_FFT[i];
            indverif = i;
        }
    }

    if(max1 < 2*maxverif)
    {
        // voir si on garde ou pas cette condition
        // return -1;
    }
    /////////////////////////////////////////////////////////////////////////////////////////////

    // recherche la premiere harmonique (OCTAVE 5)
    for(i = 66; i <= 130 ; i++ )  // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max2)
        {
            max2 = Sortie_FFT[i];
            ind2 = i;
        }
    }

    // delete le peak
    for(i = (ind2-MargeElim); i < (ind2 + MargeElim); i++)
    {
        Sortie_FFT[i] = 0;
    }

    ///////////////////// OCTAVE 6  ///////////////////
    // recherche la troisieme harmonique (OCTAVE 6)
    for(i = 131; i <= 255 ; i++ )  // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max3)
        {
            max3 = Sortie_FFT[i];
            ind3 = i;
        }
    }

    // delete le peak
    for(i = (ind3-MargeElim); i < (ind3 + MargeElim); i++)
    {
        Sortie_FFT[i] = 0;
    }

    // recherche la quatrieme harmonique dans loctave 6 (SELON LA NOTE)
    for(i = 131; i <= 255 ; i++ )  // bornes selon trame de 1024 points a 8000 Khz
    {
        if(Sortie_FFT[i] > max4)
        {
            max4 = Sortie_FFT[i];
            ind4 = i;
        }
    }

    // frequence de la fondamentale (OCTAVE 4)
    freq1 = (float)(ind1*(float)Fs/(float)ntrame);

    // frequence de la premiere harmonique (OCTAVE 5) ->
    freq2 = (float)(ind2*(float)Fs/(float)ntrame);

    // frequence de la troisieme harmonique (OCTAVE 6) -> possible premier peak
    freq3 = (float)(ind3*(float)Fs/(float)ntrame);

    // frequence de la troisieme harmonique (OCTAVE 6) -> possible deuxieme peak
    freq4 = (float)(ind4*(float)Fs/(float)ntrame);


    // boucle de comparaison aux frequences singulieres

    note = TrouveNote_W_Harm(freq1, freq2, freq3, freq4, erreur, erreur2, erreur3);

    return note;
}

/********************************************************************************************
Description : analyse le resultat de la FFT (POUR ACCORDS)
********************************************************************************************/
int AnalyseFFT_Accord(float *Sortie_FFT)
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
            if(max3 < 0.25*max2)
                ind3 = 0;
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
                //printf("Re mineur\n ");
                return 1;
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
                //printf("Re majeur\n ");
                return 2;
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
                //printf("C\n ");
                return 3;
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
                //printf("Sol inv\n ");
                return 4;
            }
        }
    }

    return -1;

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
    static int notes = -1;

    // Do
    if(freq1 > (Do-erreur) && freq1 < (Do+erreur))
    {
        //if(freq2 > 2*Do-erreur2 && freq2 < 2*Do+erreur2)
        //{
            if((freq3 >4*Do-erreur3 && freq3 < 4*Do+erreur3) || (freq4 >4*Do-erreur3 && freq4 < 4*Do+erreur3) || (freq2 > 2*Do-erreur2 && freq2 < 2*Do+erreur2))
            {
                //printf("note: Do\n ");
                //Buff_Do++;
                notes = 1;
            }
        //}
    }
    // Si
    else if(freq1 > (Si-erreur) && freq1 < (Si+erreur))
    {
        //if(freq2 > 2*Si-erreur2 && freq2 < 2*Si+erreur2)
        //{
            if((freq3 > 4*Si-erreur3 && freq3 < 4*Si+erreur3) || (freq4 > 4*Si-erreur3 && freq4 < 4*Si+erreur3) || (freq2 > 2*Si-erreur2 && freq2 < 2*Si+erreur2))
            {
               //printf("note: Si\n ");
                //Buff_Si++;
                notes = 12;
            }
        //}
    }

    // Do_dies
    else if(freq1 > (Do_dies-erreur) && freq1 < (Do_dies+erreur))
    {
        //if(freq2 > 2*Do_dies-erreur2 && freq2 < 2*Do_dies+erreur2)
        //{
            if((freq3 > 4*Do_dies-erreur3 && freq3 < 4*Do_dies+erreur3) || (freq4 > 4*Do_dies-erreur3 && freq4 < 4*Do_dies+erreur3) || (freq2 > 2*Do_dies-erreur2 && freq2 < 2*Do_dies+erreur2))
            {
                //printf("note: Do dies\n ");
                //Buff_Do_dies++;
                notes = 2;
            }
        //}
    }

    // Re
    else if(freq1 > (Re-erreur) && freq1 < (Re+erreur))
    {
        //if(freq2 > 2*Re-erreur2 && freq2 < 2*Re+erreur2)
       // {
            if((freq3 > 4*Re-erreur3 && freq3 < 4*Re+erreur3) || (freq4 > 4*Re-erreur3 && freq4 < 4*Re+erreur3) || (freq2 > 2*Re-erreur2 && freq2 < 2*Re+erreur2))
            {
                //printf("note: Re\n ");
                //Buff_Re++;
                notes = 3;
            }
       // }
    }

    // Re_dies
    else if(freq1 > (Re_dies-erreur) && freq1 < (Re_dies+erreur))
    {
       // if(freq2 > 2*Re_dies-erreur2 && freq2 < 2*Re_dies+erreur2)
        //{
            if((freq3 > 4*Re_dies-erreur3 && freq3 < 4*Re_dies+erreur3) || (freq4 > 4*Re_dies-erreur3 && freq4 < 4*Re_dies+erreur3) || (freq2 > 2*Re_dies-erreur2 && freq2 < 2*Re_dies+erreur2))
            {
                //printf("note: Re dies\n ");
                //Buff_Re_dies++;
                notes = 4;
            }
        //}
    }

    // Mi
    else if(freq1 > (Mi-erreur) && freq1 < (Mi+erreur))
    {
        //if(freq2 > 2*Mi-erreur2 && freq2 < 2*Mi+erreur2)
       // {
            if((freq3 > 4*Mi-erreur3 && freq3 < 4*Mi+erreur3) || (freq4 > 4*Mi-erreur3 && freq4 < 4*Mi+erreur3) || (freq2 > 2*Mi-erreur2 && freq2 < 2*Mi+erreur2))
            {
               //printf("note: Mi\n ");
                //Buff_Mi++;
                notes = 5;
            }
       // }
    }

    // Fa
    else if(freq1 > (Fa-erreur) && freq1 < (Fa+erreur))
    {
       // if(freq2 > 2*Fa-erreur2 && freq2 < 2*Fa+erreur2)
       // {
            if((freq3 > 4*Fa-erreur3 && freq3 < 4*Fa+erreur3) || (freq4 > 4*Fa-erreur3 && freq4 < 4*Fa+erreur3) || (freq2 > 2*Fa-erreur2 && freq2 < 2*Fa+erreur2))
            {
               //printf("note: Fa\n ");
                //Buff_Fa++;
                notes = 6;
           }
        //}
    }

    // Fa_dies
    else if(freq1 > (Fa_dies-erreur) && freq1 < (Fa_dies+erreur))
    {
       // if(freq2 > 2*Fa_dies-erreur2 && freq2 < 2*Fa_dies+erreur2)
        //{
            if((freq3 > 4*Fa_dies-erreur3 && freq3 < 4*Fa_dies+erreur3) || (freq4 > 4*Fa_dies-erreur3 && freq4 < 4*Fa_dies+erreur3) || (freq2 > 2*Fa_dies-erreur2 && freq2 < 2*Fa_dies+erreur2))
            {
                //printf("note: Fa dies\n ");
                //Buff_Fa_dies++;
                notes = 7;
            }
       // }
    }

    // Sol
    else if(freq1 > (Sol-erreur) && freq1 < (Sol+erreur))
    {
        //if(freq2 > 2*Sol-erreur2 && freq2 < 2*Sol+erreur2)
       // {
            if((freq3 > 4*Sol-erreur3 && freq3 < 4*Sol+erreur3) || (freq4 > 4*Sol-erreur3 && freq4 < 4*Sol+erreur3) || (freq2 > 2*Sol-erreur2 && freq2 < 2*Sol+erreur2))
            {
                //printf("note: Sol\n ");
                //Buff_Sol++;
                notes = 8;
            }
        //}
    }

    // Sol_dies
    else if(freq1 > (Sol_dies-erreur) && freq1 < (Sol_dies+erreur))
    {
        //if(freq2 > 2*Sol_dies-erreur2 && freq2 < 2*Sol_dies+erreur2)
       // {
            if((freq3 > 4*Sol_dies-erreur3 && freq3 < 4*Sol_dies+erreur3) || (freq4 > 4*Sol_dies-erreur3 && freq4 < 4*Sol_dies+erreur3) || (freq2 > 2*Sol_dies-erreur2 && freq2 < 2*Sol_dies+erreur2))
            {
                //printf("note: Sol dies\n ");
                ///Buff_Sol_dies++;
                notes = 9;
            }
        //}
    }

    // La
    else if(freq1 > (La-erreur) && freq1 < (La+erreur))
    {
        //if(freq2 > 2*La-erreur2 && freq2 < 2*La+erreur2)
        //{
            if((freq3 > 4*La-erreur3 && freq3 < 4*La+erreur3) || (freq4 > 4*La-erreur3 && freq4 < 4*La+erreur3) || (freq2 > 2*La-erreur2 && freq2 < 2*La+erreur2))
            {
                //printf("note: La\n ");
               // Buff_La++;
                notes = 10;
            }
        //}
    }

    // La_dies
    else if(freq1 > (La_dies-erreur) && freq1 < (La_dies+erreur))
    {
        //if(freq2 > 2*La_dies-erreur2 && freq2 < 2*La_dies+erreur2)
       // {
            if((freq3 > 4*La_dies-erreur3 && freq3 < 4*La_dies+erreur3) || (freq4 > 4*La_dies-erreur3 && freq4 < 4*La_dies+erreur3) || (freq2 > 2*La_dies-erreur2 && freq2 < 2*La_dies+erreur2))
            {
                //printf("note: La dies\n ");
               //Buff_La_dies++;
                notes = 11;
            }
        //}
    }
    else
    {
        notes = -1;
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
Description : donne un vecteur avec les timing associ�s a chaque detection de notes
********************************************************************************************/
void traitementtiming(int *bufferintensite, int *buffernote, int *buffertiming)
{
    // Indice du buffer timing:
    // 1: croche
    // 2: noire
    // 3: blanche
    // 4: ronde
    // -1: rien (un silence ou la fin dune note: le chiffre est mis au commencement de la note on pad avec des -1 apres...)

    // initialisation des parametres
    static int ii = 0;
    int compteurNoteAlike = 0;
    int notetemp = -1;
    int amptemp = 0;
    int indtemp = 0;

    for(ii = 0; ii < 32; ii++) // 32: le vecteur contient 4 mesures :)
    {
        // ON VOIT LA MEME NOTE EN PARCOURANT LE VECTEUR
        if(buffernote[ii] == notetemp && notetemp != -1)
        {

            // on detecte la meme note pour la premiere fois
            if(bufferintensite[ii] <  0.5*amptemp && compteurNoteAlike == 0) // difference assez grande pour dire que cest une noire et non une croche (A AJUSTER)
            {
                buffertiming[indtemp] = 2; // la note est une noire

                //amptemp = Buff_amp[ii]; // voir si on garde cette ligne de code******

                buffertiming[ii] = -1;

                compteurNoteAlike++;
            }

            // on detecte la meme note une deuxieme fois et lamplitude est plus petite que la premiere amplitude enregistr�e
            // voir si cest mieux de comprarer a la derniere amplitude enregistr� ou la premiere amplitude quand on detecte
            // la note pour la premiere fois -> FAIRE ATTENTION AU SUBSTAIN!!!!!
            // premiere condition remplie pour que ca soit une blanche
            else if(compteurNoteAlike == 1 && bufferintensite[ii] < 0.5*amptemp)
            {
                //  a voir si on prend 3 ou 4 detections pour la blanche
                //Buff_timing[indtemp] = 3; // la note est une blanche

                buffertiming[ii] = 5;

                compteurNoteAlike++;
            }

            // on detecte la meme note une troisieme fois et lamplitude est plus petite que la premiere amplitude enregistr�e
            // condition remplie pour que ca soit une blanche
            else if(compteurNoteAlike == 2 && bufferintensite[ii] < 0.5*amptemp)
            {
                buffertiming[indtemp] = 3; // la note est une blanche

                buffertiming[ii-1] = -1;
                buffertiming[ii] = -1;

                compteurNoteAlike++;
            }

            // on detecte la meme note une quatrieme fois et lamplitude est plus petite que la premiere amplitude enregistr�e
            // premiere condition remplie pour que ca soit une ronde
            else if(compteurNoteAlike == 3 && bufferintensite[ii] < 0.5*amptemp)
            {

                buffertiming[ii] = 5;

                compteurNoteAlike++;
            }

            // on detecte la meme note une cinquieme fois et lamplitude est plus petite que la premiere amplitude enregistr�e
            // condition remplie pour que ca soit une ronde
            // ici on detecte 6 fois la meme note car trop de glitch pour detecter 8 fois la meme note...
            // on saute donc au conclusions imm�diatement
            else if(compteurNoteAlike == 4 && bufferintensite[ii] < 0.5*amptemp)
            {
                buffertiming[indtemp] = 4; // la note est une ronde

                buffertiming[ii - 1] = -1;
                buffertiming[ii] = -1;
                buffertiming[ii+1] = -1;
                buffertiming[ii+2] = -1;

                ii+=2;
                compteurNoteAlike = 0;
            }

            // detecte la meme note mais ne baisse pas assez damplitude (on detecte encore une attaque) -> cest une croche
            else if(bufferintensite[ii] >  0.5*amptemp)
            {
                notetemp = buffernote[ii];
                indtemp = ii;
                amptemp = bufferintensite[ii];
                compteurNoteAlike = 0;

                buffertiming[indtemp] = 1; // la note est une croche
            }
        }

        // ON VOIT UN SILENCE EN PARCOURANT LE VECTEUR
        // NOTE: il faudrait mettre un seuillage pour le synchrone car des fausses detections occurent
        else if(buffernote[ii] == -1)
        {
            notetemp = -1;

            //buffertiming[ii] = -1;
            buffertiming[ii] = 5;

            //indtemp = ii;
        }

        // ON VOIT UNE NOUVELLE NOTE -> on dit que cesxt une croche des le depart, on update par la suite
        else if(buffernote[ii] != -1)
        {
            if(bufferintensite[ii] > 70) // trouver un seuil interessant
            {
                notetemp = buffernote[ii];
                indtemp = ii;
                amptemp = bufferintensite[ii];
                compteurNoteAlike = 0;

                buffertiming[indtemp] = 1; // la note est une croche
            }
            else
            {
                notetemp = -1;
                //buffertiming[ii] = -1;
                buffertiming[ii] = 5;
            }
        }
    }
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
Description : genere les parametre de la fonction de metronome selon les parametre selectionn�s
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





