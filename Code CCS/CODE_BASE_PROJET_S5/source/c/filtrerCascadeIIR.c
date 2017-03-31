/* main.c */
/*
  Créateur:    Bruno Gagnon, M. Sc.A
  Date:        16 juillet 2009
  Revisions:   

  DESCRIPTION : 
     Appeler des filtres IIR d'ordre 2 pour faire une cascade de filtres
     et appliquer le gain global.

  ENTRÉE : 
     noCorde : Numéro de la corde de guitare sélectionnée
	 x : Amplitude de l'échantillon audio à filtrer
	 coeffsIIR.dat : fichier contenant l'information des filtres

  RETOUR :
	 Amplitude de l'échantillon audio filtré
*/

#include <stdio.h>
#include <math.h>
#include "IIR_2ndOrder_directII.h"
#include "filtrerCascadeIIR.h"
#include "coeffsIIR.dat"

/***********************************************************************************
DESCRIPTION : Appeler des filtres IIR d'ordre 2 pour faire une cascade de filtres
              et appliquer le gain global
***********************************************************************************/
short filtrerCascadeIIR(int noCorde, short x)
{
	int n, ligne;
	float y;

	y = (float)x;

	if(noCorde == 1)
	{
	    // Pour chacune des sections d'ordre 2 du filtre
	    for (n=0; n<IIR_NB_ORDRE2[noCorde-1]; n++) {

	        // filtrage par le IIR biquad de structure direct II
	        ligne = IIR_NO_LIGNE[noCorde-1]+n;
	        y = IIR_2ndOrder_directII_ASM(y, &IIR_W1[n][0], &IIR_COEFFS[ligne][0]);
	    }
	}
	else if(noCorde == 2)
	{
        // Pour chacune des sections d'ordre 2 du filtre
        for (n=0; n<IIR_NB_ORDRE2[noCorde-1]; n++) {

            // filtrage par le IIR biquad de structure direct II
            ligne = IIR_NO_LIGNE[noCorde-1]+n;
            y = IIR_2ndOrder_directII_ASM(y, &IIR_W2[n][0], &IIR_COEFFS[ligne][0]);
        }
	}

	// Appliquer le gain global
	y = IIR_GAINS[noCorde-1]*y;

	return (int)y;
}

/***********************************************************************************
DESCRIPTION : Initialisation des variables intermédiaires w(n-i)
***********************************************************************************/
void init_w()
{
	int i, j;

	for (i=0; i<IIR_NB_SECTIONS_MAX; i++)	{
		for (j=0; j<3; j++) 	IIR_W1[i][j] = 0;
	}

    for (i=0; i<IIR_NB_SECTIONS_MAX; i++)   {
        for (j=0; j<3; j++)     IIR_W2[i][j] = 0;
    }
}
