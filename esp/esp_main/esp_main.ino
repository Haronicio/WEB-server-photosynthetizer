#include "timer.h"
#include "flag.h"
#include "oled.h"
#include "mqtt.h"
#include "button.h"
#include "lum.h"
#include "buzzer.h"
#include "modulator.h"


//--------------- Déclaration des tâches
struct MQTT_s MQTT;
struct Oled_s Oled;
struct Button_s Button;
struct Lum_s Lum;
struct Buzzer_s Buzzer;
struct Modulator_s Modulator;

//--------------- Déclaration des mailbox

struct mailbox_s mb_button_buzz = {.state = EMPTY,.val = 0};    //  allumer/éteinde le buzz
struct mailbox_s mb_photo_mqtt = {.state = EMPTY};              //  lumière vers le server
struct mailbox_s mb_photo_buzz = {.state = EMPTY};              //  lumière vers note a joué
struct mailbox_s mb_mqtt_buzz = {.state = EMPTY,.val = 0};      //  gamme et clé du server
struct mailbox_s mb_mqtt_mod = {.state = EMPTY,.val = 0};       //  plage et fréquence de modulation
struct mailbox_s mb_mod_buzz = {.state = EMPTY,.val = 0};       //  modulation du duty
struct mailbox_s mb_buzz_oled = {.state = EMPTY,.val = 0};      //  note joué à afficher
struct mailbox_s mb_mod_oled = {.state = EMPTY,.val = 0};       //  forme d'onde modulé à afficher
struct mailbox_s mb_button_oled = {.state = EMPTY};             //  ON/OFF

//initialisations des tâches
void setup() {
  Serial.begin(9600);

  //periodes en micro seconde
  setup_oled(&Oled, 0, 50000);
  setup_mqtt(&MQTT, 1, 200000);
  setup_button(&Button,2,20000);
  setup_lum(&Lum,3,100000);
  setup_buzzer(&Buzzer, 4, 200000);   //  à faire varier entre 50000 et 500000 pour des résultats différents (ajouter slider)
  setup_mod(&Modulator, 5, PERIODE_MOD);
//sub_task de buzzer : TIMER_SUB_TASK(=6)  , PERIODE_MOD
}

//boucles principale
void loop() {
  loop_oled(&Oled,&mb_buzz_oled,&mb_mod_oled,&mb_button_oled);
  loop_mqtt(&MQTT,&mb_photo_mqtt,&mb_mqtt_buzz,&mb_mqtt_mod);
  loop_button(&Button,&mb_button_buzz,&mb_button_oled);
  loop_lum(&Lum,&mb_photo_mqtt,&mb_photo_buzz);
  loop_buzzer(&Buzzer,&mb_button_buzz,&mb_mod_buzz,&mb_photo_buzz,&mb_mqtt_buzz,&mb_buzz_oled);
  loop_mod(&Modulator,&mb_mqtt_mod ,&mb_mod_buzz,&mb_mod_oled);
}
