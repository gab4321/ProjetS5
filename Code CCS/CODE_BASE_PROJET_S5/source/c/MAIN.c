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
#include <C6713Helper_UdeS.h>
#include <dsk6713_aic23.h>
#include <dsk6713.h>
#include "filtrerCascadeIIR.h"
#include "traitement_audio.h"

/********************************************************************************************
Variables globales pour utiliser AIC du DSK
********************************************************************************************/
//Uint32 fs=DSK6713_AIC23_FREQ_8KHZ; 	// Fréquence d'échantillonnage selon les définitions du DSK
#define DSK6713_AIC23_INPUT_LINE 0x0011
#define DSK6713_AIC23_INPUT_MIC 0x0015
//Uint16 inputsource=DSK6713_AIC23_INPUT_MIC; // selection de l'entrée (A MODIFIER SELON LENTRÉE)

#define GAUCHE 0 // Haut-parleur gauche
#define DROIT  1 // Haut-parleur droit
union {Uint32 uint; short channel[2];} AIC23_data; // Pour contenir les deux signaux

extern void vectors();

/********************************************************************************************
// Fonctions du main
********************************************************************************************/
void SetupALL();
void FonctionTEST();

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
int SeuilSon = 2500;//2200
/********************************************************************************************/
// variables pour la correlation et lenregistrement dune trame audio
#define Ntrame 1024         //N (longueur de la trame)

#define Ndecalage 64     //au choix / maximum N-1
#define Nrep 129         //2*decalage+1
#define Npadd 1152        //N+2*decalage
#define Nverif 10

int ntrame = (int)Ntrame; // longueur de la trame


int VectTrame[Ntrame]; // vecteur des donnees pour une trame

int ndecalage = (int)Ndecalage;
int nrep = (int)Nrep;
int npadd = (int)Npadd;
int VectRep[Nrep]; // vecteur de reponse
int VectPadder[Npadd];
/********************************************************************************************/
// variables pour la FFT
#pragma DATA_ALIGN(w, 8);
#pragma DATA_ALIGN(TableInputPadder, 8);

short index[32] = {0}; //sqrt(2*N) -> A CHANGER
float w[Ntrame] = {0};
float Sortie_FFT[Ntrame] = {0};
float TableInputPadder[2*Ntrame] = {0};
float FenHamming[Ntrame] = {0};
/********************************************************************************************/
// variable pour la detection de périodicité
#define NTestSon 50 //nb de lectures sur lesquelles faire une moyenne pour activé la detection de son

int ntestson = (int)NTestSon;
int TestAmp[NTestSon];
double scale = 0.60;
int nVerif = (int)Nverif; //doit detecter n intervalles egales pour conclure que cest periodique
int TabPeak[Nverif];
int FlagLed0 = 0;
/********************************************************************************************/
// variables pour le metronome
int BPM = 80;      //frequence du metronome
int freqMet = 605; // frequence du son du metronome
#define Lsinus 40     // longueur du vecteur pour contenir la frequence
int TempsMet = 50;   // en millisecondes

short TabFreqMet[Lsinus]; // table contenant un sinus a la freq du metronome

short sonMetronome = 0;

int compteurNBsinus = 0;    // compteur pour parcourir la table de sinus

int compteurpulsation = 0; // compteur du nombre de pulsations (reinitialise a 4 en 4/4)

int compteurNBfois = 0; // compteur du nb de sinus par pulsation du metronome
int Nbexecution = 0;     //nb de fois a jouer la sequence de sinus par coup de metronome

int volumeMet = 20000;

int compteurTemps = 0;      // compteur incrementer a chaque 1/Fs seconde
int NbTemps = 0;        //temps en nombre de periode (1/Fs) avant le prochain coup de metronome

int metready = 0;
/********************************************************************************************/
// buffer nombre de detection pour test de la detection des notes singulieres

int Buff_Do = 0;
int Buff_Do_dies = 0;
int Buff_Re = 0;
int Buff_Re_dies = 0;
int Buff_Mi = 0;
int Buff_Fa = 0;
int Buff_Fa_dies = 0;
int Buff_Sol = 0;
int Buff_Sol_dies = 0;
int Buff_La = 0;
int Buff_La_dies = 0;
int Buff_Si = 0;
/********************************************************************************************/
// variables pour le test initial du mode synchrone

#define Nbnotes 4

int Buff_Notes_Mesure[Nbnotes] = {0};
int nbnotes = (int)Nbnotes;
int comp_notes;
int note_act = 0;
int amp_note_act;
int MesurePrete = 0;
int start = 0;

/********************************************************************************************/
int testsynchrone = 0;
/********************************************************************************************/

int compteurBuff = 0;
int Note_actu = -1;
int Intensite_actu = 0;

int start_acq = 0;
int clavier = 0;
int acq_fini = 0;
int trig_acq = 0;
int Buff_int[50] = {0};
int Buff_note[50] = {0};
int ii = 0;

/********************************************************************************************/
// variables pour faire des tests
float TableCos3[Ntrame] = {0};
int debug = 0;
int F2 = 1000;
short SON = 0;
int desactiveINT = 1;

/********************************************************************************************
Description : Fonction principale
********************************************************************************************/
int main()
{
    /********************************************************************************************/
    // INITIALISATION DE TOUT (DÉCOMMENTER SUR LE DSK)

    SetupALL();

    /********************************************************************************************/

    if(debug)
    {
        /********************************************************************************************/
        // METTRE ICI TOUT LES TESTS POUR DEBUG DES MODULES ET AUTRES FONCTIONS

        FonctionTEST();

        /********************************************************************************************/
    }
    else
    {
        /********************************************************************************************/
        //BOUCLE PRINCIPALE: traitement sonore + communication + affichage des partitions
        while(1)
        {
            if(testsynchrone == 1 && start_acq == 0)
            {
                printf("appuyez sur enter pour commencer\n");
                while(clavier != 1)
                {
                    scanf("%i",&clavier);
                }

                if(clavier == 1)
                {
                    start_acq = 1;
                    output_sample(0);
                    clavier = 0;
                }
            }
            else
            {
                start_acq = 1;
            }

            // test de lintensité du son entrant pour faire traitement ou non
            if(IsSound == 0 && TrameEnr == 1 && !testsynchrone)
            {
                TestIntensite(TestAmp, ntestson);
                TrameEnr = 0;
            }

            // traitement audio + detection des notes et accords
            if(IsSound == 1 && TrameEnr == 1 && !testsynchrone)
            {
                // si le son est assez fort le traitement audio est activé

                /////////////////
                // CORRELATION //
                /////////////////

                //desactiveINT = 0; //desactive tout dans linterrupt principal pendant le traitement de signaux
                // A FAIRE PLUS CLEAN!!!

                // decalage + corr asm
                //CorrManip(VectTrame, VectPadder);
                //CorrASM(VectPadder, VectRep, ntrame, ndecalage, nrep, npadd);

                // test periodicité -> FONCTION A PERFECTIONNER
                //TestPeriodicite(VectRep,TabPeak, nrep); // NE MARCHE PAS...

                /////////////////
                //     FFT     //
                /////////////////

                // application fenetre hamming
                //ApplicationFentre(FenHamming, (float*)VectTrame); // A FAIRE AVANT LES FILTRES!!!!

                // paddage de 0 de la trame
                padderFFT(VectTrame, TableInputPadder);  // BUG PAS RAPPORT

                // fft du vecteur paddé
                calculFFT(TableInputPadder, Sortie_FFT, w,index);

                // analyse de la FFT pour notes singulieres -> A PERFECTIONNER
                Note_actu = AnalyseFFT_Singuliere(Sortie_FFT);

                // analyse de la FFT pour accords -> A PERFECTIONNER
                //AnalyseFFT_Accord(Sortie_FFT);

                // POUR REENREGISTRER UNE TRAME
                IsSound = 0;
                TrameEnr = 0;
                //desactiveINT = 1;

                // Wait pour test (AL ENLEVER )
                //DSK6713_waitusec(0);

                //output_sample(0);  //pour revenir dans linterrupt

                // A FAIRE LOGIQUE INTERTRAME / MODE SYNCHRONE...
            }

            // traitement audio + detection des notes et accords
            if(start_acq && acq_fini && testsynchrone)
            {
                // desactive le flag dacquisition
                acq_fini  = 0;

                // si le son est assez fort le traitement audio est activé

                /////////////////
                // CORRELATION //
                /////////////////

                //desactiveINT = 0; //desactive tout dans linterrupt principal pendant le traitement de signaux
                // A FAIRE PLUS CLEAN!!!

                // decalage + corr asm
                //CorrManip(VectTrame, VectPadder);
                //CorrASM(VectPadder, VectRep, ntrame, ndecalage, nrep, npadd);

                // test periodicité -> FONCTION A PERFECTIONNER
                //TestPeriodicite(VectRep,TabPeak, nrep); // NE MARCHE PAS...

                /////////////////
                //     FFT     //
                /////////////////

                // application fenetre hamming
                //ApplicationFentre(FenHamming, (float*)VectTrame); // A FAIRE AVANT LES FILTRES!!!!

                // paddage de 0 de la trame
                padderFFT(VectTrame, TableInputPadder);  // BUG PAS RAPPORT

                // fft du vecteur paddé
                calculFFT(TableInputPadder, Sortie_FFT, w,index);

                // analyse de la FFT pour notes singulieres -> A PERFECTIONNER
                Note_actu = AnalyseFFT_Singuliere(Sortie_FFT);

                Intensite_actu = intensitesynchrone(VectTrame);

                Buff_int[ii] = Intensite_actu;
                Buff_note[ii] = Note_actu;

                ii++;

                if(ii >= 50)
                {
                    start_acq = 0;
                    ii = 0;
                    compteurTemps = 0;
                    compteurNBfois = 0;
                    compteurNBsinus = 0;
                    compteurpulsation = 0;
                }

                // analyse de la FFT pour accords -> A PERFECTIONNER
                //AnalyseFFT_Accord(Sortie_FFT);

                //desactiveINT = 1;

                // Wait pour test (AL ENLEVER )
                //DSK6713_waitusec(0);

                //output_sample(0);  //pour revenir dans linterrupt
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
    if(desactiveINT == 1) // A ENLEVER ET FAIRE PROTECTION DE CONTEXTE
    {
        /********************************************************************************************/
        // VARIABLES
        static int inputInterne = 0, FLT_IIR = 0, debugMic = 0, outputDAC = 0, Traitement_Audio = 1, IsMetronome = 1;
        static int fenetre_actif = 1;
        static int n=1;
        static short x, y, y1;
        static float gain_reduit = 0.1;

        /********************************************************************************************/

        /** NOTES: IL FAUT ACTIVER FAIRE UN OUTPUT SAMPLE POUR RENTRER PLUSIEUR FOIS DANS CET INTERRUPT**/

        /********************************************************************************************/
        // CRÉATION DE LÉCHANTILLON
        if(inputInterne == 1)
        {
            // Génération de la fréquence désirée provenant du bouton glissoir GEL (A CHANGER DANS LONGLET EXPRESSIONS)
            x = (short)(2000*cos(2*PI*frq/Fs*n++));
        }
        else
        {
            // Capture de l'échantillon provenant de l'entrée
            // MODIFIER INPUT_SOURCE POUR LINE_IN OU MIC_IN
            x = (short)(input_sample()*GainNum);
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // APPLICATION DUNE FENETRE (POUR LINSTANT -> HAMMING)
        if(fenetre_actif == 1 && start_acq)
        {
            if(IsSound == 1 && TrameEnr == 0)
            {
                x = x*FenHamming[nbEchADC];
            }
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // FILTRAGE IIR BIQUAD
        if(FLT_IIR && start_acq)    // LE FILTRE A DEBUGGER!!!!!!!!!!!
        {
            // FILTRAGE OCTAVE 4 et 5 (EN CE MOMENT)
            // ajouter condition pour le mode ACCORD vs mode SINGULIERE!!!
            y1 = filtrerCascadeIIR(2, x); //passe haut IIR

            y = filtrerCascadeIIR(1, y1); //passe bas IIR -> filtre 1 = 4 et 5eme et 6eme octave (POUR SINGULIERE)
                                          //              -> filtre 3 = 4 eme octave seulement (POUR ACCORD)
        }
        else
        {
            y = x;
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // ENREGISTREMENT DES TRAMES DANS DES BUFFERS ( ASYNCHRONE)

        if(Traitement_Audio && !testsynchrone)
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
            else if(IsSound == 1 && TrameEnr == 0 && !testsynchrone)
            {

                // gain reduit -> pour eviter overflow dans traitement de signal
                VectTrame[nbEchADC] = (int)y*gain_reduit;
                nbEchADC++;

                if(nbEchADC >= ntrame)
                {
                    nbEchADC = 0;
                    TrameEnr = 1;
                }
            }
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // ENREGISTREMENT DES TRAMES DANS DES BUFFERS ( SYNCHRONE)

        if(Traitement_Audio && testsynchrone)
        {
            // enregistrement des trames pour effectuer lautocorellation et la FFT
            if(trig_acq == 1)
            {
                // gain reduit -> pour eviter overflow dans traitement de signal
                VectTrame[nbEchADC] = (int)y*gain_reduit;
                nbEchADC++;

                if(nbEchADC >= ntrame)
                {
                    nbEchADC = 0;
                    trig_acq = 0;
                    acq_fini = 1;
                }
            }
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // PARTI POUR GÉNÉRER UN MÉTRONOME + COMPTEUR DU MODE SYNCHRONE
        if(IsMetronome == 1)
        {
            if(metready == 1 && start_acq == 1)
            {
                sonMetronome = GenererMetronome(TabFreqMet, Nbexecution, NbTemps, (int)Lsinus, &compteurTemps, &compteurNBfois, &compteurNBsinus, &compteurpulsation);

                AIC23_data.channel[DROIT] =  sonMetronome;
                AIC23_data.channel[GAUCHE] = sonMetronome;
                output_sample(AIC23_data.uint);         // Sortir les deux signaux sur HEADPHONE

                // pour mode synchrone
                if(compteurTemps == 15 || compteurTemps == NbTemps/2+15)
                {
                    trig_acq = 1;
                }
            }
            else
            {
                output_sample(0);
            }
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
}

/********************************************************************************************
Description : Initialise tout
********************************************************************************************/
void SetupALL()
{
    // initialisation pour comm rs232
    //SPI_init();

    // fenetre de hamming
    GenHamming(FenHamming);

    // initialisation des parametres du metronome
    InitialiseMetronome(TabFreqMet, &Nbexecution, &NbTemps, (int)Lsinus, volumeMet, freqMet, TempsMet, BPM);
    metready = 1;

    // Démarrage des interruptions (CODEC)
    comm_intr(DSK6713_AIC23_FREQ_8KHZ, DSK6713_AIC23_INPUT_MIC);

    // pour les filtres IIR
    init_w();
}

/********************************************************************************************
Description : Pour faire des tests des fonctions et modules
********************************************************************************************/
void FonctionTEST()
{
    // gen fenetre hamming
    GenHamming(FenHamming);

    // vecteur dune trame de longueur avec un sinus pure que lon genere
    int n;
    for (n=0; n<ntrame; n++)
    {
            TableCos3[n] = (float)1000*cos(2*PI*F2*(n)/Fs);
    }

    // application fenetre hamming
    ApplicationFentre(FenHamming, TableCos3);

    // paddage de 0 du vecteur generé
    //padderFFT(TableCos3, TableInputPadder);

    // fft du vecteur paddé
    calculFFT(TableInputPadder, Sortie_FFT, w,index);
    n = 1;
}
