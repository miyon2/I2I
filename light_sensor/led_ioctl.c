#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/fs.h>
#include <linux/cdev.h>


MODULE_LICENSE("GPL");

#define LED1 4
#define DEV_NAME "led_ioctl_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2

#define MOSQ_IOCTL_NUM 'z'
#define LEDON 		_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define LEDOFF 	_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM2, unsigned long *)

static long led_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	switch( cmd ){
		case LEDON:
        		gpio_set_value(LED1, 1);
			break;
		case LEDOFF:
			gpio_set_value(LED1, 0);
			break;
		default:
			return -1;
	}

	return 0;
}

static int led_ioctl_open(struct inode *inode, struct file *file){
	
	return 0;
}

static int led_ioctl_release(struct inode *inode, struct file *file){
	
	return 0;
}

struct file_operations led_char_fops = {
	.unlocked_ioctl = led_ioctl,
	.open = led_ioctl_open,
	.release = led_ioctl_release,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init led_ioctl_init(void){
	gpio_request_one(LED1, GPIOF_OUT_INIT_LOW, "LED1");
	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &led_char_fops);
	cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit led_ioctl_exit(void){
	gpio_set_value(LED1, 0 );
	gpio_free(LED1);
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(led_ioctl_init);
module_exit(led_ioctl_exit);
