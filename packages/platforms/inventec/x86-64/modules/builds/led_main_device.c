/*
 * led_device.c
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


/* led_onfan_device.c */

ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p);

static ssize_t
show_attr_location1(struct device *dev_p,
                   struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_sys_color1(struct device *dev_p,
                   struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_sys_freq1(struct device *dev_p,
                   struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static DEVICE_ATTR(location1,    S_IRUGO,        show_attr_location1,     NULL);
static DEVICE_ATTR(sys_color1,   S_IRUGO,        show_attr_sys_color1,    NULL);
static DEVICE_ATTR(sys_freq1,    S_IRUGO,        show_attr_sys_freq1,     NULL);

int
led_onfan_register_attrs(struct device *device_p)
{
        char *err_msg = "ERR";

        if (device_create_file(device_p, &dev_attr_location1) < 0) {
                err_msg = "reg_attr bias_current fail";
                goto err_led_register_attrs;
        }
        if (device_create_file(device_p, &dev_attr_sys_color1) < 0) {
                err_msg = "reg_attr connector_type fail";
                goto err_led_register_attrs;
        }
        if (device_create_file(device_p, &dev_attr_sys_freq1) < 0) {
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
        device_remove_file(device_p, &dev_attr_location1);
        device_remove_file(device_p, &dev_attr_sys_color1);
        device_remove_file(device_p, &dev_attr_sys_freq1);
}


/* inventec_sysfs.c */
static struct class *inventec_class_p = NULL;

/* led_device.c */
static struct device *led_device_p = NULL;
static dev_t led_device_devt = 0;

struct device *leds_get_led_devp(void)
{
	return led_device_p;
}
EXPORT_SYMBOL(leds_get_led_devp);

typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} led_dev_t;

typedef struct {
	const char	*led_name;
	int		led_major;
        dev_t		led_devt;
	struct device	*led_dev_p;
	led_dev_t	*led_dev_namep;
	int		led_dev_total;
	char		*led_inv_pathp;
	void		*led_tracking;
	char		*led_location;
	char		*led_sys_color_grn;
	char		*led_sys_color_red;
	char		*led_sys_freq;
} led_dev_group_t;

#define LED_HEALTH_STATUS       "health_status"
#define LED_HEALTH_STATUS_LEN   (13)
#define LED_FAN_LED_GRN1        "fan_led_grn1"
#define LED_FAN_LED_GRN2        "fan_led_grn2"
#define LED_FAN_LED_GRN3        "fan_led_grn3"
#define LED_FAN_LED_GRN4        "fan_led_grn4"
#define LED_FAN_LED_RED1        "fan_led_red1"
#define LED_FAN_LED_RED2        "fan_led_red2"
#define LED_FAN_LED_RED3        "fan_led_red3"
#define LED_FAN_LED_RED4        "fan_led_red4"
#define LED_FAN_LED_LEN         (12)

static led_dev_group_t led_dev_group[] = {
	{
	  .led_name = LED_HEALTH_STATUS,
	  .led_location = "Front",
	  .led_sys_color_grn = STATUS_LED_GRN_PATH,
	  .led_sys_color_red = STATUS_LED_RED_PATH,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_RED1,
	  .led_location = "Rear",
	  .led_sys_color_grn = NULL,
	  .led_sys_color_red = FAN_LED_RED1_PATH,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_RED2,
	  .led_location = "Rear",
	  .led_sys_color_grn = NULL,
	  .led_sys_color_red = FAN_LED_RED2_PATH,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_RED3,
	  .led_location = "Rear",
	  .led_sys_color_grn = NULL,
	  .led_sys_color_red = FAN_LED_RED3_PATH,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_RED4,
	  .led_location = "Rear",
	  .led_sys_color_grn = NULL,
	  .led_sys_color_red = FAN_LED_RED4_PATH,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_GRN1,
	  .led_location = "Rear",
	  .led_sys_color_grn = FAN_LED_GRN1_PATH,
	  .led_sys_color_red = NULL,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_GRN2,
	  .led_location = "Rear",
	  .led_sys_color_grn = FAN_LED_GRN2_PATH,
	  .led_sys_color_red = NULL,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_GRN3,
	  .led_location = "Rear",
	  .led_sys_color_grn = FAN_LED_GRN3_PATH,
	  .led_sys_color_red = NULL,
	  .led_sys_freq = NULL,
	},
	{
	  .led_name = LED_FAN_LED_GRN4,
	  .led_location = "Rear",
	  .led_sys_color_grn = FAN_LED_GRN4_PATH,
	  .led_sys_color_red = NULL,
	  .led_sys_freq = NULL,
	},
};
#define LED_DEV_TOTAL	( sizeof(led_dev_group)/ sizeof(const led_dev_group_t) )

static char *led_get_invpath(const char *attr_namep, const int devmajor, led_dev_group_t **devgrp)
{
    led_dev_t *devnamep;
    int i, led_id = -1;

    for (i = 0; i < LED_DEV_TOTAL; i++) {
        if (devmajor == led_dev_group[i].led_major) {
	    led_id = i;
            break;
	}
    }
    if (led_id == -1) {
        return NULL;
    }

    *devgrp = &led_dev_group[led_id];
    devnamep = led_dev_group[led_id].led_dev_namep;
    for (i = 0; i < led_dev_group[led_id].led_dev_total; i++, devnamep++) {
	if (strcmp(devnamep->proxy_dev_name, attr_namep) == 0) {
            return devnamep->inv_dev_name;
	}
    }
    return NULL;
}

/* return 0/off 1/green 2/red */
int
status_led_check_color(void)
{
    char tmpbuf[32];
    int ret = STATUS_LED_INVALID;

    if (inventec_show_attr(&tmpbuf[0], STATUS_LED_GRN_PATH) > 0) {
	if (tmpbuf[0] == '0') {
	    ret = STATUS_LED_GRN0;
	}
	if (tmpbuf[0] == '1') {
	    ret = STATUS_LED_GRN1;
	}
	if (tmpbuf[0] == '2') {
	    ret = STATUS_LED_GRN2;
	}
	if (tmpbuf[0] == '3') {
	    ret = STATUS_LED_GRN3;
	}
	if (tmpbuf[0] == '7') {
	    ret = STATUS_LED_GRN7;
	}
        return ret;
    }

    if (inventec_show_attr(&tmpbuf[0], STATUS_LED_RED_PATH) > 0) {
	if (tmpbuf[0] == '0') {
	    ret = STATUS_LED_RED0;
	}
	if (tmpbuf[0] == '1') {
	    ret = STATUS_LED_RED1;
	}
	if (tmpbuf[0] == '2') {
	    ret = STATUS_LED_RED2;
	}
	if (tmpbuf[0] == '3') {
	    ret = STATUS_LED_RED3;
	}
	if (tmpbuf[0] == '7') {
	    ret = STATUS_LED_RED7;
	}
	return ret;
    }
    return ret;
}
EXPORT_SYMBOL(status_led_check_color);

/* INV drivers mapping */
ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    char *invpathp = NULL;
    led_dev_group_t *devgroup = NULL;
    char tmpbuf[32];

    invpathp = led_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt), &devgroup);
    if (devgroup) {
	if (strncmp(attr_p->attr.name, "location", 8) == 0) {
	    return sprintf(buf_p, "%s\n", devgroup->led_location);
	}
	else
	if (strncmp(attr_p->attr.name, "sys_color", 9) == 0) {
	    if (strncmp(dev_p->kobj.name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
		/*  For health_status. Defined in show_led() in inv_cpld.c
		    0 - 000: off
		    1 - 001: 0.5hz
		    2 - 010: 1 hz
		    3 - 011: 2 hz
		    4~6: not define
		    7 - 111: on
		*/
		if (devgroup->led_sys_color_grn) {
		    inventec_show_attr(&tmpbuf[0], devgroup->led_sys_color_grn);
		    if (tmpbuf[0] == '1' || tmpbuf[0] == '2' ||
			tmpbuf[0] == '3' || tmpbuf[0] == '7') {
			return sprintf(buf_p, "green\n");
		    }
		}
		if (devgroup->led_sys_color_red) {
		    inventec_show_attr(&tmpbuf[0], devgroup->led_sys_color_red);
		    if (tmpbuf[0] == '1' || tmpbuf[0] == '2' ||
			tmpbuf[0] == '3' || tmpbuf[0] == '7') {
			return sprintf(buf_p, "red\n");
		    }
		}
		return sprintf(buf_p, "off\n");
	    }
	    else
	    if (strncmp(dev_p->kobj.name, "fan_led_", 8) == 0) {
		if (devgroup->led_sys_color_grn) {
		    inventec_show_attr(&tmpbuf[0], devgroup->led_sys_color_grn);
		    if (tmpbuf[0] == '1') {
			return sprintf(buf_p, "on\n");
		    }
		}
		if (devgroup->led_sys_color_red) {
		    inventec_show_attr(&tmpbuf[0], devgroup->led_sys_color_red);
		    if (tmpbuf[0] == '1') {
			return sprintf(buf_p, "on\n");
		    }
		}
		return sprintf(buf_p, "off\n");
	    }
	    return sprintf(buf_p, "off\n");
	}
	else
	if (strncmp(attr_p->attr.name, "sys_freq", 8) == 0) {
	    if (strncmp(dev_p->kobj.name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
		/*  For health_status. Defined in show_led() in inv_cpld.c
		    0: off
		    1: 0.5hz
		    2: 1 hz
		    3: 2 hz
		    4~6: not define
		    7: on
		*/
		if (devgroup->led_sys_color_grn) {
		    inventec_show_attr(&tmpbuf[0], devgroup->led_sys_color_grn);
		    if (tmpbuf[0] == '1' || tmpbuf[0] == '2' ||
			tmpbuf[0] == '3' || tmpbuf[0] == '7') {
			return sprintf(buf_p, "%s", tmpbuf);
		    }
		}
		if (devgroup->led_sys_color_red) {
		    inventec_show_attr(&tmpbuf[0], devgroup->led_sys_color_red);
		    if (tmpbuf[0] == '1' || tmpbuf[0] == '2' ||
			tmpbuf[0] == '3' || tmpbuf[0] == '7') {
			return sprintf(buf_p, "%s", tmpbuf);
		    }
		}
		if (tmpbuf[0] == '0') {
		    return sprintf(buf_p, "%s", tmpbuf);
		}
		return sprintf(buf_p, "%s\n",SHOW_ATTR_WARNING);
	    }
	    return sprintf(buf_p, "%s\n",SHOW_ATTR_WARNING);
	}
    }

    if (!invpathp) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    return inventec_show_attr(buf_p, invpathp);
}

ssize_t
show_attr_location(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

ssize_t
show_attr_sys_color(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

ssize_t
show_attr_sys_freq(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

/*
 * Store attr Section
 */
static DEFINE_MUTEX(diag_mutex);

ssize_t status_led_diag_mode_enable(void)
{
    char tmp[32];
    ssize_t ret;

    ret = inventec_show_attr(&tmp[0], HWMON_DEVICE_DIAG_PATH);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '0') {
	mutex_lock(&diag_mutex);
        ret = inventec_store_attr("1", 1, HWMON_DEVICE_DIAG_PATH);
        if (ret < 0) {
	    mutex_unlock(&diag_mutex);
            return ret;
        }

        ret = inventec_store_attr("1", 1, HWMON_DEVICE_CTL_PATH);
        if (ret < 0) {
	    mutex_unlock(&diag_mutex);
            return ret;
        }
	mutex_unlock(&diag_mutex);
    }

    return ret;
}
EXPORT_SYMBOL(status_led_diag_mode_enable);

ssize_t status_led_diag_mode_disable(void)
{
    char tmp[32];
    ssize_t ret;

    ret = inventec_show_attr(&tmp[0], HWMON_DEVICE_DIAG_PATH);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '1') {
	mutex_lock(&diag_mutex);
        ret = inventec_store_attr("0", 1, HWMON_DEVICE_DIAG_PATH);
        if (ret < 0) {
	    mutex_unlock(&diag_mutex);
            return 1;
        }

        ret = inventec_store_attr("1", 1, HWMON_DEVICE_CTL_PATH);
        if (ret < 0) {
	    mutex_unlock(&diag_mutex);
            return 1;
        }
	mutex_unlock(&diag_mutex);
    }
    return 1;
}
EXPORT_SYMBOL(status_led_diag_mode_disable);

ssize_t
status_led_change(const char *path1, const char *tmp1, const char *path2, const char *tmp2)
{
    ssize_t ret;

    ret = inventec_store_attr(tmp1, strlen(tmp1), path1);
    if (ret < 0) {
        return ret;
    }
    ret = inventec_store_attr(tmp2, strlen(tmp2), path2);
    if (ret < 0) {
        return ret;
    }
    if ((ret = status_led_diag_mode_enable()) <= 0) {
        return ret;
    }
    ssleep(1);
    if ((ret = status_led_diag_mode_disable()) <= 0) {
        return ret;
    }
    return ret;
}
EXPORT_SYMBOL(status_led_change);

static int status_led_diag_mode = STATUS_LED_MODE_AUTO;

int status_led_check_diag_mode(void)
{
    return status_led_diag_mode;
}
EXPORT_SYMBOL(status_led_check_diag_mode);

static ssize_t
store_attr_sys_color(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    char tmp[32];
    char path[64];
    ssize_t ret;

    /* For Status LED */
    if (strncmp(dev_p->kobj.name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
        if (strncmp(buf_p, "auto", 4) == 0) {
	    status_led_diag_mode_disable();
            status_led_diag_mode = STATUS_LED_MODE_AUTO;
        }
	else
        if (strncmp(buf_p, "red", 3) == 0) {
            status_led_diag_mode = STATUS_LED_MODE_MANU;
	    return status_led_change(STATUS_LED_GRN_PATH, "0",
				     STATUS_LED_RED_PATH, "7");
        }
	else
        if (strncmp(buf_p, "green", 5) == 0) {
            status_led_diag_mode = STATUS_LED_MODE_MANU;
	    return status_led_change(STATUS_LED_RED_PATH, "0",
				     STATUS_LED_GRN_PATH, "7");
        }
	else
        if (strncmp(buf_p, "off", 3) == 0) {
            status_led_diag_mode = STATUS_LED_MODE_MANU;
	    return status_led_change(STATUS_LED_GRN_PATH, "0",
				     STATUS_LED_RED_PATH, "0");
        }
	else {
            return 1;
	}
    }

    /* For Fans LEDs */
    if (strncmp(buf_p, "on", 2) == 0) {
        strcpy(tmp, "1");
    }
    else
    if (strncmp(buf_p, "off", 3) == 0) {
        strcpy(tmp, "0");
    }
    else {
        return 1;
    }

    if (strncmp(dev_p->kobj.name, LED_FAN_LED_GRN1, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_GRN1_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_GRN2, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_GRN2_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_GRN3, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_GRN3_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_GRN4, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_GRN4_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_RED1, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_RED1_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_RED2, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_RED2_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_RED3, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_RED3_PATH);
    }
    else
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_RED4, LED_FAN_LED_LEN) == 0) {
        strcpy(path, FAN_LED_RED4_PATH);
    }
    else {
        return 1;
    }
//printk(KERN_INFO "%s/%d: path = %s,  tmp = %s\n",__func__,__LINE__,path,tmp);
    ret = inventec_store_attr(tmp, strlen(tmp), path);
    if (ret < 0) {
        return ret;
    }

    if ((ret = status_led_diag_mode_enable()) <= 0) {
        return ret;
    }
    ssleep(1);
    if ((ret = status_led_diag_mode_disable()) <= 0) {
        return ret;
    }
    return ret;
}

static ssize_t
store_attr_sys_freq(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    ssize_t ret;
    char buf[32];

    strcpy(buf, buf_p);
    ret = inventec_store_input(buf, strlen(buf));
    if (ret == 0) {
	/* terminate the gabage in the end of a string */
	return 1;
    }

    /* For Status LED */
    if (strncmp(dev_p->kobj.name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
        char ledval[32];
	char *retp = NULL;
	int value;

	if (ret != 1) {
	    printk(KERN_ERR "Invalid value for sys_freq: %s\n", buf);
	    return ret;
	}

	value = inventec_strtol(buf, &retp, 10);
	if (retp == NULL) {
	    printk(KERN_ERR "Invalid string for converting to integer: %s\n",buf);
	    return ret;
	}
	else
        if (value == 0) {
	    printk(KERN_ERR "Set sys_freq to 0 is not allowed\n");
	    return ret;
	}
	else
        if (value != 1 && value != 2 && value != 3 && value != 7) {
	    printk(KERN_ERR "Invalid value for sys_freq: %s\n", buf);
            return ret;
	}

	ret = inventec_show_attr(ledval, STATUS_LED_GRN_PATH);
	if ((int)ret > 0 &&
	    (ledval[0] == '1' || ledval[0] == '2' || ledval[0] == '3' || ledval[0] == '7')) {
            status_led_diag_mode = STATUS_LED_MODE_MANU;
	    ret = status_led_change(STATUS_LED_RED_PATH, "0",
				    STATUS_LED_GRN_PATH, buf);
	    return ret;
	}

	ret = inventec_show_attr(ledval, STATUS_LED_RED_PATH);
	if ((int)ret > 0 &&
	    (ledval[0] == '1' || ledval[0] == '2' || ledval[0] == '3' || ledval[0] == '7')) {
            status_led_diag_mode = STATUS_LED_MODE_MANU;
	    ret = status_led_change(STATUS_LED_GRN_PATH, "0",
				    STATUS_LED_RED_PATH, buf);
	    return ret;
	}

	return ret;
    }

    /* For Fans LEDS */
    if (strncmp(dev_p->kobj.name, LED_FAN_LED_GRN1, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_GRN2, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_GRN3, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_GRN4, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_RED1, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_RED2, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_RED3, LED_FAN_LED_LEN) == 0 ||
        strncmp(dev_p->kobj.name, LED_FAN_LED_RED4, LED_FAN_LED_LEN) == 0) {
        printk(KERN_INFO "%s is not writable\n", dev_p->kobj.name);
    }

    return ret;
}

static DEVICE_ATTR(location,	S_IRUGO,	show_attr_location,	NULL);
static DEVICE_ATTR(sys_color,	S_IRUGO|S_IWUSR,show_attr_sys_color,	store_attr_sys_color);
static DEVICE_ATTR(sys_freq,	S_IRUGO|S_IWUSR,show_attr_sys_freq,	store_attr_sys_freq);

#ifdef YOCTO_REDWOOD
void led_set_gpio_to_change_status_led(void)
{
    ssize_t ret = inventec_store_attr("57", 2, "/sys/class/gpio/export");
    if (ret < 0) {
	SYSFS_LOG("Write 57 to /sys/class/gpio/export failed\n");
	return;
    }
    ret = inventec_store_attr("low", 3, "/sys/class/gpio/gpio57/direction");
    if (ret < 0) {
	SYSFS_LOG("Write low to /sys/class/gpio/gpio57/direction failed\n");
	return;
    }
    SYSFS_LOG("Set gpio to support status led change successfully\n");
}
EXPORT_SYMBOL(led_set_gpio_to_change_status_led);
#endif

static int
led_register_attrs(struct device *device_p)
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

static void
led_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_location);
	device_remove_file(device_p, &dev_attr_sys_color);
	device_remove_file(device_p, &dev_attr_sys_freq);
}

static int
led_register_device(int led_id)
{
    char *err_msg	= "ERR";
    int path_len	= 64;
    void		*curr_tracking;
    led_dev_group_t *group = &led_dev_group[led_id];

    /* Prepare INV driver access path */
    group->led_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->led_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_led_register_device_1;
    }
    curr_tracking = group->led_tracking;
    /* Creatr Sysfs device object */
    group->led_dev_p = inventec_child_device_create(led_device_p,
                                       group->led_devt,
                                       group->led_inv_pathp,
                                       group->led_name,
                                       curr_tracking);

    if (IS_ERR(group->led_dev_p)){
        err_msg = "device_create fail";
        goto err_led_register_device_2;
    }

    if (strncmp(group->led_name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
	if (led_register_attrs(group->led_dev_p) < 0){
	    goto err_led_register_device_3;
	}
    }
    else {
	if (led_onfan_register_attrs(group->led_dev_p) < 0){
	    goto err_led_register_device_3;
	}
    }
    return 0;

err_led_register_device_3:
    if (strncmp(group->led_name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
	led_unregister_attrs(group->led_dev_p);
    }
    else {
	led_onfan_unregister_attrs(group->led_dev_p);
    }
    inventec_child_device_destroy(group->led_tracking);
err_led_register_device_2:
    kfree(group->led_inv_pathp);
err_led_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
led_unregister_device(int led_id)
{
    led_dev_group_t *group = &led_dev_group[led_id];

    if (strncmp(group->led_name, LED_HEALTH_STATUS, LED_HEALTH_STATUS_LEN) == 0) {
	led_unregister_attrs(group->led_dev_p);
    }
    else {
	led_onfan_unregister_attrs(group->led_dev_p);
    }
    inventec_child_device_destroy(group->led_tracking);
    kfree(group->led_inv_pathp);
}

static int
led_device_create(void)
{
    char *err_msg  = "ERR";
    led_dev_group_t *group;
    int led_id;

    if (alloc_chrdev_region(&led_device_devt, 0, 1, LED_DEVICE_NAME) < 0) {
        err_msg = "alloc_chrdev_region fail!";
        goto err_led_device_create_1;
    }

    /* Creatr Sysfs device object */
    led_device_p = device_create(inventec_class_p,	/* struct class *cls     */
                             NULL,			/* struct device *parent */
			     led_device_devt,	/* dev_t devt */
                             NULL,			/* void *private_data    */
                             LED_DEVICE_NAME);	/* const char *fmt       */
    if (IS_ERR(led_device_p)){
        err_msg = "device_create fail";
        goto err_led_device_create_2;
    }

    /* portX interface tree create */
    for (led_id = 0; led_id < LED_DEV_TOTAL; led_id++) {
        group = &led_dev_group[led_id];

	if (alloc_chrdev_region(&group->led_devt, 0, group->led_dev_total, group->led_name) < 0) {
            err_msg = "portX alloc_chrdev_region fail!";
            goto err_led_device_create_4;
        }
	group->led_major = MAJOR(group->led_devt);
    }

    for (led_id = 0; led_id < LED_DEV_TOTAL; led_id++) {
        group = &led_dev_group[led_id];

        if (led_register_device(led_id) < 0){
            err_msg = "register_test_device fail";
            goto err_led_device_create_5;
        }
    }
    return 0;

err_led_device_create_5:
    while (--led_id >= 0) {
        led_unregister_device(led_id);
    }
    led_id = LED_DEV_TOTAL;
err_led_device_create_4:
    while (--led_id >= 0) {
        group = &led_dev_group[led_id];
        unregister_chrdev_region(group->led_devt, group->led_dev_total);
    }
    device_destroy(inventec_class_p, led_device_devt);
err_led_device_create_2:
    unregister_chrdev_region(led_device_devt, 1);
err_led_device_create_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
led_device_destroy(void)
{
    led_dev_group_t *group;
    int led_id;

    for (led_id = 0; led_id < LED_DEV_TOTAL; led_id++) {
        led_unregister_device(led_id);
    }
    for (led_id = 0; led_id < LED_DEV_TOTAL; led_id++) {
        group = &led_dev_group[led_id];
        unregister_chrdev_region(group->led_devt, group->led_dev_total);
    }
    device_destroy(inventec_class_p, led_device_devt);
    unregister_chrdev_region(led_device_devt, 1);
}

/*
 * Start of ledinfo_device Jul 5, 2016
 *
 * Inventec Platform Proxy Driver for platform ledinfo devices.
 */
typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} ledinfo_dev_t;

typedef struct {
	const char	*ledinfo_name;
	int		ledinfo_major;
        dev_t		ledinfo_devt;
	struct device	*ledinfo_dev_p;
	void		*ledinfo_tracking;
	ledinfo_dev_t	*ledinfo_dev_namep;
	int		ledinfo_dev_total;
	char		*ledinfo_inv_pathp;
} ledinfo_dev_group_t;

static ledinfo_dev_t ledinfo_dev_name[] = {
	{ "led_number",	NULL },
};

static ledinfo_dev_group_t ledinfo_dev_group[] = {
	{
		.ledinfo_name		= LEDINFO_DEVICE_NAME,
		.ledinfo_major		= 0,
		.ledinfo_devt		= 0L,
		.ledinfo_dev_p		= NULL,
		.ledinfo_tracking	= NULL,
		.ledinfo_dev_namep	= &ledinfo_dev_name[0],
		.ledinfo_dev_total	= sizeof(ledinfo_dev_name) / sizeof(const ledinfo_dev_t),
		.ledinfo_inv_pathp	= NULL,
	},
};
#define LEDINFO_DEV_TOTAL	( sizeof(ledinfo_dev_group)/ sizeof(const ledinfo_dev_group_t) )

static ssize_t
show_attr_led_number(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return sprintf(buf_p, "%lu\n", LED_DEV_TOTAL);
}

/* INV drivers mapping */
static DEVICE_ATTR(led_number,	S_IRUGO,	show_attr_led_number,	NULL);

static int
ledinfo_register_attrs(struct device *device_p)
{
    if (device_create_file(device_p, &dev_attr_led_number) < 0) {
        goto err_ledinfo_register_attrs;
    }
    return 0;

err_ledinfo_register_attrs:
    printk(KERN_ERR "[%s/%d] device_create_file() fail\n",__func__,__LINE__);
    return -1;
}

static void
ledinfo_unregister_attrs(struct device *device_p)
{
    device_remove_file(device_p, &dev_attr_led_number);
}

static int
ledinfo_register_device(ledinfo_dev_group_t *group)
{
    int path_len	= 64;
    char *err_msg	= "ERR";

    /* Prepare INV driver access path */
    group->ledinfo_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->ledinfo_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_ledinfo_register_device_1;
    }
    /* Get Sysfs device object */
    group->ledinfo_dev_p = led_device_p;
    if (!group->ledinfo_dev_p) {
        err_msg = "leds_get_led_devp() fail";
        goto err_ledinfo_register_device_2;
    }
    if (ledinfo_register_attrs(group->ledinfo_dev_p) < 0){
        goto err_ledinfo_register_device_3;
    }
    return 0;

err_ledinfo_register_device_3:
    device_destroy(inventec_class_p, group->ledinfo_devt);
err_ledinfo_register_device_2:
    kfree(group->ledinfo_inv_pathp);
err_ledinfo_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
ledinfo_unregister_device(ledinfo_dev_group_t *group)
{
    ledinfo_unregister_attrs(group->ledinfo_dev_p);
    device_destroy(inventec_class_p, group->ledinfo_devt);
    kfree(group->ledinfo_inv_pathp);
}

static int
ledinfo_device_create(void)
{
    char *err_msg  = "ERR";
    ledinfo_dev_group_t *group;
    int ledinfo_id;

    for (ledinfo_id = 0; ledinfo_id < LEDINFO_DEV_TOTAL; ledinfo_id++) {
        group = &ledinfo_dev_group[ledinfo_id];

        if (alloc_chrdev_region(&group->ledinfo_devt, 0, group->ledinfo_dev_total, group->ledinfo_name) < 0) {
            err_msg = "alloc_chrdev_region fail!";
            goto err_ledinfo_device_init_1;
        }
        group->ledinfo_major = MAJOR(group->ledinfo_devt);
    }

    for (ledinfo_id = 0; ledinfo_id < LEDINFO_DEV_TOTAL; ledinfo_id++) {
        group = &ledinfo_dev_group[ledinfo_id];

        /* Register device number */
        if (ledinfo_register_device(group) < 0){
            err_msg = "ledinfo_register_device fail";
            goto err_ledinfo_device_init_2;
        }
    }
    return 0;

err_ledinfo_device_init_2:
    while (--ledinfo_id >= 0) {
        group = &ledinfo_dev_group[ledinfo_id];
        ledinfo_unregister_device(group);
    }
    ledinfo_id = LEDINFO_DEV_TOTAL;
err_ledinfo_device_init_1:
    while (--ledinfo_id >= 0) {
        group = &ledinfo_dev_group[ledinfo_id];
        unregister_chrdev_region(group->ledinfo_devt, group->ledinfo_dev_total);
    }
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void ledinfo_device_destroy(void)
{
    int ledinfo_id;
    ledinfo_dev_group_t *group;

    for (ledinfo_id = 0; ledinfo_id < LEDINFO_DEV_TOTAL; ledinfo_id++) {
        group = &ledinfo_dev_group[ledinfo_id];
        ledinfo_unregister_device(group);
    }
    for (ledinfo_id = 0; ledinfo_id < LEDINFO_DEV_TOTAL; ledinfo_id++) {
        group = &ledinfo_dev_group[ledinfo_id];
        unregister_chrdev_region(group->ledinfo_devt, group->ledinfo_dev_total);
    }
}

static int
ledinfo_device_init(void)
{
    char *err_msg  = "ERR";

    if (ledinfo_device_create() < 0){
        err_msg = "ledinfo_device_create() fail!";
        goto err_ledinfo_device_init;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_ledinfo_device_init:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void
ledinfo_device_exit(void)
{
    ledinfo_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}

/* End of ledinfo_device */

static int __init
led_main_device_init(void)
{
    char *err_msg  = "ERR";
    int led_id;

    inventec_class_p = inventec_get_class();
    if (!inventec_class_p) {
        err_msg = "inventec_get_class() fail!";
        goto err_led_device_init_1;
    }

    for (led_id = 0; led_id < LED_DEV_TOTAL; led_id++) {
	if (strncmp(led_dev_group[led_id].led_name, "port", 4) == 0) {
	    led_dev_group[led_id].led_location = "Front";
	    led_dev_group[led_id].led_sys_color_grn = NULL;
	    led_dev_group[led_id].led_sys_color_red = NULL;
	    led_dev_group[led_id].led_sys_freq = NULL;
	}
	led_dev_group[led_id].led_dev_namep = NULL;
	led_dev_group[led_id].led_dev_total = 0;
        led_dev_group[led_id].led_major = 0;
        led_dev_group[led_id].led_devt = 0L;
        led_dev_group[led_id].led_dev_p = NULL;
        led_dev_group[led_id].led_inv_pathp = NULL;
        led_dev_group[led_id].led_tracking = NULL;
    }

    if (led_device_create() < 0){
        err_msg = "led_device_create() fail!";
        goto err_led_device_init_1;
    }

    if (ledinfo_device_init() < 0) {
        err_msg = "ledinfo_device_init() fail!";
        goto err_led_device_init_2;
    }

    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_led_device_init_2:
    led_device_destroy();
err_led_device_init_1:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void __exit
led_main_device_exit(void)
{
    ledinfo_device_exit();
    led_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}


MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);

module_init(led_main_device_init);
module_exit(led_main_device_exit);
