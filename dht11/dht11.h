#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/gpio.h>
#include <linux/delay.h>

// MODULE_LICENSE("GPL");

#define MAX_TIMING 85
#define DHT11 21

static int hum_data[4] = {0, };		// readable humidity & temporature data Hum:[0].[1], Temp:[2]:[3]
static int dht11_data[5] = {0, };
static void dht11_read(void);

