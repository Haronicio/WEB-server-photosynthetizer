#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>

#include "jsonparser.h"

int msgcount = 0;


#define MAXServerResquest 1024

void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
    printf("(GATEWAY) Connecté au broker MQTT\n");
    mosquitto_subscribe(mosq, NULL, "esp2pi", 0);  // s'abonne au sujet "esp2pi"
}

void on_disconnect(struct mosquitto *mosq, void *userdata, int rc)
{
    fprintf(stderr, "(GATEWAY) Disconnected from broker\n");
    mosquitto_reconnect(mosq);
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    cJSON * json;
    char xVal[1024];
    msgcount++;

    // printf("(GATEWAY) Message reçu sur le sujet %s : %s\n", msg->topic, (char *)msg->payload);

    // write(*((int *)userdata),(char *)msg->payload,(int)msg->payloadlen);
    // write(*((int *)userdata),"\n",1);

    sprintf(xVal,"%d",msgcount);
    if(!(json = open_JSON_file("../server/www/bd/bd.json")))
    {
        printf("(GATEWAY) Error in open\n");
        return;
    }
    add_xydata_to_JSON(json,xVal,atoi((char *)msg->payload));
    if(!save_JSON_file(json,"../server/www/bd/bd.json"))
    {
        printf("(GATEWAY) Error in save\n");
        return;
    }
    cJSON_Delete(json);
}

int main() {
    struct mosquitto *mosq = NULL;
    int rc;

    mosquitto_lib_init();  // initialise la bibliothèque mosquitto
    void * obj;
    mosq = mosquitto_new("pi", true, obj);  // crée un client MQTT nommé "client1"
    if (!mosq) {
        printf("(GATEWAY) Impossible de créer le client MQTT\n");
        exit(-1);
    }
    mosquitto_connect_callback_set(mosq, on_connect);  // définit la fonction de callback on_connect

    mosquitto_disconnect_callback_set(mosq, on_disconnect);

    
    mosquitto_message_callback_set(mosq, on_message);  // définit la fonction de callback on_message
    rc = mosquitto_connect(mosq, "localhost", 1884, 60);  // se connecte au broker MQTT
    if (rc != MOSQ_ERR_SUCCESS) {
        printf("(GATEWAY) Connexion au broker MQTT impossible\n");
        mosquitto_destroy(mosq);
        exit(-1);
    }

    /* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "(GATEWAY) Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

    int     s2g, bd_w;                                       // fifo file descriptors
    char    *s2gName = "/tmp/s2g";
    char    *bdName = "bd";
    char    serverRequest[MAXServerResquest];
    fd_set  rfds;
    struct  timeval tv;                                     // timeout
    tv.tv_sec = 5;                                          // 1 second
    tv.tv_usec = 0;

    if(!mkfifo(s2gName, 0777))
    {
        printf("(GATEWAY) WARNING Errors in FIFO\n");
    }

    s2g = open(s2gName,O_RDWR);
    if(!s2g)
    {
        printf("(GATEWAY) Error opening FIFO's fd\n");
        return -1;
    }

    while (1) {

        FD_ZERO(&rfds);
        FD_SET(s2g,&rfds);

        //on surveille le pipe et on publie 
        if(select(s2g+1,&rfds,NULL,NULL,&tv) != 0)
        {
            if(FD_ISSET(s2g,&rfds))
            {
                int nbchar;
                if ((nbchar = read(s2g, serverRequest, MAXServerResquest)) == 0) break;
                serverRequest[nbchar]='\0';
                fprintf(stderr,"(GATEWAY) from server : %s", serverRequest);
                mosquitto_publish(mosq, NULL, "pi2esp", nbchar, serverRequest, 0, false);
            }
        }
        else
        {
            printf("(GATEWAY) Pas de commandes du serveur depuis 10 secondes\n");
            tv.tv_sec = 10;                                          // 1 second
            tv.tv_usec = 0;  
        }
    }

    mosquitto_destroy(mosq);  // détruit le client MQTT
    mosquitto_lib_cleanup();  // libère les ressources de la bibliothèque mosquitto

    return 0;
}