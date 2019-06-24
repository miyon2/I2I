#include "mosq_pub_lib.c"

int main(void){
    int i;
    mosq_pub_init();
    while(1){
	for (i=0;i<30;i++)
		ioctl_sensor_refresh();
    	mosq_pub_hum();
        mosq_pub_temp();
//	sleep(1);
    }
    mosq_pub_disconn();

    return 0;
}
