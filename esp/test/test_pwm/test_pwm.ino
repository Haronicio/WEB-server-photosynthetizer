#include "timer.h"
#include "flag.h"
#include "buzzer.h"
#include "modulator.h"


//--------------- Déclaration des tâches
// struct Buzzer_s Buzzer;
struct Modulator_s Modulator;

//--------------- Déclaration des mailbox

struct mailbox_s mb_mod_buzz = {.state = EMPTY};

void setup() {
  Serial.begin(9600);
  // Serial.println(sizeof(int));
  //en micro seconde
  // setup_buzzer(&Buzzer, 0, 100000);
  setup_mod(&Modulator, 0, 10);
}

void loop() {
  // loop_buzzer(&Buzzer,&mb_mod_buzz);
  loop_mod(&Modulator, &mb_mod_buzz);
}
