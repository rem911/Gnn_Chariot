#include "Gnx_Project_Chariot.h"
#include "OLedAffi_MyLogo.h"

#define SEUIL 2048
#define PIN_MICRO A5
#define PIN_SHARP PD_3
#define BTSerial Serial1

unsigned long moteur_timeout = 0;
short sens = 0, old_vit = 0;

void Deplacement(short vitesse, short direc, short tourn);

void setup()
{
    Serial.begin(9600);
    BTSerial.begin(9600);

    pinMode(PIN_LED_R, OUTPUT);
    pinMode(PIN_LED_G, OUTPUT);
    pinMode(PIN_LED_B, OUTPUT);

    pinMode(PIN_POTAR, INPUT);
    pinMode(PIN_BOUTON, INPUT);
    pinMode(PIN_SHARP, INPUT);

    delay(1000);

    digitalWrite(PIN_LED_R, HIGH);
    digitalWrite(PIN_LED_B, LOW);
    digitalWrite(PIN_LED_G, LOW);

    Serial.println("+++ INIT Motor  Debut  ++++");
    Motor_Init();
    Serial.println("+++ INIT Motor Fin    ++++");
    Motor_Stop();

    Serial.println("+++ INIT AOLED  Debut  ++++");
    AOLED_InitScreen();
    Serial.println("+++ INIT AOLED Fin    ++++");
    delay(2000);
    AOLED_DisplayImage((char*)Gnn_myLogo);

    Sensor_Init();

    digitalWrite(PIN_LED_R, LOW);
    delay(1000);
}

void loop()
{
    short valpotar, vitesse, etat, freq;
    int valSharp;
    float tensionSharp;

    // Vérification commande Bluetooth
    if (BTSerial.available()) {
        String msg = BTSerial.readStringUntil('\n');
        msg.trim();
        if (msg == "stop") {
            Serial.println("Commande 'stop' reçue, arrêt moteur.");
            Motor_Stop();
            digitalWrite(PIN_LED_R, LOW);
            digitalWrite(PIN_LED_G, LOW);
            digitalWrite(PIN_LED_B, LOW);
            moteur_timeout = 0;
            old_vit = 0;
        }
    }

    etat = digitalRead(PIN_BOUTON);
    if (etat == 0) {
        Motor_Stop();
        old_vit = 0;
        while (digitalRead(PIN_BOUTON) == 0);
        sens = (sens + 1) & 1;
    }

    // Lecture fréquence
    freq = Sensor_Measure();
    Serial.print("Fréquence détectée : ");
    Serial.println(freq);

    if (freq >= 480 && freq <= 520) {
        moteur_timeout = millis() + 5000;
    }

    if (millis() > moteur_timeout) {
        Serial.println(">> Pas de 500Hz récent : arrêt moteur");
        Motor_Stop();
        old_vit = 0;
        goto sortie;
    }

    // Détection obstacle avec capteur Sharp
    valSharp = analogRead(PIN_SHARP);
    tensionSharp = ((float)valSharp * 3.3) / 4096.0;

    Serial.print("Tension Sharp : ");
    Serial.println(tensionSharp);

    if (tensionSharp > 2.0) {
        Serial.println("🚨 Obstacle détecté ! Arrêt immédiat.");
        Motor_Stop();
        moteur_timeout = 0;
        old_vit = 0;
        goto sortie;
    }

    // Contrôle vitesse via potentiomètre
    valpotar = analogRead(PIN_POTAR);
    vitesse = map(valpotar, 0, 4095, 1, 5);

    if (vitesse == old_vit)
        goto sortie;

    Serial.print("    --- Nouvelle vitesse = ");
    Serial.println(vitesse);
    Deplacement(vitesse, sens, 0);
    old_vit = vitesse;

sortie:
    delay(500);
}

void Deplacement(short vitesse, short direc, short tourn)
{
    short ledR, ledV, ledB;

    if (vitesse == 0) {
        ledR = 0;   ledB = 0;   ledV = 0;
        Motor_Stop();
    }
    else if (direc == 0 && tourn == 0) {
        ledR = 0;   ledB = 0;   ledV = 1;
        Motor_Avance(vitesse);
    }
    else if (direc == 1 && tourn == 0) {
        ledR = 0;   ledB = 1;   ledV = 0;
        Motor_Recule(vitesse);
    }
    else if (direc == 0 && tourn == 1) {
        ledR = 1;   ledB = 0;   ledV = 1;
        Motor_TournGL(2);
    }
    else if (direc == 1 && tourn == 1) {
        ledR = 1;   ledB = 1;   ledV = 0;
        Motor_TournDR(2);
    }

    digitalWrite(PIN_LED_R, ledR);
    digitalWrite(PIN_LED_B, ledB);
    digitalWrite(PIN_LED_G, ledV);
}
