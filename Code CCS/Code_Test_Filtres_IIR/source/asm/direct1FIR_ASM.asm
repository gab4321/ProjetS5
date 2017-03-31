;****************************************************************************************
; APPEL :
; short * direct1FIR_ASM(short *cPtr, short ech, const short h[], short *y);
;
; A4 : Adresse du pointeur sur l'échantillon précédent
; B4 : Valeur de l'échantillon courant
; A6 : Adresse du début du tableau de coefficients du filtre, h[]
; B6 : Adresse de l'échantillon de sortie
;
; Return : la nouvelle adresse de cPtr (A4)
;
; NB:
; 1) La longueur du filtre passe-haut, N = 63 (ordre = 62), est fixe (hardcoded)
;    et n'est donc pas passée comme paramètre à la fonction
; 2) Comme le mode d'adressage circulaire est employé pour le tampon, il n'est pas nécessaire
;    de spécifier l'adresse de début du tampon mais simplement l'adresse de l'échantillon
;    courant. En effet, quand le pointeur du tampon est incrémenté ou décrémenté, le
;    mode d'adressage circulaire force le pointeur à rester dans le tampon ("wrapping"). Il suffit simplement
;    de spécifier la bonne longueur avec l'AMR et de bien aligner le tampon lors de sa
;    déclaration dans le programme C avec "#pragma DATA_ALIGN"
;
;****************************************************************************************

		.def	_direct1FIR_ASM
_direct1FIR_ASM:

; Mémoriser le contenu de AMR 
			MVC	AMR, B5
			STW	B5,*--B15 

; Ajuster le registre AMR
			MVKL 0x00C0A000, B9	; Utiliser B7 comme pointeur avec BK1 comme compteur vers
			MVKH 0x00C0A000, B9	; Le tampon contient 64 échantillons de 16 bits chacuns, donc 128 octets (2 X 64)
			MVC B9, AMR

; Écrire l'échantillon courant dans le tampon
			MV A4, B7	;p
			STH B4, *++B7
			MV B7, A12
			MV B6, A13

; Charger l'adresse de l'échantillon précédent du tampon dans B7
			MV B7, B6	;p1
			LDH *B6++[2], A4	; offset pour eliminer derniere valeur (a droite de x(n))

; Retourner l'adresse de l'échantillon courant

; Initialisation y[n]=0
			ZERO A11;

;**********************************************************************
; Filtrage : boucle y[n] = y[n] + x[n-k]*h[k], pour k = 0..N, où N = 63
;            et où A11 est un accumulateur auquel on sont ajoutés les
;            produits x[n-k]*h[k] à chaque itération, k, de la boucle.
;            Le filtre n'est pas récursif...
;
;            ATTENTION: les x[n] sont contenus dans le tampon circulaire
;                       et sont donc loadés avec B7 et non A4.
;**********************************************************************

			MVK 31, A1
			ZERO A11; initialisation y[n]=0

boucle:	; début de la boucle
			SUB A1,1,A1		; Diminution du compteur de tour 		(delay slot 0)
		||	LDH *A6++, A5	; Chargement du coefficient dans A5  	(delay slot 4)
		||	LDH *B7--, B5   ; Chargement de l'échantillon dans B5 	(delay slot 4)
			NOP 4
			LDH *B6++, B4	;chargement echantillon du boutee
			NOP 4
			ADD B4, B5, B4
			MPY A5, B4 ,A9	; A9 = x[n-i]*h[n+j] 					(delay slot 1)
			NOP 1
	 		ADD A9, A11, A11; y[n] = y[n] + x[n-i]*h[n+j] 			(delay slot 0)

	[A1] 	B 	boucle		; boucle si A1 n'est pas 0				(delay slot 5)
			NOP 5

		||	LDH *B6, B4
		||	LDH *A6, A5
			NOP 4
			MPY B4, A5, B4
			NOP 1

			ADD B4, A11, A11; y[n] = y[n] + x[n-i]*h[n+j]

; Enregistrer la sortie y[n]
			SHR A11, 15, A11;
			;STW A11, *A8	; Partie haute pour Q30 à Q15			(delay slot 0)
			STH A11, *A13

; DEBUGGAGE: balancer l'échantillion d'entrée directement dans la sortie
			;STH B4, *B6

; Rétablir le contenu de AMR
			LDW	*B15++,B5
			NOP	5
			MVC	B5,AMR

			MV A12, A4

; Retourner l'adresse à la fonction qui a fait l'appel (return)
			B B3
			NOP 5

		.end
