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
#include <linux/module.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/io.h>

static struct gpio_config led_port_cfg[10];
int led_open(struct inode *pstInode, struct file *pstFile)
{
    printk("led_open\r\n");
    return 0;
}

long led_ioctl(struct file *pstFile, unsigned int uiCmd, unsigned long ulArgc)
{
    printk("led_ioctl\r\n");
    return -1;
}

static ssize_t led_read(struct file *filp, char __user *buf, size_t size, loff_t *offset) 
{
    printk("read%d\r\n",buf[0]);
    gpio_set_value(led_port_cfg[0].gpio,0);
    return 11;
}
static ssize_t led_write(struct file *filp, const char __user *buf, size_t size, loff_t *offset) 
{ 
    printk("write[%d]=%d\r\n",buf[0],buf[1]); 
    gpio_set_value(led_port_cfg[buf[0]].gpio,buf[1]);
    return 12; 
} 
static struct file_operations led_ops =
{
    .owner = THIS_MODULE,
    .open = led_open,
    .read =led_read,
    .write=led_write,
    .unlocked_ioctl=led_ioctl,
};
static struct miscdevice led_dev = {
    .minor            = MISC_DYNAMIC_MINOR,
    .name            ="myled",
    .fops            = &led_ops,
};
static int __init led_init(void)
{
    script_item_u val;
    script_item_value_type_e type;
    printk("led_init\r\n");
    type = script_get_item("MATA_GPIO", "port_0", &val);
    if(SCIRPT_ITEM_VALUE_TYPE_PIO != type) 
    {
        printk("script_get_item MATA_GPIO port err\n");
        return -1; 
    }
    led_port_cfg[0] = val.gpio;
    gpio_direction_output(led_port_cfg[0].gpio,0);
    gpio_set_value(led_port_cfg[0].gpio,0);
    misc_register(&led_dev);
    return 0;
}

static void __exit led_exit(void)
{
    printk("led_exit\r\n");
    misc_deregister(&led_dev);//注销设备
    return;
}


module_init(led_init);
module_exit(led_exit);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("sanby");
