#include "esp32-hal-gpio.h"
#include "timer.h"
#include "flag.h"

#include "macro.h"

#define BP_PIN 23     // ***

struct Button_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode 
  int val_prec;                                           // valeur précédemment lu
  int val_nouv;                                           // valeur qui vient d'être lu
}; 

void setup_button(Button_s * ctx,int timer,int period)
{
  while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->val_prec = 1;
  ctx->val_nouv = 1;

  //initialisation du bouton en mode pullup
  pinMode(BP_PIN, INPUT_PULLUP);
}

void loop_button(Button_s * ctx,mailbox_s * mb_buzz,mailbox_s * mb_oled)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente
  
  ctx->val_nouv = digitalRead(BP_PIN);
  if((ctx->val_prec != ctx->val_nouv) && mb_buzz->state  == EMPTY)
  {
    if(ctx->val_nouv == 0)  //  appuyé
      mb_buzz->val |= 0x1;
    else                    //  relaché
      mb_buzz->val |= 0x10;
    
    mb_buzz->state == FULL;
  }
  if((ctx->val_prec != ctx->val_nouv) && mb_oled->state  == EMPTY)
  {
    if(ctx->val_nouv == 0)  //  appuyé
      mb_oled->val |= 0x1;
    else                    //  relaché
      mb_oled->val |= 0x10;
    
    mb_oled->state == FULL;
  }
  ctx->val_prec = ctx->val_nouv;
}