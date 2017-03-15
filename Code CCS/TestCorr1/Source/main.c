#include <C6713Helper_UdeS.h>
#include "signals_test.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

//SETTING DU DAC
Uint32 fs=DSK6713_AIC23_FREQ_8KHZ; //set sampling rate 8KHZ
Uint16 inputsource=0x0011; // select input

//DEFINE DES ADRESSES MEMOIRE DES PERIPHERIQUES
#define ADC_ADRESS 0XB0380000 //pour lADC (pas adresse officielle)
#define CPLD_USER_REG (unsigned int *) 0x90080000   //adresse du USER_REG du CPLD pour LED et Switch
#define SDRAM_ADRESS_BASE 0x80000000 //adresse de la sdram

//DEFINE DES ADRESSES MEMOIRE DES REGISTRES
// pour le timer 1
#define CTL_REG_TIMER_1 0x01980000
#define PRD_REG_TIMER_1 0x01980004

//pour EMIF
#define GBLCTL_REG_EMIF 0x01800000 //emif global reg
#define CECTL1_REG_EMIF 0x01800004 //emif ce1 space control
#define CECTL0_REG_EMIF 0x01800008 //emif ce1 space control
#define CECTL2_REG_EMIF 0x01800010 //emif ce2 space control
#define CECTL3_REG_EMIF 0x01800014 //emif ce3 space control

//pour les GPIO
#define GPEN_REG 0x01B00000
#define GPDIR_REG 0x01B00004
#define GPPOL_REG 0x01B00024

//DEFINITION DES VARIABLES
short ValADCIn = 0;
short ValSon = 0;
int i = 0;
int ledBool = 0;
int tabTest[10] = {0,0,0,0,0,0,0,0,0,0};
int nbEchADC = 0; //pour 10 sec a une freq de 50 kHz
unsigned int* Addr_SDRAM = (unsigned int*) SDRAM_ADRESS_BASE; //variable pointeur sdram
int enablerejouer = 0;
int enableenregistrement = 0;
int IsADCconvTermine = 0;
int IsDACreadtermine = 0;
int oldDIP0 = 0;
int oldDIP1 = 0;
int oldDIP2 = 0;
int oldDIP3 = 0;
int etat1 = 0;
int etat2 = 0;
int etat3 = 0;
int etat4 = 0;
int compLED = 0;
int firstPlay = 1;
int FlagLed0 = 0;

//VARIABLES POUR lIMPLEMENTATION DUN FILTRE FIR POUR  TESTER
unsigned int* ActualPos;
int GainNum = 1; //pour reentendre le son le gain optimal est 3
int filterON = 1;
//buffer pour enregistrement des valeur avant le traitement
int bufferFiltre[20];
//coefficients filtre FIR passe bas 1 kHz ordre 20
double FIRCoe[21] = {0.000641913427233995,-0.000401088207792939,-0.00518217616287256,-0.00253967290556278,0.0166207670986993,0.0185913411250711,-0.0343479573022921,-0.0703572345137644,0.0514674311997605,0.304537742002049,0.441362218478942,0.304537742002049,0.0514674311997605,-0.0703572345137644,-0.0343479573022921,0.0185913411250711,0.0166207670986993,-0.00253967290556278,-0.00518217616287256,-0.000401088207792939,0.000641913427233995};
int BuffFiltre[21];

//DECLARATION DES FONCTIONS
void configAndStartTimer1();
void enableInterruptGPIO4(); //enable interrupt 4 pour le GPIO 4
void enableInterruptTIMER1(); //active linterrupt du timer 1
void enableInterruptDAC();
void disableInterruptDAC(); //desactive interrupt du DAC (int 11)
void disableInterruptTIMER1(); //desactive linterrupt du timer 1
interrupt void ISRConvTermine();
interrupt void ISRTimer1();
interrupt void ISROUTDAC();
void setGPIO(); //setter GPIO pour lADC (ext_int_4)
void StartADCConversion(); //ecrit a ladresse de lADC pour partir la conversion
void ReadADCConversion(); //lit la valeur dans ladresse EMIF de lADC
void SetEMIFRegister(); //set le timing de LEMIF
int etatDIP0();
int etatDIP1();
int etatDIP2();
int etatDIP3();
void resetDIP();
void initialiseLED();
void SetGainNum();
void SetupAll();

//VARIABLES POUR LE TEST DE CORRELATION
//defines
#define Ncorr 256         //N
#define Ndecalage 100     //au choix / maximum N-1
#define Nrep 201         //2*decalage+1
#define Npadd 456        //N+2*decalage
//variables
int ncorr = (int)Ncorr;
int ndecalage = (int)Ndecalage;
int nrep = (int)Nrep;
int npadd = (int)Npadd;
int VectCorr[Ncorr] = {1,5,6,8,23,45,1,0,5,4,6,2,1,4,1}; // vecteur des donnees de lautocorrelation
int VectRep[Nrep]; // vecteur de reponse
int VectPadder[Npadd];
//fonctions
void CorrASM(int *VectPadder, int *VectRep, int ncorr, int ndecalage, int nrep, int npadd);// fonction en assembleur effectuant la correlation
void CorrManip(int *VectCorr, int *VectPadder); // fonction en C effectuant le paddage de 0 et autre preparation a la correlation

//VARIABLES POUR TEST ALGORITHME DE DETECTION DE PERIODICITÉ
//defines
#define NTestSon 10 //nb de lectures sur lesquelles faire une moyenne pour activé la detection de son
//variables
int ntestson = (int)NTestSon;
int TestAmp[NTestSon];
int SeuilSon = 3000;
int TrameEnr = 0;
int IsSound = 0;
//fonctions
void TestIntensite(int *TestAmp, int ntestSon);
void TestPeriodicite(int *VectRep, int *TabPeaks, int nrep);
int debugMic = 0;
int scale = 0.60;
int TabPeak[Nrep];
int nVerif = 2; //doit detecter n intervalles egales pour conclure que cest periodique

//PROGRAMME PRINCIPAL (LOOP)
int main(){

    SetupAll();

    //DEBUG CORRELATION
    //padding du vecteur de donnees pour faire la correlation
//    CorrManip(VectCorr, VectPadder);
    //correlation du vecteur paddr en ASM vecteur de sortie est VectRep
//    CorrASM(VectPadder, VectRep, ncorr, ndecalage, nrep, npadd);

    // A
    //TEST DETECTION PERIODICITE DES SONS ENREGISTRÉS
    printf("entree le seuil de detection");
    scanf("%i",&SeuilSon);
    while(1)
    {
        // CODE A VALIDER !!!

        if(IsSound == 0 && TrameEnr == 1)
        {
            TestIntensite(TestAmp, ntestson);
            TrameEnr = 0;
        }

        if(IsSound == 1 && TrameEnr == 1)
        {
            CorrManip(VectCorr, VectPadder);
            CorrASM(VectPadder, VectRep, ncorr, ndecalage, nrep, npadd);

            TestPeriodicite(VectRep,TabPeak, nrep);
            IsSound = 0;
            TrameEnr = 0;
        }

    }
}

//DEFINITION DES FONTIONS

//fonction de preparation du vecteur de correlation
void CorrManip(int *VectCorr, int *VectPadder)
{
    // CODE A VALIDER !!!

    int i;

    for(i = 0; i<(int)Ndecalage; i++)
    {
        VectPadder[i] = 0;
    }

    for(i = 0; i<(int)Ncorr; i++)
    {
        VectPadder[i+Ndecalage] = VectCorr[i];
    }

    for(i = Npadd-1 ; i>((int)Npadd-1-(int)Ndecalage);i--)
    {
        VectPadder[i] = 0;
    }
}

//fonction interrupt timer 1: lit lADC du CODEC AUDIO
interrupt void ISRTimer1()
{
    // CODE A VALIDER !!!

    if(IsSound == 0 && TrameEnr == 0)
    {
        ValADCIn = input_left_sample();

        TestAmp[nbEchADC] = (int)(ValADCIn * GainNum);

        nbEchADC++;

        if(nbEchADC >= ntestson)
        {
            nbEchADC = 0;
            TrameEnr = 1;
        }
    }
    else if(IsSound == 1 && TrameEnr == 0)
    {
        ValADCIn = input_sample();

        VectCorr[nbEchADC] = (int)(ValADCIn * GainNum);

        nbEchADC++;

        if(nbEchADC >= ncorr)
        {
            nbEchADC = 0;
            TrameEnr = 1;
        }
    }


    if(debugMic == 1)    //pour debug (verif intensite du mic)
    {
        ValADCIn = input_left_sample();

        nbEchADC++;

        if(nbEchADC > 4000)
        {
            printf("amplitude %i \n",(int)ValADCIn);
            nbEchADC = 0;
        }
    }
}

//fonction qui test lintensite du son avant de lancer lautocorrelation
void TestIntensite(int *TestAmp, int ntestson)
{

    //CODE A VALIDER !!!

    int i;
    int intensite = 0;

    for(i = 0; i < ntestson; i++)
    {
        if(TestAmp[i] <0)
        {TestAmp[i] = -1*TestAmp[i];}
        intensite = intensite + TestAmp[i];
    }

    intensite = intensite/ntestson;


    if(intensite > SeuilSon)
    {
        IsSound = 1;
    }
    else
    {
//        printf("pas de son\n");  //a remplacer par des lumieres
        if(FlagLed0==0)
        {

            *CPLD_USER_REG &=~0x06;      //éteindre led1 & 2
            *CPLD_USER_REG |=0x01;      //allumer led0
            FlagLed0 =1;
        }


    }
}

//fonction qui test la periodicite dun vecteur de sortie de lautocorrelation
void TestPeriodicite(int *VectRep, int *TabPeaks, int nrep)
{

    //CODE A VALIDER !!!

    int i;
    int maxcorr = 0;
    int npeaks = 0;
    int interIni;
    int detect = 1;
    int interK;

    //detection de lamplitude maximale de la correlation
    for(i = 0; i < nrep; i++)
    {
        if ( abs(VectRep[i]) > maxcorr )
            maxcorr = abs(VectRep[i]);
    }

    //scaling du maximum
    maxcorr = maxcorr * scale;

    //detection des indices i des peaks de la correlation
    for(i = 2; i < nrep-2; i++)
    {
        if(VectRep[i] > VectRep[i-1] && VectRep[i] > VectRep[i-2])
        {
            if(VectRep[i] > VectRep[i+1] && VectRep[i] > VectRep[i+2])
            {
                if(VectRep[i] > maxcorr)
                {
                    TabPeaks[npeaks] = i;
                    npeaks+=1;
                }
            }
        }
    }

    //detection des intervalles K entre les peaks
    //(le nb de verifications necessaires est a valider min = 2)
    interIni = TabPeak[1] - TabPeak[0];
    for(i = 1; i < nVerif; i++ )
    {
        interK = TabPeak[i+1] - TabPeak[i];
        if((int)abs((double)(interK-interIni)/(double)interIni)*100 > 5) //si ecart relative sup a 5 %
        {
            detect = 0;
            break;
        }
    }

    if(detect == 1)
    {
//        printf("signal periodique\n "); //a remplacer par des lumieres
        FlagLed0 = 0;
        *CPLD_USER_REG &=~0x01;      //éteindre led0
        *CPLD_USER_REG |=0x02;      //allumer led1
    }
    else
    {
//        printf("signal non periodique \n"); //a remplacer par des lumieres
        FlagLed0 = 0;
        *CPLD_USER_REG &=~0x01;      //éteindre led0
        *CPLD_USER_REG |=0x03;      //allumer led2
    }
}

//fonction interrupt DAC
interrupt void ISROUTDAC()
{
    if(IsADCconvTermine == 1)
    {
        if(filterON == 1)
        {
            ActualPos = Addr_SDRAM;

            for(i = 20; i >= 0; i--)
            {
                if(Addr_SDRAM < (unsigned int*) SDRAM_ADRESS_BASE)
                    BuffFiltre[i] = 0;
                else
                    BuffFiltre[i] = *Addr_SDRAM;

                Addr_SDRAM--;
            }

            ValSon = 0;

            for(i = 0; i <= 20; i++)
            {
                ValSon = ValSon + FIRCoe[i] * BuffFiltre[20-i];
            }

            Addr_SDRAM = ActualPos;
        }
        else
        {
            ValSon = *Addr_SDRAM;
        }

            output_left_sample(ValSon * GainNum);

            Addr_SDRAM++;

            nbEchADC++;

            if(nbEchADC > 40000)
            {
                Addr_SDRAM = (unsigned int*) SDRAM_ADRESS_BASE;  //reinitialise adresse SDRAM
                nbEchADC = 0;  //reinitialise nombre echantillons
                IsADCconvTermine = 0;
            }
        }
    else
    {
        output_left_sample(0);
    }
}

//fonction interrupt GPIO 4
interrupt void ISRConvTermine()
{

}

//fonction pour regler le gain numérique
void SetGainNum()
{
    oldDIP2 = etat3;
    etat3 = etatDIP2();
    if(oldDIP2 != etat3)
    {
        GainNum = GainNum + 6;
        if(GainNum > 64)
        {
            GainNum = 64;
        }
    }

    oldDIP3 = etat4;
    etat4 = etatDIP3();
    if(oldDIP3 != etat4)
    {
        GainNum = GainNum - 6;
        if(GainNum < 0)
        {
            GainNum = 0;
        }
    }
}

//fonction qui initialise le timer 1
void configAndStartTimer1()
{

    *(unsigned int*) CTL_REG_TIMER_1 &= 0xFFFFFFF7F; //HLD a 0

    *(unsigned int*) PRD_REG_TIMER_1 = 0x00000DBC; //freq = 8 khz    (si veut 10 Hz 0x002AEA54)

    *(unsigned int*) CTL_REG_TIMER_1 &= 0xFFFFFFFFB; //DATAOUT a 0 ->driven  on TOUT

    *(unsigned int*) CTL_REG_TIMER_1 |= 0x00000301; //CLKSRC A 1 /CP a 1/FUNC a 1

    *(unsigned int*) CTL_REG_TIMER_1 |= 0x0000000C0; //HDL et GO a 1

}

//fonction qui initialise LEMIF pour lADC
void SetEMIFRegister()
{
    //*(unsigned int*) GBLCTL_REG_EMIF |= 0x00000000; //pas a changer
    *(unsigned int*) CECTL3_REG_EMIF &= 0x0140C52B; //valeur a determiner (POUR LADC)
    *(unsigned int*) CECTL3_REG_EMIF |= 0x01400523; //valeur a determiner (POUR LADC)
}

//fonction qui initialise le GPIO 4
void setGPIO()
{
    *(unsigned int*) GPEN_REG |= 0x00000010; //enable gpio 4 (1)
    *(unsigned int*) GPDIR_REG &= 0xFFFFFFEF; //set gpio 4 as input (0)
    *(unsigned int*) GPPOL_REG = 0x0000FFFF; //interrupt sur un falling edge
}

//fonction qui fait une lecture de lADC
void ReadADCConversion()
{
    ValADCIn = *(unsigned int*) ADC_ADRESS;
}

//fonction qui fait un write a lADC
void StartADCConversion()
{
    *(unsigned int*) ADC_ADRESS = 0x00000001; //ecriture bidon pour commencer la conversion de lADC
}

//fonction qui initialise tous les parametres initiaux
void SetupAll()
{
    //met les led a 0
    initialiseLED();

    //initialise le gpio 4
    setGPIO();

    //enable les interrupts
    //enableInterruptGPIO4();
    enableInterruptTIMER1();
    //enableInterruptDAC();

    comm_intr();


    //set emif pour ladc
    SetEMIFRegister();

    //read ADC au depart pour eviter probleme
    ReadADCConversion();

    //start timer 1
    configAndStartTimer1();

    //initialise les variables oldDIP1
    etat1 = etatDIP0();
    oldDIP0 = etat1;

    //initialise les variables oldDIP2
    etat2 = etatDIP1();
    oldDIP1 = etat2;

    //initialise les variables oldDIP1
    etat3 = etatDIP2();
    oldDIP2 = etat3;

    //initialise les variables oldDIP2
    etat4 = etatDIP3();
    oldDIP3 = etat4;
}

//donne etat de la DIP 0
int etatDIP0()
{
    unsigned int temp = *(unsigned int*) CPLD_USER_REG & 0x10;

    if(temp == 0x10)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//donne etat de la DIP 1
int etatDIP1()
{
    unsigned int temp = *(unsigned int*) CPLD_USER_REG & 0x20;

    if(temp == 0x20)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//donne etat de la DIP 2
int etatDIP2()
{
    unsigned int temp = *(unsigned int*) CPLD_USER_REG & 0x40;

    if(temp == 0x40)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//donne etat de la DIP 3
int etatDIP3()
{
    unsigned int temp = *(unsigned int*) CPLD_USER_REG & 0x80;

    if(temp == 0x80)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

//fonction qui initialise les LED (toutes a OFF)
void initialiseLED()
{
    *(unsigned int*) CPLD_USER_REG &= 0xF0;
}




