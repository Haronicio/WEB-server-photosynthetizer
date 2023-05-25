#include "timer.h"
#include "flag.h"

#include "macro.h"


struct Modulator_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode
  int d_min;                                              // duty minimum
  int d_max;                                              // duty maximum
  double freq;                                            // fréquence de modulation en Hz
  double d_current;                                       // le duty actuel (haute précision à virgule flottante)
  int icr;                                                // coefficient d'incrémentation du duty
}; 

void setup_mod(Modulator_s * ctx,int timer,int period)
{
  // while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->d_min = pow(2,PWM_RES - PWM_RES);
  ctx->d_max = pow(2,PWM_RES) - 1;
  ctx->d_current = pow(2,PWM_RES - 1);
  ctx->freq = 0.01;
  ctx->icr = 2; // une periode complète (d_min -> d_max -> d_min)
}

void loop_mod(Modulator_s * ctx,mailbox_s * mb_mqtt,mailbox_s * mb_buzz,mailbox_s * mb_oled)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente

  //récupère valeur du mqtt
  if(mb_mqtt->state == FULL)
  {
    //mapper les valeurs ?
    ctx->d_min = Decode_Min(mb_mqtt->val);
    ctx->d_max = Decode_Max(mb_mqtt->val);
    ctx->freq = 1/100.0*pow((double)(Decode_Freq(mb_mqtt->val)),2); //valeurs proche de 0~1 + intéressante
    ctx->d_current = (float)(ctx->d_max - ctx->d_min);
    mb_mqtt->state = EMPTY;
  }

  // changment de l'incrémentation
  if(ctx->d_current >= ctx->d_max)//atteint la valeur max
    ctx->icr = -ctx->icr;
  else if(ctx->d_current <= ctx->d_min)//atteint la valeur min
    ctx->icr = -ctx->icr;

  //        freq en Hz   periode en us  :  
  //                                          (1M/periode) -> fréquence de la tâche
  //                                          le rapport entre la fréquence de la tâche et la fréquence de modulation
  //                                          donne le nombre d'éxécution nécessaire de la tâche pour effectuer une modulation
  //                                          si on le divise la plage de valeur a faire varier par ce nombre d'éxécution
  //                                          on obtient le pas du duty pour chaque éxécution pour atteindre la plage de valeur

  ctx->d_current += ctx->icr*(double)(ctx->d_max - ctx->d_min)/(1000000.0/((double)ctx->period)/(double)ctx->freq);

  // borne les valeurs (retirer cette partie donne des résultats intéressant)
  if(ctx->d_current > ctx->d_max)//atteint la valeur max
    ctx->d_current = ctx->d_max;
  else if(ctx->d_current < ctx->d_min)//atteint la valeur min
    ctx->d_current = ctx->d_min;

  //envoi les valeurs au buzzer
  if(mb_buzz->state == EMPTY)
  {
    //le duty réél ne peut être que un entier qui peut prendre 2^PWM_RES valeurs différente
    //si la fréquence == 0 alors le duty est déterminé par les extremas, comme la forme d'onde sur la page WEB et l'écran oled
    if(ctx->freq == 0.0)
      mb_buzz->val = ctx->d_max - ctx->d_min;
    else
      mb_buzz->val = (int)ctx->d_current; //map ici ???
    mb_buzz->state = FULL;
  }

  //envoi valeur à l'oled
  if(mb_oled->state == EMPTY)
  {
    if(ctx->d_current < 0)
      ctx->d_current = 0;

    mb_oled->val = Encode_Duty(mb_oled->val,(int)ctx->d_current);
    mb_oled->val = Encode_Freq_f(mb_oled->val,ctx->freq);
    
    mb_oled->state = FULL;
  }
  
}

