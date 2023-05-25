#include "esp32-hal-ledc.h"
#include "timer.h"
#include "flag.h"

#define PWM_CHANNEL 0
#define PWM_PIN 17
#define PWM_RES 4

struct Modulator_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  int d_min;                                             // etats interne du bouton
  int d_max;
  int freq;
  double d_current;
  int icr;
}; 

void setup_mod(Modulator_s * ctx,int timer,int period)
{
  // while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->d_min = 50;//duty min
  ctx->d_max = 100;//duty max
  ctx->d_current = ctx->d_min;//duty actuel
  ctx->freq = 2;//fréquence de modulation
  // ctx->icr = (ctx->d_max-ctx->d_min)/(2*period*(ctx->d_max-ctx->d_min));
  ctx->icr = 2;


}

void loop_mod(Modulator_s * ctx,mailbox_s * mb_buzz)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente

  if(ctx->d_current > ctx->d_max)
  {
    ctx->icr = -ctx->icr;
  }
  if(ctx->d_current < ctx->d_min)
  {
    ctx->icr = -ctx->icr;
    Serial.println("done");
  }

  ctx->d_current += ctx->icr*(double)(ctx->d_max - ctx->d_min)/(1000000.0/((double)ctx->period)/(float)ctx->freq);
  // Serial.println((int)ctx->d_current);
}




/*
void loop_mod(Modulator_s * ctx,mailbox_s * mb_buzz)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente
  
  if(ctx->d_current > ctx->d_max)//j'ai atteint la valeur max
    ctx->icr = -ctx->icr;
  else if(ctx->d_current < ctx->d_min)//j'ai atteint la valeur min
    ctx->icr = -ctx->icr;

  ctx->d_current += ctx->icr;
  ctx->period = ctx->freq;

  if(mb_buzz->state == EMPTY)
  {
    mb_buzz->val = ctx->d_current;
    mb_buzz->state = FULL;
  }
  Serial.printf("%d %d %d\n",ctx->d_max,ctx->d_min,ctx->d_current);
}*/

