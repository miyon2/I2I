#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

#include <mosquitto.h>

#define mqtt_host "localhost"
#define mqtt_port 1883
#define MQTT_TOPIC "myTopic"

FILE *temp_fp;
FILE *hum_fp;

static int run = 1;
int rc = 0;
int call_count = 0;
struct mosquitto *mosq;

void handle_signal(int s){
    run = 0;
}

void connect_callback(struct mosquitto *mosq, void *obj, int result){
    printf("connect callback, rc=%d\n", result);
}

void message_callback(struct mosquitto *mosq, void *obj, const struct mosquitto_message *message){
    bool match = 0;

    printf("receive message(%s) : %s\n", message->topic, message->payload);
    if(match){
        printf("got message for ADC topic\n");
    }
    
    if(!(call_count % 2)){
        hum_fp = fopen("hum.txt", "w");
        fprintf(hum_fp, "%s", message->payload);
        fclose(hum_fp);
        call_count++;
    }
    else{
        temp_fp = fopen("temp.txt", "w");
        fprintf(temp_fp, "%s", message->payload);
        fclose(temp_fp);
        call_count++;
    }
}

int mosq_sub_init(){
    uint8_t reconnect = true;

    rc = 0;

    signal(SIGINT, handle_signal);
    signal(SIGTERM, handle_signal);

    mosquitto_lib_init();

    mosq = mosquitto_new(NULL, true, 0);

    if(mosq){
        mosquitto_connect_callback_set(mosq, connect_callback);
        mosquitto_message_callback_set(mosq, message_callback);

        rc = mosquitto_connect(mosq, mqtt_host, mqtt_port, 60);
    }

    return rc;
}

int mosq_sub(){
    if(mosq){
        mosquitto_subscribe(mosq, NULL, MQTT_TOPIC, 0);

        while(run){
            rc = mosquitto_loop(mosq, -1, 1);

            if(run && rc){
                printf("connection error!\n");
                sleep(10);
                mosquitto_reconnect(mosq);
            }
        }
    }

    return rc;
}

int mosq_del(){
    if(mosq){
        mosquitto_destroy(mosq);
    }

    mosquitto_lib_cleanup();

    return rc;
}
