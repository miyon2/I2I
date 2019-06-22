#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/gpio.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/time.h>
#include <linux/kthread.h>


MODULE_LICENSE("GPL");

#define PIN1 6
#define PIN2 13
#define PIN3 19
#define PIN4 26

#define TRIG 23
#define ECHO 24

#define HXSK 17
#define HXDO 27

#define STEPS 8
#define W_THRESHOLD 1300

int step[STEPS][4]= 
{{1,0,0,0}
,{1,1,0,0}
,{0,1,0,0}
,{0,1,1,0}
,{0,0,1,0}
,{0,0,1,1}
,{0,0,0,1}
,{1,0,0,1}};


static struct timer_list my_timer;
static long height=15;
static long w_offset = 0;
static char w_flag =0;
struct task_struct *motor_thread;
struct tasklet_struct usonic_tasklet;


static void setStep(int p1, int p2, int p3, int p4)
{
	gpio_set_value(PIN1,p1);
	gpio_set_value(PIN2,p2);
	gpio_set_value(PIN3,p3);
	gpio_set_value(PIN4,p4);
}
static void moveDegree(int degree, int delay, int direction)
{
	int i=0;
	int j=0;
	int count = (degree<<2) + (degree<<3) ;
	for(i=0;i<count;i++)
	{
		if(direction)
			j=7-i%8;
		else
			j=i%8;
		setStep(step[j][0],step[j][1],step[j][2],step[j][3]);
		udelay(delay);
	}
}

static int motor_thread_func(void *data)
{
	long my_h=15;
	while(!kthread_should_stop())
	{
		if(my_h>height)
		{
			moveDegree((my_h-height)<<2,900,1);
			my_h=height;
		}
		else if(my_h<height)
		{
			moveDegree((height-my_h)<<2,900,0);
			my_h=height;
		}
		msleep(100);

	}
	return 0;
}
unsigned long  ReadHeight(void)
{
	struct timeval start_time, end_time;
	unsigned long distance, flags;

	local_irq_save(flags);

	gpio_set_value(TRIG,1);
	udelay(10);
	gpio_set_value(TRIG,0);

	while(gpio_get_value(ECHO) == 0){};
	do_gettimeofday(&start_time);
	while(gpio_get_value(ECHO)==1){};
	do_gettimeofday(&end_time);

	local_irq_restore(flags);
	
	distance = (end_time.tv_usec - start_time.tv_usec)/58;

	return distance;
}

void usonic_tasklet_func(unsigned long data)
{
	unsigned long tmp;

	tmp=ReadHeight();

	if( tmp> 15)
		tmp = 15;
	height = tmp;
}
unsigned long ReadCount(void)
{
	long count;
	unsigned long flags;
	unsigned char i;
	count =0;
	local_irq_save(flags);
	gpio_set_value(HXSK,0);
	while(gpio_get_value(HXDO));
	for(i=0;i<24;i++)
	{
		gpio_set_value(HXSK,1);
		count= count<<1;
		gpio_set_value(HXSK,0);
		if(gpio_get_value(HXDO))
			count++;
	}
	gpio_set_value(HXSK,1);
	count = count^0x800000;
	gpio_set_value(HXSK,0);

	local_irq_restore(flags);
	count = count>>2;
	count = count - w_offset;
	return count;
}

static void check_weight_timer(unsigned long data)
{
	long count;
	count = ReadCount();
	if(abs(count)>W_THRESHOLD)
	{
		w_flag = w_flag<<1;
		w_flag = w_flag|1;
	}
	else
	{
		w_flag = w_flag<<1;
	}
	w_flag = w_flag&0b11;
	if( ( w_flag == 0b01 ) )
		tasklet_schedule(&usonic_tasklet);
	my_timer.expires =  jiffies +(HZ/8);
	add_timer(&my_timer);
}
static int __init inpi_init(void)
{
	int i;
	long mean_count=0;
	gpio_request_one(PIN1, GPIOF_OUT_INIT_LOW, "p1");
	gpio_request_one(PIN2, GPIOF_OUT_INIT_LOW, "p2");
	gpio_request_one(PIN3, GPIOF_OUT_INIT_LOW, "p3");
	gpio_request_one(PIN4, GPIOF_OUT_INIT_LOW, "p4");
	gpio_request_one(TRIG, GPIOF_OUT_INIT_LOW,"trig");
	gpio_request_one(ECHO, GPIOF_IN,"echo");
	gpio_request_one(HXDO, GPIOF_IN,"hxdo");
	gpio_request_one(HXSK, GPIOF_OUT_INIT_LOW, "hxsk");

	tasklet_init(&usonic_tasklet, usonic_tasklet_func, 0);

	motor_thread = kthread_create(motor_thread_func, NULL, "motor_thead");
	if(IS_ERR(motor_thread))
	{
		printk("fail to motor thread create \n");
		motor_thread = NULL;
	}
	for(i=0; i<8; i++)
	{
		mean_count = mean_count +ReadCount();
	}
	w_offset = mean_count/8;

	init_timer(&my_timer);
	my_timer.function = check_weight_timer;
	my_timer.expires = jiffies + (HZ/8);
	add_timer(&my_timer);
	wake_up_process(motor_thread);
	return 0;
}
static void __exit inpi_exit(void)
{
	
	tasklet_kill(&usonic_tasklet);

	if(motor_thread)
		kthread_stop(motor_thread);
	del_timer(&my_timer);
	gpio_free(PIN1);
	gpio_free(PIN2);
	gpio_free(PIN3);
	gpio_free(PIN4);
	gpio_free(TRIG);
	gpio_free(ECHO);
	gpio_free(HXSK);
	gpio_free(HXDO);

}

module_init(inpi_init);
module_exit(inpi_exit);
