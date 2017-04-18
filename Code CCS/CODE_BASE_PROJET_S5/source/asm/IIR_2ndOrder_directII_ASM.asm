	.def	_IIR_2ndOrder_directII_ASM
_IIR_2ndOrder_directII_ASM:

	; Mémoriser le contenu de AMR
	MVC	AMR, B5
	STW	B5,*--B15
	STW A10,*--B15
	STW A11,*--B15
	STW A12,*--B15
	STW B10,*--B15
	STW B11,*--B15
	STW B12,*--B15

	; enregistrement parametres de la fonction
	MV A4, B7	;x(n)
	MV B4, B9	;w(n)
	MV A6, B8	;C -> b0 b1 b2 a0 a1 a2


	; fonction de saturation 25 bits
	;MVKL 0x00FFFFFF, A8
	;MVKH 0x00FFFFFF, A8
	;CMPGT B7,A8, A1
	;[A1] MV A8, B7

	;MVKL 0xFF000000, A8
	;MVKH 0xFF000000, A8
	;CMPGT A8,B7, A1
	;[A1] MV A8, B7

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
						;; w(n) = a0*x(n)-a1*w(n-1)-a2*w(n-2) ;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;;;;;;;;;;;;;;;;;;; a0*x(n) ;;;;;;;;;;;;;;;;;;;

	MV B7,A8
	LDW *B8[3],A9
	NOP 4

	MPYSP A8,A9,A4
	NOP 3

	;SHRU A4, 13, A4
	;SHL A5, 19, A5
	;ADD A4, A5, A4

	MV A4, A10	; enregistre la reponse a0*x(n)

	;;;;;;;;;;;;;;;;;;; a1*w(n-1) ;;;;;;;;;;;;;;;;;;;
	LDW *B9[1],A8
	NOP 4
	LDW *B8[4],A9
	NOP 4

	MPYSP A8,A9,A4
	NOP 9

	;SHRU A4, 13, A4
	;SHL A5, 19, A5
	;ADD A4, A5, A4

	MV A4, A11	; enregistre la reponse a1*w(n-1)

	;;;;;;;;;;;;;;;;;;; a2*w(n-2) ;;;;;;;;;;;;;;;;;;;
	LDW *B9[2],A8
	NOP 4
	LDW *B8[5],A9
	NOP 4

	MPYSP A8,A9,A4
	NOP 9

	;SHRU A4, 13, A4
	;SHL A5, 19, A5
	;ADD A4, A5, A4

	MV A4, A12	; enregistre la reponse a2*w(n-2)

	;;;;;;;;;;;;;;;;;;; w(n) = a0*x(n)-a1*w(n-1)-a2*w(n-2) ;;;;;;;;;;;;;;;;;;;
	SUBSP A10,A11,A10
	NOP 3
	SUBSP A10,A12,A10
	NOP 3

	; MANQUE SAT 25 BITS !!!!!!!! (A VOIR AVEC ALEX)
	; fonction de saturation 25 bits
	;MVKL 0x00FFFFFF, A8
	;MVKH 0x00FFFFFF, A8
	;CMPGT A10,A8, A1
	;[A1] MV A8, A10

	;MVKL 0xFF000000, A8
	;MVKH 0xFF000000, A8
	;CMPGT A8,A10, A1
	;[A1] MV A8, A10

	STW A10, *B9 ; enregistre la valeur de w(n) dans le vecteur associé
	NOP 4

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
						;; y(n) = b0*w(n)+b1*w(n-1)+b2*w(n-2) ;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;;;;;;;;;;;;;;;;;;; b0*w(n) ;;;;;;;;;;;;;;;;;;;
	LDW *B8,A8
	NOP 4
	MV A10, A9

	MPYSP A8,A9,A4
	NOP 3

	;SHRU A4, 13, A4
	;SHL A5, 19, A5
	;ADD A4, A5, A4

	MV A4, A10	; enregistre la reponse b0*w(n)

	;;;;;;;;;;;;;;;;;;; b1*w(n-1) ;;;;;;;;;;;;;;;;;;;
	LDW *B8[1],A8
	NOP 4
	LDW *B9[1],A9
	NOP 4

	MPYSP A8,A9,A4
	NOP 9

	;SHRU A4, 13, A4
	;SHL A5, 19, A5
	;ADD A4, A5, A4

	MV A4, A11	; enregistre la reponse b1*w(n-1)

	;;;;;;;;;;;;;;;;;;; b2*w(n-2) ;;;;;;;;;;;;;;;;;;;
	LDW *B8[2],A8
	NOP 4
	LDW *B9[2],A9
	NOP 4

	MPYSP A8,A9,A4
	NOP 9

	;SHRU A4, 13, A4
	;SHL A5, 19, A5
	;ADD A4, A5, A4

	MV A4, A12	; enregistre la reponse b2*w(n-1)

	;;;;;;;;;;;;;;;;;;; y(n)= b0*w(n)+b1*w(n-1)+b2*w(n-2) ;;;;;;;;;;;;;;;;;;;
	ADDSP A10,A11,A10
	NOP 5
	ADDSP A10,A12,A10
	NOP 5

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
								;; finalisation ;;
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	LDW *B9,A8
	NOP 4
	LDW *B9[1],A9
	NOP 4

	STW A9, *B9[2]
	NOP 4
	STW A8, *B9[1]
	NOP 4

	; return de la reponse
	MV A10,A4

	; Rétablir le contenu de AMR
	LDW *B15++,B12
	LDW *B15++,B11
	LDW *B15++,B10
	LDW *B15++,A12
	LDW *B15++,A11
	LDW *B15++,A10
	LDW	*B15++,B5
	NOP	5
	MVC	B5,AMR

	; Retourner l'adresse à la fonction qui a fait l'appel (return)
	B B3
	NOP 5

	.end



