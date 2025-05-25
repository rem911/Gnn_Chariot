//-----------------------------------------------------------------------------
//  OLedAffi_Fonc.c
//
//-----------------------------------------------------------------------------

#include "inc/hw_types.h"
#include "OLedAffi_Vars.h"


// L afficheur est connecte au port I2C-2
#define     I2C_NUM     2

// L'adresse semble etre 0x3C sur 7 bits, soit 0x78 sur 8 bits 
#define LCD_SLAVE_ADDR      0x78


void    TIVA_I2C2_InitModule(short i2cmun);
short   TIVA_I2C2_SendData(char slaveAdr, short nbdata, char* pBuff);


static void    AOLED_WriteCmde(char cmde)
//-------------------------------------------------------------
//-------------------------------------------------------------
{   char Buff[4];

    Buff[0] = 0x00;     Buff[1] = cmde;   // 00 indique le mode commande
    TIVA_I2C2_SendData(LCD_SLAVE_ADDR, 2, Buff);
}

static void    AOLED_WriteData(char data)
//-------------------------------------------------------------
//-------------------------------------------------------------
{    char Buff[4];

    Buff[0] = 0x40;     Buff[1] = data;   // 40 indique le mode commande
    TIVA_I2C2_SendData(LCD_SLAVE_ADDR, 2, Buff);
}

static void     AOLED_SetAdrsLin(char addrs)
//-------------------------------------------------------------
//-------------------------------------------------------------
{
    addrs = 0xB0 | (addrs & 0x07);
    AOLED_WriteCmde(addrs);
}

static void     AOLED_SetAdrsCol(char addrs)
//-------------------------------------------------------------
//-------------------------------------------------------------
{   char msb, lsb;

    addrs += 2;
    msb = 0x10 | (addrs >> 4);     lsb = addrs & 0x0F;
    AOLED_WriteCmde(msb);
    AOLED_WriteCmde(lsb);
}


void    AOLED_InvertDisplay(int invert)
//-------------------------------------------------------------
//-------------------------------------------------------------
{
    if (invert)
        AOLED_WriteCmde(0xA7);      // INVERT DISPLAY
    else
        AOLED_WriteCmde(0xA6);      // NORMAL DISPLAY
}

void    AOLED_FillScreen(char value)
//-------------------------------------------------------------
//-------------------------------------------------------------
{    short numcol, numlin, idln;

    for (numlin = 0; numlin < 8; numlin++) {
        AOLED_SetAdrsLin(numlin);
        AOLED_SetAdrsCol(0);
        for (numcol = 0; numcol < 128; numcol++) {
            AOLED_WriteData(value);
        }
    }
}

void    AOLED_DisplayImage(char *pBuff)
//-------------------------------------------------------------
//-------------------------------------------------------------
{    short numcol, numlin, idln;

    for (numlin = 0; numlin < 8; numlin++) {
        AOLED_SetAdrsLin(numlin);
        AOLED_SetAdrsCol(0);
        idln = numlin * 128;
        for (numcol = 127; numcol >= 0; numcol--) {
            AOLED_WriteData(pBuff[idln + numcol]);
        }
    }
}

void    AOLED_DisplayCarac(int x, int y, char car)
//-------------------------------------------------------------
//-------------------------------------------------------------
{   short numcol, debcol;      char valcol;

    x = 128 - x;
    AOLED_SetAdrsLin(y);
    AOLED_SetAdrsCol(x - 5);
    debcol = (car - 32) * 5;
    for(numcol = 4; numcol >= 0; numcol--) {
        valcol = Isep_Font[debcol + numcol];
        AOLED_WriteData(valcol);
    }
}

void    AOLED_DisplayTexte(int x, int y, char *texte)
//-------------------------------------------------------------
//-------------------------------------------------------------
{
    while (*texte) {
        AOLED_DisplayCarac(x, y, *texte);
        x += 6;     texte++;
    }
}

const char Ecole[] = "  Ecole d'Ingenieurs";
const char Numrq[] = "     du Numerique";

void    AOLED_AffiLogoIsep(void)
//-------------------------------------------------------------
//-------------------------------------------------------------
{
    AOLED_DisplayImage((char*)Isep_Logo);
    AOLED_DisplayTexte(1, 6, (char*)Ecole);
    AOLED_DisplayTexte(1, 7, (char*)Numrq);
}

void    AOLED_InitScreen(void)
//-------------------------------------------------------------
//-------------------------------------------------------------
{
    TIVA_I2C2_InitModule(I2C_NUM);
    delay(1000);
    AOLED_WriteCmde(0xAE);     // Set display OFF
    delay(1000);

    AOLED_WriteCmde(0xD4);     // Set Display Clock Divide Ratio / OSC Frequency
    AOLED_WriteCmde(0x80);     // Display Clock Divide Ratio / OSC Frequency

    AOLED_WriteCmde(0xA8);     // Set Multiplex Ratio
    AOLED_WriteCmde(0x3F);     // Multiplex Ratio for 128x64 (64-1)

    AOLED_WriteCmde(0xD3);     // Set Display Offset
    AOLED_WriteCmde(0x00);     // Display Offset

    AOLED_WriteCmde(0x40);     // Set Display Start Line 0

    AOLED_WriteCmde(0x8D);     // Set Charge Pump
    AOLED_WriteCmde(0x14);     // Charge Pump (0x10 External, 0x14 Internal DC/DC)

    AOLED_WriteCmde(0xA0);     // Set Segment Re-Map
    AOLED_WriteCmde(0xC8);     // Set Com Output Scan Direction

    AOLED_WriteCmde(0xDA);     // Set COM Hardware Configuration
    AOLED_WriteCmde(0x12);     // COM Hardware Configuration

    AOLED_WriteCmde(0x81);     // Set Contrast
    AOLED_WriteCmde(0xCF);     // Contrast

    AOLED_WriteCmde(0xD9);     // Set Pre-Charge Period
    AOLED_WriteCmde(0xF1);     // Set Pre-Charge Period (0x22 External, 0xF1 Internal)

    AOLED_WriteCmde(0xDB);     // Set VCOMH Deselect Level
    AOLED_WriteCmde(0x40);     // VCOMH Deselect Level

    AOLED_WriteCmde(0xB0);

    AOLED_WriteCmde(0xA4);     // Set all pixels OFF
    AOLED_WriteCmde(0xA6);     // Set display not inverted
    AOLED_WriteCmde(0xAF);     // Set display On
    delay(2000);
    AOLED_AffiLogoIsep();
}


#define I2C1_BASE       0x40021000
#define I2C2_BASE       0x40022000

#define I2Cx_MSA        0x0000
#define I2Cx_MCS        0x0004
#define I2Cx_MDR        0x0008
#define I2Cx_FIFOCTL    0x0F04

#define MCS_RUN         0x01
#define MCS_START       0x02
#define MCS_STOP        0x04
#define MCS_ACK         0x08
#define MCS_BUSY        0x01
#define MCS_ERROR       0x02


short   TIVA_I2C2_SendData(char slaveAdr, short nbdata, char* pBuff)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{    unsigned int value, cmde, PortI2c;    short nerr, nbtst;

    PortI2c = I2C2_BASE;
    //Serial.println("    ---    1 I2Cx_SendData");
    nerr = -2;
    if (nbdata < 0 || nbdata > 127)
        goto sortie;
    HWREG(PortI2c + I2Cx_MSA) = slaveAdr & 0x00FE;
    nerr = -3;
    value = *pBuff++;   nbdata--;
    HWREG(PortI2c + I2Cx_MDR) = value;
    cmde = MCS_RUN | MCS_START;
    while (1) {
        //Serial.println("    ---    3 I2Cx_SendData");
        if (nbdata == 0)
            cmde |= MCS_STOP;
        //Serial.print("        --- cmde = 0x");  Serial.println(cmde, HEX);
        HWREG(PortI2c + I2Cx_MCS) = cmde;   nbtst = 10;
        while (nbtst > 0) {
            value = HWREG(PortI2c + I2Cx_MCS);
            if ((value & MCS_BUSY) == 0)
                break;
            delayMicroseconds(20);
            nbtst--;
        }
        delayMicroseconds(100);
        //if ((value & MCS_BUSY) != 0)
        //    Serial.print("    --- val busy = 0x");  Serial.println(value, HEX);
        //Serial.println("    ---    4 I2Cx_SendData ok");
        if (nbdata == 0)
            break;
        value = *pBuff++;   nbdata--;
        HWREG(PortI2c + I2Cx_MDR) = value;
        cmde = MCS_RUN;
        //Serial.println("    ---    5 I2Cx_SendData");
    }
    nerr = 0;
sortie:
    //Serial.println("    ---    6 I2Cx_SendData");
    delayMicroseconds(100);
    return nerr;
}

#include "driverlib/i2c.h"
#include "driverlib/sysctl.h"


void    TIVA_I2C2_InitModule(short i2cmun)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{   unsigned int PortI2c;

    Serial.println("       +++ Debut I2Cx_InitModule");
    if (i2cmun != 2)
        goto sortie;
    PortI2c = I2C2_BASE;
    
    SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C2);
    //reset module
    SysCtlPeripheralReset(SYSCTL_PERIPH_I2C2);
    //enable GPIO peripheral that contains I2C 2
    SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);
    // Configure the pin muxing for I2C0 functions on port B2 and B3.
    GPIOPinConfigure(GPIO_PE4_I2C2SCL);
    GPIOPinConfigure(GPIO_PE5_I2C2SDA);
    // Select the I2C function for these pins.
    GPIOPinTypeI2CSCL(GPIO_PORTE_BASE, GPIO_PIN_4);
    GPIOPinTypeI2C(GPIO_PORTE_BASE, GPIO_PIN_5);

    // Enable and initialize the I2C0 master module. 
    // The last parameter sets the I2C data transfer rate.
    // false = 100kbps and true = 400kbps.
     I2CMasterInitExpClk(PortI2c, SysCtlClockGet(), true);

    //clear I2C FIFOs
    HWREG(PortI2c + I2Cx_FIFOCTL) = 80008000;

sortie:
    Serial.println("       +++ Fin I2Cx_InitModule");
    return;
}
