//------------------------------------------------------------------------------------
// TM4C123_Pwm25.c
// Generer deux signaux PWM sur les sorties PB4 et PB5.
// Peroide = 20 ms. Duree impulsion = 1 ms (avant), 1.5 ms (arret) ou 2 ms (arriere).
//------------------------------------------------------------------------------------


#define SYSCTL_RCC				(*((volatile unsigned int *)0x400FE060))
#define SYSCTL_RCC2				(*((volatile unsigned int *)0x400FE070))
#define SYSCTL_SRPWM			(*((volatile unsigned int *)0x400FE540))
#define SYSCTL_RCGCGPIO			(*((volatile unsigned int *)0x400FE608))
#define SYSCTL_RCGCPWM			(*((volatile unsigned int *)0x400FE640))

#define GPIO_PORTB_AFSEL		(*((volatile unsigned int *)0x40005420))
#define GPIO_PORTB_DEN			(*((volatile unsigned int *)0x4000551C))
#define GPIO_PORTB_PCTL			(*((volatile unsigned int *)0x4000552C))

#define PWM0_ENABLE				(*((volatile unsigned int *)0x40028008))
#define PWM0G1_CTL				(*((volatile unsigned int *)0x40028080))
#define PWM0G1_LOAD				(*((volatile unsigned int *)0x40028090))
#define PWM0G1_CMPA				(*((volatile unsigned int *)0x40028098))
#define PWM0G1_CMPB				(*((volatile unsigned int *)0x4002809C))
#define PWM0G1_GENA				(*((volatile unsigned int *)0x400280A0))
#define PWM0G1_GENB				(*((volatile unsigned int *)0x400280A4))


//-------------------------------------------------
// Clock = 80 MHz/64 = 1.25 MHz --> T = 0.8 us
// Tempo pour avoir une pulse de 1, 1.5, et 2 ms
// Sur une periode de 25 ms
//-------------------------------------------------
#define VALLOAD_25P0MS   31250
#define VCOMPAR_1P0MS    1250
#define VCOMPAR_1P5MS    1875
#define VCOMPAR_2P0MS    2500
//   DELTA_V = ((VCOMPAR_1P5MS-VCOMPAR_1P0MS)/10)
#define DELTA_VIT         125


void    Motor_Pwm_Speed(void);
void    Motor_Rampe(short vitesse);

static  int Vit_M1, Vit_M2, Mx_Sens;



void    Motor_Stop(void)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{
    Vit_M1 = VCOMPAR_1P5MS;        Vit_M2 = VCOMPAR_1P5MS;
    Motor_Pwm_Speed();
}

void    Motor_Avance(short vitesse)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{
    Mx_Sens = 1;
    Motor_Rampe(vitesse);
}

void    Motor_Recule(short vitesse)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{
    Mx_Sens = -1;
    Motor_Rampe(vitesse);
}

void    Motor_TournGL(short vitesse)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{
    if (vitesse > 5)
        vitesse = 5;
    if (vitesse < 2)
        vitesse = 2;
    if (vitesse > 0) {
        vitesse = 1;
        vitesse = vitesse * DELTA_VIT;
        if (Mx_Sens >= 0) {
            //Vit_M1 = VCOMPAR_1P5MS - DELTA_VIT;     // Mot_G avant
            Vit_M1 = VCOMPAR_1P5MS;     // Mot_G avant
            Vit_M2 = VCOMPAR_1P5MS + vitesse;       // Mot_D arriere
        }
        else {
            //Vit_M1 = VCOMPAR_1P5MS + DELTA_VIT;     // Mot_G arriere
            Vit_M1 = VCOMPAR_1P5MS;     // Mot_G avant
            Vit_M2 = VCOMPAR_1P5MS - vitesse;       // Mot_D avant
        }
        Motor_Pwm_Speed();
    }
}

void    Motor_TournDR(short vitesse)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{
    if (vitesse > 5)
        vitesse = 5;
    if (vitesse < 2)
        vitesse = 2;
    if (vitesse > 0) {
        vitesse = 1;
        vitesse = vitesse * DELTA_VIT;
        if (Mx_Sens >= 0) {
            Vit_M1 = VCOMPAR_1P5MS - vitesse;       // Mot_G avant
            Vit_M2 = VCOMPAR_1P5MS;     // Mot_D arriere
        }
        else {
            Vit_M1 = VCOMPAR_1P5MS + vitesse;       // Mot_G arriere
            Vit_M2 = VCOMPAR_1P5MS;     // Mot_D avant
        }
        Motor_Pwm_Speed();
    }
}

void    Motor_Rampe(short vitesse)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{    short np, delta;

    Motor_Stop();
    if (vitesse > 5)
        vitesse = 5;
    if (vitesse <= 0)
        goto sortie;
    vitesse = vitesse * DELTA_VIT;
    delta   = vitesse/5;
    for (np = 1; np < 4; np++) {
        delay(250);
        if (Mx_Sens >= 0) {
            Vit_M1 -= delta;        Vit_M2 += delta;
        }
        else {
            Vit_M1 += delta;        Vit_M2 -= delta;
        }
        Motor_Pwm_Speed();
    }
    delay(250);
    if (Mx_Sens >= 0) {
        Vit_M1 = VCOMPAR_1P5MS - vitesse;   // Mot_G avant
        Vit_M2 = VCOMPAR_1P5MS + vitesse;   // Mot_D arriere
    }
    else {
        Vit_M1 = VCOMPAR_1P5MS + vitesse;   // Mot_G arriere
        Vit_M2 = VCOMPAR_1P5MS - vitesse;   // Mot_D avant
    }
    Motor_Pwm_Speed();
sortie:
    return;
}

void    Motor_Pwm_Speed(void)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{   int pwm1, pwm2, tim1, tim2;

    PWM0G1_CTL  = 0x00;         // Disable Generator 1 counter
    PWM0G1_CMPA = (VALLOAD_25P0MS - Vit_M1);
    PWM0G1_CMPB = (VALLOAD_25P0MS - Vit_M2);
    PWM0G1_CTL  = 0x01;         // Enable  Generator 1 counter
    /*
    tim1 = Vit_M1/125;       tim2 = Vit_M2/125;
    Serial.print("        Time M1 = ");	Serial.print(tim1);
    Serial.print("        Time M2 = ");	Serial.println(tim2);
    */
}


void    Motor_Init(void)
//--------------------------------------------------------------------
//--------------------------------------------------------------------
{   int value;

    // Clock setting for PWM and GPIO PORT
    //--------------------------------------------------------
    SYSCTL_RCC      |= 0x000E0000;          // Use pre-divider valur of 64
    SYSCTL_RCC      |= (1 << 20);           // Enable PWM Clock Divisor function 
    SYSCTL_RCGCPWM  |= 3;                   // Enable clock to PWM0 module
    SYSCTL_RCGCGPIO |= 0x02;                // Enable system clock to PORTB

    // Setting of PB4/PB5 pin for M0PWM2/3 channel output pin
    //--------------------------------------------------------
    GPIO_PORTB_AFSEL |= 0x30;                // PB4,PB5 sets an alternate function
    value = GPIO_PORTB_PCTL;
    value = (value & 0xFF00FFFF) | 0x00440000;
    GPIO_PORTB_PCTL   = value;               // make PB4,PB5 PWM output pin
    GPIO_PORTB_DEN   |= 0x30;                // Enable PB4,PB5 as a digital pin

    delay(10);      // Obligatoire

    PWM0G1_CTL    = 0x00;       // Disable Generator 1 counter
    SYSCTL_SRPWM  = 0x03;       // Reset Module
    delay(2);
    SYSCTL_SRPWM  = 0x00;       // end Reset Module

    delay(10);      // Obligatoire

    PWM0G1_CTL      = 0x00;             // Disable Generator 1 counter
    PWM0G1_LOAD     = VALLOAD_25P0MS;   // set load value for 40Hz : 80 MHz / 64 / 40 = 25.000
    PWM0G1_GENA     = 0x008C;           // Set PWM2(PB4) when reloaded and clear when matches CMPA
    PWM0G1_GENB     = 0x080C;           // Set PWM3(PB5) when reloaded and clear when matches CMPB
    PWM0_ENABLE     = 0x000C;           // Enable PWM2/3

    Mx_Sens = 1;
    Vit_M1 = VCOMPAR_1P5MS;      Vit_M2 = VCOMPAR_1P5MS;
}
