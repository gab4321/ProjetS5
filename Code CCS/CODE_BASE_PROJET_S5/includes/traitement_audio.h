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
void TestPeriodicite(int *VectRep, int *TabPeaks, int nrep);

// fonction qui effectue la FFT de la trame
void calculFFT(float *TableInputPadder, float *Sortie_FFT,float *w, short *index);

// fonction qui fait un vecteur padder de 0 au Imaginaire pour la FFT
void padderFFT(float *TableFFT, float *TableInputPadder);

// fonction qui genre une fenetre de hamming
void GenHamming(float* FenHamming);

// fonction qui applique un fenetre
void ApplicationFentre(float *Fenetre, float *TableFFT);

// fonction qui analyse la FFT
void AnalyseFFT(float *Sortie_FFT);

// fonction qui genere le son de metronome a une frequence désirée (BPM)
short GenererMetronome(short *table, int nbexecution, int nbcompteur, int nbValSinus, int *compteurTemps, int *compteurNbFois, int *compteurSinus);

// genere le sinus pour le metronome
void InitialiseMetronome(short *table, int *nbexecution, int *nbcompteur, int nbValSinus,int volumeMet, int freqMet,int tempsMet, int BPM);


#endif
