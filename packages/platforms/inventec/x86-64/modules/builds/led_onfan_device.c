/*
 * led_onfan_device.c
 * Aug 5, 2016 Inital
 * Jan 26, 2017 To support Redwood
 *
 * Inventec Platform Proxy sysfs for led device.
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/inventec/common/inventec_sysfs.h>

extern ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p);

static ssize_t
show_attr_location(struct device *dev_p,
                   struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_sys_color(struct device *dev_p,
                   struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_sys_freq(struct device *dev_p,
                   struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static DEVICE_ATTR(location,	S_IRUGO,	show_attr_location,	NULL);
static DEVICE_ATTR(sys_color,	S_IRUGO,	show_attr_sys_color,	NULL);
static DEVICE_ATTR(sys_freq,	S_IRUGO,	show_attr_sys_freq,	NULL);

int
led_onfan_register_attrs(struct device *device_p)
{
	char *err_msg = "ERR";

	if (device_create_file(device_p, &dev_attr_location) < 0) {
		err_msg = "reg_attr bias_current fail";
		goto err_led_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_sys_color) < 0) {
		err_msg = "reg_attr connector_type fail";
		goto err_led_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_sys_freq) < 0) {
		err_msg = "reg_attr connector_type fail";
		goto err_led_register_attrs;
	}
	return 0;

err_led_register_attrs:
	printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
	return -1;
}

void
led_onfan_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_location);
	device_remove_file(device_p, &dev_attr_sys_color);
	device_remove_file(device_p, &dev_attr_sys_freq);
}
