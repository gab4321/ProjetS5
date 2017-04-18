;definitions des fonctions
	.def _CorrASM


;fonction de la correlation croisee
_CorrASM
	.asmfunc

		; Mémoriser le contenu de AMR
	MVC	AMR, B5
	STW	B5,*--B15
	STW A10,*--B15
	STW A11,*--B15
	STW A12,*--B15
	STW A13,*--B15
	STW A14,*--B15


	;adresse memoire du vecteur dentree et de sortie
	;A FAIRE: REMPLACER PAR LADRESSE DUN ESPACES DE LA RAM DEFINI ET MEME CHOSE POUR VECTEUR DE SORTIE
	MV A4, A14 			;met A4 dans A8 pour utiliser A4 dans la multiplication
	NOP 4
	MV A14, A9 			;premier pointeur du vecteur de la trame
	NOP 4
	MV A14, A10 			;deuxieme pointeur du vecteur de la trame
	NOP 4
	MV A14, A12			;pour conserver pointeur correspondant a valeur presente du decalage
	NOP 5
	MV A8, A1		;compteur longueur de la reponse
	NOP 4

	;load le decalage (nombre de 0 padder avant la trame) et multipli par 4 bytes (pour le pointeur)
	MV B6, A0
	NOP 4
	MPYID A0,4,A5:A4	;multiplication des valeur
	NOP 9
	MV A4,A0

	MV A6, B6

; ITERATIONS
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BOUCLECORR:

	MVKL 0x00000000, A13;met la somme a 0
	MVKH 0x00000000, A13

	MV A14, A9 			;retabli le pointeur A9 a valeur initiale
	NOP 4

	ADD A9, A0, A9		;decalage du pointeur du nombre de padding de 0
	NOP 4

	;load la longueur de la trame (retabli a chaque tour pour le compteur)
	MV B6, A2
	NOP 4

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
BOUCLESOMME:

	LDW  *A9++,A6		;load la premiere valeur du tableau en entree
	NOP 4
	LDW	 *A10++,A11		;load la deuxieme valeur du tableau en entree
	NOP 4

	MPYID A6,A11,A5:A4	;multiplication des valeur
	NOP 9

	ADD A13,A4,A13 		;sommation des resultats des multiplications

	SUB A2,1,A2 		;decrementation du compteur longueur trame
	[A2] B BOUCLESOMME
	NOP 5
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	STW A13, *B4++		;enregistre les sommations dans le tableau de reponse
	NOP 4

	ADD A12,4,A12		;incremente le pointeur du decalage
	NOP 4

	MV A12, A10			;met la presente valeur du decalage dans le pointeur de la somme
	NOP 4

	SUB A1,1,A1 		;decrementation du compteur decalage de la correlation
	[A1] B BOUCLECORR
	NOP 5
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	 ;Rétablir le contenu de AMR
	LDW *B15++,A14
	LDW *B15++,A13
	LDW *B15++,A12
	LDW *B15++,A11
	LDW *B15++,A10
	LDW	*B15++,B5
	NOP	5
	MVC	B5,AMR

	B B3
	NOP 5

	.endasmfunc
