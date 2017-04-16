/*
CODE TEST PROJET S5

Modules présents:
Corrélation / FFT / Filtres IIR / algorithmes de detection / comm / affichage / etc.

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
// VARIABLES POUR LE CODEC
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
// FONCTION DU MAIN
********************************************************************************************/
void SetupALL();
void FonctionTEST();
void printpartitionTest();

/********************************************************************************************
// VARIABLES GLOBALES
********************************************************************************************/

/********************************************************************************************/
// variables si utilise le filtre FIR de LAPP 5
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
int SeuilSon = 2000;//2200

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
int VectRep[Nrep]; // vecteur de reponse corr
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
int BPM = 120;      //frequence du metronome
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

int compteurBuff = 0;
int Note_actu = -1;
int Intensite_actu = 0;

int start_acq = 0;
int clavier = 0;
int acq_fini = 0;
int trig_acq = 0;
int Buff_int[32] = {0};
int Buff_note[32] = {0};
int ii = 0;
int traiteaffichage = 0;
int timingdetecte = 0;

int compteurNoteAlike = 0;
int notetemp = -1;
int amptemp = 0;
int indtemp = 0;
int Buff_timing[32] = {0};

/********************************************************************************************/
// variables de choix de modes
int testsynchrone = 1;
int modeaccords = 0;

/********************************************************************************************/
// variables pour faire des tests
float TableCos3[Ntrame] = {0};
int debug = 0;
int F2 = 1000;
short SON = 0;
int desactiveINT = 1;

int Buff_int_test[32] = {5,2,5,2,1,1,5,2,5,2,1,1,5,2,5,2,1,1,5,2,5,2,1,1,5,2,5,2,1,1,5,2};
int Buff_note_test[32] = {8,8,8,8,8,8,3,3,4,4,4,4,2,2,6,6,6,6,6,6,8,8,8,8,8,8,8,8,8,8,9,9};
int z;

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

        //FonctionTEST();

        // fonction de traitement du timing
        traitementtiming(Buff_int_test, Buff_note_test, Buff_timing);

        // pour affichage
        for(z = 0; z < 32; z++)
        {
            Buff_note[z] = Buff_note_test[z];
        }

        // fonction du template daffichage en prinf pour linstant
        printpartitionTest();

        // line bidon
        desactiveINT = 0;

        /********************************************************************************************/
    }
    else
    {
        /********************************************************************************************/
        //BOUCLE PRINCIPALE
        /********************************************************************************************/
        while(1)
        {
            /********************************************************************************************/
            // test pour commencer la sequence dacquisition
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
                    IsSound = 1;
                    output_sample(0);
                    clavier = 0;
                }
            }
            else
            {
                start_acq = 1;
            }

            /********************************************************************************************/
            // test de lintensité du son entrant pour faire traitement ou non (POUR ASYNCHRONE)
            if(IsSound == 0 && TrameEnr == 1 && !testsynchrone)
            {
                TestIntensite(TestAmp, ntestson);
                TrameEnr = 0;
            }

            /********************************************************************************************/
            // traitement audio + detection des notes et accords (POUR ASYNCHRONE)
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
                //Note_actu = AnalyseFFT_Singuliere(Sortie_FFT);

                // analyse de la FFT pour accords -> A PERFECTIONNER
                AnalyseFFT_Accord(Sortie_FFT);

                // POUR REENREGISTRER UNE TRAME
                IsSound = 0;
                TrameEnr = 0;
                //desactiveINT = 1;

                // Wait pour test (AL ENLEVER )
                //DSK6713_waitusec(0);

                //output_sample(0);  //pour revenir dans linterrupt

                // A FAIRE LOGIQUE INTERTRAME / MODE SYNCHRONE...
            }

            /********************************************************************************************/
            // traitement audio + detection des notes sdingulieres (POUR MODE ASYNCHRONE)
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

                // enregistrement de lintensité
                Buff_int[ii] = Intensite_actu;
                // enregistrement de la note detecté
                Buff_note[ii] = Note_actu;

                ii++;

                if(ii >= 32)
                {
                    start_acq = 0;
                    ii = 0;
                    compteurTemps = 0;
                    compteurNBfois = 0;
                    compteurNBsinus = 0;
                    compteurpulsation = 0;
                    traiteaffichage = 1;
                }

                //desactiveINT = 1;

                // Wait pour test (A ENLEVER )
                //DSK6713_waitusec(0);
            }

            /********************************************************************************************/
            // test de detection du temps des notes avec les notes / amplitudes
            if(traiteaffichage && testsynchrone)
            {

                traitementtiming(Buff_int, Buff_note, Buff_timing);
                traiteaffichage = 0;
                timingdetecte = 1;
            }

            /********************************************************************************************/
            // affichage des mesures a la console
            if(timingdetecte && testsynchrone)
            {
                // METTRE ICI FONCTION DAFFICHAGE A LA CONSOLE !!!!

                printpartitionTest(); // fonction test avec printf qui affiche 4 mesures
                timingdetecte = 0;
            }
        }
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
        static int inputInterne = 0, FLT_IIR = 1, debugMic = 0, outputDAC = 0, Traitement_Audio = 1, IsMetronome = 1;
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
            // MODIFIER INPUT_SOURCE POUR LINE_IN OU MIC_IN SELON LE CHOIX
            x = (short)(input_sample()*GainNum);
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // APPLICATION DUNE FENETRE (POUR LINSTANT -> HAMMING VA TRES BIEN)
        if(fenetre_actif && start_acq && IsSound == 1 )
        {
                x = x*FenHamming[nbEchADC];
        }
        /********************************************************************************************/

        /********************************************************************************************/
        // FILTRAGE IIR BIQUAD
        if(FLT_IIR && start_acq)
        {
            // FILTRAGE OCTAVE 4 et 5 (EN CE MOMENT)
            // ajouter condition pour le mode ACCORD vs mode SINGULIERE!!!
            y1 = filtrerCascadeIIR(2, x); //passe haut IIR

            if(!modeaccords)
                y = filtrerCascadeIIR(1, y1); //passe bas IIR -> filtre 1 = 4 et 5eme et 6eme octave (POUR SINGULIERE)
            else
                y = filtrerCascadeIIR(3, y1); //              -> filtre 3 = 4 eme octave seulement (POUR ACCORD)
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
            if(metready == 1 && start_acq == 1 && testsynchrone )
            {
                sonMetronome = GenererMetronome(TabFreqMet, Nbexecution, NbTemps, (int)Lsinus, &compteurTemps, &compteurNBfois, &compteurNBsinus, &compteurpulsation);

                AIC23_data.channel[DROIT] =  sonMetronome;
                AIC23_data.channel[GAUCHE] = sonMetronome;
                output_sample(AIC23_data.uint);         // Sortir les deux signaux sur HEADPHONE

                // pour mode synchrone : active lacquisition dune trame synchronisé sur le metronome
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
Description : fait un printf dune partition de 4 mesures (POUR FAIRE DES TESTS PAS BON POUR TEMPS REEL)
********************************************************************************************/
void printpartitionTest()
{
    int i = 0;


    printf("XX|----------------|----------------|----------------|----------------|\n");
    printf("XX|                |                |                |                |\n");

    // ecrire la ligne de la note SI
    printf("SI|");
    for(i = 0; i < 32; i++)
    {
        if(Buff_timing[i] == 1 && Buff_note[i] == 12) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 12) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 12) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 12) // ronde
            printf("r-");
        // autre
        else
            printf("--"); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");

    // ecrire la ligne de la note LA ou LA #
    printf("LA|");
    for(i = 0; i < 32; i++)
    {
        // LA #
        if(Buff_timing[i] == 1 && Buff_note[i] == 11) // croche
            printf("c#");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 11) // noire
            printf("n#");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 11) // blanche
            printf("b#");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 11) // ronde
            printf("r#");

        // LA
        else if(Buff_timing[i] == 1 && Buff_note[i] == 10) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 10) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 10) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 10) // ronde
            printf("r-");
        // autre
        else
            printf("  "); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");

    // ecrire la ligne de la note SOL ou SOL #
    printf("SO|");
    for(i = 0; i < 32; i++)
    {
        // SOL #
        if(Buff_timing[i] == 1 && Buff_note[i] == 9) // croche
            printf("c#");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 9) // noire
            printf("n#");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 9) // blanche
            printf("b#");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 9) // ronde
            printf("r#");

        // SOL
        else if(Buff_timing[i] == 1 && Buff_note[i] == 8) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 8) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 8) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 8) // ronde
            printf("r-");
        // autre
        else
            printf("--"); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");

    // ecrire la ligne de la note FA ou FA #
    printf("FA|");
    for(i = 0; i < 32; i++)
    {
        // FA #
        if(Buff_timing[i] == 1 && Buff_note[i] == 7) // croche
            printf("c#");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 7) // noire
            printf("n#");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 7) // blanche
            printf("b#");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 7) // ronde
            printf("r#");

        // FA
        else if(Buff_timing[i] == 1 && Buff_note[i] == 6) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 6) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 6) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 6) // ronde
            printf("r-");
        // autre
        else
            printf("  "); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");

    // ecrire la ligne de la note MI
    printf("MI|");
    for(i = 0; i < 32; i++)
    {
        if(Buff_timing[i] == 1 && Buff_note[i] == 5) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 5) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 5) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 5) // ronde
            printf("r-");
        // autre
        else
            printf("--"); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");

    // ecrire la ligne de la note RE ou RE #
    printf("RE|");
    for(i = 0; i < 32; i++)
    {
        // RE #
        if(Buff_timing[i] == 1 && Buff_note[i] == 4) // croche
            printf("c#");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 4) // noire
            printf("n#");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 4) // blanche
            printf("b#");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 4) // ronde
            printf("r#");

        // RE
        else if(Buff_timing[i] == 1 && Buff_note[i] == 3) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 3) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 3) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 3) // ronde
            printf("r-");
        // autre
        else
            printf("  "); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");

    // ecrire la ligne de la note DO ou DO #
    printf("DO|");
    for(i = 0; i < 32; i++)
    {
        // DO #
        if(Buff_timing[i] == 1 && Buff_note[i] == 2) // croche
            printf("c#");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 2) // noire
            printf("n#");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 2) // blanche
            printf("b#");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 2) // ronde
            printf("r#");

        // DO
        else if(Buff_timing[i] == 1 && Buff_note[i] == 1) // croche
            printf("c-");
        else if(Buff_timing[i] == 2 && Buff_note[i] == 1) // noire
            printf("n-");
        else if(Buff_timing[i] == 3 && Buff_note[i] == 1) // blanche
            printf("b-");
        else if(Buff_timing[i] == 4 && Buff_note[i] == 1) // ronde
            printf("r-");
        // autre
        else
            printf("--"); // aucune note

        if((i+1)%8 == 0 && i != 0)
            printf("|"); // fin dune mesure
    }
    printf("\n");
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
