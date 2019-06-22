#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <sys/fcntl.h>
#include <sys/ioctl.h>

#include <unistd.h>
#include <mosquitto.h>

#define DEV_NAME "mosq_ioctl_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3

#define MOSQ_IOCTL_NUM 'z'
#define GET_DUST 		_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define GET_HUMIDITY 	_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define GET_LIGHT 		_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM3, unsigned long *)

#define MQTT_HOSTNAME "localhost"
#define MQTT_PORT 1883
#define MQTT_USERNAME "admin"
#define MQTT_PASSWORD "admin"
#define MQTT_TOPIC "myTopic"

typedef struct dht{
    int dht1;
    int dht2;
    int dht3;
    int dht4;
}dht;

int ret;
struct mosquitto *mosq;
char text[20];
int dev;

FILE *hum_fp;
FILE *temp_fp;

int mosq_pub_init(){
    dev = open("/dev/mosq_ioctl_dev", O_RDWR);

    mosq = NULL;
    mosquitto_lib_init();
    mosq = mosquitto_new(NULL, true, NULL);

    if(!mosq){
        printf("Cant initialize mosquitto library\n");
	    printf("1\n");
        exit(-1);
    }

    mosquitto_username_pw_set(mosq, MQTT_USERNAME, MQTT_PASSWORD);

    ret = mosquitto_connect(mosq, MQTT_HOSTNAME, MQTT_PORT, 0);

    if(ret){
        printf("Cant connect to mosquitto server\n");
	    printf("2\n");
        exit(-1);
    }
}

int mosq_pub_temp(){
    dht *my_dht = (dht *)malloc(sizeof(struct dht));

    ioctl(dev, GET_HUMIDITY, my_dht);
    if(((my_dht->dht1)!=0)&&((my_dht->dht2)!=0)&&((my_dht->dht3)!=0)&&((my_dht->dht4)!=0)){
        temp_fp = fopen("temp.txt", "w");
        printf("[hum]my_dht is : %d\n", my_dht->dht1);
        printf("[hum]my_dht is : %d\n", my_dht->dht2);
        printf("[hum]my_dht is : %d\n", my_dht->dht3);
        printf("[hum]my_dht is : %d\n", my_dht->dht4);

        sprintf(text, "%d.%d",my_dht->dht1, my_dht->dht2);
        fprintf(temp_fp, "%s", text);

        ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(text), text, 0, false);

        if(ret){
            printf("Cant connect to mosquitto server\n");
            printf("3\n");
            exit(-1);
        }

        sleep(1);
        fclose(temp_fp);

        return 0;
    }

    return 0;
}

int mosq_pub_hum(){
    dht *my_dht = (dht *)malloc(sizeof(struct dht));
    
    ioctl(dev, GET_HUMIDITY, my_dht);

    if(((my_dht->dht1)!=0)&&((my_dht->dht2)!=0)&&((my_dht->dht3)!=0)&&((my_dht->dht4)!=0)){
        hum_fp = fopen("hum.txt", "w");
        printf("[hum]my_dht is : %d\n", my_dht->dht1);
        printf("[hum]my_dht is : %d\n", my_dht->dht2);
        printf("[hum]my_dht is : %d\n", my_dht->dht3);
        printf("[hum]my_dht is : %d\n", my_dht->dht4);

        sprintf(text, "%d.%d", my_dht->dht3, my_dht->dht4);
        fprintf(hum_fp, "%s", text);

        ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(text), text, 0, false);

        if(ret){
            printf("Cant connect to mosquitto server\n");
            printf("3\n");
            exit(-1);
        }

        sleep(1);
        fclose(hum_fp);

        return 0;
    }
    
    return 0;
}

// int mosq_pub_dust(){
//     int *ret_ioctl = (int *)malloc(sizeof(int));
//     float my_dust;

//     ioctl(dev, GET_DUST, ret_ioctl);
//     printf("ret_ioctl is : %d\n", *ret_ioctl);
//     my_dust = (*ret_ioctl) / 100;

//     sprintf(text, "%f", my_dust);

//     ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(text), text, 0, false);

//     if(ret){
//         printf("Cant connect to mosquitto server\n");
// 	    printf("3\n");
//         exit(-1);
//     }

//     sleep(1);

//     return 0;
// }

// int mosq_pub_light(){
//     int *ret_ioctl = (int *)malloc(sizeof(int));
//     float my_light;

//     ioctl(dev, GET_LIGHT, ret_ioctl);
//     printf("ret_ioctl is : %d\n", *ret_ioctl);
//     my_light = (*ret_ioctl) / 100;

//     // text = "Nice to meet u\n";
//     sprintf(text, "%f", my_light);

//     ret = mosquitto_publish(mosq, NULL, MQTT_TOPIC, strlen(text), text, 0, false);

//     if(ret){
//         printf("Cant connect to mosquitto server\n");
// 	    printf("3\n");
//         exit(-1);
//     }

//     sleep(1);

//     return 0;
// }

int mosq_pub_disconn(){
    mosquitto_disconnect(mosq);
    mosquitto_destroy(mosq);
    mosquitto_lib_cleanup();
    close(dev);
}
