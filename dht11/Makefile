KDIR := ~/linux-rpi
ARM := ARCH=arm CROSS_COMPILE=arm-linux-gnueabi-
PWD := $(shell pwd)

obj-m := dht11.o

default:
	$(MAKE) -C $(KDIR) M=$(PWD) $(ARM) modules
copy:
	scp dht11.ko pi@10.10.10.13:~/embso/dht11/
