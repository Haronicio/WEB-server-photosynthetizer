#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <time.h>
#include <errno.h> 

int msleep(long msec)
{
    struct timespec ts;
    int res;

    if (msec < 0)
    {
        errno = EINVAL;
        return -1;
    }

    ts.tv_sec = msec / 1000;
    ts.tv_nsec = (msec % 1000) * 1000000;

    do {
        res = nanosleep(&ts, &ts);
    } while (res && errno == EINTR);

    return res;
}

void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
    printf("Connecté au broker MQTT\n");
    mosquitto_subscribe(mosq, NULL, "pi2esp", 0);  // s'abonne au sujet "pi2esp"
}

void on_disconnect(struct mosquitto *mosq, void *userdata, int rc)
{
    fprintf(stderr, "Disconnected from broker\n");
    mosquitto_reconnect(mosq);
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    printf("Message reçu sur le sujet %s : %s\n", msg->topic, (char *)msg->payload);
}

int main() {
    struct mosquitto *mosq = NULL;
    int rc;
    srand(time(NULL));
    mosquitto_lib_init();  // initialise la bibliothèque mosquitto

    mosq = mosquitto_new("esp", true, NULL);  // crée un client MQTT nommé "client1"
    if (!mosq) {
        printf("Impossible de créer le client MQTT\n");
        exit(-1);
    }

    mosquitto_connect_callback_set(mosq, on_connect);
    mosquitto_disconnect_callback_set(mosq, on_disconnect);
    mosquitto_message_callback_set(mosq, on_message);

    rc = mosquitto_connect(mosq, "localhost", 1884, 60);
    if (rc != MOSQ_ERR_SUCCESS) {
        printf("Connexion au broker MQTT impossible\n");
        mosquitto_destroy(mosq);
        exit(-1);
    }

    /* Run the network loop in a background thread, this call returns quickly. */
	rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		return 1;
	}

    while (1) {
        char message[10];
        // printf("Entrez un message : ");
        // scanf("%s", message);
        sprintf(message,"%d",rand()%100);
        mosquitto_publish(mosq, NULL, "esp2pi", strlen(message), message, 0, false);  // publie sur le sujet "pi2esp"
        msleep(500);
    }

    mosquitto_destroy(mosq);  // détruit le client MQTT
    mosquitto_lib_cleanup();  // libère les ressources de la bibliothèque mosquitto

    return 0;
}