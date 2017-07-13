/*
 * fan_device.c
 * Aug 5, 2016 Initial
 * Jan 26, 2017 To support Redwood
 *
 * Inventec Platform Proxy Driver for fan device.
 *
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
#include <linux/ctype.h>
#include <linux/inventec/common/inventec_sysfs.h>

/* inventec_sysfs.c */
static struct class *inventec_class_p = NULL;

/* fan_device.c */
static struct device *fan_device_p = NULL;
static dev_t fan_device_devt = 0;

typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
	char *hardcode;
} fan_dev_t;

typedef struct {
	const char		*fan_name;
	int			fan_major;
        dev_t			fan_devt;
	struct device		*fan_dev_p;
	fan_dev_t		*fan_dev_namep;
	int			fan_dev_total;
	char			*fan_inv_pathp;
	void			*fan_tracking;
} fan_dev_group_t;

static fan_dev_t fan_dev_name[] = {
	{ "state",		"/sys/class/hwmon/hwmon0/device/fan_gpi", NULL },
	{ "direction",		"/sys/class/hwmon/hwmon0/device/fan_gpi", "Front to Rear" },
	{ "speed",		"/sys/class/hwmon/hwmon0/device/%s_input", NULL },
	{ "pwm",		"/sys/class/hwmon/hwmon0/device/%s_input", NULL },
};

static fan_dev_group_t fan_dev_group[] = {
	{
	  .fan_name = "fan1",
	  .fan_dev_namep = &fan_dev_name[0],
	  .fan_dev_total = sizeof(fan_dev_name) / sizeof(const fan_dev_t),
	},
	{
	  .fan_name = "fan2",
	  .fan_dev_namep = &fan_dev_name[0],
	  .fan_dev_total = sizeof(fan_dev_name) / sizeof(const fan_dev_t),
	},
	{
	  .fan_name = "fan3",
	  .fan_dev_namep = &fan_dev_name[0],
	  .fan_dev_total = sizeof(fan_dev_name) / sizeof(const fan_dev_t),
	},
	{
	  .fan_name = "fan4",
	  .fan_dev_namep = &fan_dev_name[0],
	  .fan_dev_total = sizeof(fan_dev_name) / sizeof(const fan_dev_t),
	},
};

#define FAN_STATE_NORMAL	"normal"
#define FAN_STATE_FAULTY	"faulty"
#define FAN_STATE_UNINSTALLED	"uninstalled"
#define FAN_STATE_UNKNOW	"unknown state"
#define FAN_STATE_INVALID	"Invalid state value"
#define FAN_STATE_READ_ERROR	"state read error"

#define FAN_LOG_UNINSTALLED	"removed"
#define FAN_LOG_NORMAL		"inserted"

//#define FAN_STATE_BIT_NORMAL		0
#define FAN_STATE_BIT_FAULTY		0
#define FAN_STATE_BIT_UNINSTALLED	1
#define FAN_STATE_BIT_UNKNOW		2
#define FAN_STATE_BIT_INVALID		3
#define FAN_STATE_BIT_READ_ERROR	4

static struct fans_tbl_s {
        char *fan_name;
        char *fan_front;
        char *fan_rear;
        unsigned int fan_state;
} fans_tbl[] = {
        {"fan1",	"/sys/class/hwmon/hwmon0/device/fan1_input",
			"/sys/class/hwmon/hwmon0/device/fan2_input",	0},
        {"fan2",	"/sys/class/hwmon/hwmon0/device/fan3_input",
			"/sys/class/hwmon/hwmon0/device/fan4_input",	0},
        {"fan3",	"/sys/class/hwmon/hwmon0/device/fan5_input",
			"/sys/class/hwmon/hwmon0/device/fan6_input",	0},
        {"fan4",	"/sys/class/hwmon/hwmon0/device/fan7_input",
			"/sys/class/hwmon/hwmon0/device/fan8_input",	0},
};
#define FAN_TBL_TOTAL	( sizeof(fans_tbl)/ sizeof(const struct fans_tbl_s) )
#define FAN_DEV_TOTAL	( sizeof(fan_dev_group)/ sizeof(const fan_dev_group_t) )

#define FAN_STATE_CHECK(i,b)	(fans_tbl[i].fan_state & (1<<b))
#define FAN_STATE_SET(i,b)	(fans_tbl[i].fan_state |= (1<<b))
#define FAN_STATE_CLEAR(i,b)	(fans_tbl[i].fan_state &= ~(1<<b))
#define FAN_STATE_INIT(i)	(fans_tbl[i].fan_state = 0)

int fan_get_fan_number(void)
{
    return FAN_DEV_TOTAL;
}
EXPORT_SYMBOL(fan_get_fan_number);

static char *fan_get_invpath(const char *attr_namep, const int devmajor)
{
    fan_dev_t *devnamep;
    int i;
    int fan_id = -1;

    for (i = 0; i < FAN_DEV_TOTAL; i++) {
        if (devmajor == fan_dev_group[i].fan_major) {
            fan_id = i;
            break;
	}
    }
    if (fan_id == -1) {
        return NULL;
    }

    devnamep = fan_dev_group[fan_id].fan_dev_namep;
    for (i = 0; i < fan_dev_group[fan_id].fan_dev_total; i++, devnamep++) {
        if (strcmp(devnamep->proxy_dev_name, attr_namep) == 0) {
            return devnamep->inv_dev_name;
	}
    }
    return NULL;
}

static int
fans_faulty_log(int fan_id)
{
    char buf[64];
    int pwm;

    memset(&buf[0], 0, 64);
    if (inventec_show_attr(buf, fans_tbl[fan_id].fan_front) < 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_READ_ERROR)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_READ_ERROR);
	    SYSFS_LOG("%s: front %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_READ_ERROR);
	}
	return 1;
    }
    pwm = simple_strtol(buf, NULL, 10);
    if (pwm <= 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_FAULTY)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_FAULTY);
	    //SYSFS_LOG("%s: %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_FAULTY);
	}
	return 1;
    }

    memset(&buf[0], 0, 64);
    if (inventec_show_attr(buf, fans_tbl[fan_id].fan_rear) < 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_READ_ERROR)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_READ_ERROR);
	    SYSFS_LOG("%s: rear %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_READ_ERROR);
	}
	return 1;
    }
    pwm = simple_strtol(buf, NULL, 10);
    if (pwm <= 0) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_FAULTY)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_FAULTY);
	    //SYSFS_LOG("%s: %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_FAULTY);
	}
	return 1;
    }

    if(fans_tbl[fan_id].fan_state != 0) {
	fans_tbl[fan_id].fan_state = 0;
	SYSFS_LOG("%s: %s\n",fans_tbl[fan_id].fan_name,FAN_LOG_NORMAL);
    }
    return 0;
}

/* INV drivers mapping */
static int
fans_control_log(int fan_id)
{
    char buf[64];
    unsigned int statebit2, statebit3, bitshift;

    if (inventec_show_attr(buf, "/sys/class/hwmon/hwmon0/device/fan_gpi") < 0) {
	printk(KERN_INFO "read /sys/class/hwmon/hwmon0/device/fan_gpi failed\n");
	return 1;
    }

    if (buf[0] != '0' || (buf[1] != 'x' && buf[1] != 'X') || buf[2] != buf[3]) {
	printk(KERN_DEBUG "%s/%d: %s %s\n",__func__,__LINE__,FAN_STATE_INVALID, buf);
	return 1;
    }

    statebit2 = buf[2] - '0';
    if (statebit2 < 0 || statebit2 > 9) {
	statebit2 = buf[2] - 'a' + 10;
	if (statebit2 < 10 || statebit2 > 15) {
	    statebit2 = buf[2] - 'A' + 10;
	    if (statebit2 < 10 || statebit2 > 15) {
		printk(KERN_DEBUG "%s/%d: %s %s\n",__func__,__LINE__,FAN_STATE_INVALID, buf);
		return 1;
	    }
	}
    }
    statebit3 = buf[3] - '0';
    if (statebit3 < 0 || statebit3 > 9) {
	statebit3 = buf[3] - 'a' + 10;
	if (statebit3 < 10 || statebit3 > 15) {
	    statebit3 = buf[3] - 'A' + 10;
	    if (statebit3 < 10 || statebit3 > 15) {
		printk(KERN_DEBUG "%s/%d: %s %s\n",__func__,__LINE__,FAN_STATE_INVALID, buf);
		return 1;
	    }
	}
    }

    bitshift = fan_id;
    //printk(KERN_INFO "1: statebit2 = 0x%x statebit3 = 0x%x bitshift = 0x%x\n",statebit2,statebit3,bitshift);
    if ((statebit2 & 1<<bitshift) && (statebit3 & 1<<bitshift)) {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_UNINSTALLED)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_UNINSTALLED);
	    SYSFS_LOG("%s: %s\n",fans_tbl[fan_id].fan_name,FAN_LOG_UNINSTALLED);
	}
	return 1;
    }
    else
    if (!(statebit2 & 1<<bitshift) && !(statebit3 & 1<<bitshift)) {
	return fans_faulty_log(fan_id);
    }
    else {
	if(!FAN_STATE_CHECK(fan_id, FAN_STATE_BIT_FAULTY)) {
	    FAN_STATE_SET(fan_id, FAN_STATE_BIT_FAULTY);
	    //SYSFS_LOG("%s: %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_FAULTY);
	}
	return 1;
    }
}

int fans_control(void)
{
    int i, flag = 0;

    for (i = 0; i < FAN_TBL_TOTAL; i++) {
	if(fans_control_log(i) == 1) {
	    flag = 1;
	}
    }

    if (flag == 1) {
	status_led_change(STATUS_LED_GRN_PATH, "0", STATUS_LED_RED_PATH, "3");
    }
    return flag;
}
EXPORT_SYMBOL(fans_control);

static ssize_t
fans_faulty_check(char *buf_p, int fan_id)
{
    char buf[64];
    int pwm;

    memset(&buf[0], 0, 64);
    if (inventec_show_attr(buf, fans_tbl[fan_id].fan_front) < 0) {
	return sprintf(buf_p, "%s: front fan %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_READ_ERROR);
    }
    pwm = simple_strtol(buf, NULL, 10);
    if (pwm <= 0) {
	return sprintf(buf_p, "%s\n", FAN_STATE_FAULTY);
    }

    memset(&buf[0], 0, 64);
    if (inventec_show_attr(buf, fans_tbl[fan_id].fan_rear) < 0) {
	return sprintf(buf_p, "%s: rear fan %s\n",fans_tbl[fan_id].fan_name,FAN_STATE_READ_ERROR);
    }
    pwm = simple_strtol(buf, NULL, 10);
    if (pwm <= 0) {
	return sprintf(buf_p, "%s\n", FAN_STATE_FAULTY);
    }

    return sprintf(buf_p, "%s\n", FAN_STATE_NORMAL);
}

/* INV drivers mapping */
static ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    char tmp_buf[64], buf[64];
    char *invpathp = fan_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));

    if (!invpathp) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    memset(&tmp_buf[0], 0, 64);
    if (strncmp(attr_p->attr.name, "speed", 5) == 0) {
	sprintf(tmp_buf, invpathp, dev_p->kobj.name);
        return inventec_show_attr(buf_p, tmp_buf);
    }

    if (strncmp(attr_p->attr.name, "pwm", 3) == 0) {
	int pwm;

        sprintf(tmp_buf, invpathp, dev_p->kobj.name);
        if (inventec_show_attr(buf, tmp_buf) < 0) {
            return sprintf(buf_p, "speed value NA\n");
        }

	memset(&tmp_buf[0], 0, 64);
	pwm = simple_strtol(buf, NULL, 10);
	sprintf(tmp_buf, "%d.%d", pwm/255, pwm%255);
	
        return sprintf(buf_p, "%s\n", tmp_buf);
    }

    if (strncmp(attr_p->attr.name, "direction", 9) == 0) {
        ssize_t count = inventec_show_attr(&tmp_buf[0], invpathp);
        if (count > 0) {
            if (strncmp(&tmp_buf[0], "0x00", 4) == 0) {
                return sprintf(buf_p, "Front to Rear\n");
            }
        }
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    if (strncmp(attr_p->attr.name, "state", 5) == 0) {
        unsigned int bitshift, statebit2, statebit3;

        if (inventec_show_attr(buf, invpathp) < 0) {
            return sprintf(buf_p, "read state value failed\n");
        }

	if (buf[0] != '0' || (buf[1] != 'x' && buf[1] != 'X') || buf[2] != buf[3]) {
	    return sprintf(buf_p, "%s: %s\n", FAN_STATE_INVALID, buf);
	}

	statebit2 = buf[2] - '0';
	if (statebit2 < 0 || statebit2 > 9) {
	    statebit2 = buf[2] - 'a' + 10;
	    if (statebit2 < 10 || statebit2 > 15) {
		statebit2 = buf[2] - 'A' + 10;
		if (statebit2 < 10 || statebit2 > 15) {
		    return sprintf(buf_p, "%s: %s\n", FAN_STATE_INVALID, buf);
		}
	    }
	}

	statebit3 = buf[3] - '0';
	if (statebit3 < 0 || statebit3 > 9) {
	    statebit3 = buf[3] - 'a' + 10;
	    if (statebit3 < 10 || statebit3 > 15) {
		statebit3 = buf[3] - 'A' + 10;
		if (statebit3 < 10 || statebit3 > 15) {
		    return sprintf(buf_p, "%s: %s\n", FAN_STATE_INVALID, buf);
		}
	    }
	}

	if (strncmp(dev_p->kobj.name, "fan1", 4) == 0) {
            bitshift = 0;
        }
        else
	if (strncmp(dev_p->kobj.name, "fan2", 4) == 0) {
            bitshift = 1;
        }
        else
	if (strncmp(dev_p->kobj.name, "fan3", 4) == 0) {
            bitshift = 2;
        }
        else
	if (strncmp(dev_p->kobj.name, "fan4", 4) == 0) {
            bitshift = 3;
        }
        else {
            return sprintf(buf_p, "Invalid fan name %s\n", dev_p->kobj.name);
        }

	//printk(KERN_INFO "2: statebit2 = 0x%x statebit3 = 0x%x bitshift = 0x%x\n",statebit2,statebit3,bitshift);
        if ((statebit2 & 1<<bitshift) && (statebit3 & 1<<bitshift)) {
	    return sprintf(buf_p, "%s\n", FAN_STATE_UNINSTALLED);
        }
        else
        if (!(statebit2 & 1<<bitshift) && !(statebit3 & 1<<bitshift)) {
	    return fans_faulty_check(buf_p, bitshift);
        }
        else {
	    return sprintf(buf_p, "%s\n", FAN_STATE_FAULTY);
        }
    }

    return inventec_show_attr(buf_p, invpathp);
}

static ssize_t
show_attr_state(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_direction(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_speed(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_pwm(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static DEVICE_ATTR(state,	S_IRUGO,	 show_attr_state,	NULL);
static DEVICE_ATTR(direction,	S_IRUGO,	 show_attr_direction,	NULL);
static DEVICE_ATTR(speed,	S_IRUGO,	 show_attr_speed,	NULL);
static DEVICE_ATTR(pwm,		S_IRUGO,	 show_attr_pwm,		NULL);

static int
fan_register_attrs(struct device *device_p)
{
	char *err_msg = "ERR";

	if (device_create_file(device_p, &dev_attr_state) < 0) {
		err_msg = "reg_attr state fail";
		goto err_fan_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_direction) < 0) {
		err_msg = "reg_attr direction fail";
		goto err_fan_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_speed) < 0) {
		err_msg = "reg_attr speed fail";
		goto err_fan_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_pwm) < 0) {
		err_msg = "reg_attr pwm fail";
		goto err_fan_register_attrs;
	}
	return 0;

err_fan_register_attrs:
	printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
	return -1;
}

static void
fan_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_state);
	device_remove_file(device_p, &dev_attr_direction);
	device_remove_file(device_p, &dev_attr_speed);
	device_remove_file(device_p, &dev_attr_pwm);
}

static int
fan_register_device(int fan_id)
{
    char *err_msg	= "ERR";
    int path_len	= 64;
    void		*curr_tracking;
    fan_dev_group_t *group = &fan_dev_group[fan_id];

    /* Prepare INV driver access path */
    group->fan_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->fan_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_fan_register_device_1;
    }
    curr_tracking = group->fan_tracking;
    /* Creatr Sysfs device object */
    group->fan_dev_p = inventec_child_device_create(fan_device_p,
                                       group->fan_devt,
                                       group->fan_inv_pathp,
                                       group->fan_name,
                                       curr_tracking);

    if (IS_ERR(group->fan_dev_p)){
        err_msg = "device_create fail";
        goto err_fan_register_device_2;
    }

    if (fan_register_attrs(group->fan_dev_p) < 0){
        goto err_fan_register_device_3;
    }
    return 0;

err_fan_register_device_3:
    fan_unregister_attrs(group->fan_dev_p);
    inventec_child_device_destroy(group->fan_tracking);
err_fan_register_device_2:
    kfree(group->fan_inv_pathp);
err_fan_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
fan_unregister_device(int fan_id)
{
    fan_dev_group_t *group = &fan_dev_group[fan_id];

    fan_unregister_attrs(group->fan_dev_p);
    inventec_child_device_destroy(group->fan_tracking);
    kfree(group->fan_inv_pathp);
}

static int
fan_device_create(void)
{
    char *err_msg  = "ERR";
    fan_dev_group_t *group;
    int fan_id;

    if (alloc_chrdev_region(&fan_device_devt, 0, 1, FAN_DEVICE_NAME) < 0) {
        err_msg = "alloc_chrdev_region fail!";
        goto err_fan_device_create_1;
    }

    /* Creatr Sysfs device object */
    fan_device_p = device_create(inventec_class_p,	/* struct class *cls     */
                             NULL,			/* struct device *parent */
			     fan_device_devt,		/* dev_t devt */
                             NULL,			/* void *private_data    */
                             FAN_DEVICE_NAME);		/* const char *fmt       */
    if (IS_ERR(fan_device_p)){
        err_msg = "device_create fail";
        goto err_fan_device_create_2;
    }

    /* portX interface tree create */
    for (fan_id = 0; fan_id < FAN_DEV_TOTAL; fan_id++) {
        group = &fan_dev_group[fan_id];

	if (alloc_chrdev_region(&group->fan_devt, 0, group->fan_dev_total, group->fan_name) < 0) {
            err_msg = "portX alloc_chrdev_region fail!";
            goto err_fan_device_create_4;
        }
	group->fan_major = MAJOR(group->fan_devt);
    }

    for (fan_id = 0; fan_id < FAN_DEV_TOTAL; fan_id++) {
        group = &fan_dev_group[fan_id];

        if (fan_register_device(fan_id) < 0){
            err_msg = "register_test_device fail";
            goto err_fan_device_create_5;
        }
    }
    return 0;

err_fan_device_create_5:
    while (--fan_id >= 0) {
        fan_unregister_device(fan_id);
    }
    fan_id = FAN_DEV_TOTAL;
err_fan_device_create_4:
    while (--fan_id >= 0) {
        group = &fan_dev_group[fan_id];
        unregister_chrdev_region(group->fan_devt, group->fan_dev_total);
    }
    device_destroy(inventec_class_p, fan_device_devt);
err_fan_device_create_2:
    unregister_chrdev_region(fan_device_devt, 1);
err_fan_device_create_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
fan_device_destroy(void)
{
    fan_dev_group_t *group;
    int fan_id;

    for (fan_id = 0; fan_id < FAN_DEV_TOTAL; fan_id++) {
        fan_unregister_device(fan_id);
    }
    for (fan_id = 0; fan_id < FAN_DEV_TOTAL; fan_id++) {
        group = &fan_dev_group[fan_id];
        unregister_chrdev_region(group->fan_devt, group->fan_dev_total);
    }
    device_destroy(inventec_class_p, fan_device_devt);
    unregister_chrdev_region(fan_device_devt, 1);
}

/*
 * Start of faninfo_device Jul 5, 2016
 *
 * Inventec Platform Proxy Driver for platform faninfo devices.
 */
typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} faninfo_dev_t;

typedef struct {
	const char	*faninfo_name;
	int		faninfo_major;
        dev_t		faninfo_devt;
	struct device	*faninfo_dev_p;
	void		*faninfo_tracking;
	faninfo_dev_t	*faninfo_dev_namep;
	int		faninfo_dev_total;
	char		*faninfo_inv_pathp;
} faninfo_dev_group_t;

static faninfo_dev_t faninfo_dev_name[] = {
	{ "fan_number",	NULL },
};

static faninfo_dev_group_t faninfo_dev_group[] = {
	{
		.faninfo_name		= FANINFO_DEVICE_NAME,
		.faninfo_major		= 0,
		.faninfo_devt		= 0L,
		.faninfo_dev_p		= NULL,
		.faninfo_tracking	= NULL,
		.faninfo_dev_namep	= &faninfo_dev_name[0],
		.faninfo_dev_total	= sizeof(faninfo_dev_name) / sizeof(const faninfo_dev_t),
		.faninfo_inv_pathp	= NULL,
	},
};
#define FANINFO_DEV_TOTAL	( sizeof(faninfo_dev_group)/ sizeof(const faninfo_dev_group_t) )

static ssize_t
show_attr_fan_number(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return sprintf(buf_p, "%lu\n", FAN_DEV_TOTAL);
}

/* INV drivers mapping */
static DEVICE_ATTR(fan_number,	S_IRUGO,	show_attr_fan_number,	NULL);

static int
faninfo_register_attrs(struct device *device_p)
{
    if (device_create_file(device_p, &dev_attr_fan_number) < 0) {
        goto err_faninfo_register_attrs;
    }
    return 0;

err_faninfo_register_attrs:
    printk(KERN_ERR "[%s/%d] device_create_file() fail\n",__func__,__LINE__);
    return -1;
}

static void
faninfo_unregister_attrs(struct device *device_p)
{
    device_remove_file(device_p, &dev_attr_fan_number);
}

static int
faninfo_register_device(faninfo_dev_group_t *group)
{
    int path_len	= 64;
    char *err_msg	= "ERR";

    /* Prepare INV driver access path */
    group->faninfo_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->faninfo_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_faninfo_register_device_1;
    }
    /* Get Sysfs device object */
    group->faninfo_dev_p = fan_device_p;
    if (!group->faninfo_dev_p) {
        err_msg = "fans_get_fan_devp() fail";
        goto err_faninfo_register_device_2;
    }
    if (faninfo_register_attrs(group->faninfo_dev_p) < 0){
        goto err_faninfo_register_device_3;
    }
    return 0;

err_faninfo_register_device_3:
    device_destroy(inventec_class_p, group->faninfo_devt);
err_faninfo_register_device_2:
    kfree(group->faninfo_inv_pathp);
err_faninfo_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
faninfo_unregister_device(faninfo_dev_group_t *group)
{
    faninfo_unregister_attrs(group->faninfo_dev_p);
    device_destroy(inventec_class_p, group->faninfo_devt);
    kfree(group->faninfo_inv_pathp);
}

static int
faninfo_device_create(void)
{
    char *err_msg  = "ERR";
    faninfo_dev_group_t *group;
    int faninfo_id;

    for (faninfo_id = 0; faninfo_id < FANINFO_DEV_TOTAL; faninfo_id++) {
        group = &faninfo_dev_group[faninfo_id];

        if (alloc_chrdev_region(&group->faninfo_devt, 0, group->faninfo_dev_total, group->faninfo_name) < 0) {
            err_msg = "alloc_chrdev_region fail!";
            goto err_faninfo_device_init_1;
        }
        group->faninfo_major = MAJOR(group->faninfo_devt);
    }

    for (faninfo_id = 0; faninfo_id < FANINFO_DEV_TOTAL; faninfo_id++) {
        group = &faninfo_dev_group[faninfo_id];

        /* Register device number */
        if (faninfo_register_device(group) < 0){
            err_msg = "faninfo_register_device fail";
            goto err_faninfo_device_init_2;
        }
    }
    return 0;

err_faninfo_device_init_2:
    while (--faninfo_id >= 0) {
        group = &faninfo_dev_group[faninfo_id];
        faninfo_unregister_device(group);
    }
    faninfo_id = FANINFO_DEV_TOTAL;
err_faninfo_device_init_1:
    while (--faninfo_id >= 0) {
        group = &faninfo_dev_group[faninfo_id];
        unregister_chrdev_region(group->faninfo_devt, group->faninfo_dev_total);
    }
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void faninfo_device_destroy(void)
{
    int faninfo_id;
    faninfo_dev_group_t *group;

    for (faninfo_id = 0; faninfo_id < FANINFO_DEV_TOTAL; faninfo_id++) {
        group = &faninfo_dev_group[faninfo_id];
        faninfo_unregister_device(group);
    }
    for (faninfo_id = 0; faninfo_id < FANINFO_DEV_TOTAL; faninfo_id++) {
        group = &faninfo_dev_group[faninfo_id];
        unregister_chrdev_region(group->faninfo_devt, group->faninfo_dev_total);
    }
}

static int
faninfo_device_init(void)
{
    char *err_msg  = "ERR";

    if (faninfo_device_create() < 0){
        err_msg = "faninfo_device_create() fail!";
        goto err_faninfo_device_init;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_faninfo_device_init:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void
faninfo_device_exit(void)
{
    faninfo_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}

/* End of faninfo_device */

static int __init
fan_device_init(void)
{
    char *err_msg  = "ERR";
    int fan_id;

    inventec_class_p = inventec_get_class();
    if (!inventec_class_p) {
        err_msg = "inventec_get_class() fail!";
        goto err_fan_device_init_1;
    }

    for (fan_id = 0; fan_id < FAN_DEV_TOTAL; fan_id++) {
        fan_dev_group[fan_id].fan_major = 0;
        fan_dev_group[fan_id].fan_devt = 0L;
        fan_dev_group[fan_id].fan_dev_p = NULL;
        fan_dev_group[fan_id].fan_inv_pathp = NULL;
        fan_dev_group[fan_id].fan_tracking = NULL;
    }

    if (fan_device_create() < 0){
        err_msg = "fan_device_create() fail!";
        goto err_fan_device_init_1;
    }

    if (faninfo_device_init() < 0){
        err_msg = "faninfo_device_init() fail!";
        goto err_fan_device_init_2;
    }

    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_fan_device_init_2:
    fan_device_destroy();
err_fan_device_init_1:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void __exit
fan_device_exit(void)
{
    faninfo_device_exit();
    fan_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}


MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);

module_init(fan_device_init);
module_exit(fan_device_exit);
