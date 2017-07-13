/*
 * sensor.c
 * Aug 5, 2016 Inital
 * Jan 26, 2017 To support Redwood
 *
 * Inventec Platform Proxy Driver for sensor device.
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
#include <linux/inventec/common/inventec_sysfs.h>

/* inventec_sysfs.c */
static struct class *inventec_class_p = NULL;

/* sensor_device.c */
static struct device *sensor_device_p = NULL;
static dev_t sensor_device_devt = 0;

struct device *sensors_get_sensor_devp(void)
{
	return sensor_device_p;
}
EXPORT_SYMBOL(sensors_get_sensor_devp);

typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} sensor_dev_t;

typedef struct {
	const char		*sensor_name;
	int			sensor_major;
        dev_t			sensor_devt;
	struct device		*sensor_dev_p;
	sensor_dev_t		*sensor_dev_namep;
	int			sensor_dev_total;
	char			*sensor_inv_pathp;
	void			*sensor_tracking;
	char			*sensor_location;
	char			*sensor_model;
	char			*sensor_temp;
} sensor_dev_group_t;

static sensor_dev_t sensor_dev_name[] = {
	{ "location",		NULL },
	{ "model",		NULL },
	{ "temperature",	NULL },
};

#if defined PLATFORM_MAGNOLIA
/* Sensor Location for Magnolia - from Magnolia HW Spec
No.4 sensor stands for the ambient temperature.
No.1 and No.7 sensors are used to monitor the air temperature behind the CPU and SW.
No.3 sensor monitors the temperature near the front panel.
No.4 and No.6 sensors monitor the temperature between the SFP+ cage and BCM56854.
No.5 sensor monitors the air temperature in the back of the mainboard.
No.7 and No.8 are the thermal sensors on the DDR 3 DIMM1 and DIMM2, respectively.
*/
static sensor_dev_group_t sensor_dev_group[] = {
	{
	  .sensor_name = "sensor1",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP442(local)",
	  .sensor_location  = "swich board  ENV",	//"behind the CPU and SM",
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp1_input",
	},
	{
	  .sensor_name = "sensor2",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP442(remote)",
	  .sensor_location  = "swich board between CPU and ASIC",	//"behind the CPU and SM",
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp2_input",
	},
	{
	  .sensor_name = "sensor5",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP75",
	  .sensor_location  = "cpu board  CPU",	//"in the back of the mainboard",
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp5_input",
	},
};
#elif defined PLATFORM_REDWOOD
static sensor_dev_group_t sensor_dev_group[] = {
	{
	  .sensor_name = "sensor1",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP75",
	  .sensor_location  = "I2C Bus",	//Redwood HW Spec p35
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp1_input",
	},
	{
	  .sensor_name = "sensor2",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP75",
	  .sensor_location  = "I2C Bus",	//Redwood HW Spec p35
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp1_input",
	},
	{
	  .sensor_name = "sensor3",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP75",
	  .sensor_location  = "I2C Bus",	//Redwood HW Spec p35
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp1_input",
	},
	{
	  .sensor_name = "sensor4",
	  .sensor_dev_namep = &sensor_dev_name[0],
	  .sensor_dev_total = sizeof(sensor_dev_name) / sizeof(const sensor_dev_t),
	  .sensor_model     = "TMP75",
	  .sensor_location  = "I2C Bus",	//Redwood HW Spec p35
	  .sensor_temp      = "/sys/class/hwmon/hwmon0/device/temp1_input",
	},
};
#elif defined PLATFORM_CYPRESS
TBD
#endif
#define SENSOR_DEV_TOTAL	( sizeof(sensor_dev_group)/ sizeof(const sensor_dev_group_t) )

static char *sensor_get_invpath(const char *attr_namep, const int devmajor, sensor_dev_group_t **devgrp)
{
    sensor_dev_t *devnamep;
    int i;
    int sensor_id = -1;

    for (i = 0; i < SENSOR_DEV_TOTAL; i++) {
        if (devmajor == sensor_dev_group[i].sensor_major) {
            sensor_id = i;
            break;
	}
    }
    if (sensor_id == -1) {
        return NULL;
    }

    *devgrp = &sensor_dev_group[sensor_id];
    devnamep = sensor_dev_group[sensor_id].sensor_dev_namep;
    for (i = 0; i < sensor_dev_group[sensor_id].sensor_dev_total; i++, devnamep++) {
        if (strcmp(devnamep->proxy_dev_name, attr_namep) == 0) {
            return devnamep->inv_dev_name;
	}
    }
    return NULL;
}

/* INV drivers mapping */
static ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    sensor_dev_group_t *devgroup = NULL;
    char *invpathp = NULL;

    invpathp = sensor_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt), &devgroup);
    if (devgroup) {
	/* hardcode attributes */
	if (strncmp(attr_p->attr.name, "model", 5) == 0) {
	    return sprintf(buf_p, "%s\n", devgroup->sensor_model);
	}
	if (strncmp(attr_p->attr.name, "location", 8) == 0) {
	    return sprintf(buf_p, "%s\n", devgroup->sensor_location);
	}
	if (strncmp(attr_p->attr.name, "temperature", 11) == 0) {
            char buf[16];
            ssize_t count = inventec_show_attr(&buf[0], devgroup->sensor_temp);
            if (count > 0) {
                int i, n = strlen(buf);
                for (i = 0; i < n; i++) {
                    if (n-i == 4) {
                        buf_p[i] = '.';
                        buf_p[i+1] = buf[i];
                        buf_p[i+2] = buf[i+1];
                        buf_p[i+3] = buf[i+2];
                        buf_p[i+4] = '\n';
                        buf_p[i+5] = '\0';
                        return count+1;;
                    }
                    buf_p[i] = buf[i];
                }
            }
            return sprintf(buf_p, "Invalid value\n");
	}
    }

    if (!invpathp) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    return inventec_show_attr(buf_p, invpathp);
}

static ssize_t
show_attr_location(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_model(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_temperature(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static DEVICE_ATTR(location,	S_IRUGO,	show_attr_location,	NULL);
static DEVICE_ATTR(model,	S_IRUGO,	show_attr_model,	NULL);
static DEVICE_ATTR(temperature,	S_IRUGO,	show_attr_temperature,	NULL);

static int
sensor_register_attrs(struct device *device_p)
{
	char *err_msg = "ERR";

	if (device_create_file(device_p, &dev_attr_location) < 0) {
		err_msg = "reg_attr bias_current fail";
		goto err_sensor_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_model) < 0) {
		err_msg = "reg_attr bias_current fail";
		goto err_sensor_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_temperature) < 0) {
		err_msg = "reg_attr connector_type fail";
		goto err_sensor_register_attrs;
	}
	return 0;

err_sensor_register_attrs:
	printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
	return -1;
}

static void
sensor_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_location);
	device_remove_file(device_p, &dev_attr_model);
	device_remove_file(device_p, &dev_attr_temperature);
}

static int
sensor_register_device(int sensor_id)
{
    char *err_msg	= "ERR";
    int path_len	= 64;
    void		*curr_tracking;
    sensor_dev_group_t *group = &sensor_dev_group[sensor_id];

    /* Prepare INV driver access path */
    group->sensor_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->sensor_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_sensor_register_device_1;
    }
    curr_tracking = group->sensor_tracking;
    /* Creatr Sysfs device object */
    group->sensor_dev_p = inventec_child_device_create(sensor_device_p,
                                       group->sensor_devt,
                                       group->sensor_inv_pathp,
                                       group->sensor_name,
                                       curr_tracking);

    if (IS_ERR(group->sensor_dev_p)){
        err_msg = "device_create fail";
        goto err_sensor_register_device_2;
    }

    if (sensor_register_attrs(group->sensor_dev_p) < 0){
        goto err_sensor_register_device_3;
    }
    return 0;

err_sensor_register_device_3:
    sensor_unregister_attrs(group->sensor_dev_p);
    inventec_child_device_destroy(group->sensor_tracking);
err_sensor_register_device_2:
    kfree(group->sensor_inv_pathp);
err_sensor_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
sensor_unregister_device(int sensor_id)
{
    sensor_dev_group_t *group = &sensor_dev_group[sensor_id];

    sensor_unregister_attrs(group->sensor_dev_p);
    inventec_child_device_destroy(group->sensor_tracking);
    kfree(group->sensor_inv_pathp);
}

static int
sensor_device_create(void)
{
    char *err_msg  = "ERR";
    sensor_dev_group_t *group;
    int sensor_id;

    if (alloc_chrdev_region(&sensor_device_devt, 0, 1, SENSOR_DEVICE_NAME) < 0) {
        err_msg = "alloc_chrdev_region fail!";
        goto err_sensor_device_create_1;
    }

    /* Creatr Sysfs device object */
    sensor_device_p = device_create(inventec_class_p,	/* struct class *cls     */
                             NULL,			/* struct device *parent */
			     sensor_device_devt,	/* dev_t devt */
                             NULL,			/* void *private_data    */
                             SENSOR_DEVICE_NAME);	/* const char *fmt       */
    if (IS_ERR(sensor_device_p)){
        err_msg = "device_create fail";
        goto err_sensor_device_create_2;
    }

    /* portX interface tree create */
    for (sensor_id = 0; sensor_id < SENSOR_DEV_TOTAL; sensor_id++) {
        group = &sensor_dev_group[sensor_id];

	if (alloc_chrdev_region(&group->sensor_devt, 0, group->sensor_dev_total, group->sensor_name) < 0) {
            err_msg = "portX alloc_chrdev_region fail!";
            goto err_sensor_device_create_4;
        }
	group->sensor_major = MAJOR(group->sensor_devt);
    }

    for (sensor_id = 0; sensor_id < SENSOR_DEV_TOTAL; sensor_id++) {
        group = &sensor_dev_group[sensor_id];

        if (sensor_register_device(sensor_id) < 0){
            err_msg = "register_test_device fail";
            goto err_sensor_device_create_5;
        }
    }
    return 0;

err_sensor_device_create_5:
    while (--sensor_id >= 0) {
        sensor_unregister_device(sensor_id);
    }
    sensor_id = SENSOR_DEV_TOTAL;
err_sensor_device_create_4:
    while (--sensor_id >= 0) {
        group = &sensor_dev_group[sensor_id];
        unregister_chrdev_region(group->sensor_devt, group->sensor_dev_total);
    }
    device_destroy(inventec_class_p, sensor_device_devt);
err_sensor_device_create_2:
    unregister_chrdev_region(sensor_device_devt, 1);
err_sensor_device_create_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
sensor_device_destroy(void)
{
    sensor_dev_group_t *group;
    int sensor_id;

    for (sensor_id = 0; sensor_id < SENSOR_DEV_TOTAL; sensor_id++) {
        sensor_unregister_device(sensor_id);
    }
    for (sensor_id = 0; sensor_id < SENSOR_DEV_TOTAL; sensor_id++) {
        group = &sensor_dev_group[sensor_id];
        unregister_chrdev_region(group->sensor_devt, group->sensor_dev_total);
    }
    device_destroy(inventec_class_p, sensor_device_devt);
    unregister_chrdev_region(sensor_device_devt, 1);
}

/*
 * Start of sensorinfo_device Jul 5, 2016
 *
 * Inventec Platform Proxy Driver for platform sensorinfo devices.
 */
typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} sensorinfo_dev_t;

typedef struct {
	const char	*sensorinfo_name;
	int		sensorinfo_major;
        dev_t		sensorinfo_devt;
	struct device	*sensorinfo_dev_p;
	void		*sensorinfo_tracking;
	sensorinfo_dev_t	*sensorinfo_dev_namep;
	int		sensorinfo_dev_total;
	char		*sensorinfo_inv_pathp;
} sensorinfo_dev_group_t;

static sensorinfo_dev_t sensorinfo_dev_name[] = {
	{ "sensor_number",	NULL },
};

static sensorinfo_dev_group_t sensorinfo_dev_group[] = {
	{
		.sensorinfo_name	= SENSORINFO_DEVICE_NAME,
		.sensorinfo_major	= 0,
		.sensorinfo_devt	= 0L,
		.sensorinfo_dev_p	= NULL,
		.sensorinfo_tracking	= NULL,
		.sensorinfo_dev_namep	= &sensorinfo_dev_name[0],
		.sensorinfo_dev_total	= sizeof(sensorinfo_dev_name) / sizeof(const sensorinfo_dev_t),
		.sensorinfo_inv_pathp	= NULL,
	},
};
#define SENSORINFO_DEV_TOTAL	( sizeof(sensorinfo_dev_group)/ sizeof(const sensorinfo_dev_group_t) )

static ssize_t
show_attr_sensor_number(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return sprintf(buf_p, "%lu\n", SENSOR_DEV_TOTAL);
}

/* INV drivers mapping */
static DEVICE_ATTR(sensor_number,	S_IRUGO,	show_attr_sensor_number,	NULL);

static int
sensorinfo_register_attrs(struct device *device_p)
{
    if (device_create_file(device_p, &dev_attr_sensor_number) < 0) {
        goto err_sensorinfo_register_attrs;
    }
    return 0;

err_sensorinfo_register_attrs:
    printk(KERN_ERR "[%s/%d] device_create_file() fail\n",__func__,__LINE__);
    return -1;
}

static void
sensorinfo_unregister_attrs(struct device *device_p)
{
    device_remove_file(device_p, &dev_attr_sensor_number);
}

static int
sensorinfo_register_device(sensorinfo_dev_group_t *group)
{
    int path_len	= 64;
    char *err_msg	= "ERR";

    /* Prepare INV driver access path */
    group->sensorinfo_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->sensorinfo_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_sensorinfo_register_device_1;
    }
    /* Get Sysfs device object */
    group->sensorinfo_dev_p = sensor_device_p;
    if (!group->sensorinfo_dev_p) {
        err_msg = "sensors_get_sensor_devp() fail";
        goto err_sensorinfo_register_device_2;
    }
    if (sensorinfo_register_attrs(group->sensorinfo_dev_p) < 0){
        goto err_sensorinfo_register_device_3;
    }
    return 0;

err_sensorinfo_register_device_3:
    device_destroy(inventec_class_p, group->sensorinfo_devt);
err_sensorinfo_register_device_2:
    kfree(group->sensorinfo_inv_pathp);
err_sensorinfo_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
sensorinfo_unregister_device(sensorinfo_dev_group_t *group)
{
    sensorinfo_unregister_attrs(group->sensorinfo_dev_p);
    device_destroy(inventec_class_p, group->sensorinfo_devt);
    kfree(group->sensorinfo_inv_pathp);
}

static int
sensorinfo_device_create(void)
{
    char *err_msg  = "ERR";
    sensorinfo_dev_group_t *group;
    int sensorinfo_id;

    for (sensorinfo_id = 0; sensorinfo_id < SENSORINFO_DEV_TOTAL; sensorinfo_id++) {
        group = &sensorinfo_dev_group[sensorinfo_id];

        if (alloc_chrdev_region(&group->sensorinfo_devt, 0, group->sensorinfo_dev_total, group->sensorinfo_name) < 0) {
            err_msg = "alloc_chrdev_region fail!";
            goto err_sensorinfo_device_init_1;
        }
        group->sensorinfo_major = MAJOR(group->sensorinfo_devt);
    }

    for (sensorinfo_id = 0; sensorinfo_id < SENSORINFO_DEV_TOTAL; sensorinfo_id++) {
        group = &sensorinfo_dev_group[sensorinfo_id];

        /* Register device number */
        if (sensorinfo_register_device(group) < 0){
            err_msg = "sensorinfo_register_device fail";
            goto err_sensorinfo_device_init_2;
        }
    }
    return 0;

err_sensorinfo_device_init_2:
    while (--sensorinfo_id >= 0) {
        group = &sensorinfo_dev_group[sensorinfo_id];
        sensorinfo_unregister_device(group);
    }
    sensorinfo_id = SENSORINFO_DEV_TOTAL;
err_sensorinfo_device_init_1:
    while (--sensorinfo_id >= 0) {
        group = &sensorinfo_dev_group[sensorinfo_id];
        unregister_chrdev_region(group->sensorinfo_devt, group->sensorinfo_dev_total);
    }
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void sensorinfo_device_destroy(void)
{
    int sensorinfo_id;
    sensorinfo_dev_group_t *group;

    for (sensorinfo_id = 0; sensorinfo_id < SENSORINFO_DEV_TOTAL; sensorinfo_id++) {
        group = &sensorinfo_dev_group[sensorinfo_id];
        sensorinfo_unregister_device(group);
    }
    for (sensorinfo_id = 0; sensorinfo_id < SENSORINFO_DEV_TOTAL; sensorinfo_id++) {
        group = &sensorinfo_dev_group[sensorinfo_id];
        unregister_chrdev_region(group->sensorinfo_devt, group->sensorinfo_dev_total);
    }
}

static int
sensorinfo_device_init(void)
{
    char *err_msg  = "ERR";

    if (sensorinfo_device_create() < 0) {
        err_msg = "sensorinfo_device_create() fail!";
        goto err_sensorinfo_device_init;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_sensorinfo_device_init:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void
sensorinfo_device_exit(void)
{
    sensorinfo_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}

/* End of sensorinfo_device */

static int __init
sensor_device_init(void)
{
    char *err_msg  = "ERR";
    int sensor_id;

    inventec_class_p = inventec_get_class();
    if (!inventec_class_p) {
        err_msg = "inventec_get_class() fail!";
        goto err_sensor_device_init_1;
    }

    for (sensor_id = 0; sensor_id < SENSOR_DEV_TOTAL; sensor_id++) {
        sensor_dev_group[sensor_id].sensor_major = 0;
        sensor_dev_group[sensor_id].sensor_devt = 0L;
        sensor_dev_group[sensor_id].sensor_dev_p = NULL;
        sensor_dev_group[sensor_id].sensor_inv_pathp = NULL;
        sensor_dev_group[sensor_id].sensor_tracking = NULL;
    }

    if (sensor_device_create() < 0){
        err_msg = "sensor_device_create() fail!";
        goto err_sensor_device_init_1;
    }

    if (sensorinfo_device_init() < 0){
        err_msg = "sensorinfo_device_init() fail!";
        goto err_sensor_device_init_2;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_sensor_device_init_2:
    sensor_device_destroy();
err_sensor_device_init_1:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void __exit
sensor_device_exit(void)
{
    sensorinfo_device_exit();
    sensor_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}


MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);

module_init(sensor_device_init);
module_exit(sensor_device_exit);
