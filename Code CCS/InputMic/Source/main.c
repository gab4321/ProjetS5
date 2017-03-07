#include <C6713Helper_UdeS.h>

//SETTING DU DAC
Uint32 fs=DSK6713_AIC23_FREQ_8KHZ; //set sampling rate 8KHZ
Uint16 inputsource=0x0015; // select input

//DEFINE DES ADRESSES MEMOIRE DES PERIPHERIQUES
#define ADC_ADRESS 0XB0380000 //pour lADC (pas adresse officielle)
#define CPLD_USER_REG 0x90080000 //pour le CPLD
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

//VARIABLES POUR LE FILTRAGE
unsigned int* ActualPos;
int GainNum = 3;
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

//PROGRAMME PRINCIPAL (LOOP)
int main(){

    SetupAll();

    while(1)
    {


    }
}

//DEFINITION DES FONTIONS

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

//fonction interrupt timer 1
interrupt void ISRTimer1()
{
    if(IsADCconvTermine == 0)
    {
        ValADCIn = input_left_sample();

        *Addr_SDRAM = (unsigned int)ValADCIn; //enregistrement sdram

        Addr_SDRAM++;

        nbEchADC++;

        if(nbEchADC > 40000)
        {
            Addr_SDRAM = (unsigned int*) SDRAM_ADRESS_BASE;  //reinitialise adresse SDRAM
            nbEchADC = 0;  //reinitialise nombre echantillons
            IsADCconvTermine = 1;
        }
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
    enableInterruptGPIO4();
    enableInterruptTIMER1();
    enableInterruptDAC();

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




