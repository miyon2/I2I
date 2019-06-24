#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <asm/uaccess.h>

#include <linux/gpio.h>
#include <linux/delay.h>

#include <linux/fs.h>
#include <linux/cdev.h>

#define MAX_TIMING 85
#define DHT11 21

MODULE_LICENSE("GPL");

static int hum_data[4] = {34, 0, 26, 0 };		// readable humidity & temporature data Hum:[0].[1], Temp:[2]:[3]
static int dht11_data[5] = {0, };

#define DEV_NAME "mosq_ioctl_dev"

#define IOCTL_START_NUM 0x80
#define IOCTL_NUM1 IOCTL_START_NUM+1
#define IOCTL_NUM2 IOCTL_START_NUM+2
#define IOCTL_NUM3 IOCTL_START_NUM+3
#define IOCTL_NUM4 IOCTL_START_NUM+4

#define MOSQ_IOCTL_NUM 'z'
#define GET_DUST 		_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM1, unsigned long *)
#define GET_HUMIDITY 		_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM2, unsigned long *)
#define GET_LIGHT 		_IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM3, unsigned long *)
#define REFRESH_DATA            _IOWR(MOSQ_IOCTL_NUM, IOCTL_NUM4, unsigned long *)

static void dht11_read(void) {
	int last_state = 1;
	int counter = 0;
	int i = 0, j = 0;

	dht11_data[0] = dht11_data[1] = dht11_data[2] = dht11_data[3] = dht11_data[4] = 0;

	gpio_direction_output(DHT11, 0);
	gpio_set_value(DHT11, 0);
	mdelay(18);
	gpio_set_value(DHT11, 1);
	udelay(40);
	gpio_direction_input(DHT11);


	for(i = 0; i < MAX_TIMING; i++) {
		counter = 0;
		while(gpio_get_value(DHT11) == last_state) {
			counter++;
			udelay(1);
			if(counter == 255) {
				break;
			}
		}
		last_state = gpio_get_value(DHT11);

		if(counter == 255) {
			break;
		}


		if((i >= 4) && (i % 2 == 0)) {
			dht11_data[j/8] <<= 1;
			if(counter > 30) {
				dht11_data[j / 8] |= 1;
			}
			j++;
		}
	}

		if((j >= 40) &&
		(dht11_data[4] == ( (dht11_data[0] + dht11_data[1] + dht11_data[2] + dht11_data[3]) & 0xFF) ) ) {
			printk("Humidity: %d.%d Temperature = %d.%d C\n", dht11_data[0], dht11_data[1], dht11_data[2], dht11_data[3]);
			hum_data[0] = dht11_data[0];
			hum_data[1] = dht11_data[1];
			hum_data[2] = dht11_data[2];
			hum_data[3] = dht11_data[3];
		} else {
//			printk("Data not good, skip\n");
	}
}

struct dht{
	int dht1;
	int dht2;
	int dht3;
	int dht4;
}dht;

int *dust_data;
int *light_data;

static long mosq_ioctl(struct file *file, unsigned int cmd, unsigned long arg){
	int my_ret = 10001;

	*dust_data = my_ret;
	*light_data = my_ret;
	struct dht *my_dht = kmalloc(sizeof(struct dht), GFP_KERNEL);

	// copy_from_user(test_ret, (int *)buf, sizeof(int));
	switch( cmd ){
		case GET_DUST:
			printk("get dust\n");
			copy_to_user((int *)arg, dust_data, sizeof(int));
			break;
		case GET_HUMIDITY:
			printk("get humidity\n");
//			dht11_read();
			my_dht->dht1 = hum_data[0];
			my_dht->dht2 = hum_data[1];
			my_dht->dht3 = hum_data[2];
			my_dht->dht4 = hum_data[3];
			copy_to_user((struct dht*)arg, my_dht, sizeof(struct dht));
			break;
		case GET_LIGHT:
			printk("get light\n");
			copy_to_user((int *)arg, light_data, sizeof(int));
			break;
		case REFRESH_DATA:
                        dht11_read();
                        break;
		default:
			return -1;
	}

	return 0;
}

static int mosq_ioctl_open(struct inode *inode, struct file *file){
	printk("mosq_ioctl open\n");
	return 0;
}

static int mosq_ioctl_release(struct inode *inode, struct file *file){
	printk("mosq_ioctl release\n");
	return 0;
}

struct file_operations mosq_char_fops = {
	.unlocked_ioctl = mosq_ioctl,
	.open = mosq_ioctl_open,
	.release = mosq_ioctl_release,
};

static dev_t dev_num;
static struct cdev *cd_cdev;

static int __init mosq_ioctl_init(void){
	printk("Init Module\n");
	
	gpio_request(DHT11, "DHT11");

	dust_data = kmalloc(sizeof(int) * 4, GFP_KERNEL);
	// *hum_data = kmalloc(sizeof(int *)* 4, GFP_KERNEL);
	// hum_data = kmalloc(sizeof(int), GFP_KERNEL);
	light_data = kmalloc(sizeof(int), GFP_KERNEL);

	alloc_chrdev_region(&dev_num, 0, 1, DEV_NAME);
	cd_cdev = cdev_alloc();
	cdev_init(cd_cdev, &mosq_char_fops);
	cdev_add(cd_cdev, dev_num, 1);

	return 0;
}

static void __exit mosq_ioctl_exit(void){
	printk("Exit Module\n");
	gpio_set_value(DHT11, 0);
        gpio_free(DHT11);
	cdev_del(cd_cdev);
	unregister_chrdev_region(dev_num, 1);
}

module_init(mosq_ioctl_init);
module_exit(mosq_ioctl_exit);
