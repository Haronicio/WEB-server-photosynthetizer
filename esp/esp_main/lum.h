#include "timer.h"
#include "flag.h"

#include "macro.h"

#define PHOTO_PIN 36    // ***
#define PHOTO_MAX 4095  //valeur max de la photo-résistance calibré à la main


//--------- définition de la tache Lum

struct Lum_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode 
  int val;                                                // valeur de la photo resistance
}; 

void setup_lum(Lum_s * ctx,int timer, int period) {
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->val = 0;
}

void loop_lum(Lum_s * ctx,mailbox_s * mb_mqtt ,mailbox_s * mb_buzz) {
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente

  ctx->val = analogRead(PHOTO_PIN);

  // envoi vers mqtt
  if (mb_mqtt->state == EMPTY);
  {
    //plage de valeur en %
    mb_mqtt->val = map(ctx->val,0,PHOTO_MAX,0,100);
    mb_mqtt->state = FULL;
  }

  // envoi vers buzzer
  if (mb_buzz->state == EMPTY);
  {
    //    plage de valeur déterminer un peu au pif : les gammes les plus petites (pentatonique)
    //    contiennent 5 notes en général, j'ai déterminer que une bonne plage de note (au total) contenait 
    //    donc max oct * 5
    //    A l'avenir le mapping devrait être dans le buzzer pour dépendre de la taille de la gamme
    mb_buzz->val = map(ctx->val,0,PHOTO_MAX,0,MAX_OCTAVE*5);
    mb_buzz->state = FULL;
  }
  
}