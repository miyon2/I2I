sudo apt-get install libmosquitto-dev
gcc -c mosq_pub_lib.c -lmosquitto
ar rc libmosqpub.a mosq_pub_lib.o
ranlib libmosqpub.a
gcc -o test_pub test_app_pub.c -L -lmosqpub -lmosquitto

gcc -c mosq_sub_lib.c -lmosquitto
ar rc libmosqsub.a mosq_sub_lib.o
ranlib libmosqsub.a
gcc -o test_sub test_app_sub.c -L -lmosqpub -lmosquitto

sudo insmod ./ioctl/mosq_pub_ioctl.ko
sudo sh ./ioctl/mknod.sh

