/********************************************************************************************
Version 1.0
Auteur : Bruno Gagnon
Date de cr�ation : 22 juillet 2009
Date de la derni�re modification : 25 octobre 2009
Modif� par : Paul Charette
Description : Filtre FIR de type direct 1
Propri�t� : Universit� de Sherbrooke, d�partement de g�nie �lectrique et informatique

Param�tres:
	cPtr: pointeur de l'�chantillon courant dans le tampon
	ech : �chantillon � ajouter dans le tampon
	coeffs: coeifficients du filtre FIR, de longueur N
	tampon: tampon contenant les �chantillons, de longueur L
	y: adresse du r�sultat de l'op�ration de filtrage

********************************************************************************************/

short * direct1FIR(short *cPtr, short ech, const short coeffs[], short N, short tampon[], short L, short *y)

{
    int test_sym = 1;
	int k;
	int out=0; // Doit �tre un int pour accepter le Q30
	short *p;
	short *p1;

	if(test_sym)
	{
        /******************************************************************************
        TEST ALGORITHME EN C -> CONVERTIR EN ASSEMBLEUR
        ******************************************************************************/
        /******************************************************************************
        Enregistrement du nouvel �chantillon dans le tampon
        ******************************************************************************/
        cPtr++;
        if (cPtr == tampon+L) // Si adresse de l'�ch. courant � la fin du tampon
            cPtr = tampon;
        *cPtr = ech;		  // Enregistrement de l'�ch. courant dans le tampon

        /******************************************************************************
        Filtrage du tampon par convolution
        ******************************************************************************/

        p = cPtr;
        p1= cPtr;
        p1++;

        if (p1 > tampon+L-2)   // Si l'adresse de l'�ch. courant sortie du tampon
            p1 = tampon;    // Remettre l'adresse � la fin du tampon

        for (k=0; k<(63-1)/2; k++)	{    // Pour tous les �chantillons du tampon
            // Convolution y[n] = sommation(h[k]x[n-k])
            out = out + coeffs[k] *((*p--)+(*p1++));	  // cette multiplication donne Q30

            if (p < tampon)   // Si l'adresse de l'�ch. courant sortie du tampon
                p = tampon+L-2;    // Remettre l'adresse � la fin du tampon

            if (p1 > tampon+L-2)   // Si l'adresse de l'�ch. courant sortie du tampon
                p1 = tampon;    // Remettre l'adresse � la fin du tampon
        }

        out = out + coeffs[(63-1)/2] * (*p);

        out = out >> 15; // d�calage arithm�thique � droite pour passer de Q30 � Q15
        *y = out;

        return cPtr;
	}
	else
	{
        /******************************************************************************
        Enregistrement du nouvel �chantillon dans le tampon
        ******************************************************************************/
        cPtr++;
        if (cPtr == tampon+L) // Si adresse de l'�ch. courant � la fin du tampon
            cPtr = tampon;
        *cPtr = ech;          // Enregistrement de l'�ch. courant dans le tampon

        /******************************************************************************
        Filtrage du tampon par convolution
        ******************************************************************************/
        p = cPtr;
        for (k=0; k<N; k++) {    // Pour tous les �chantillons du tampon
            // Convolution y[n] = sommation(h[k]x[n-k])
            out = out + coeffs[k] * (*p--);   // cette multiplication donne Q30

            if (p < tampon)   // Si l'adresse de l'�ch. courant sortie du tampon
                p = tampon+L-1;    // Remettre l'adresse � la fin du tampon
        }

        out = out >> 15; // d�calage arithm�thique � droite pour passer de Q30 � Q15
        *y = out;

        return cPtr;
	}
}

void init_tampon(short tampon[])
{
    int i;

    for (i=0; i<64; i++)
    {
        tampon[i] = 0;
    }
}

