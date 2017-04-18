/*
 * traitement_audio.h
 */

#ifndef TRAITEMENT_AUDIO_H_
#define TRAITEMENT_AUDIO_H_

/*************************************************************
Liste des prototypes de fonctions
*************************************************************/

// fonction assembleur qui effectue lautocorrelation
void CorrASM(int *VectPadder, int *VectRep, int ncorr, int ndecalage, int nrep, int npadd);

// fonction de preparation du vecteur de correlation
void CorrManip(int *VectCorr, int *VectPadder);

// fonction qui test lintensite du son avant de lancer lautocorrelation
void TestIntensite(int *TestAmp, int ntestson);

// fonction qui test la periodicite dun vecteur de sortie de lautocorrelation
int TestPeriodicite(int *VectRep, int *VectRep2, int *TabPeaks, int *TabPeaks2, int nrep);

// fonction qui effectue la FFT de la trame
void calculFFT(float *TableInputPadder, float *Sortie_FFT,float *w, short *index);

// fonction qui fait un vecteur padder de 0 au Imaginaire pour la FFT
void padderFFT(int *TableFFT, float *TableInputPadder);

// fonction qui genre une fenetre de hamming
void GenHamming(float* FenHamming);

// fonction qui applique un fenetre
void ApplicationFentre(float *Fenetre, float *TableFFT);

// fonction qui analyse la FFT
int AnalyseFFT_Singuliere(float *Sortie_FFT);

// fonction qui analyse la FFT
int AnalyseFFT_Accord(float *Sortie_FFT);

// fonction qui genere le son de metronome a une frequence désirée (BPM)
short GenererMetronome(short table[], int nbexecution, int nbcompteur, int nbValSinus, int *compteurTemps, int *compteurNbFois, int *compteurSinus, int *compteurmesure);

// genere le sinus pour le metronome
void InitialiseMetronome(short *table, int *nbexecution, int *nbcompteur, int nbValSinus,int volumeMet, int freqMet,int tempsMet, int BPM);

// donne la note associée a la frequence (pas dharmonique)
int TrouveNote_WO_Harm(float freq, float erreur);

// donne la note associée a la frequence (avec harmonique)
int TrouveNote_W_Harm(float freq1, float freq2, float freq3, float freq4, float erreur, float erreur2, float erreur3);

// donne la moyenne damplitude de la trame mode synchrone
int intensitesynchrone(int *Vectacq);

// traite les info recolté et donne le timing de la note dans un vecteur
void traitementtiming(int *bufferintensite, int *buffernote, int *buffertiming);

#endif
