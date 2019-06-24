obj-m := mosq_pub_ioctl.o

KDIR := ~/linux-rpi
PWD := $(shell pwd)
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
clean:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) CLEAN
