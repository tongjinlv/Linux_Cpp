#include <linux/kernel.h>
#include <linux/platform_device.h>
#include <linux/err.h>
#include <linux/gpio.h>
#include <linux/module.h>

#include <linux/miscdevice.h>
#include <mach/hardware.h>
#include <mach/sys_config.h>

#include <mach/irqs.h>
#include <linux/interrupt.h>
#include <linux/input.h>

#include <linux/delay.h>


static struct gpio_config feedback_port_cfg;
static struct input_dev *IR_feedback_dev;
static struct work_struct work;
#define KEY_DETECTION 250

static unsigned int IRMotionDet_status = 0;

static ssize_t IRMotionDet_enable_ctl_show(struct device *dev,
		struct device_attribute *attr, char *buf)
{
	return sprintf(buf, "%u\n", IRMotionDet_status);
}

static ssize_t IRMotionDet_enable_ctl(struct device *dev,struct device_attribute *attr,
		const char *buf, size_t size)
{
    unsigned int value;
    char *after;
    int i;
    
    IRMotionDet_status = simple_strtoul(buf, &after, 10);

	printk("IRMotionDet_status change to %d\n", IRMotionDet_status);
    switch(IRMotionDet_status)
    {
		case 0: // 关闭模块
		    IRMotionDet_status = 0;
            break;
        case 1: // 打开模块
            IRMotionDet_status = 1;
            break;
        default:
            pr_err("IRMotionDet para err!");
            break;
    }

    return size;
}

static DEVICE_ATTR(IRMotionDet_ctl, S_IRUGO|S_IWUSR|S_IWGRP,
		IRMotionDet_enable_ctl_show, IRMotionDet_enable_ctl);

static struct attribute *IRMotionDet_attributes[] = {
	&dev_attr_IRMotionDet_ctl.attr,
	NULL
};

static struct attribute_group IRMotionDet_attribute_group = {
	.name = "IRMotionDet",
	.attrs = IRMotionDet_attributes
};

static struct miscdevice IRMotionDet_dev = {
	.minor =	MISC_DYNAMIC_MINOR,
	.name =		"IRMotionDet",
};

static void __worker(struct work_struct *work)
{
	int val = 0;
	
	val = __gpio_get_value(feedback_port_cfg.gpio);
	if(IRMotionDet_status && val)
	{
		input_report_key(IR_feedback_dev, KEY_DETECTION, 0);
		input_sync(IR_feedback_dev);
		mdelay(10);
		input_report_key(IR_feedback_dev, KEY_DETECTION, 1);
		input_sync(IR_feedback_dev);
		mdelay(10);
	}
}

static int feedback_gpio_init()
{
	int err;
	script_item_u val;
	script_item_value_type_e type;

	IR_feedback_dev = input_allocate_device();
	if(!IR_feedback_dev)
	{
		printk("[IRMotionDet] input allocate device failed.\n");
		err = -ENOMEM;
		return err;
	}
	
	IR_feedback_dev->name		= "IR_feedback";
	IR_feedback_dev->phys		= "IRMotionDet/input0";
	IR_feedback_dev->id.bustype = BUS_HOST;
	IR_feedback_dev->id.vendor	= 0x0001;
	IR_feedback_dev->id.product = 0x0001;
	IR_feedback_dev->id.version = 0x0100;
	
	IR_feedback_dev->evbit[0] = BIT_MASK(EV_KEY);
	set_bit(KEY_DETECTION, IR_feedback_dev->keybit);
	
	type = script_get_item("MATA_GPIO", "port_0", &val);
	if (SCIRPT_ITEM_VALUE_TYPE_PIO != type) {
		printk("script_get_item MATA_GPIO port err\n");
		input_free_device(IR_feedback_dev);
		return -1;
	}
	feedback_port_cfg = val.gpio;
	printk("ctp_irq gpio number is %d\n", feedback_port_cfg.gpio);
	if(0 != gpio_request(feedback_port_cfg.gpio, NULL))
	{
		printk("gpio feedback_port request failed.\n");
		return -1;
	}
	printk("gpio gpio_direction_inputc.\n");
	gpio_direction_input(feedback_port_cfg.gpio);
	printk("gpio gpio_direction_output.\n");
	gpio_direction_output(feedback_port_cfg.gpio,0);
	printk("gpio gpio_set_value.\n");
	gpio_set_value(feedback_port_cfg.gpio,0);
	err = input_register_device(IR_feedback_dev);
	if(err)
	{
		printk("[IRMotionDet] input register device failed.\n");
		gpio_free(feedback_port_cfg.gpio);
		return -1;
	}
	return 0;

}

static int feedback_gpio_uninit()
{
	input_free_device(IR_feedback_dev);
	gpio_free(feedback_port_cfg.gpio);
	
	return 0;
}

static int __init IRMotionDet_init(void)
{
	int err,i, irq_number;
	script_item_u   val;
	script_item_value_type_e  type;


	err = misc_register(&IRMotionDet_dev);
	if(err) {
		printk("%s driver as misc device error\n", __FUNCTION__);
		return -1;
	}

	err=sysfs_create_group(&IRMotionDet_dev.this_device->kobj,
						 &IRMotionDet_attribute_group);
	if(err){
	    printk("%s sysfs_create_group  error\n", __FUNCTION__);
	}
    
	feedback_gpio_init();

	return 0;
}

static void __exit IRMotionDet_exit(void) {

	sysfs_remove_group(&IRMotionDet_dev.this_device->kobj,
						 &IRMotionDet_attribute_group);
	misc_deregister(&IRMotionDet_dev);
	feedback_gpio_uninit();
}

module_init(IRMotionDet_init);
module_exit(IRMotionDet_exit);

MODULE_DESCRIPTION("a simple IR Motion Detect driver");
MODULE_AUTHOR("sc");
MODULE_LICENSE("GPL");
