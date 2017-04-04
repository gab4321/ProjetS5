/*
CODE TEST PROJET S5

Modules présents:
Corrélation / FFT / Filtres IIR / algorithmes

Auteurs:
Équipe P4
*/	 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "dsk6713_aic23.h"
#include "C6713Helper_UdeS.h"
#include "filtrerCascadeIIR.h"
#include "traitement_audio.h"

/********************************************************************************************
Variables globales pour utiliser AIC du DSK
********************************************************************************************/
Uint32 fs=DSK6713_AIC23_FREQ_8KHZ; 	// Fréquence d'échantillonnage selon les définitions du DSK
#define DSK6713_AIC23_INPUT_LINE 0x0011
#define DSK6713_AIC23_INPUT_MIC 0x0015
Uint16 inputsource=DSK6713_AIC23_INPUT_MIC; // selection de l'entrée (A MODIFIER SELON LENTRÉE)

#define GAUCHE 0 // Haut-parleur gauche
#define DROIT  1 // Haut-parleur droit
union {Uint32 uint; short channel[2];} AIC23_data; // Pour contenir les deux signaux

/********************************************************************************************
// Variables globales
********************************************************************************************/
const double PI =3.141592653589793;
const int Fs = 8000;

/********************************************************************************************/
// variables si utilise le filtre FIR de LAPP 5
#define TAMPON_L  64
#pragma DATA_ALIGN(tampon, TAMPON_L*2); // Requis pour l'adressage circulaire en assembleur
short tampon[TAMPON_L]={0}; 		// Tampon d'échantillons
short *pTampon=&tampon[TAMPON_L-1];	// Pointeur sur l'échantillon courant
/********************************************************************************************/
// Fréquence générée avec le GEL slider (POUR DEBUG)
float frq=0;
/********************************************************************************************/
// pour lenregistrement des trames
int nbEchADC = 0;
int nbEchDebugMic = 0;
int IsSound = 0;
int TrameEnr = 0;
int GainNum = 1;
int SeuilSon = 200;
/********************************************************************************************/
// variables pour la correlation
#define Ncorr 256         //N
#define Ndecalage 100     //au choix / maximum N-1
#define Nrep 201         //2*decalage+1
#define Npadd 456        //N+2*decalage
#define Nverif 10

int ncorr = (int)Ncorr;
int ndecalage = (int)Ndecalage;
int nrep = (int)Nrep;
int npadd = (int)Npadd;
int VectCorr[Ncorr]; // vecteur des donnees de lautocorrelation
int VectRep[Nrep]; // vecteur de reponse
int VectPadder[Npadd];
/********************************************************************************************/
// variables pour la FFT
#pragma DATA_ALIGN(w, 8);
#pragma DATA_ALIGN(TableInputPadder, 8);

short index[32] = {0}; //sqrt(2*N) -> A CHANGER
float w[Ncorr] = {0};
float Sortie_FFT[Ncorr] = {0};
float TableInputPadder[2*Ncorr] = {0};
float FenHamming[Ncorr] = {0};
/********************************************************************************************/
// pour la detection de périodicité
#define NTestSon 10 //nb de lectures sur lesquelles faire une moyenne pour activé la detection de son

int ntestson = (int)NTestSon;
int TestAmp[NTestSon];
double scale = 0.60;
int nVerif = (int)Nverif; //doit detecter n intervalles egales pour conclure que cest periodique
int TabPeak[Nverif];
int FlagLed0 = 0;
/********************************************************************************************/
// variables pour faire des tests
float TableCos3[Ncorr] = {0};
int debug = 1;
int F2 = 1000;
/********************************************************************************************/
// variables pour le metronome
int BPM = 120;      //frequence du metronome
int freqMet = 600; // frequence du son du metronome
#define Lsinus 40     // longueur du vecteur pour contenir la frequence
int TempsMet = 30;   // en millisecondes

short TabFreqMet[Lsinus]; // table contenant un sinus a la freq du metronome

short sonMetronome = 0;

int compteurNBsinus = 0;    // compteur

int compteurNBfois = 0; // compteur
int Nbexecution = 0;     //nb de fois a jouer la sequence de sinus par coup de metronome

int volumeMet = 20000;

int compteurTemps = 0;      // compteur
int NbTemps = 0;        //temps en nombre de periode (1/Fs) avant le prochain coup de metronome

/********************************************************************************************
Description : Fonction principale
********************************************************************************************/
int main()
{
    /********************************************************************************************/
    // INITIALISATION DE TOUT (DÉCOMMENTER SUR LE DSK)

    // Initialisation des variables intermédiaires w(n-i) pour les IIR
    init_w();

    // Démarrage des interruptions (CODEC)
    comm_intr();

    // fenetre de hamming
    GenHamming(FenHamming);

    // initialisation des parametres du metronome
    InitialiseMetronome(TabFreqMet, &Nbexecution, &NbTemps, (int)Lsinus, volumeMet, freqMet, TempsMet, BPM);
    /********************************************************************************************/

    if(debug)
    {
        /********************************************************************************************/
        // METTRE ICI TOUT LES PETITS TESTS POUR DEBUG

        // gen fenetre hamming
        GenHamming(FenHamming);

        // vecteur dune trame de longueur avec un sinus pure que lon genere
        int n;
        for (n=0; n<ncorr; n++)
        {
                TableCos3[n] = (float)1000*cos(2*PI*F2*(n)/Fs);
        }

        // application fenetre hamming
        ApplicationFentre(FenHamming, TableCos3);

        // paddage de 0 du vecteur generé
        padderFFT(TableCos3, TableInputPadder);

        // fft du vecteur paddé
        calculFFT(TableInputPadder, Sortie_FFT, w,index);
        n = 1;

        /********************************************************************************************/
    }
    else
    {
        /********************************************************************************************/
        //BOUCLE DE TRAITEMENT SONORE
        while(1)
        {
            if(IsSound == 0 && TrameEnr == 1)
            {
                // test de lintensité du son entrant
                TestIntensite(TestAmp, ntestson);
                TrameEnr = 0;
            }

            if(IsSound == 1 && TrameEnr == 1)
            {
                // si le son est assez fort le traitement audio est activé

                /////////////////
                // CORRELATION //
                /////////////////

                // decalage + corr asm
                CorrManip(VectCorr, VectPadder);
                CorrASM(VectPadder, VectRep, ncorr, ndecalage, nrep, npadd);

                // test periodicité -> FONCTION A PERFECTIONNER
                TestPeriodicite(VectRep,TabPeak, nrep);

                /////////////////
                //     FFT     //
                /////////////////

                // application fenetre hamming
                ApplicationFentre(FenHamming, (float*)VectCorr);

                // paddage de 0 de la trame
                padderFFT((float*)VectCorr, TableInputPadder);

                // fft du vecteur paddé
                calculFFT(TableInputPadder, Sortie_FFT, w,index);

                // analyse de la FFT -> FONCTION A FAIRE
                AnalyseFFT(Sortie_FFT);

                // POUR REENREGITRER UNE TRAME
                IsSound = 0;
                TrameEnr = 0;
            }
        }
        /********************************************************************************************/
    }
}

/********************************************************************************************
Description : Code executé à toutes les interruptions du CODEC audio
********************************************************************************************/
interrupt void c_int11()
{
    /********************************************************************************************/
    // VARIABLES
	static int inputInterne = 1, FLT_IIR = 0, debugMic = 0, outputDAC = 1, Traitement_Audio = 0, IsMetronome = 0;
    static int n=1;
	short x, y, y1;
    /********************************************************************************************/

    /********************************************************************************************/
	// CRÉATION DE LÉCHANTILLON
    if(inputInterne)
        // Génération de la fréquence désirée provenant du bouton glissoir GEL (A CHANGER DANS LONGLET EXPRESSIONS)
        x = (short)(2000*cos(2*PI*frq/Fs*n++));
    else
        // Capture de l'échantillon provenant de l'entrée
        // MODIFIER INPUT_SOURCE POUR LINE_IN OU MIC_IN
        x = (short)(input_sample()*GainNum);
    /********************************************************************************************/

    /********************************************************************************************/
    // FILTRAGE IIR BIQUAD
    if(FLT_IIR)
    {
        // FILTRAGE OCTAVE 523,25 Hz a 1046,50 Hz (UTILISE LES FILTRES 1 ET 2)
        y1 = filtrerCascadeIIR(1, x); //passe haut IIR

        y = filtrerCascadeIIR(2, y1); //passe bas IIR
    }
    else
    {
        y = x;
    }
    /********************************************************************************************/

    /********************************************************************************************/
    // ENREGISTREMENT DES TRAMES DANS DES BUFFERS
    if(Traitement_Audio)
    {
        // enregistrement des trames pour vérifier lamplitude
        if(IsSound == 0 && TrameEnr == 0)
        {
            TestAmp[nbEchADC] = (int)y;
            nbEchADC++;
            if(nbEchADC >= ntestson)
            {
                nbEchADC = 0;
                TrameEnr = 1;
            }
        }
        // enregistrement des trames pour effectuer lautocorellation et la FFT
        else if(IsSound == 1 && TrameEnr == 0)
        {
            VectCorr[nbEchADC] = (int)y;
            nbEchADC++;

            if(nbEchADC >= ncorr)
            {
                nbEchADC = 0;
                TrameEnr = 1;
            }
        }
    }
    /********************************************************************************************/

    /********************************************************************************************/
    // PARTI POUR GÉNÉRER UN MÉTRONOME
    if(IsMetronome)
    {
        sonMetronome = GenererMetronome(TabFreqMet, Nbexecution, NbTemps, (int)Lsinus, &compteurTemps, &compteurNBfois, &compteurNBsinus);

        AIC23_data.channel[DROIT] =  sonMetronome;
        AIC23_data.channel[GAUCHE] = sonMetronome;
        output_sample(AIC23_data.uint);         // Sortir les deux signaux sur HEADPHONE
    }
    /********************************************************************************************/


    /********************************************************************************************/
    // PARTIES DE CODE DESTINÉES AU DEBUG
    //pour debug (verif intensite du mic)
    if(debugMic == 1)
    {
        nbEchDebugMic++;

        if(nbEchDebugMic > 4000)
        {
            printf("amplitude: %i \n",(int)y);
            nbEchDebugMic = 0;
        }
    }
    //pour debug (ecouter ou mettre a loscilloscope les signaux dentrée et de sortie)
    if(outputDAC == 1)
    {
        AIC23_data.channel[DROIT] =  x;
        AIC23_data.channel[GAUCHE] = y;
        output_sample(AIC23_data.uint);			// Sortir les deux signaux sur HEADPHONE
    }
    /********************************************************************************************/

	return;
}
