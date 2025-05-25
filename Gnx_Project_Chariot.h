//-----------------------------------------------------------------------------
//  Projet_Chariot.h
// 
//-----------------------------------------------------------------------------

#define PIN_LED_R     PF_1
#define PIN_LED_G     PF_2
#define PIN_LED_B     PF_3
#define PIN_POTAR     PE_3
#define PIN_BOUTON    PC_6


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void	Sensor_Init(void);
short	Sensor_Measure(void);


//------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------
void    AOLED_InitScreen(void);
void    AOLED_AffiLogoIsep(void);
void    AOLED_FillScreen(   char  value);
void    AOLED_InvertDisplay(short invert);
void    AOLED_DisplayImage( char  *pBuff);
void    AOLED_DisplayCarac( int x, int y, char car);
void    AOLED_DisplayTexte( int x, int y, char* texte);


//------------------------------------------------------------------------------------
// Generer deux signaux PWM sur les sorties PB4 et PB5.
// Peroide = 20 ms. Duree impulsion = 1 ms (avant), 1.5 ms (arret) ou 2 ms (arriere).
// Dans la fonction setup(), Appeler la fonction Motor_Init() pour initialiser le module PWM.
// Dans la fonction loop(),  Appeler les fonctions :
//		- Motor_xxxxn(short vitesse) 
//		Utiliser une valeur numerique entre 1 et 5 pour le parametre 'vitesse'
//------------------------------------------------------------------------------------
void    Motor_Init(void);
void    Motor_Stop(void);
void    Motor_Avance(short vitesse);
void    Motor_Recule(short vitesse);
void    Motor_TournGL(short vitesse);
void    Motor_TournDR(short vitesse);
