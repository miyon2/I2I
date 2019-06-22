obj-m := led_ioctl.o

KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
	arm-linux-gnueabi-gcc -o light light_test.c spi.c
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) CLEAN
copy:
	scp light led_ioctl.ko led_mknod.sh pi@10.10.10.13:~/embso/team_project/
