/*
 * psu_device.c
 * Aug 5, 2016 Initial
 * Jan 26, 2017 To support Redwood
 *
 * Inventec Platform Proxy Driver for psu device.
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
#include <linux/inventec/common/inventec_sysfs.h>

/* inventec_sysfs.c */
static struct class *inventec_class_p = NULL;
static unsigned int psu_voltin = 0;
#define PSU_VOLTIN_ACDC	(70000)

/* psu_device.c */
static struct device *psu_device_p = NULL;
static dev_t psu_device_devt = 0;

struct device *psus_get_psu_devp(void)
{
	return psu_device_p;
}
EXPORT_SYMBOL(psus_get_psu_devp);

/*
 * normal/unpower/uninstall/fault are PSU states output from driver level
 * checkpsu/error are defined by sysfs
 */
#define PSU_STATE_VAL_NORMAL	(0)
#define PSU_STATE_VAL_UNPOWER	(2)
#define PSU_STATE_VAL_FAULT	(4)
#define PSU_STATE_VAL_UNINSTALL	(7)
#define PSU_STATE_VAL_CHECKPSU	(8)
#define PSU_STATE_VAL_ERROR	(9)

#define PSU_STATE_NORMAL	("0 : normal")
#define PSU_STATE_UNPOWER	("2 : unpowered")
#define PSU_STATE_FAULT		("4 : fault")
#define PSU_STATE_UNINSTALL	("7 : not installed")
#define PSU_STATE_CHECKPSU	("8 : check psu")
#define PSU_STATE_ERROR		("9 : state error")

#define PSU_STATE_LEN_NORMAL	(strlen(PSU_STATE_NORMAL))
#define PSU_STATE_LEN_UNPOWER	(strlen(PSU_STATE_UNPOWER))
#define PSU_STATE_LEN_FAULT	(strlen(PSU_STATE_FAULT))
#define PSU_STATE_LEN_UNINSTALL	(strlen(PSU_STATE_UNINSTALL))
#define PSU_STATE_LEN_CHECKPSU	(strlen(PSU_STATE_CHECKPSU))

typedef struct {
	char *inv_dev_attrp;
	char *inv_dev_pathp;
} psu_dev_t;

typedef struct {
	const char		*psu_name;
	int			psu_major;
        dev_t			psu_devt;
	struct device		*psu_dev_p;
	psu_dev_t		*psu_dev_namep;
	int			psu_dev_total;
	char			*psu_inv_pathp;
	void			*psu_tracking;
	char			*psu_currentin;
	char			*psu_currentout;
	char			*psu_powerin;
	char			*psu_powerout;
	char			*psu_voltin;
	char			*psu_voltout;
} psu_dev_group_t;

static psu_dev_t psu_dev_name[] = {
	{ "state",		"/sys/class/hwmon/hwmon1/device/%s" },	// Using cpld
	{ "vendor",		"/sys/class/hwmon/hwmon0/device/%s_vendor" },
	{ "version",		"/sys/class/hwmon/hwmon0/device/%s_version" },
	{ "sn",			"/sys/class/hwmon/hwmon0/device/%s_sn", },
	{ "temperature",	"/sys/class/hwmon/hwmon0/device/thermal_%s" },
	{ "fan_speed",		"/sys/class/hwmon/hwmon0/device/rpm_%s" },
	{ "fan_pwm",		"/sys/class/hwmon/hwmon0/device/pwm_%s" },
	{ "fan_faulty",		"/sys/class/hwmon/hwmon0/device/rpm_%s" },
	{ "psu_currentin",	"/sys/class/hwmon/hwmon0/device/%s_iin" },
	{ "psu_currentout",	"/sys/class/hwmon/hwmon0/device/%s_iout" },
	{ "psu_powerin",	"/sys/class/hwmon/hwmon0/device/%s_pin" },
	{ "psu_powerout",	"/sys/class/hwmon/hwmon0/device/%s_pout" },
	{ "psu_voltin",		"/sys/class/hwmon/hwmon0/device/%s_vin" },
	{ "psu_voltout",	"/sys/class/hwmon/hwmon0/device/%s_vout" },
	{ "psu_pwm",		"/sys/class/hwmon/hwmon0/device/pwm_%s" },
	{ "psu_rpm",		"/sys/class/hwmon/hwmon0/device/rpm_%s" },
};
#define PSU_DEV_NAME_TOTAL	( sizeof(psu_dev_name) / sizeof(const psu_dev_t) )

static psu_dev_group_t psu_dev_group[] = {
	{
	  .psu_name = "psu1",
	  .psu_dev_namep = &psu_dev_name[0],
	  .psu_dev_total = sizeof(psu_dev_name) / sizeof(const psu_dev_t),
	},
	{
	  .psu_name = "psu2",
	  .psu_dev_namep = &psu_dev_name[0],
	  .psu_dev_total = sizeof(psu_dev_name) / sizeof(const psu_dev_t),
	},
};
#define PSU_DEV_GROUP_TOTAL	( sizeof(psu_dev_group)/ sizeof(const psu_dev_group_t) )

static char psu_state[32][32];

static struct psu_wire_tbl_s {
        char *psu_attr;
        char *psu_name;
	char *psu_wire;
	char *psu_state;
} psu_wire_tbl[] = {
#if defined PLATFORM_MAGNOLIA
        { "state",		"psu1",	"psu0", psu_state[0] },	// Using cpld
        { "state",		"psu2",	"psu1", psu_state[1] },
	{ "vendor",		"psu1",	"psu1", psu_state[2] },
	{ "vendor",		"psu2",	"psu2", psu_state[3] },
	{ "version",		"psu1",	"psu1", psu_state[4] },
	{ "version",		"psu2",	"psu2", psu_state[5] },
	{ "sn",			"psu1",	"psu1", psu_state[6] },
	{ "sn",			"psu2",	"psu2", psu_state[7] },
	{ "temperature",	"psu1",	"psu1", psu_state[8] },
	{ "temperature",	"psu2",	"psu2", psu_state[9] },
	{ "fan_speed",		"psu1",	"psu1", psu_state[10] },
	{ "fan_speed",		"psu2",	"psu2", psu_state[11] },
	{ "fan_pwm",		"psu1",	"psu1", psu_state[12] },
	{ "fan_pwm",		"psu2",	"psu2", psu_state[13] },
	{ "fan_faulty",		"psu1",	"psu1", psu_state[14] },
	{ "fan_faulty",		"psu2",	"psu2", psu_state[15] },
	{ "psu_currentin",	"psu1",	"psu1", psu_state[16] },
	{ "psu_currentin",	"psu2",	"psu2", psu_state[17] },
	{ "psu_currentout",	"psu1",	"psu1", psu_state[18] },
	{ "psu_currentout",	"psu2",	"psu2", psu_state[19] },
	{ "psu_powerin",	"psu1",	"psu1", psu_state[20] },
	{ "psu_powerin",	"psu2",	"psu2", psu_state[21] },
	{ "psu_powerout",	"psu1",	"psu1", psu_state[22] },
	{ "psu_powerout",	"psu2",	"psu2", psu_state[23] },
	{ "psu_voltin",		"psu1",	"psu1", psu_state[24] },
	{ "psu_voltin",		"psu2",	"psu2", psu_state[25] },
	{ "psu_voltout",	"psu1",	"psu1", psu_state[26] },
	{ "psu_voltout",	"psu2",	"psu2", psu_state[27] },
	{ "psu_pwm",		"psu1",	"psu1", psu_state[28] },
	{ "psu_pwm",		"psu2",	"psu2", psu_state[29] },
	{ "psu_rpm",		"psu1",	"psu1", psu_state[30] },
	{ "psu_rpm",		"psu2",	"psu2", psu_state[31] },
#elif defined PLATFORM_REDWOOD
        { "state",		"psu1",	"psu1", psu_state[0] },	// Using cpld
        { "state",		"psu2",	"psu0", psu_state[1] },
	{ "vendor",		"psu1",	"psu1", psu_state[2] },
	{ "vendor",		"psu2",	"psu2", psu_state[3] },
	{ "version",		"psu1",	"psu1", psu_state[4] },
	{ "version",		"psu2",	"psu2", psu_state[5] },
	{ "sn",			"psu1",	"psu1", psu_state[6] },
	{ "sn",			"psu2",	"psu2", psu_state[7] },
	{ "temperature",	"psu1",	"psu1", psu_state[8] },
	{ "temperature",	"psu2",	"psu2", psu_state[9] },
	{ "fan_speed",		"psu1",	"psu2", psu_state[10] },
	{ "fan_speed",		"psu2",	"psu1", psu_state[11] },
	{ "fan_pwm",		"psu1",	"psu2", psu_state[12] },
	{ "fan_pwm",		"psu2",	"psu1", psu_state[13] },
	{ "fan_faulty",		"psu1",	"psu2", psu_state[14] },
	{ "fan_faulty",		"psu2",	"psu1", psu_state[15] },
	{ "psu_currentin",	"psu1",	"psu1", psu_state[16] },
	{ "psu_currentin",	"psu2",	"psu2", psu_state[17] },
	{ "psu_currentout",	"psu1",	"psu1", psu_state[18] },
	{ "psu_currentout",	"psu2",	"psu2", psu_state[19] },
	{ "psu_powerin",	"psu1",	"psu1", psu_state[20] },
	{ "psu_powerin",	"psu2",	"psu2", psu_state[21] },
	{ "psu_powerout",	"psu1",	"psu1", psu_state[22] },
	{ "psu_powerout",	"psu2",	"psu2", psu_state[23] },
	{ "psu_voltin",		"psu1",	"psu1", psu_state[24] },
	{ "psu_voltin",		"psu2",	"psu2", psu_state[25] },
	{ "psu_voltout",	"psu1",	"psu1", psu_state[26] },
	{ "psu_voltout",	"psu2",	"psu2", psu_state[27] },
	{ "psu_pwm",		"psu1",	"psu1", psu_state[28] },
	{ "psu_pwm",		"psu2",	"psu2", psu_state[29] },
	{ "psu_rpm",		"psu1",	"psu2", psu_state[30] },
	{ "psu_rpm",		"psu2",	"psu1", psu_state[31] },
#elif defined PLATFORM_CYPRESS
TBD
#endif
};
#define PSU_WIRE_TBL_TOTAL   ( sizeof(psu_wire_tbl)/ sizeof(const struct psu_wire_tbl_s) )

static char *
psu_attr_get_wirep(const char *psu_attrp, const char *psu_namep, char **psu_statepp)
{
    int i;

    for (i = 0; i < PSU_WIRE_TBL_TOTAL; i++) {
	if (strncmp(psu_wire_tbl[i].psu_attr, psu_attrp, strlen(psu_attrp)) == 0 &&
	    strncmp(psu_wire_tbl[i].psu_name, psu_namep, strlen(psu_namep)) == 0) {
	    if (psu_statepp) {
		*psu_statepp = psu_wire_tbl[i].psu_state;
	    }
	    return psu_wire_tbl[i].psu_wire;
	}
    }
    return NULL;
}

int psu_check_state_normal(char *statep)
{
    if (strstr(statep, "normal")) {
	return 1;
    }
    return 0;
}
EXPORT_SYMBOL(psu_check_state_normal);

static char *psu_get_invpath(const char *attr_namep, const int devmajor)
{
    psu_dev_t *devnamep;
    int i;
    int psu_id = -1;

    for (i = 0; i < PSU_DEV_GROUP_TOTAL; i++) {
        if (devmajor == psu_dev_group[i].psu_major) {
            psu_id = i;
            break;
	}
    }
    if (psu_id == -1) {
        return NULL;
    }

    devnamep = psu_dev_group[psu_id].psu_dev_namep;
    for (i = 0; i < psu_dev_group[psu_id].psu_dev_total; i++, devnamep++) {
        if (strcmp(devnamep->inv_dev_attrp, attr_namep) == 0) {
            return devnamep->inv_dev_pathp;
	}
    }
    return NULL;
}

#define PSU_ATTR_VOLTIN		("psu_voltin")
#define PSU_ATTR_VOLTIN_LEN	(10)

/* Get PSU voltin for determon AC(110v) or DC(48v) */
void psu_get_voltin(void)
{
    char acc_path[64], volt[32];
    psu_dev_t *devnamep;
    unsigned int voltin;
    char *invwirep;
    int i, j;

    for (i = 0; i < PSU_DEV_GROUP_TOTAL; i++) {
	//psu_dev_group[i].psu_name;
	devnamep = psu_dev_group[i].psu_dev_namep;
	for (j = 0; j < psu_dev_group[i].psu_dev_total; j++, devnamep++) {
	    if (strncmp(devnamep->inv_dev_attrp, PSU_ATTR_VOLTIN, PSU_ATTR_VOLTIN_LEN) == 0) {
		invwirep = psu_attr_get_wirep(PSU_ATTR_VOLTIN, psu_dev_group[i].psu_name, NULL);
		if (invwirep == NULL) {
		    printk(KERN_DEBUG "Invalid psuname: %s\n", psu_dev_group[i].psu_name);
		    continue;
		}
		sprintf(acc_path, devnamep->inv_dev_pathp, invwirep);
		if (inventec_show_attr(volt, acc_path) <= 0) {
		    printk(KERN_DEBUG "Read %s failed\n", acc_path);
		    continue;
		}
		else {
		    voltin = simple_strtol(&volt[0], NULL, 10);
		    printk(KERN_DEBUG "Read %s %s = %u\n",acc_path,volt,voltin);
		    if (voltin > psu_voltin) {
			psu_voltin = voltin;
		    }
		}
	    }
	}
    }

    SYSFS_LOG("PSU voltin = %u\n", psu_voltin);
}
EXPORT_SYMBOL(psu_get_voltin);

#define PSU_ATTR_STATE		("state")
#define PSU_ATTR_STATE_LEN	(5)

/* psus_control() by inv_thread */
int psus_control(void)
{
    char acc_path[64], state[32];
    psu_dev_t *devnamep = NULL;
    char *invwirep = NULL;
    char *psu_statep = NULL;
    int i, j, flag = 0;

    for (i = 0; i < PSU_DEV_GROUP_TOTAL; i++) {
	devnamep = psu_dev_group[i].psu_dev_namep;
	for (j = 0; j < psu_dev_group[i].psu_dev_total; j++, devnamep++) {
	    if (strncmp(devnamep->inv_dev_attrp, PSU_ATTR_STATE, PSU_ATTR_STATE_LEN) == 0) {
		invwirep = psu_attr_get_wirep(PSU_ATTR_STATE, psu_dev_group[i].psu_name, &psu_statep);
		if (invwirep == NULL) {
		    printk(KERN_DEBUG "Invalid psuname: %s\n", psu_dev_group[i].psu_name);
		    continue;
		}
		sprintf(acc_path, devnamep->inv_dev_pathp, invwirep);
		if (inventec_show_attr(state, acc_path) <= 0) {
		    printk(KERN_DEBUG "Read %s failed\n", acc_path);
		    if (strncmp(psu_statep, PSU_STATE_ERROR, strlen(PSU_STATE_ERROR)) != 0) {
			strcpy(psu_statep, PSU_STATE_ERROR);
			SYSFS_LOG("%s: %s\n",psu_dev_group[i].psu_name,PSU_STATE_ERROR);
		    }
		    flag = 1;
		}
		else
		if (strstr(state, "normal")) {
		    //printk(KERN_INFO "%s: %s\n", psu_dev_group[i].psu_name, state);
		    if (strncmp(psu_statep, state, strlen(state)) != 0) {
			strcpy(psu_statep, state);
			SYSFS_LOG("%s: %s\n",psu_dev_group[i].psu_name,state);
		    }
		}
		else
#if 0
		if (strstr(state, "fault")) {
		    /* Bug2371: Do not show "4 : fault" in psu cable unstable moment */
		    continue;
		}
		else
#endif
		if (psu_voltin > PSU_VOLTIN_ACDC) {	/* AC PSUS */
		    if (strncmp(psu_statep, state, strlen(state)) != 0) {
			strcpy(psu_statep, state);
			SYSFS_LOG("%s: %s\n",psu_dev_group[i].psu_name,state);
		    }
		    flag = 1;
		}
		else {					/* DC PSUS */
		    if (strncmp(psu_statep, PSU_STATE_CHECKPSU, PSU_STATE_LEN_CHECKPSU) != 0) {
			strcpy(psu_statep, PSU_STATE_CHECKPSU);
			SYSFS_LOG("%s: %s\n",psu_dev_group[i].psu_name,PSU_STATE_CHECKPSU);
		    }
		    flag = 1;
		}
	    }
	}
    }

    if (flag == 1) {
	status_led_change(STATUS_LED_RED_PATH, "0", STATUS_LED_GRN_PATH, "3");
	return 1;
    }
    return 0;
}
EXPORT_SYMBOL(psus_control);

/* INV drivers mapping */
static ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    char *invpathp, *invwirep;
    char acc_path[64] = "ERR";

    /* check psu device name table to get path */
    invpathp = psu_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));
    if (!invpathp) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* To get state path */
    if (strncmp(attr_p->attr.name, "state", 5) == 0 ||
        strncmp(attr_p->attr.name, "vendor", 6) == 0 ||
	strncmp(attr_p->attr.name, "version", 7) == 0 ||
	strncmp(attr_p->attr.name, "sn", 2) == 0 ||
	strncmp(attr_p->attr.name, "psu_currentin", 13) == 0 ||
	strncmp(attr_p->attr.name, "psu_currentout", 14) == 0 ||
        strncmp(attr_p->attr.name, "psu_powerin", 11) == 0 ||
	strncmp(attr_p->attr.name, "psu_powerout", 12) == 0 ||
        strncmp(attr_p->attr.name, "psu_voltin", 10) == 0 ||
	strncmp(attr_p->attr.name, "psu_voltout", 11) == 0 ||
        strncmp(attr_p->attr.name, "psu_pwm", 7) == 0 ||
	strncmp(attr_p->attr.name, "psu_rpm", 7) == 0 ||
	strncmp(attr_p->attr.name, "fan_faulty", 10) == 0 ||
	strncmp(attr_p->attr.name, "fan_pwm", 7) == 0 ||
	strncmp(attr_p->attr.name, "fan_speed", 9) == 0 ||
	strncmp(attr_p->attr.name, "temperature", 11) == 0) {
	invwirep = psu_attr_get_wirep(attr_p->attr.name, dev_p->kobj.name, NULL);
	if (invwirep == NULL) {
            return sprintf(buf_p, "Invalid psu number: %s\n", dev_p->kobj.name);
	}
	sprintf(acc_path, invpathp, invwirep);
	return inventec_show_attr(&buf_p[0], acc_path);
    }

    return sprintf(buf_p, "Invalid psu attribute name: %s\n", attr_p->attr.name);
}

static ssize_t
show_attr_vendor(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_version(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    char buf[32];
    int i;
    ssize_t count = show_attr_common(dev_p, attr_p, &buf[0]);
    if (count <= 0) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* Bug2297: This is a special case that the version of DC PSU is not burned by vendor */
    for(i = 0; i < strlen(buf)-1; i++) {
	//printk(KERN_INFO "%c %x\n",buf[i],buf[i]);
	if (buf[i] == 0xffffffff) {
	    return sprintf(buf_p, "%s\n", SHOW_ATTR_NOTPRINT);
	}
    }

    return sprintf(buf_p, "%s", buf);
}

static ssize_t
show_attr_sn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    char buf[32];
    int i;
    ssize_t count = show_attr_common(dev_p, attr_p, &buf[0]);
    if (count <= 0) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* Bug2297: This is a special case that the sn of DC PSU is not burned by vendor */
    for(i = 0; i < strlen(buf)-1; i++) {
	//printk(KERN_INFO "%c %x\n",buf[i],buf[i]);
	if (buf[i] == 0xffffffff) {
	    return sprintf(buf_p, "%s\n", SHOW_ATTR_NOTPRINT);
	}
    }

    return sprintf(buf_p, "%s", buf);
}

static ssize_t
show_attr_state(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    char buf[32];
    ssize_t count = show_attr_common(dev_p, attr_p, &buf[0]);
    if (count <= 0) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    if (strstr(buf, "normal")) {
	return sprintf(buf_p, "%s", buf);
    }

    if (psu_voltin > PSU_VOLTIN_ACDC) {
	/* AC PSU */
	return sprintf(buf_p, "%s", buf);
    }

    /* DC PSU */
    return sprintf(buf_p, "%s\n", PSU_STATE_CHECKPSU);
}

static ssize_t
show_attr_temperature(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    char buf[32], temp[32];
    ssize_t count = show_attr_common(dev_p, attr_p, &buf[0]);
    if (count <= 0) {
        return sprintf(buf_p, "0\n");
    }

    memset(temp, 0, 32);
    if (count > 0) {
	int n = strlen(buf);
        if (n < 1) {
           strcpy(temp, "0.000");
	}
	else if (n < 2) {
           strcpy(temp, "0.000");
           strcat(temp, buf);
	}
	else if (n < 3) {
           strcpy(temp, "0.00");
           strcat(temp, buf);
	}
	else if (n < 4) {
           strcpy(temp, "0.0");
           strcat(temp, buf);
	}
	else if (n < 5) {
           strcpy(temp, "0.");
           strcat(temp, buf);
	}
	else {
           strncpy(temp, buf, n-4);
           strcat(temp, ".");
           strcat(temp, &buf[n-4]);
	}
	return sprintf(buf_p, "%s", temp);
    }
    return sprintf(buf_p, "Invalid value: %s\n", buf);
}

static ssize_t
show_attr_fan_speed(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_fan_pwm(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_fan_faulty(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    char buf[32];
    int speed;
    ssize_t count;

    count = show_attr_common(dev_p, attr_p, &buf[0]);
    if (count <= 0) {
        return sprintf(buf_p, "0\n");
    }

    speed = simple_strtol(&buf[0], NULL, 10);
    if (speed > 30) {				// rpm > 100
        return sprintf(buf_p, "1\n");
    }
    return sprintf(buf_p, "0\n");
}

static ssize_t
show_attr_psu_currentin(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_currentout(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_powerin(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_powerout(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_voltin(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_voltout(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_pwm(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_psu_rpm(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

#if 0
static ssize_t
store_attr_common(struct device *dev_p, struct device_attribute *attr_p, const char *buf_p, size_t count)
{
    char acc_path[64] = "ERR";
    char *invpathp = NULL;

    /* To get state path */
    if (strncmp(attr_p->attr.name, "state", 5) == 0) {
	invpathp = psu_attr_get_wirep(attr_p->attr.name, dev_p->kobj.name, NULL);
	if (invpathp == NULL) {
            printk(KERN_INFO "Invalid psu number: %s\n", dev_p->kobj.name);
            return 0;
	}
	sprintf(acc_path, "%s", invpathp);
        return inventec_store_attr(buf_p, count, acc_path);
    }

#if 0
    /* If add new any writable attributes here */
    invpathp = psu_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));
    if (!invpathp) {
        printk(KERN_INFO "%s\n", SHOW_ATTR_WARNING);
        return 0;
    }

    if (strncmp(attr_p->attr.name, "XXX", XXX_length) == 0) {
	sprintf(acc_path, invpathp, dev_p->kobj.name);
        return inventec_store_attr(buf_p, count, acc_path);
    }
#endif

    printk(KERN_INFO "Invalid psu attribute name: %s\n", attr_p->attr.name);
    return 0;
}

static ssize_t
store_attr_state(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, buf_p, count);
}
#endif

static DEVICE_ATTR(vendor,		S_IRUGO,	 show_attr_vendor,		NULL);
static DEVICE_ATTR(version,		S_IRUGO,	 show_attr_version,		NULL);
static DEVICE_ATTR(sn,			S_IRUGO,	 show_attr_sn,			NULL);
static DEVICE_ATTR(state,		S_IRUGO,	 show_attr_state,		NULL);
static DEVICE_ATTR(temperature,		S_IRUGO,	 show_attr_temperature,		NULL);
static DEVICE_ATTR(fan_speed,		S_IRUGO,	 show_attr_fan_speed,		NULL);
static DEVICE_ATTR(fan_pwm,		S_IRUGO,	 show_attr_fan_pwm,		NULL);
static DEVICE_ATTR(fan_faulty,		S_IRUGO,	 show_attr_fan_faulty,		NULL);
static DEVICE_ATTR(psu_currentin,	S_IRUGO,	 show_attr_psu_currentin,	NULL);
static DEVICE_ATTR(psu_currentout,	S_IRUGO,	 show_attr_psu_currentout,	NULL);
static DEVICE_ATTR(psu_powerin,		S_IRUGO,	 show_attr_psu_powerin,		NULL);
static DEVICE_ATTR(psu_powerout,	S_IRUGO,	 show_attr_psu_powerout,	NULL);
static DEVICE_ATTR(psu_voltin,		S_IRUGO,	 show_attr_psu_voltin,		NULL);
static DEVICE_ATTR(psu_voltout,		S_IRUGO,	 show_attr_psu_voltout,		NULL);
static DEVICE_ATTR(psu_pwm,		S_IRUGO,	 show_attr_psu_pwm,		NULL);
static DEVICE_ATTR(psu_rpm,		S_IRUGO,	 show_attr_psu_rpm,		NULL);

static int
psu_register_attrs(struct device *device_p)
{
	char *err_msg = "ERR";

	if (device_create_file(device_p, &dev_attr_vendor) < 0) {
		err_msg = "reg_attr vendor fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_version) < 0) {
		err_msg = "reg_attr version fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_sn) < 0) {
		err_msg = "reg_attr sn fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_state) < 0) {
		err_msg = "reg_attr state fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_temperature) < 0) {
		err_msg = "reg_attr temperature fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_fan_speed) < 0) {
		err_msg = "reg_attr fan_speed fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_fan_pwm) < 0) {
		err_msg = "reg_attr fan_pwm fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_fan_faulty) < 0) {
		err_msg = "reg_attr fan_faulty fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_currentin) < 0) {
		err_msg = "reg_attr psu_currentin fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_currentout) < 0) {
		err_msg = "reg_attr psu_currentout fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_powerin) < 0) {
		err_msg = "reg_attr psu_powerin fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_powerout) < 0) {
		err_msg = "reg_attr psu_powerout fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_voltin) < 0) {
		err_msg = "reg_attr psu_voltin fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_voltout) < 0) {
		err_msg = "reg_attr psu_voltout fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_pwm) < 0) {
		err_msg = "reg_attr psu_pwm fail";
		goto err_psu_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_psu_rpm) < 0) {
		err_msg = "reg_attr psu_rpm fail";
		goto err_psu_register_attrs;
	}
	return 0;

err_psu_register_attrs:
	printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
	return -1;
}

static void
psu_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_vendor);
	device_remove_file(device_p, &dev_attr_version);
	device_remove_file(device_p, &dev_attr_sn);
	device_remove_file(device_p, &dev_attr_state);
	device_remove_file(device_p, &dev_attr_fan_speed);
	device_remove_file(device_p, &dev_attr_fan_pwm);
	device_remove_file(device_p, &dev_attr_fan_faulty);
	device_remove_file(device_p, &dev_attr_psu_currentin);
	device_remove_file(device_p, &dev_attr_psu_currentout);
	device_remove_file(device_p, &dev_attr_psu_powerin);
	device_remove_file(device_p, &dev_attr_psu_powerout);
	device_remove_file(device_p, &dev_attr_psu_voltin);
	device_remove_file(device_p, &dev_attr_psu_voltout);
	device_remove_file(device_p, &dev_attr_psu_pwm);
	device_remove_file(device_p, &dev_attr_psu_rpm);
}

static int
psu_register_device(int psu_id)
{
    char *err_msg	= "ERR";
    int path_len	= 64;
    void		*curr_tracking;
    psu_dev_group_t *group = &psu_dev_group[psu_id];

    /* Prepare INV driver access path */
    group->psu_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->psu_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_psu_register_device_1;
    }
    curr_tracking = group->psu_tracking;
    /* Creatr Sysfs device object */
    group->psu_dev_p = inventec_child_device_create(psu_device_p,
                                       group->psu_devt,
                                       group->psu_inv_pathp,
                                       group->psu_name,
                                       curr_tracking);

    if (IS_ERR(group->psu_dev_p)){
        err_msg = "device_create fail";
        goto err_psu_register_device_2;
    }

    if (psu_register_attrs(group->psu_dev_p) < 0){
        goto err_psu_register_device_3;
    }
    return 0;

err_psu_register_device_3:
    psu_unregister_attrs(group->psu_dev_p);
    inventec_child_device_destroy(group->psu_tracking);
err_psu_register_device_2:
    kfree(group->psu_inv_pathp);
err_psu_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
psu_unregister_device(int psu_id)
{
    psu_dev_group_t *group = &psu_dev_group[psu_id];

    psu_unregister_attrs(group->psu_dev_p);
    inventec_child_device_destroy(group->psu_tracking);
    kfree(group->psu_inv_pathp);
}

static int
psu_device_create(void)
{
    char *err_msg  = "ERR";
    psu_dev_group_t *group;
    int psu_id;

    if (alloc_chrdev_region(&psu_device_devt, 0, 1, PSU_DEVICE_NAME) < 0) {
        err_msg = "alloc_chrdev_region fail!";
        goto err_psu_device_create_1;
    }

    /* Creatr Sysfs device object */
    psu_device_p = device_create(inventec_class_p,	/* struct class *cls     */
                             NULL,			/* struct device *parent */
			     psu_device_devt,		/* dev_t devt */
                             NULL,			/* void *private_data    */
                             PSU_DEVICE_NAME);		/* const char *fmt       */
    if (IS_ERR(psu_device_p)){
        err_msg = "device_create fail";
        goto err_psu_device_create_2;
    }

    /* portX interface tree create */
    for (psu_id = 0; psu_id < PSU_DEV_GROUP_TOTAL; psu_id++) {
        group = &psu_dev_group[psu_id];

	if (alloc_chrdev_region(&group->psu_devt, 0, group->psu_dev_total, group->psu_name) < 0) {
            err_msg = "portX alloc_chrdev_region fail!";
            goto err_psu_device_create_4;
        }
	group->psu_major = MAJOR(group->psu_devt);
    }

    for (psu_id = 0; psu_id < PSU_DEV_GROUP_TOTAL; psu_id++) {
        group = &psu_dev_group[psu_id];

        if (psu_register_device(psu_id) < 0){
            err_msg = "register_test_device fail";
            goto err_psu_device_create_5;
        }
    }
    return 0;

err_psu_device_create_5:
    while (--psu_id >= 0) {
        psu_unregister_device(psu_id);
    }
    psu_id = PSU_DEV_GROUP_TOTAL;
err_psu_device_create_4:
    while (--psu_id >= 0) {
        group = &psu_dev_group[psu_id];
        unregister_chrdev_region(group->psu_devt, group->psu_dev_total);
    }
    device_destroy(inventec_class_p, psu_device_devt);
err_psu_device_create_2:
    unregister_chrdev_region(psu_device_devt, 1);
err_psu_device_create_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
psu_device_destroy(void)
{
    psu_dev_group_t *group;
    int psu_id;

    for (psu_id = 0; psu_id < PSU_DEV_GROUP_TOTAL; psu_id++) {
        psu_unregister_device(psu_id);
    }
    for (psu_id = 0; psu_id < PSU_DEV_GROUP_TOTAL; psu_id++) {
        group = &psu_dev_group[psu_id];
        unregister_chrdev_region(group->psu_devt, group->psu_dev_total);
    }
    device_destroy(inventec_class_p, psu_device_devt);
    unregister_chrdev_region(psu_device_devt, 1);
}

/*
 * Start of psuinfo_device Jul 5, 2016
 *
 * Inventec Platform Proxy Driver for platform psuinfo devices.
 *
 */
typedef struct {
	char *info_dev_namep;
	char *info_dev_pathp;
} psuinfo_dev_t;

typedef struct {
	const char	*psuinfo_name;
	int		psuinfo_major;
        dev_t		psuinfo_devt;
	struct device	*psuinfo_dev_p;
	void		*psuinfo_tracking;
	psuinfo_dev_t	*psuinfo_dev_namep;
	int		psuinfo_dev_total;
	char		*psuinfo_inv_pathp;
} psuinfo_dev_group_t;

static psuinfo_dev_t psuinfo_dev_name[] = {
	{ "psu_number",	NULL },
};

static psuinfo_dev_group_t psuinfo_dev_group[] = {
	{
		.psuinfo_name		= PSUINFO_DEVICE_NAME,
		.psuinfo_major		= 0,
		.psuinfo_devt		= 0L,
		.psuinfo_dev_p		= NULL,
		.psuinfo_tracking	= NULL,
		.psuinfo_dev_namep	= &psuinfo_dev_name[0],
		.psuinfo_dev_total	= sizeof(psuinfo_dev_name) / sizeof(const psuinfo_dev_t),
		.psuinfo_inv_pathp	= NULL,
	},
};
#define PSUINFO_DEV_TOTAL	( sizeof(psuinfo_dev_group)/ sizeof(const psuinfo_dev_group_t) )

static ssize_t
show_attr_psu_number(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return sprintf(buf_p, "%lu\n", PSU_DEV_GROUP_TOTAL);
}

/* INV drivers mapping */
static DEVICE_ATTR(psu_number,	S_IRUGO,	show_attr_psu_number,	NULL);

static int
psuinfo_register_attrs(struct device *device_p)
{
    if (device_create_file(device_p, &dev_attr_psu_number) < 0) {
        goto err_psuinfo_register_attrs;
    }
    return 0;

err_psuinfo_register_attrs:
    printk(KERN_ERR "[%s/%d] device_create_file() fail\n",__func__,__LINE__);
    return -1;
}

static void
psuinfo_unregister_attrs(struct device *device_p)
{
    device_remove_file(device_p, &dev_attr_psu_number);
}

static int
psuinfo_register_device(psuinfo_dev_group_t *group)
{
    int path_len	= 64;
    char *err_msg	= "ERR";

    /* Prepare INV driver access path */
    group->psuinfo_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->psuinfo_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_psuinfo_register_device_1;
    }
    /* Get Sysfs device object */
    group->psuinfo_dev_p = psu_device_p;
    if (!group->psuinfo_dev_p) {
        err_msg = "psus_get_psu_devp() fail";
        goto err_psuinfo_register_device_2;
    }
    if (psuinfo_register_attrs(group->psuinfo_dev_p) < 0){
        goto err_psuinfo_register_device_3;
    }
    return 0;

err_psuinfo_register_device_3:
    device_destroy(inventec_class_p, group->psuinfo_devt);
err_psuinfo_register_device_2:
    kfree(group->psuinfo_inv_pathp);
err_psuinfo_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
psuinfo_unregister_device(psuinfo_dev_group_t *group)
{
    psuinfo_unregister_attrs(group->psuinfo_dev_p);
    device_destroy(inventec_class_p, group->psuinfo_devt);
    kfree(group->psuinfo_inv_pathp);
}

static int
psuinfo_device_create(void)
{
    char *err_msg  = "ERR";
    psuinfo_dev_group_t *group;
    int psuinfo_id;

    for (psuinfo_id = 0; psuinfo_id < PSUINFO_DEV_TOTAL; psuinfo_id++) {
        group = &psuinfo_dev_group[psuinfo_id];

        if (alloc_chrdev_region(&group->psuinfo_devt, 0, group->psuinfo_dev_total, group->psuinfo_name) < 0) {
            err_msg = "alloc_chrdev_region fail!";
            goto err_psuinfo_device_init_1;
        }
        group->psuinfo_major = MAJOR(group->psuinfo_devt);
    }

    for (psuinfo_id = 0; psuinfo_id < PSUINFO_DEV_TOTAL; psuinfo_id++) {
        group = &psuinfo_dev_group[psuinfo_id];

        /* Register device number */
        if (psuinfo_register_device(group) < 0){
            err_msg = "psuinfo_register_device fail";
            goto err_psuinfo_device_init_2;
        }
    }
    return 0;

err_psuinfo_device_init_2:
    while (--psuinfo_id >= 0) {
        group = &psuinfo_dev_group[psuinfo_id];
        psuinfo_unregister_device(group);
    }
    psuinfo_id = PSUINFO_DEV_TOTAL;
err_psuinfo_device_init_1:
    while (--psuinfo_id >= 0) {
        group = &psuinfo_dev_group[psuinfo_id];
        unregister_chrdev_region(group->psuinfo_devt, group->psuinfo_dev_total);
    }
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void psuinfo_device_destroy(void)
{
    int psuinfo_id;
    psuinfo_dev_group_t *group;

    for (psuinfo_id = 0; psuinfo_id < PSUINFO_DEV_TOTAL; psuinfo_id++) {
        group = &psuinfo_dev_group[psuinfo_id];
        psuinfo_unregister_device(group);
    }
    for (psuinfo_id = 0; psuinfo_id < PSUINFO_DEV_TOTAL; psuinfo_id++) {
        group = &psuinfo_dev_group[psuinfo_id];
        unregister_chrdev_region(group->psuinfo_devt, group->psuinfo_dev_total);
    }
}

static int
psuinfo_device_init(void)
{
    char *err_msg  = "ERR";

    if (psuinfo_device_create() < 0){
        err_msg = "psuinfo_device_create() fail!";
        goto err_psuinfo_device_init;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_psuinfo_device_init:
    printk(KERN_ERR "Module initial failure. %s\n",err_msg);
    return -1;
}


static void
psuinfo_device_exit(void)
{
    psuinfo_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}

/* End of psuinfo_device */

static int __init
psu_device_init(void)
{
    char *err_msg  = "ERR";
    int psu_id;

    inventec_class_p = inventec_get_class();
    if (!inventec_class_p) {
        err_msg = "inventec_get_class() fail!";
        goto err_psu_device_init_1;
    }

    for (psu_id = 0; psu_id < PSU_DEV_GROUP_TOTAL; psu_id++) {
        psu_dev_group[psu_id].psu_major = 0;
        psu_dev_group[psu_id].psu_devt = 0L;
        psu_dev_group[psu_id].psu_dev_p = NULL;
        psu_dev_group[psu_id].psu_inv_pathp = NULL;
        psu_dev_group[psu_id].psu_tracking = NULL;
    }

    if (psu_device_create() < 0){
        err_msg = "psu_device_create() fail!";
        goto err_psu_device_init_1;
    }

    if (psuinfo_device_init() < 0) {
        err_msg = "psuinfo_device_init() fail!";
        goto err_psu_device_init_2;
    }

    SYSFS_LOG("Module initial success.\n");

    return 0;

err_psu_device_init_2:
    psu_device_destroy();
err_psu_device_init_1:
    printk(KERN_ERR "Module initial failure. %s\n",err_msg);
    return -1;
}


static void __exit
psu_device_exit(void)
{
    psuinfo_device_exit();
    psu_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}


MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);

module_init(psu_device_init);
module_exit(psu_device_exit);
