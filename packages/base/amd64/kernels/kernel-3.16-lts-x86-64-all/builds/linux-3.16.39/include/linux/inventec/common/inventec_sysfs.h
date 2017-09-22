#ifndef _INVENTEC_SYSFS_H_
#define _INVENTEC_SYSFS_H_

/* Module informations */
#define INV_SYSFS_AUTHOR    "Robert <yu.robertxk@inventec.com>"
#define INV_SYSFS_DESC      "Inventec Platform Sysfs System"
#define INV_SYSFS_LICENSE   "GPL"
#define INV_SYSFS_VERSION   "version 2.0.5"
#define INV_SYSFS_RL_DATE   "July 7, 2017"

#define INVENTEC_CLASS_NAME	"inventec"
#define SYSTEM_DEVICE_NAME	"system"
#define PORTS_DEVICE_NAME	"ports"
#define PORTSINFO_DEVICE_NAME	"portsinfo"
#define PSU_DEVICE_NAME		"psus"
#define PSUINFO_DEVICE_NAME	"psuinfo"
#define LED_DEVICE_NAME		"leds"
#define LEDINFO_DEVICE_NAME	"ledinfo"
#define FAN_DEVICE_NAME		"fans"
#define FANINFO_DEVICE_NAME	"faninfo"
#define SENSOR_DEVICE_NAME	"sensors"
#define SENSORINFO_DEVICE_NAME	"sensorinfo"

#if 0
//#define SYSFS_LOG(fmt, args...) printk(KERN_WARNING "[SYSFS] %s/%d: " fmt, __func__, __LINE__, ##args)
#define SYSFS_LOG(fmt, args...) printk(KERN_WARNING "[SYSFS] " fmt, ##args)
#else
/* For timestamps in SYSFS_LOG */
extern void SYSFS_LOG(char *fmt, ...);
#endif
#define SHOW_ATTR_WARNING	("N/A")
#define SHOW_ATTR_NOTPRINT	("Not Available")

#define INV_MAX_LEN_CHAR_STRING	(32)
#define INV_MAX_LEN_SYS_VERSION	(3)

/*
 * Platform Identifier for system build
 * Only one turned on
 *
#define PLATFORM_MAGNOLIA
#define PLATFORM_REDWOOD
#define PLATFORM_CYPRESS
#define PLATFORM_SPRUCE
 *
 * For yocto/redwood image only
#define YOCTO_REDWOOD
 */
#define PLATFORM_REDWOOD

#if defined PLATFORM_MAGNOLIA
/*
 * Magnolia ports
 * 48 SFP ports and 6 QSFP ports
 * port number in inventec /sys/class/swps is 0 based.
 * We convert it to 1 based in inventec sysfs
 */
#define PORT_SFP_DEV_START	(0)
#define PORT_SFP_DEV_TOTAL	(PORT_SFP_DEV_START + 48)
#define PORT_QSFP_DEV_START	(PORT_SFP_DEV_TOTAL)
#define PORT_QSFP_DEV_TOTAL	(PORT_QSFP_DEV_START + 6)
#define PORTS_NUMBER_TOTAL	(PORT_QSFP_DEV_TOTAL - PORT_SFP_DEV_START)

#define SYSFS_SFP_DEV_START	(1)
#define SYSFS_SFP_DEV_END	(PORT_SFP_DEV_TOTAL)
#define SYSFS_QSFP_DEV_START	(PORT_QSFP_DEV_START + 1)
#define SYSFS_QSFP_DEV_END	(PORT_QSFP_DEV_TOTAL)
#define SYSFS_NUMBER_TOTAL	(PORTS_NUMBER_TOTAL)

#define INV_CPLD_NUMBER		1
//TOBE MOVED #define INV_MAG_PSU_MODEL	"DPS-460DB-9A"
//#define INV_PSU_MODULE_NAME	"DPS-460DB-9A"

#elif defined PLATFORM_REDWOOD
/*
 * Redwood ports
 * 0 SFP ports and 32 QSFP ports
 * port number in inventec /sys/class/swps is 0 based.
 * We convert it to 1 based in inventec sysfs
 */
#define PORT_SFP_DEV_START	(0)
#define PORT_SFP_DEV_TOTAL	(PORT_SFP_DEV_START + 0)
#define PORT_QSFP_DEV_START	(PORT_SFP_DEV_TOTAL)
#define PORT_QSFP_DEV_TOTAL	(PORT_QSFP_DEV_START + 32)
#define PORTS_NUMBER_TOTAL	(PORT_QSFP_DEV_TOTAL - PORT_SFP_DEV_START)

#define SYSFS_SFP_DEV_START	(1)
#define SYSFS_SFP_DEV_END	(PORT_SFP_DEV_TOTAL)
#define SYSFS_QSFP_DEV_START	(PORT_QSFP_DEV_START + 1)
#define SYSFS_QSFP_DEV_END	(PORT_QSFP_DEV_TOTAL)
#define SYSFS_NUMBER_TOTAL	(PORTS_NUMBER_TOTAL)

#define INV_CPLD_NUMBER		1
#define INV_PSU_MODULE_NAME	"Redwood PSU Model Name"

#elif defined PLATFORM_CYPRESS
/*
 * Cypress ports
 */
TBD
#define INV_PSU_MODULE_NAME	"Cypress PSU Model Name"
#endif

typedef struct dev_child_tracking_s {
	dev_t devt;
	struct device *dev;
	struct dev_child_tracking_s *next;
} dev_child_tracking_t;

extern struct class *inventec_get_class(void);
extern struct device *inventec_child_device_create(struct device *parent,
              dev_t devt, void *drvdata, const char *fmt, void *tracking);
extern void inventec_child_device_destroy(void *tracking);

extern ssize_t inventec_show_attr(char *buf_p, const char *invdevp);
extern ssize_t inventec_store_attr(const char *buf_p, size_t count, const char *invdevp);
extern ssize_t inventec_show_bin_attr(char *buf_p, char *invdevp, int pos);
extern ssize_t inventec_show_attr_pos(char *buf_p, char *invdevp, int pos);

/*
 * LED definitions
 */
#define STATUS_LED_MODE_AUTO	0
#define STATUS_LED_MODE_DIAG	1
#define STATUS_LED_MODE_MANU	2

#define STATUS_LED_GRN0		10      // 0 - 000: off
#define STATUS_LED_GRN1		11      // 1 - 001: 0.5hz
#define STATUS_LED_GRN2		12      // 2 - 010: 1 hz
#define STATUS_LED_GRN3		13      // 3 - 011: 2 hz
#define STATUS_LED_GRN7		17      // 7 - 111: on
#define STATUS_LED_RED0		20      // 0 - 000: off
#define STATUS_LED_RED1		21      // 1 - 001: 0.5hz
#define STATUS_LED_RED2		22      // 2 - 010: 1 hz
#define STATUS_LED_RED3		23      // 3 - 011: 2 hz
#define STATUS_LED_RED7		27      // 7 - 111: on
#define STATUS_LED_INVALID	0	// Invalid

#define STATUS_LED_GRN_PATH	"/sys/class/hwmon/hwmon1/device/grn_led"
#define STATUS_LED_RED_PATH	"/sys/class/hwmon/hwmon1/device/red_led"

#define FAN_LED_GRN1_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_grn1"
#define FAN_LED_GRN2_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_grn2"
#define FAN_LED_GRN3_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_grn3"
#define FAN_LED_GRN4_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_grn4"
#define FAN_LED_RED1_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_red1"
#define FAN_LED_RED2_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_red2"
#define FAN_LED_RED3_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_red3"
#define FAN_LED_RED4_PATH	"/sys/class/hwmon/hwmon0/device/fan_led_red4"

#define HWMON_DEVICE_DIAG_PATH	"/sys/class/hwmon/hwmon0/device/diag"
#define HWMON_DEVICE_CTL_PATH	"/sys/class/hwmon/hwmon1/device/ctl"

extern ssize_t status_led_change(const char *path1, const char *tmp1, const char *path2, const char *tmp2);
extern ssize_t status_led_diag_mode_enable(void);
extern ssize_t status_led_diag_mode_disable(void);
extern int status_led_check_color(void);
extern int status_led_check_diag_mode(void);
extern int psu_check_state_normal(char *statep);
extern int inventec_store_input(char *inputp, int count);
extern int inventec_strtol(const char *sbufp, char **endp, unsigned int base);
#ifdef YOCTO_REDWOOD
extern void led_set_gpio_to_change_status_led(void);
#endif

#endif