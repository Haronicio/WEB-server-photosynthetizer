#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "timer.h"
#include "flag.h"

#include "macro.h"

void testdrawchar();

void displayNotePlay(uint8_t,uint8_t);
void displayWaveForm(uint8_t,double);
void displayPower(bool);


//--------- définition de la tache Oled

struct Oled_s {
  int timer;                                              // numéro du timer pour cette tâche utilisé par WaitFor
  unsigned long period;                                   // periode de clignotement
  bool button_state;                                      // état du bouton
  uint8_t duty;                                           // duty
  double freq;                                            // fréquence de modulation
  uint8_t note;                                           // note 
  uint8_t octave;                                         // octave
}; 

//--------------------------- Pour l'écran OLED 128x32

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// Declaration for an SSD1306 display connected to I2C (SDA, SCL pins)
#define OLED_RESET     16 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void setup_oled(struct Oled_s * ctx, int timer, unsigned long period ) {

  ctx->timer = timer;
  ctx->period = period;
  ctx->button_state = true;
  ctx->duty = 0;
  ctx->freq = 0.0;
  ctx->note = 0;
  ctx->octave = 0;

  Wire.begin(4, 15); // pins SDA , SCL
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for(;;); // Don't proceed, loop forever
  }

  // Show initial display buffer contents on the screen --
  // the library initializes this with an Adafruit splash screen.
  display.display();
  display.clearDisplay();
  display.setTextSize(2); // Draw 2X-scale text
  display.setTextColor(WHITE);
  display.setCursor(0,0);

  display.printf("%s\n",F("Connexion au MQTT.."));

  display.display();

  display.setTextSize(1);
  display.setTextColor(WHITE);

  delay(1000);
}

void loop_oled(struct Oled_s * ctx,mailbox_s * mb_buzz, mailbox_s * mb_mod,mailbox_s * mb_button) {
  if (!waitFor(ctx->timer,ctx->period)) return;         // sort s'il y a moins d'une'période écoulée
  
  display.clearDisplay();

  //Affichage note + octave
  if(mb_buzz->state == FULL)
  {
    ctx->note = Decode_Note(mb_buzz->val);
    ctx->octave = Decode_Octave(mb_buzz->val);
    mb_buzz->state = EMPTY;
  }
  if(ctx->button_state == true)
    displayNotePlay(ctx->note,ctx->octave);

  //Affichage Waveform + fréquence mod
  if(mb_mod->state == FULL)
  {
    ctx->duty = Decode_Duty(mb_mod->val);
    uint16_t short_freq = Decode_Freq(mb_mod->val);
    ctx->freq = decode_f(short_freq);
    mb_mod->state = EMPTY;
  }
  displayWaveForm(ctx->duty,ctx->freq);

  //Affichage du bouton
  if (mb_button->state == FULL); // gestion de l'appuie sur le bouton
  {
    if((mb_button->val & 0x10) == 0x10)
    {
      mb_button->val ^= 0x10;
      ctx->button_state =  !ctx->button_state;
    }
    mb_button->state = EMPTY;
  }
  displayPower(ctx->button_state);

  display.display();
}

//-------------------------- Fonctions pour afficher

void displayNotePlay(uint8_t note,uint8_t octave)
{
  //associe une note (0 à 11) à son équivalent littérale
  char str_note[3] = {0};
  switch(note)
  {
    case 0:   strcpy(str_note,"C"); break;
    case 1:   strcpy(str_note,"C#"); break;
    case 2:   strcpy(str_note,"D"); break;
    case 3:   strcpy(str_note,"Eb"); break;
    case 4:   strcpy(str_note,"E"); break;
    case 5:   strcpy(str_note,"F"); break;
    case 6:   strcpy(str_note,"F#"); break;
    case 7:   strcpy(str_note,"G"); break;
    case 8:   strcpy(str_note,"G#"); break;
    case 9:   strcpy(str_note,"A"); break;
    case 10:  strcpy(str_note,"Bb"); break;
    case 11:  strcpy(str_note,"B"); break;
  }

  display.setCursor(10, 0);
  display.printf("%s%d\n",str_note,octave);
}

void displayPower(bool button_state)
{
  char str_state[4] = {0};
  if(button_state)
    strcpy(str_state,"ON");
  else
    strcpy(str_state,"OFF");

  display.setCursor(110, 0);
  display.printf("%s",str_state);
}

void displayWaveForm(uint8_t duty,double freq)
{
  //dessine forme d'onde
  //          pour l'instant le duty ne va pas jusqu'à sa valeur max (0 à 100 au lieu de 0 à 2^PW_RES(127))
  //          sauf au lancement (cf setup_mod) UPDATE : corrigé simplement dans le code du server
  //          true_d le % du duty par rapport au duty maximum, front montant
  //          off_d front descendant
  uint8_t true_d = duty/*(uint8_t)((100.0*(float)duty)/(float)(pow(2,PWM_RES) - 1.0))*/; 
  uint8_t off_d = (uint8_t)((100.0 - (float)duty)/2.0);
  uint8_t xmargin = 15;
  uint8_t hauteuron = 10;
  uint8_t hauteuroff = 30;

  //                                   ___________       Comment est dessiner l'onde 
  //                                  |           |
  //                                  |           |
  //                          ---------           ---------
  //             100 - duty(%)    /2     duty(%)       100 - duty(%)   /2

  display.drawLine(xmargin,hauteuroff,xmargin + off_d,hauteuroff,SSD1306_WHITE);//trait 1 horizontal off
  display.drawLine(xmargin + off_d,hauteuroff,xmargin + off_d,hauteuron,SSD1306_WHITE);//trait 2 vertical
  display.drawLine(xmargin + off_d,hauteuron,xmargin + off_d + true_d,hauteuron,SSD1306_WHITE);//trait 3 horizontal on
  display.drawLine(xmargin + off_d + true_d,hauteuron,xmargin + off_d + true_d,hauteuroff,SSD1306_WHITE);//trait 4 vertical
  display.drawLine(xmargin + off_d + true_d,hauteuroff,xmargin + off_d + true_d + off_d,hauteuroff,SSD1306_WHITE);//trait 5 horizontal off


  //affichage fréquence
  display.setCursor(50, 0);
  display.printf("%.2fHz",freq);
}


