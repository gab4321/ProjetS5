/********************************************************
**  Session 5 - APP6 - Téléphonie par DSP
**  Fichier SPI_driver.c
**  Auteurs : < vos noms >
**  Date : < derniere modification >
********************************************************/

/***************************************************************************
	Include headers :
***************************************************************************/

#include <csl_Mcbsp.h>
//#include <C6713Helper_UdeS.h>
//#include <dsk6713_aic23.h>
#include <csl_gpio.h>
#include <csl_irq.h>
#include <dsk6713.h>
//#include <dsk6713_led.h>

/***************************************************************************
	Include Module Header :
***************************************************************************/

#define SPI_DRIVER_MODULE_IMPORT
#include "SPI_driver.h"

/****************************************************************************
	Extern content declaration :
****************************************************************************/

extern void vectors();   // Vecteurs d'interruption  (FAR ????)
//extern MCBSP_Handle DSK6713_AIC23_CONTROLHANDLE;
extern int is_comp;

/****************************************************************************
	Private macros and constants :
****************************************************************************/

// These defines are only valid is this .c

//pour le CPLD
#define CPLD_MISC_REG 0x90080006

// commandes de base MAX3111E (VERIF POURQUOI IL FAUT MOTS DE 32 bits)
#define SPI_WRITE_CONFIG 0xE441
#define SPI_READ_CONFIG 0x4000 // disable test mode ou non?
#define SPI_WRITE_DATA 0x8000  // pas certain?? (pour ecrire -> faire ou logique avec la data)
#define SPI_READ_DATA 0x00FF   // pour lire -> faire et logique avec la data pour la conserver

/****************************************************************************
	Private Types :
****************************************************************************/

// These type declaration are only valid in this .c

/****************************************************************************
	Private global variables :
****************************************************************************/

MCBSP_Handle My_MCBSP_handle;
GPIO_Handle My_GPIO_Handle;
Uint32 DirectionSettings;
Uint32 Test_bidon;
Uint32 Verif = 1;
unsigned int Config_act = 0;
unsigned int SPI_Config_Verif = 0;
MCBSP_Handle DSK6713_AIC23_CONTROLHANDLE;

/****************************************************************************
	Private functions :
****************************************************************************/

// these function can only be called by this .c
// Use static keyword

/****************************************************************************
	Public functions :
****************************************************************************/

void SPI_init(void)
{
    //handle du MCBSP0
    DSK6713_AIC23_CONTROLHANDLE = MCBSP_open(MCBSP_DEV0, MCBSP_OPEN_RESET);

    //defini la configuration désirée (A REVOIR)
    MCBSP_Config MCBSP0_SPI_Cfg = {
            MCBSP_FMKS(SPCR, FREE, NO)              |
            MCBSP_FMKS(SPCR, SOFT, NO)              |
            MCBSP_FMKS(SPCR, FRST, YES)             |
            MCBSP_FMKS(SPCR, GRST, YES)             |
            MCBSP_FMKS(SPCR, XINTM, XRDY)           |
            MCBSP_FMKS(SPCR, XSYNCERR, NO)          |
            MCBSP_FMKS(SPCR, XRST, YES)             | // NO
            MCBSP_FMKS(SPCR, DLB, OFF)              |
            MCBSP_FMKS(SPCR, RJUST, RZF)            |
            MCBSP_FMKS(SPCR, CLKSTP, DELAY)         |// FALLING EDGE DELAY
            MCBSP_FMKS(SPCR, DXENA, OFF)            | // DX ENABLE
            MCBSP_FMKS(SPCR, RINTM, RRDY)           |
            MCBSP_FMKS(SPCR, RSYNCERR, NO)          |// ???
            MCBSP_FMKS(SPCR, RRST, YES),              // NO

            MCBSP_FMKS(RCR, RPHASE, SINGLE)         |
            MCBSP_FMKS(RCR, RFRLEN2, DEFAULT)       |
            MCBSP_FMKS(RCR, RWDLEN2, DEFAULT)       |
            MCBSP_FMKS(RCR, RCOMPAND, MSB)          |
            MCBSP_FMKS(RCR, RFIG, NO)               |
            MCBSP_FMKS(RCR, RDATDLY, 1BIT)          |
            MCBSP_FMKS(RCR, RFRLEN1, OF(0))         | // This changes to 1 FRAME
            MCBSP_FMKS(RCR, RWDLEN1, 16BIT)         | // This changes to 32 bits per frame
            MCBSP_FMKS(RCR, RWDREVRS, DISABLE),

            MCBSP_FMKS(XCR, XPHASE, SINGLE)         |
            MCBSP_FMKS(XCR, XFRLEN2, DEFAULT)       |
            MCBSP_FMKS(XCR, XWDLEN2, DEFAULT)       |
            MCBSP_FMKS(XCR, XCOMPAND, MSB)          |
            MCBSP_FMKS(XCR, XFIG, NO)               |
            MCBSP_FMKS(XCR, XDATDLY, 1BIT)          |
            MCBSP_FMKS(XCR, XFRLEN1, OF(0))         | // This changes to 1 FRAME
            MCBSP_FMKS(XCR, XWDLEN1, 16BIT)         | // This changes to 32 bits per frame
            MCBSP_FMKS(XCR, XWDREVRS, DISABLE),


            MCBSP_FMKS(SRGR, GSYNC, DEFAULT)        |
            MCBSP_FMKS(SRGR, CLKSP, DEFAULT)        |
            MCBSP_FMKS(SRGR, CLKSM, DEFAULT)        |
            MCBSP_FMKS(SRGR, FSGM, DEFAULT)         |
            MCBSP_FMKS(SRGR, FPER, DEFAULT)         |
            MCBSP_FMKS(SRGR, FWID, DEFAULT)         |
            MCBSP_FMKS(SRGR, CLKGDV, OF(78)),

            MCBSP_MCR_DEFAULT,
            MCBSP_RCER_DEFAULT,
            MCBSP_XCER_DEFAULT,

            MCBSP_FMKS(PCR, XIOEN, SP)              |
            MCBSP_FMKS(PCR, RIOEN, SP)              |
            MCBSP_FMKS(PCR, FSXM, INTERNAL)         |//etait external
            MCBSP_FMKS(PCR, FSRM, EXTERNAL)         |
            MCBSP_FMKS(PCR, CLKXM, OUTPUT)          |// OUTPUT
            MCBSP_FMKS(PCR, CLKRM, INPUT)           |// DONT CARE
            MCBSP_FMKS(PCR, CLKSSTAT, DEFAULT)      |
            MCBSP_FMKS(PCR, DXSTAT, 1)              |// HIGH
            MCBSP_FMKS(PCR, FSXP, ACTIVELOW)        |// ACTIVELOW
            MCBSP_FMKS(PCR, FSRP, ACTIVELOW)        |
            MCBSP_FMKS(PCR, CLKXP, RISING)          |// FALLING
            MCBSP_FMKS(PCR, CLKRP, RISING)           // DONt care
    };

    // configure le MCBSP avec la configuration ci-haut (HANDLE A REVERIFIER)
    MCBSP_config(DSK6713_AIC23_CONTROLHANDLE, &MCBSP0_SPI_Cfg);

    // dirige le MCBSP0 offboard a laide du CPLD (VOIR SIL Y A UNE FONCTION)
    DSK6713_rset(DSK6713_MISC, 1);

    // start le MCBSP apres avoir fait la config
    MCBSP_start(DSK6713_AIC23_CONTROLHANDLE, MCBSP_XMIT_START | MCBSP_RCV_START |
                MCBSP_SRGR_START | MCBSP_SRGR_FRAMESYNC, 220);//start data channel again

    // ECRIRE CONFIG EN BOUCLE!!!
    SPI_Config_Verif = 0x00003FFF & SPI_WRITE_CONFIG;
    while(Config_act != SPI_Config_Verif)
    {
        // envoie la trame de configuration au MAX3111E (TRAME DE CONFIG A REVERIFIER)
        while(!MCBSP_xrdy(DSK6713_AIC23_CONTROLHANDLE)); // polling pour savoir si MCBSP0 est pres
        MCBSP_write(DSK6713_AIC23_CONTROLHANDLE, SPI_WRITE_CONFIG); // envoie la config
        DSK6713_waitusec(10); // wait ->voir le guide pour savoir pourquoi ???

        // write pour clear buffer
        while(!MCBSP_rrdy(DSK6713_AIC23_CONTROLHANDLE));
        MCBSP_read(DSK6713_AIC23_CONTROLHANDLE);
        DSK6713_waitusec(10);

        // write pour lire la config de luart
        while(!MCBSP_xrdy(DSK6713_AIC23_CONTROLHANDLE));
        MCBSP_write(DSK6713_AIC23_CONTROLHANDLE,0x00004000);
        DSK6713_waitusec(10);
        while(!MCBSP_rrdy(DSK6713_AIC23_CONTROLHANDLE));
        // lecture pour verif la lecture
        Config_act = MCBSP_read(DSK6713_AIC23_CONTROLHANDLE); // read data bidon pour vider buffer
        DSK6713_waitusec(10); // wait ->voir le guide pour savoir pourquoi ???

        Config_act &= 0x00003FFF;
    }

    DSK6713_waitusec(100000);

    // setter le GPIO 4 pour linterrupt du receive
    My_GPIO_Handle = GPIO_open(GPIO_DEV0,GPIO_OPEN_RESET);
    GPIO_pinEnable(My_GPIO_Handle,GPIO_PIN4);
    DirectionSettings = GPIO_pinDirection(My_GPIO_Handle,GPIO_PIN4,GPIO_INPUT);
    GPIO_intPolarity(My_GPIO_Handle,GPIO_GPINT4,GPIO_FALLING);

    //setter linterrupt pour le GPIO 4 (A METTRE DANS LE MAIN??????)
    IRQ_setVecs(vectors);
    IRQ_map(IRQ_EVT_EXTINT4,4);
    IRQ_reset(IRQ_EVT_EXTINT4);
    IRQ_enable(IRQ_EVT_EXTINT4);

    return;
}

void SPI_Write(short SPI_data)
{
    /*
    if(!is_comp)
    {
        //16 bit en 8 bit
        SPI_data = SPI_data/256;
    }
    */
     DSK6713_waitusec(100);
    SPI_data &= 0x00FF;
    SPI_data |= SPI_WRITE_DATA;

    while(!MCBSP_xrdy(DSK6713_AIC23_CONTROLHANDLE));
    MCBSP_write(DSK6713_AIC23_CONTROLHANDLE,SPI_data);

    while(!MCBSP_rrdy(DSK6713_AIC23_CONTROLHANDLE));
    MCBSP_read(DSK6713_AIC23_CONTROLHANDLE);

    // set du flag SPI
}

short SPI_Read()
{
    short data;

    while(!MCBSP_xrdy(DSK6713_AIC23_CONTROLHANDLE));
    MCBSP_write(DSK6713_AIC23_CONTROLHANDLE,0x00000000);

    while(!MCBSP_rrdy(DSK6713_AIC23_CONTROLHANDLE));
    data = MCBSP_read(DSK6713_AIC23_CONTROLHANDLE);

    data &= SPI_READ_DATA;

    /*
    if(!is_comp)
    {
        data = data*256;
    }
    */

    return data;
}

/****************************************************************************
	ISR :
****************************************************************************/

// pour effectuer une lecture quand un data est pres dans le MAX3111E
// (survient quand IRQ' du MAX3111E tombe a low sur le GPIO 4)
interrupt void c_int04(void)
{
    // flag pour aviser le main de faire lecture si reception dune donnée
    flag_SPI = 1;
}

// end of SPI_driver.c
