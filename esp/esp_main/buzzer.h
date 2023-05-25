#include "esp32-hal-ledc.h"
#include "driver/ledc.h"
#include "esp32-hal.h"
#include "timer.h"
#include "flag.h"

#include "macro.h"

#define PWM_CHANNEL 0
#define PWM_PIN 17        // ***

  //les notes assoscié a leurs fréquence pour le calcul de la note à joué
 const uint16_t noteFrequencyBase[12] = {
    //   C        C#       D        Eb       E        F       F#        G       G#        A       Bb        B
        4186,    4435,    4699,    4978,    5274,    5588,    5920,    6272,    6645,    7040,    7459,    7902
    };

  //les différentes gammes : espacement entre des notes les valeurs communément utiliser son 1 2 et 3 
  //(vous pouvez les ajouter ici et dans le switch plus bas)
  const char gammeChrom[] = {1,1,1,1,1,1,1,1,1,1,1,1};
  const char gammeMaj[] = {2,2,1,2,2,2,1};
  const char gammeMin[] = {1,2,1,1,2,1,1};
  const char gammePenMaj[] = {2,2,3,2,3};
                                                      //    ***

//--------- définition de la tache Buzzer

struct Buzzer_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode
  note_t current_note    ;                                // note courrante
  int current_octave ;                                    // octave courante
  uint16_t current_duty;                                  // duty courant
  bool state;                                             // état du bouton
  uint8_t current_gamme;                                  // gamme courante
  uint8_t current_cle;                                    // clé courante

  //pour la sub tâche   on aurait pu fire une autre structure mais par soucis de simplicité
  
  bool sub_task;                                              // sous tâche on/off
  int sub_timer;                                              // numéro du timer pour la sous tâche
  unsigned long sub_period;                                   // periode de la sous tâche
}; 

//ATTENTION cette fonction retournait initialement un tableau de note_t cependant 
//l'ESP crash à la libération de la mémoire, retourner un tableau de byte contourne ce problème
//Génère une gamme de note en fonction de la gamme (interval entre les notes) et de la clé (offset)
byte * genererGamme(const char * gamme,int gammeSize,note_t cle)
{
    int j = 0;
    byte * res = (byte *)malloc(gammeSize);
    if(!res)
    {
      Serial.println("memalloc error");
      return NULL;
    }
    for (int i = (int)cle; j < gammeSize; i++)
    {
        res[j] = (byte)cle;
        cle = (note_t)(((int)cle + (int)gamme[j])%(int)NOTE_MAX);
        j++;
    }
    return res;
}

void setup_buzzer(Buzzer_s * ctx,int timer,int period)
{
  // while(!Serial);
  ctx->timer = timer;
  ctx->period = period;
  ctx->current_note = NOTE_C;
  ctx->current_octave = MIN_OCTAVE;
  ctx->current_duty = pow(2,PWM_RES-1);
  ctx->current_gamme = 1;
  ctx->current_cle = NOTE_C;
  ctx->state = true;

  //configuration sous tâche
  ctx->sub_task = SUB_TASK;
  ctx->sub_timer = TIMER_SUB_TASK;
  ctx->sub_period = PERIODE_MOD;

  //configuration de la broche PWM pour le buzzer, on associe le channel PWM au pin utilisé
  ledcAttachPin(PWM_PIN, PWM_CHANNEL);
}

//Fonction basé sur la bibliothèque "esp32-hal-ledc.h", joue sur le channel la note au duty indiqué
void writeNote(note_t note, uint8_t octave,uint32_t duty)
{
    //en dehors des notes jouable
    if(octave > 8 || note >= NOTE_MAX){
        return;
    }

    //calcule de la note à jouer avec le tableau de correspondance note fréquence
    uint32_t noteFreq =  (uint32_t)noteFrequencyBase[note] / (uint32_t)(1 << (8-octave));
    //écriture du signal PWM sur la broche
    ledcSetup((uint8_t)PWM_CHANNEL,noteFreq,(uint8_t)PWM_RES);
    ledcWrite(PWM_CHANNEL, duty);
}

void loop_buzzer(Buzzer_s * ctx,mailbox_s * mb_button,mailbox_s * mb_mod, mailbox_s * mb_photo, mailbox_s * mb_mqtt,mailbox_s * mb_oled)
{
  // dilemme du modulator, la fréquence de modulation du duty peut être trop grande par rapport au timer du buzzer
  // pour obtenir une vrai modulation de la wave form, alors on défini une sous tâche

  if(ctx->sub_task == true && waitFor(ctx->sub_timer,ctx->sub_period))
  {
    if(mb_mod->state == FULL)
    {
      ctx->current_duty = mb_mod->val;
      ledcWrite(PWM_CHANNEL, ctx->current_duty);
      mb_mod->state = EMPTY;
    }
  }


  if (!waitFor(ctx->timer,ctx->period)) return; // sort si moins d'une periode d'attente

  if (mb_button->state == FULL); // gestion de l'appuie sur le bouton
  {
    if((mb_button->val & 0x10) == 0x10)
    {
      mb_button->val ^= 0x10;
      ctx->state =  !ctx->state;
      ledcAttachPin(PWM_PIN, PWM_CHANNEL);
    }
    mb_button->state = EMPTY;
  }
  if(ctx->state == false)  //on quitte s'il est éteint
  {
    ledcDetachPin(PWM_PIN);
    return;    
  }

  //modulation du duty du PWM par le modulator
  if(mb_mod->state == FULL)
  {
    ctx->current_duty = mb_mod->val;

    mb_mod->state = EMPTY;
  }

  //récupère valeurs du mqtt
  if(mb_mqtt->state == FULL)
  {
    ctx->current_gamme = Decode_Gamme(mb_mqtt->val);
    ctx->current_cle = Decode_Cle(mb_mqtt->val);

    mb_mqtt->state = EMPTY;
  }
    
  if(mb_photo->state == FULL)//lecture de la valeure à joué en fonction de la photor
  {
    //la plage de note à joué en fonction de la gamme et de la clé
    int notesGamme_len = 0;
    byte * notesGammeCle = NULL;
    //sélectionne la gamme où joué la note
    switch(ctx->current_gamme)
    {
      case 0: //chromatique
              notesGamme_len = sizeof(gammeChrom);
              notesGammeCle = genererGamme(gammeChrom, notesGamme_len, (note_t)ctx->current_cle);
              break;
      case 1: //majeure
              notesGamme_len = sizeof(gammeMaj);
              notesGammeCle = genererGamme(gammeMaj, notesGamme_len, (note_t)ctx->current_cle);
              break;
      case 2: //mineure
              notesGamme_len = sizeof(gammeMin);
              notesGammeCle = genererGamme(gammeMin, notesGamme_len, (note_t)ctx->current_cle);
              break;
      case 3: //pentatonique majeur
              notesGamme_len = sizeof(gammePenMaj);
              notesGammeCle = genererGamme(gammePenMaj, notesGamme_len, (note_t)ctx->current_cle);
              break;
    }
    // la note a joué dépend de la valeur de la photorésistance, elle est calculé suivant le nombre de  note dans la gamme
    ctx->current_note = (note_t)notesGammeCle[mb_photo->val % notesGamme_len];
    ctx->current_octave = (mb_photo->val / notesGamme_len) + MIN_OCTAVE;
    //le free buggé de l'ESP
    free(notesGammeCle);
    notesGammeCle = NULL;

    mb_photo->state = EMPTY;
  }


  //envoi de la note et l'octave à l'écran oled
  if(mb_oled->state == EMPTY)
  {
    mb_oled->val = Encode_Note(mb_oled->val, ((uint8_t)ctx->current_note));
    mb_oled->val = Encode_Octave(mb_oled->val, ((uint8_t)ctx->current_octave));

    mb_oled->state = FULL;
  }




  writeNote(ctx->current_note,ctx->current_octave,ctx->current_duty);
}


