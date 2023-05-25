#include "esp32-hal-ledc.h"
#include "driver/ledc.h"
#include "esp32-hal.h"
#include "timer.h"
#include "flag.h"

#define PWM_CHANNEL 0
#define PWM_PIN 17
#define PWM_RES 4
#define MIN_OCTAVE 3
#define MAX_OCTAVE 5

 const uint16_t noteFrequencyBase[12] = {
    //   C        C#       D        Eb       E        F       F#        G       G#        A       Bb        B
        4186,    4435,    4699,    4978,    5274,    5588,    5920,    6272,    6645,    7040,    7459,    7902
    };

//--------- définition de la tache Buzzer

struct Buzzer_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  note_t current_note    ;                                    // note courrante
  int current_octave ;                                     //octave courante
  uint16_t current_duty;
  bool state;
}; 

void setup_buzzer(Buzzer_s * ctx,int timer,int period)
{
  // while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->current_note = NOTE_C;
  ctx->current_octave = MIN_OCTAVE;
  ctx->current_duty = 2^(PWM_RES-1);
  ctx->state = true;

  //configuration de la broche PWM pour le buzzer, on associe le channel au pin
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

void WriteNote(note_t note, uint8_t octave,uint32_t duty)
{
    //calcul de la note
    if(octave > 8 || note >= NOTE_MAX){
        return;
    }
    uint32_t noteFreq =  (uint32_t)noteFrequencyBase[note] / (uint32_t)(1 << (8-octave));
    ledcSetup((uint8_t)PWM_CHANNEL,noteFreq,(uint8_t)PWM_RES);
    ledcWrite(PWM_CHANNEL, duty);
}

void loop_buzzer(Buzzer_s * ctx,mailbox_s * mb_mod)
{
  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente

  if(mb_mod->state == FULL)
  {
    ctx->current_duty = mb_mod->val;
    mb_mod->state = EMPTY;
  }
    

  WriteNote(ctx->current_note,4,ctx->current_duty);
  // ledcWriteNote(PWM_CHANNEL, ctx->current_note, 4);
}


