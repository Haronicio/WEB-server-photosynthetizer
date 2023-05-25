//Macros partagées entre les tâches
//Ce qui est marqué de *** peut être modifié 

#if !defined(_MACRO_H)
#define _MACRO_H

//Résolution du signal PWM
#define PWM_RES 7           // *** ne pas oublier de changer 'resolution' dans le code du server
//Les octaves or de cette plage de valeur son quasiment inaudible
#define MIN_OCTAVE 3        // ***
#define MAX_OCTAVE 5        // ***

//periode de la tâche modulator et la sous tâche du buzzer
#define PERIODE_MOD 50000   
#define TIMER_SUB_TASK 6
#define SUB_TASK true       // ***

// masque et calcul pour déterminer la position des valeurs plus petite qu un int à placer dans un int
// ce méfier de l'utilisation de ces macros (contrainte de type et parsing buggé)
#define MASQ_0_BUZZ 0x0000FFFF
#define MASQ_1_BUZZ 0xFFFF0000
#define MASQ_0_MOD 0x000000FF
#define MASQ_1_MOD 0x0000FF00
#define MASQ_2_MOD 0x00FF0000

#define Decode_Min(val) (val & MASQ_0_MOD)
#define Decode_Max(val) (val & MASQ_1_MOD) >> 8
#define Decode_Freq(val) (val & MASQ_2_MOD) >> 16
#define Decode_Gamme(val) (val & MASQ_0_BUZZ)
#define Decode_Cle(val) (val & MASQ_1_BUZZ) >> 16

#define Encode_Note(val,enc) ((val & MASQ_1_BUZZ) | (enc))
#define Encode_Octave(val,enc) ((val & MASQ_0_BUZZ) | (enc << 16))

#define Decode_Note(val) (val & MASQ_0_BUZZ)
#define Decode_Octave(val) (val & MASQ_1_BUZZ) >> 16

#define Encode_Duty(val,enc) ((val & MASQ_1_BUZZ) | (enc))
#define Encode_Freq(val,enc) ((val & MASQ_0_BUZZ) | (enc << 16))

#define Decode_Duty(val) (val & MASQ_0_BUZZ)
#define Decode_Freq(val) (val & MASQ_1_BUZZ) >> 16

//fonction interne pour encoder un nombre flottant a 2 décimale vers un nombre sur 2 octets
static short encode_f(double value) {
  //conversion vers vers un double à 2 decimale, les valeurs comme 0.001 et 0.0001 fonctionne mais pas les autres a + de décimale
  int cnt = (int)(value * 100.0);
  value = (double)cnt / 100.0;

  cnt = 0;
  while (value != floor(value)) {
    value *= 10.0;
    cnt++;
  }
  return (short)((cnt << 12) + (int)value);
}

static double decode_f(short value) {
  int cnt = value >> 12;
  double result = value & 0xfff;
  while (cnt > 0) {
    result /= 10.0;
    cnt--;
  }
  return result;
}

// toujours pareil mais spécialement pour un float (encode ne marche pas comme prévu ??)

#define Encode_Freq_f(val,enc) ((val & MASQ_0_BUZZ) | (encode_f(enc) << 16))

#define Decode_Freq_f(val) decode_f((val & MASQ_1_BUZZ) >> 16)

#endif // _MACRO_H