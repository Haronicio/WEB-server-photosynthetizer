#include <mosquitto.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <unistd.h>


#define MAXServerResquest 1024

void on_connect(struct mosquitto *mosq, void *userdata, int rc) {
    printf("Connecté au broker MQTT\n");
    mosquitto_subscribe(mosq, NULL, "esp2pi", 0);  // s'abonne au sujet "esp2pi"
}

void on_disconnect(struct mosquitto *mosq, void *userdata, int rc)
{
    fprintf(stderr, "Disconnected from broker\n");
    mosquitto_reconnect(mosq);
}

void on_message(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *msg) {
    printf("Message reçu sur le sujet %s : %s\n", msg->topic, (char *)msg->payload);
    write(*((int *)userdata),(char *)msg->payload,(int)msg->payloadlen);
    write(*((int *)userdata),"\n",1);
}

int main() {
    struct mosquitto *mosq = NULL;
    int rc;

    mosquitto_lib_init();  // initialise la bibliothèque mosquitto
    /*Create a new mosquitto client instance.

    Parameters
    id	String to use as the client id.  If NULL, a random client id will be generated.  If id is NULL, clean_session must be true.
    clean_session	set to true to instruct the broker to clean all messages and subscriptions on disconnect, false to instruct it to keep them.  See the man page mqtt(7) for more details.  Note that a client will never discard its own outgoing messages on disconnect.  Calling mosquitto_connect or mosquitto_reconnect will cause the messages to be resent.  Use mosquitto_reinitialise to reset a client to its original state.  Must be set to true if the id parameter is NULL.
    obj	A user pointer that will be passed as an argument to any callbacks that are specified.
    
    Returns
    Pointer to a struct mosquitto on success.  NULL on failure.  Interrogate errno to determine the cause for the failure:

    ENOMEM on out of memory.
    EINVAL on invalid input parameters.
*/
    void * obj;
    mosq = mosquitto_new("pi", true, obj);  // crée un client MQTT nommé "client1"
    if (!mosq) {
        printf("Impossible de créer le client MQTT\n");
        exit(-1);
    }
    /*Set the connect callback.  This is called when the broker sends a CONNACK message in response to a connection.

    Parameters
    mosq	a valid mosquitto instance.
    on_connect	a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, int rc)
    Callback Parameters
    mosq	the mosquitto instance making the callback.
    obj	the user data provided in mosquitto_new
    rc	the return code of the connection response.  The values are defined by the MQTT protocol version in use.
    */

    mosquitto_connect_callback_set(mosq, on_connect);  // définit la fonction de callback on_connect
    
    /*
    Set the disconnect callback.  This is called when the broker has received the DISCONNECT command and has disconnected the client.

    Parameters
    mosq	a valid mosquitto instance.
    on_disconnect	a callback function in the following form: void callback(struct mosquitto *mosq, void *obj)
    Callback Parameters
    mosq	the mosquitto instance making the callback.
    obj	the user data provided in mosquitto_new
    rc	integer value indicating the reason for the disconnect.  A value of 0 means the client has called mosquitto_disconnect.  Any other value indicates that the disconnect is unexpected.
*/

    mosquitto_disconnect_callback_set(mosq, on_disconnect);

    /*Set the message callback.  This is called when a message is received from the broker.

    Parameters
    mosq	a valid mosquitto instance.
    on_message	a callback function in the following form: void callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message)
    Callback Parameters
    mosq	the mosquitto instance making the callback.
    obj	the user data provided in mosquitto_new
    message	the message data.  This variable and associated memory will be freed by the library after the callback completes.  The client should make copies of any of the data it requires.
    */

    
    mosquitto_message_callback_set(mosq, on_message);  // définit la fonction de callback on_message

    /*Connect to an MQTT broker.
    It is valid to use this function for clients using all MQTT protocol versions.  If you need to set MQTT v5 CONNECT properties, use mosquitto_connect_bind_v5 instead.

    Parameters
    mosq	a valid mosquitto instance.
    host	the hostname or ip address of the broker to connect to.
    port	the network port to connect to.  Usually 1883.
    keepalive	the number of seconds after which the broker should send a PING message to the client if no other messages have been exchanged in that time.
    
    Returns
    MOSQ_ERR_SUCCESS	on success.
    MOSQ_ERR_INVAL	if the input parameters were invalid, which could be any of:
    mosq == NULL
    host == NULL
    port < 0
    keepalive < 5
    MOSQ_ERR_ERRNO	if a system call returned an error.  
    The variable errno contains the error code, even on Windows.  
    Use strerror_r() where available or FormatMessage() on Windows.
    */
    rc = mosquitto_connect(mosq, "localhost", 1884, 60);  // se connecte au broker MQTT
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

    int     s2g, bd_w;                                       // fifo file descriptors
    char    *s2gName = "/tmp/s2g";
    char    *bdName = "bd";
    char    serverRequest[MAXServerResquest];
    fd_set  rfds;
    struct  timeval tv;                                     // timeout
    tv.tv_sec = 5;                                          // 1 second
    tv.tv_usec = 0;

    mkfifo(s2gName, 0777);

    s2g = open(s2gName,O_RDWR);
    bd_w = open(bdName,O_WRONLY);

    //passé le fd en argument
    obj = &bd_w;
    mosquitto_user_data_set(mosq,obj);

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
                fprintf(stderr,"from server : %s", serverRequest);
                /*Publish a message on a given topic.

                Parameters
                mosq	a valid mosquitto instance.
                mid	pointer to an int.  If not NULL, the function will set this to the message id of this particular message.  This can be then used with the publish callback to determine when the message has been sent.
                Note that although the MQTT protocol doesn’t use message ids for messages with QoS=0, libmosquitto assigns them message ids so they can be tracked with this parameter.
                topic	null terminated string of the topic to publish to.
                payloadlen	the size of the payload (bytes).  Valid values are between 0 and 268,435,455.
                payload	pointer to the data to send.  If payloadlen > 0 this must be a valid memory location.
                qos	integer value 0, 1 or 2 indicating the Quality of Service to be used for the message.
                retain	set to true to make the message retained.
                Returns
                MOSQ_ERR_SUCCESS	on success.
                MOSQ_ERR_INVAL	if the input parameters were invalid.
                MOSQ_ERR_NOMEM	if an out of memory condition occurred.
                MOSQ_ERR_NO_CONN	if the client isn’t connected to a broker.
                MOSQ_ERR_PROTOCOL	if there is a protocol error communicating with the broker.
                MOSQ_ERR_PAYLOAD_SIZE	if payloadlen is too large.
                MOSQ_ERR_MALFORMED_UTF8	if the topic is not valid UTF-8
                MOSQ_ERR_QOS_NOT_SUPPORTED	if the QoS is greater than that supported by the broker.
                MOSQ_ERR_OVERSIZE_PACKET	if the resulting packet would be larger than supported by the broker.*/
                
                mosquitto_publish(mosq, NULL, "pi2esp", nbchar, serverRequest, 0, false);  // publie sur le sujet "pi2esp"
            }
        }
        else
        {
            printf("Pas de commandes du serveur depuis 1 secondes\n");
            tv.tv_sec = 1;                                          // 1 second
            tv.tv_usec = 0;  
        }
    }

    mosquitto_destroy(mosq);  // détruit le client MQTT
    mosquitto_lib_cleanup();  // libère les ressources de la bibliothèque mosquitto

    return 0;
}