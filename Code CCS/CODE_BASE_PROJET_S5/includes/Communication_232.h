/*
 * communication_232.h
 */

#ifndef COMMUNICATION_232_H_
#define COMMUNICATION_232_H_

/*************************************************************
Liste des prototypes de fonctions
*************************************************************/

// fonction envoyant les notes et accords au LCD en mode ASYNCHRONE
void NotesToPIC(int mode, int note);

// fonction envoyant les partition a la console en mode SYNCHRONE
void printpartitionConsole(int *buffernote, int *buffertiming);

#endif
