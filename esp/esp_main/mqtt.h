#include "pgmspace.h"
#include "stdio.h"
#include "timer.h"
#include "flag.h"

#include <SPI.h>
#include <WiFi.h>
#include <PubSubClient.h>


#include "macro.h"
#define MAX_MQTT_RCV 1024

//Getsion du MQTT + WiFin, évite au maximum les variables globales avec une tone de macro
//Les objets indispensable sont sauvegarder dans la structure
//Ce qui est marqué de *** est à modifié 


struct MQTT_s{
  int timer;                                              
  unsigned long period; 
  WiFiClient wifiClient;
  IPAddress server;
  PubSubClient client;

};

//buffer et variable global pour les requêtes
byte buffer[MAX_MQTT_RCV] = {0};
boolean Rflag=false;
int r_len;

// WiFi
#define ssid "Freebox-7D792E"           // ***        
#define wifi_password "SebastienSexy7"  // ***

//MQTT   
#define MQTT_PORT 1884                  // ***
#define MQTT_BROKER_ADDR 192,168,1,12   // ***

#define clientID "esp_synth"
#define sub_topic "pi2esp"
#define pub_topic "esp2pi"

// callback s'éxécutant à chaque réception d'un msg
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");

  Rflag=true;
  r_len=length;

  for (int i=0;i<length;i++) {
   buffer[i] = payload[i];
  }
  buffer[length] = '\0';
}

// (re)connexion au broker
void reconnect(PubSubClient * client) {

  while (!client->connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client->connect(clientID)) {
      Serial.println("connected");
      client->subscribe(sub_topic);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client->state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

// Paramétrage MQTT
void connect_MQTT(IPAddress * server,PubSubClient * client)
{
  client->setServer(*server, MQTT_PORT);
  client->setCallback(callback);
}

// Paramétrage WiFi
void connect_WiFi()
{
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup_mqtt(MQTT_s * ctx,int timer,int period)
{
  ctx->timer = timer;
  ctx->period = period;
  ctx->client = PubSubClient(ctx->wifiClient);
  ctx->server = IPAddress(MQTT_BROKER_ADDR);

  connect_MQTT(&ctx->server,&ctx->client);
  connect_WiFi();
}

void loop_mqtt(MQTT_s * ctx,mailbox_s * mb_photo,mailbox_s * mb_buzz,mailbox_s * mb_mod)
{
  if (!waitFor(ctx->timer,ctx->period)) return;
  if (!ctx->client.connected())
    reconnect(&ctx->client);

  //envoi de la valeur de la photores
  if(mb_photo->state == FULL)
  {
    char buff[5] = {0};
    ctx->client.publish(pub_topic,itoa(mb_photo->val,buff,10));
    mb_photo->state = EMPTY;
  }

  //récupération des valeurs reçues (s'il y en a) et sérialisation vers les autres tâches
  if(Rflag)
  {
    // buffer pour chaque paramètre
    char str_d_min[5] = {0};
    char str_d_max[5] = {0};
    char str_freq[5] = {0};
    char str_gamme[5] = {0};
    char str_cle[5] = {0};

    sscanf((char *)buffer,"%s %s %s %s %s",str_d_min,str_d_max,str_freq,str_gamme,str_cle);

    //  qu'une seule variable dans une mailbox, mais on peut les concaténer bit à bit (voir macro.h)
    //                   val(vers Buzz)  0xCCCCGGGG  G gamme C clé
    //                   val(vers Mod )  0x00FFMMmm  F fréquence m min M max

    //envoi vers modulator
    if(mb_mod->state == EMPTY)
    {
      if(strcmp(str_d_min,"None") != 0)
      mb_mod->val = (mb_mod->val & (MASQ_2_MOD | MASQ_1_MOD)) | (atoi(str_d_min));
      if(strcmp(str_d_max,"None") != 0)
        mb_mod->val = (mb_mod->val & (MASQ_2_MOD | MASQ_0_MOD)) | (atoi(str_d_max) << 8);
      if(strcmp(str_freq,"None") != 0)
        mb_mod->val = (mb_mod->val & (MASQ_0_MOD | MASQ_1_MOD)) | (atoi(str_freq) << 16);

        mb_mod->state = FULL;
    }
    
    //envoi vers buzzer
    if(mb_buzz->state == EMPTY)
    {
      if(strcmp(str_gamme,"None") != 0)
        mb_buzz->val = (mb_buzz->val & MASQ_1_BUZZ) | (atoi(str_gamme));
      if(strcmp(str_cle,"None") != 0)
        mb_buzz->val = (mb_buzz->val & MASQ_0_BUZZ) | (atoi(str_cle) << 16);

        mb_buzz->state = FULL;
    }


    // Serial.println((mb_mod->val & MASQ_0_MOD));
    // Serial.println((mb_mod->val & MASQ_1_MOD) >> 8);
    // Serial.println((mb_mod->val & MASQ_2_MOD) >> 16);
    // Serial.println(mb_buzz->val & MASQ_0_BUZZ);
    // Serial.println((mb_buzz->val & MASQ_1_BUZZ) >> 16);

    Rflag=false;
  }

  // MQTT
  ctx->client.loop();
}