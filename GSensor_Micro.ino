#include "Gnx_Project_Chariot.h"

// ##TAG##1 deb
#define   SENSOR_PIN  A5

#define   SAMPLE_FREQ   4000
#define   SAMPLE_PERIOD (1000000/SAMPLE_FREQ)
#define   SIZE_BUFF   512

float Buffer_Sample[SIZE_BUFF];
float Power_Inst[SIZE_BUFF];

#define   LEN_K (SAMPLE_FREQ*5)
#define   SIZE_AUTOCORR (SIZE_BUFF/2)
float Buff_AutoCorr[SIZE_AUTOCORR];
// ##TAG##1 end

void  Read_Samples(void);
float myPowerEstimation(void);
short myFreqEstimation(void);

// ##TAG##4 deb
#define   LOW_LEVEL   0.1

short Sensor_Measure(void)
//-----------------------------------------------------
// Main Fonction 
//-----------------------------------------------------
{
  float SignalPower;
  short freqSig;

  Serial.println(" ");
  Serial.println("--- Signal Power Measurement ---");

  // Étape 1 : lecture des échantillons
  Read_Samples();

  // Étape 2 : estimation de la puissance
  SignalPower = myPowerEstimation();

  // Affichage
  Serial.print(" Power = "); Serial.println(SignalPower);
  if (SignalPower < LOW_LEVEL) {
    Serial.println("Signal trop faible (bruit ou silence)");
    freqSig = -1;
  } else {
    Serial.println("Signal détecté");
    freqSig = myFreqEstimation();
    Serial.print(" Fréquence détectée = "); Serial.println(freqSig);
  }
  return freqSig;
}
// ##TAG##4 end


// ##TAG##3 deb
float myPowerEstimation(void)
//------------------------------------------------------------------------------
//  Read signal and compute mean power
//------------------------------------------------------------------------------
{
  float power, ech, pwr_sum = 0.0, pval, val_moy = 0.0;

  // Moyenne
  for (int i = 0; i < SIZE_BUFF; i++) {
    val_moy += Buffer_Sample[i];
  }
  val_moy /= SIZE_BUFF;

  // Puissance instantanée
  for (int i = 0; i < SIZE_BUFF; i++) {
    ech = Buffer_Sample[i] - val_moy;
    pval = ech * ech;
    Power_Inst[i] = pval;
    pwr_sum += pval;
  }
  power = pwr_sum / SIZE_BUFF;
  return power;
}

short myFreqEstimation(void)
//------------------------------------------------------------------------------
//  compute signal frequency : Autocorrelation
//------------------------------------------------------------------------------
{
  int lag_max = SIZE_AUTOCORR;
  float R;
  int best_lag = 1;
  float R_max = 0;

  for (int k = 1; k < lag_max; k++) {
    R = 0;
    for (int n = 0; n < SIZE_BUFF - k; n++) {
      R += (Buffer_Sample[n] * Buffer_Sample[n + k]);
    }
    Buff_AutoCorr[k] = R;
    if (R > R_max) {
      R_max = R;
      best_lag = k;
    }
  }

  float freq_estimee = (float)SAMPLE_FREQ / best_lag;
  return (short)freq_estimee;
}
// ##TAG##3 end


// ##TAG##2 deb
void Sensor_Init(void)
//-----------------------------------------------------
{
  Serial.begin(9600);
  pinMode(SENSOR_PIN, INPUT);
}

#define ADC_MAX_VDD 3.3
#define ADC_MAX_NUM 4096

void Read_Samples(void)
//------------------------------------------------------------------------------
{
  float val_float;
  short idx_sample, val_integer;
  unsigned long curtime, nextime;

  nextime = micros() + SAMPLE_PERIOD;
  idx_sample = 0;
  while (idx_sample < SIZE_BUFF) {
    do { curtime = micros(); } while (curtime < nextime);

    val_integer = analogRead(SENSOR_PIN);
    val_float = ((float)val_integer * ADC_MAX_VDD) / ADC_MAX_NUM;
    Buffer_Sample[idx_sample] = val_float;
    idx_sample++;

    nextime += SAMPLE_PERIOD;
  }
}
// ##TAG##2 end
