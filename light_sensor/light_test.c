#include "spi.h"
#include <sys/fcntl.h>

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define MOSQ_IOCTL_NUM 'z'
#define LEDON           _IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define LEDOFF  _IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM2, unsigned long *)


int main(void) {
        int dev, led_dev;
	unsigned long value;
        uint8_t snd = 0, rcv = 0;
	uint8_t thld = 60;
        dev = spiOpen(spi_device0);
	led_dev = open("/dev/led_ioctl_dev", O_RDWR);
        spi_init(dev);


        while(1){

//                rcv = read_light(dev, snd);
                rcv = read_light_mean(dev, snd, 8);
//                printf("Light value : %u\n", rcv);
		if(rcv < thld){
			ioctl(led_dev, LEDON, &value);
		} else {
			ioctl(led_dev, LEDOFF, &value);
		}
                spiDelay(10000000);
        }
        spiClose(dev);
        return 0;
}
