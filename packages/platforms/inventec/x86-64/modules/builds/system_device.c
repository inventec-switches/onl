/*
 * system_device.c Jul 5, 2016
 *
 * Inventec Platform Proxy Driver for platform system devices.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/ctype.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/proc_fs.h>
#include <asm/uaccess.h>
#include <linux/inventec/common/inventec_sysfs.h>

/* inventec_sysfs.c */
static struct class *inventec_class_p = NULL;

typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} system_dev_t;

typedef struct {
	const char	*system_name;
	int		system_major;
        dev_t		system_devt;
	struct device	*system_dev_p;
	void		*system_tracking;
	system_dev_t	*system_dev_namep;
	int		system_dev_total;
	char		*system_inv_pathp;
} system_dev_group_t;

#define SYSTEM_DEV_NAME		"name"
#define SYSTEM_DEV_MANUFACTURE	"manufacture"
#define SYSTEM_DEV_DATE		"date"
#define SYSTEM_DEV_VENDOR	"vendor"
#define SYSTEM_DEV_PLATFORM	"platform"
#define SYSTEM_DEV_MAC		"mac"
#define SYSTEM_DEV_VERSION	"version"
#define SYSTEM_DEV_SERIAL       "serial"
#define SYSTEM_DEV_CPLD_VER	"cpld_ver"
#define SYSTEM_DEV_CPLD_HB	"cpld_heartbeat"
#define SYSTEM_DEV_PSOC_VER	"psoc_ver"
#define SYSTEM_DEV_PSOC_HB	"psoc_heartbeat"
#define SYSTEM_DEV_SWPS_VER	"swps_ver"
//#define SYSTEM_DEV_INVTRD_CTRL	"inventec_thread"
#define SYSTEM_SYSFS_VERSION	"SYSFS_VERSION"

static system_dev_t system_dev_name[] = {
	{ SYSTEM_DEV_NAME,		"/sys/class/swps/vpd/product_name" },
	{ SYSTEM_DEV_MANUFACTURE,	"/sys/class/swps/vpd/manufacturer" },
	{ SYSTEM_DEV_DATE,		"/sys/class/swps/vpd/man_date" },
	{ SYSTEM_DEV_VENDOR,		"/sys/class/swps/vpd/vendor_name" },
	{ SYSTEM_DEV_PLATFORM,		"/sys/class/swps/vpd/plat_name" },
	{ SYSTEM_DEV_MAC,		"/sys/class/swps/vpd/base_mac_addr" },
	{ SYSTEM_DEV_VERSION,		"/sys/class/swps/vpd/dev_ver"},
	{ SYSTEM_DEV_SERIAL,		"/sys/class/swps/vpd/sn" },
	{ SYSTEM_DEV_CPLD_VER,		"/sys/class/hwmon/hwmon1/device/info" },
	{ SYSTEM_DEV_CPLD_HB,		"/sys/class/hwmon/hwmon1/device/psu%d" },
	{ SYSTEM_DEV_PSOC_VER,		"/sys/class/hwmon/hwmon0/device/version" },
	{ SYSTEM_DEV_PSOC_HB,		"/sys/class/hwmon/hwmon0/device/psu%d" },
	{ SYSTEM_DEV_SWPS_VER,		"/sys/class/swps/module/version" },
	//{ SYSTEM_DEV_INVTRD_CTRL,	NULL },
	{ SYSTEM_SYSFS_VERSION,		NULL },
#ifdef MAGNOLIA_NOT_SUPPORT
	{ "bmc_ver",		NULL },
#endif
};

static system_dev_group_t system_dev_group[] = {
	{
		.system_name		= SYSTEM_DEVICE_NAME,
		.system_major		= 0,
		.system_devt		= 0L,
		.system_dev_p		= NULL,
		.system_tracking	= NULL,
		.system_dev_namep	= &system_dev_name[0],
		.system_dev_total	= sizeof(system_dev_name) / sizeof(const system_dev_t),
		.system_inv_pathp	= NULL,
	},
};
#define SYSTEM_DEV_TOTAL	( sizeof(system_dev_group)/ sizeof(const system_dev_group_t) )

//static DEFINE_MUTEX(thread_mutex);
static int inventec_thread_control = 1;

int thread_control(void)
{
    return inventec_thread_control;
}
EXPORT_SYMBOL(thread_control);

void thread_control_set(int val)
{
    //mutex_lock(&thread_mutex);
    inventec_thread_control = val;
    //mutex_unlock(&thread_mutex);
}
EXPORT_SYMBOL(thread_control_set);

static char *system_get_invpath(const char *attr_namep, const int devmajor)
{
    system_dev_t *devnamep;
    int i;
    int system_id = -1;

    for (i = 0; i < SYSTEM_DEV_TOTAL; i++) {
        if (devmajor == system_dev_group[i].system_major) {
            system_id = i;
            break;
	}
    }
    if (system_id == -1) {
        return NULL;
    }
    devnamep = system_dev_group[system_id].system_dev_namep;
    for (i = 0; i < system_dev_group[system_id].system_dev_total; i++, devnamep++) {
        if (strcmp(devnamep->proxy_dev_name, attr_namep) == 0) {
            return devnamep->inv_dev_name;
	}
    }
    return NULL;
}

static ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    char *invpathp = system_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));
    char tmp[256];

    if (!invpathp) {
        printk(KERN_INFO "%s\n", SHOW_ATTR_WARNING);
        return 1;
    }

    if (strncmp(attr_p->attr.name, "psoc_heartbeat", 14) == 0 ||
	strncmp(attr_p->attr.name, "cpld_heartbeat", 15) == 0) {
	char buf[32];
	int i;

	sprintf(tmp, invpathp, 0);	// psu0
	for (i = 0; i < 3; i++) {
	    if (inventec_show_attr(&buf[0], tmp) > 0 && psu_check_state_normal(&buf[0])) {
		return sprintf(buf_p, "1\n");
	    }
	}
	sprintf(tmp, invpathp, 1);	// psu1
	for (i = 0; i < 3; i++) {
	    if (inventec_show_attr(&buf[0], tmp) > 0 && psu_check_state_normal(&buf[0])) {
		return sprintf(buf_p, "1\n");
	    }
	}
	return sprintf(buf_p, "0\n");
    }

    if (inventec_show_attr(&tmp[0], invpathp) < 0) {
        printk(KERN_INFO "inventec_show_attr() failed on %s\n", invpathp);
        return 1;
    }

    return sprintf(buf_p, "%s\n", tmp);
}

static ssize_t
show_attr_pos_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p, int pos)
{
    char *invpathp = system_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));

    if (!invpathp) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    return inventec_show_attr_pos(buf_p, invpathp, pos);
}

static ssize_t
show_attr_name(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_manufacture(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_date(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_vendor(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_platform(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_mac(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_version(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_serial(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

/* 
 * clpd1_version is extracted from cpld_version /sys/class/hwmon/hwmon1/device/info
 * The cpld version value is located on the 3rd line in /sys/class/hwmon/hwmon1/device/info
 */
#define CPLD_VERSION_LINENO	3
static ssize_t
show_attr_cpld_ver(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    ssize_t sz;
    int i, pos;

    for (i = 0, pos = 0; i < CPLD_VERSION_LINENO; i++, pos+=sz) {
        sz = show_attr_pos_common(dev_p, attr_p, buf_p, pos);
    }
    return sz;
}

static ssize_t
show_attr_psoc_ver(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_swps_ver(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_cpld_heartbeat(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psoc_heartbeat(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_SYSFS_VERSION(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return sprintf(buf_p, "%s\n%s\n%s\n",INV_SYSFS_DESC,INV_SYSFS_VERSION,INV_SYSFS_RL_DATE);
}

#ifdef MAGNOLIA_NOT_SUPPORT
static ssize_t
show_attr_bmc_ver(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}
#endif

#define MAC_ALL_ZERO	("00:00:00:00:00:00")
#define MAC_BROADCAST	("ff:ff:ff:ff:ff:ff")	// Broadcast Mac
#define MAC_ADDR_LENG	(17)

static int inv_isMacFormat(const char *macp)
{
    int i = 0;

    while(macp[i] != '\0' && macp[i] != '\n') {
        if(i % 3 != 2 && !isxdigit(macp[i]))
	    return -1;
        if(i % 3 == 2 && macp[i] != ':')
	    return -1;
	i++;
    }
    if (i == MAC_ADDR_LENG) {
	return 0;
    }
    return -1;
}

static inline int inv_isMulticastEtherAddr(const char *macp)
{
    char a1[8];
    u_int8_t  i1;

    if (!strncpy(a1, macp, 2)) {
	return 0;
    }
    i1 = (u_int8_t)simple_strtol(a1, NULL, 16);
    return i1 & 0x01;
}

static inline int inv_isBroadcastEtherAddr(const char *macp)
{
    if (strncmp(macp,  MAC_BROADCAST, MAC_ADDR_LENG) == 0) {
	return 1;
    }
    return 0;
}

static inline int inv_isZeroEtherAddr(const char *macp)
{
    if (strncmp(macp,  MAC_ALL_ZERO, MAC_ADDR_LENG) == 0) {
	return 1;
    }
    return 0;
}

static inline int inv_isValidEtherAddr(const char *macp)
{
    if (inv_isBroadcastEtherAddr(macp)) {
	printk(KERN_ERR "Broadcast mac address is not allowed for system mac: %s", macp);
	return 0;
    }
    else
    if (inv_isZeroEtherAddr(macp)) {
	printk(KERN_ERR "all zero mac address is not allowed for system mac: %s", macp);
	return 0;
    }
    else
    if (inv_isMulticastEtherAddr(macp)) {
	printk(KERN_ERR "Multicast mac address is not allowed for system mac: %s", macp);
	return 0;
    }
    return 1;
}

static char *inv_strchr(const int c)
{
	const char *strset = "._-";

	return strchr(strset, c);
}

static int inv_isMax32Chars(const char *bufp, ssize_t count)
{
	int i = 0;
	int lcount = count - 1;
	if (lcount < 1 || lcount > INV_MAX_LEN_CHAR_STRING) {
	    printk(KERN_ERR "%d chars - out of range: %s",lcount,bufp);
	    return -1;
	}

	while (bufp[i] != '\n' && bufp[i] != '\0') {
	        if(!isalnum(bufp[i]) && !inv_strchr(bufp[i])) {
			printk(KERN_ERR "Invalid char %c at position %d\n", bufp[i],i);
			return -1;
		}
		i++;
	}
	return 0;
}

static ssize_t
store_attr_common(struct device *dev_p, struct device_attribute *attr_p, const char *buf_p, size_t count)
{
    char *invpathp = system_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));

    if (!invpathp) {
        //printk(KERN_INFO "RYU: %s\n", SHOW_ATTR_WARNING);
        return count;
    }

    return inventec_store_attr(buf_p, count, invpathp);
}

static ssize_t
store_attr_name(struct device *dev_p,
		struct device_attribute *attr_p,
		const char *buf_p,
		size_t count)
{
    if (inv_isMax32Chars(buf_p, count) < 0) {
        return count;
    }

    return store_attr_common(dev_p, attr_p, buf_p, count);
}

static ssize_t
store_attr_platform(struct device *dev_p,
		struct device_attribute *attr_p,
		const char *buf_p,
		size_t count)
{
    if (inv_isMax32Chars(buf_p, count) < 0) {
        return count;
    }

    return store_attr_common(dev_p, attr_p, buf_p, count);
}

static ssize_t
store_attr_mac(struct device *dev_p,
		struct device_attribute *attr_p,
		const char *buf_p,
		size_t count)
{
    int ret = 0;

    if ((ret = inv_isMacFormat(buf_p)) < 0) {
        printk(KERN_DEBUG "%d = MAC: %s(%d)\n",ret,buf_p,(int)strlen(buf_p));
        printk(KERN_ERR "Invalid mac format: %s", buf_p);
        return count;
    }

    if (!(ret = inv_isValidEtherAddr(buf_p))) {
        printk(KERN_DEBUG "%d = MAC: %s(%d)\n",ret,buf_p,(int)strlen(buf_p));
        //printk(KERN_ERR "Invalid mac address: %s", buf_p);
        return count;
    }

    return store_attr_common(dev_p, attr_p, buf_p, count);
}

static ssize_t
store_attr_version(struct device *dev_p,
		struct device_attribute *attr_p,
		const char *buf_p,
		size_t count)
{
    int i = 0;

    while (buf_p[i] != '\n' && buf_p[i] != '\0') {
	if (!isdigit(buf_p[i])) {
	    printk(KERN_ERR "Invalid version format: %s", buf_p);
	    return count;
	}
	i++;
    }
    if (i > 0 && i <= INV_MAX_LEN_SYS_VERSION) {
	return store_attr_common(dev_p, attr_p, buf_p, count);
    }

    printk(KERN_ERR "Invalid version format: %s", buf_p);
    return count;
}

static ssize_t
store_attr_manufacture(struct device *dev_p,
		struct device_attribute *attr_p,
		const char *buf_p,
		size_t count)
{
    if (inv_isMax32Chars(buf_p, count) < 0) {
        return count;
    }

    return store_attr_common(dev_p, attr_p, buf_p, count);
}

static DEVICE_ATTR(name,	S_IRUGO|S_IWUSR,show_attr_name,		store_attr_name);
static DEVICE_ATTR(manufacture,	S_IRUGO|S_IWUSR,show_attr_manufacture,	store_attr_manufacture);
static DEVICE_ATTR(date,	S_IRUGO,	show_attr_date,		NULL);
static DEVICE_ATTR(vendor,	S_IRUGO,	show_attr_vendor,	NULL);
static DEVICE_ATTR(platform,	S_IRUGO|S_IWUSR,show_attr_platform,	store_attr_platform);
static DEVICE_ATTR(mac,		S_IRUGO|S_IWUSR,show_attr_mac,		store_attr_mac);
static DEVICE_ATTR(version,	S_IRUGO|S_IWUSR,show_attr_version,	store_attr_version);
static DEVICE_ATTR(serial,	S_IRUGO,	show_attr_serial,	NULL);
static DEVICE_ATTR(cpld_ver,	S_IRUGO,	show_attr_cpld_ver,	NULL);
static DEVICE_ATTR(cpld_heartbeat,	S_IRUGO,	show_attr_cpld_heartbeat,	NULL);
static DEVICE_ATTR(psoc_ver,	S_IRUGO,	show_attr_psoc_ver,	NULL);
static DEVICE_ATTR(psoc_heartbeat,	S_IRUGO,	show_attr_psoc_heartbeat,	NULL);
static DEVICE_ATTR(swps_ver,	S_IRUGO,	show_attr_swps_ver,	NULL);
//static DEVICE_ATTR(inventec_thread,	S_IRUGO|S_IWUSR,	show_attr_inventec_thread,	store_attr_inventec_thread);
static DEVICE_ATTR(SYSFS_VERSION,	S_IRUGO,	show_attr_SYSFS_VERSION,	NULL);
#ifdef MAGNOLIA_NOT_SUPPORT
static DEVICE_ATTR(bmc_ver,	S_IRUGO,	show_attr_bmc_ver,	NULL);
#endif

static int
system_register_attrs(struct device *device_p)
{
    if (device_create_file(device_p, &dev_attr_name) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_manufacture) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_date) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_vendor) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_platform) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_mac) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_version) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_serial) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_cpld_ver) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_cpld_heartbeat) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_psoc_ver) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_psoc_heartbeat) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_swps_ver) < 0) {
        goto err_system_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_SYSFS_VERSION) < 0) {
        goto err_system_register_attrs;
    }
#ifdef MAGNOLIA_NOT_SUPPORT
    if (device_create_file(device_p, &dev_attr_bmc_ver) < 0) {
        goto err_system_register_attrs;
    }
#endif
    return 0;

err_system_register_attrs:
    printk(KERN_ERR "[%s/%d] device_create_file() fail\n",__func__,__LINE__);
    return -1;
}

static void
system_unregister_attrs(struct device *device_p)
{
    device_remove_file(device_p, &dev_attr_name);
    device_remove_file(device_p, &dev_attr_manufacture);
    device_remove_file(device_p, &dev_attr_date);
    device_remove_file(device_p, &dev_attr_vendor);
    device_remove_file(device_p, &dev_attr_platform);
    device_remove_file(device_p, &dev_attr_mac);
    device_remove_file(device_p, &dev_attr_version);
    device_remove_file(device_p, &dev_attr_serial);
    device_remove_file(device_p, &dev_attr_cpld_ver);
    device_remove_file(device_p, &dev_attr_cpld_heartbeat);
    device_remove_file(device_p, &dev_attr_psoc_ver);
    device_remove_file(device_p, &dev_attr_psoc_heartbeat);
    device_remove_file(device_p, &dev_attr_swps_ver);
    //device_remove_file(device_p, &dev_attr_inventec_thread);
    device_remove_file(device_p, &dev_attr_SYSFS_VERSION);
#ifdef MAGNOLIA_NOT_SUPPORT
    device_remove_file(device_p, &dev_attr_bmc_ver);
#endif
}

static int
system_register_device(system_dev_group_t *group)
{
    int path_len	= 64;
    char *err_msg	= "ERR";

    /* Prepare INV driver access path */
    group->system_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->system_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_system_register_device_1;
    }
    /* Creatr Sysfs device object */
    group->system_dev_p = device_create(inventec_class_p,
                                       NULL,
                                       group->system_devt,
                                       group->system_inv_pathp,
                                       group->system_name);
    if (IS_ERR(group->system_dev_p)){
        err_msg = "device_create fail";
        goto err_system_register_device_2;
    }
    if (system_register_attrs(group->system_dev_p) < 0){
        goto err_system_register_device_3;
    }
    return 0;

err_system_register_device_3:
    device_destroy(inventec_class_p, group->system_devt);
err_system_register_device_2:
    kfree(group->system_inv_pathp);
err_system_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
system_unregister_device(system_dev_group_t *group)
{
    system_unregister_attrs(group->system_dev_p);
    device_destroy(inventec_class_p, group->system_devt);
    kfree(group->system_inv_pathp);
}

static int
system_device_create(void)
{
    char *err_msg  = "ERR";
    system_dev_group_t *group;
    int system_id;

    for (system_id = 0; system_id < SYSTEM_DEV_TOTAL; system_id++) {
        group = &system_dev_group[system_id];

        if (alloc_chrdev_region(&group->system_devt, 0, group->system_dev_total, group->system_name) < 0) {
            err_msg = "alloc_chrdev_region fail!";
            goto err_system_device_init_1;
        }
        group->system_major = MAJOR(group->system_devt);
    }

    for (system_id = 0; system_id < SYSTEM_DEV_TOTAL; system_id++) {
        group = &system_dev_group[system_id];

        /* Register device number */
        if (system_register_device(group) < 0){
            err_msg = "system_register_device fail";
            goto err_system_device_init_2;
        }
    }
    return 0;

err_system_device_init_2:
    while (--system_id >= 0) {
        group = &system_dev_group[system_id];
        system_unregister_device(group);
    }
    system_id = SYSTEM_DEV_TOTAL;
err_system_device_init_1:
    while (--system_id >= 0) {
        group = &system_dev_group[system_id];
        unregister_chrdev_region(group->system_devt, group->system_dev_total);
    }
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void system_device_destroy(void)
{
    int system_id;
    system_dev_group_t *group;

    for (system_id = 0; system_id < SYSTEM_DEV_TOTAL; system_id++) {
        group = &system_dev_group[system_id];
        system_unregister_device(group);
    }
    for (system_id = 0; system_id < SYSTEM_DEV_TOTAL; system_id++) {
        group = &system_dev_group[system_id];
        unregister_chrdev_region(group->system_devt, group->system_dev_total);
    }
}

static int __init
system_device_init(void)
{
    char *err_msg  = "ERR";

    inventec_class_p = inventec_get_class();
    if (!inventec_class_p) {
        err_msg = "inventec_get_class() fail!";
        goto err_system_device_init;;
    }

    if (system_device_create() < 0){
        err_msg = "system_device_create() fail!";
        goto err_system_device_init;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_system_device_init:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void __exit
system_device_exit(void)
{
    system_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}

module_init(system_device_init);
module_exit(system_device_exit);

MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);
