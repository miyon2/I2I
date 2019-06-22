#include "mosq_pub_lib.c"

int main(void){
    mosq_pub_init();
    while(1){
    	mosq_pub_hum();
    	mosq_pub_dust();
    	mosq_pub_light();
	sleep(3);
    }
    mosq_pub_disconn();

    return 0;
}
