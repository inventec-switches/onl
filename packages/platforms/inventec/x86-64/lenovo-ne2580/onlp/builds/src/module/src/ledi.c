/************************************************************
 * ledi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <pthread.h>
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define filename    "brightness"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* devfiles__[LED_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_CPLD_PREFIX"/service_led",
    INV_CPLD_PREFIX"/power_led",
    INV_CPLD_PREFIX"/fan_led",
    INV_CPLD_PREFIX"/stacking_led",
    INV_CPLD_PREFIX"/fanmodule1_led",
    INV_CPLD_PREFIX"/fanmodule2_led",
    INV_CPLD_PREFIX"/fanmodule3_led",
    INV_CPLD_PREFIX"/fanmodule4_led",
    INV_CPLD_PREFIX"/fanmodule5_led",
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[LED_MAX] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "SERVICE LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE | ONLP_LED_CAPS_BLUE_BLINKING ,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_POWER), "POWER LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Fan LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_STACKING), "STACKING LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Fan Module LED 1", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Fan Module LED 2", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN3), "Fan Module LED 3", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN4), "Fan Module LED 4", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN5), "Fan Module LED 5", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
	ONLP_LED_MODE_ON, '0',
    },
};

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
static pthread_mutex_t diag_mutex;
static int onlp_chassis_led_diag_disable(void);

int
onlp_ledi_init(void)
{
    pthread_mutex_init(&diag_mutex, NULL);

    /*
     * Diag LED Off
     */
    onlp_chassis_led_diag_disable();

    return ONLP_STATUS_OK;
}

int onlp_chassis_led_read(char *pathp, char *buf, size_t len)
{
   FILE * fp;

   fp = fopen (pathp, "r");
   if(fp == NULL) {
      perror("Error opening file");
      return(-1);
   }
   if( fgets (buf, len, fp) == NULL ) {
      perror("Error fgets operation");
   }
   fclose(fp);
   
   return(0);
}

int onlp_chassis_led_write(char *pathp, char *buf)
{
   FILE * fp;

   fp = fopen (pathp, "w");
   if(fp == NULL) {
      perror("Error opening file");
      return(-1);
   }
   if( fputs (buf, fp) == 0 ) {
      perror("Error fputs operation");
   }
   fclose(fp);
   
   return(0);
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  local_id, mret = 0;
    char led_fullpath[50] = {0};
    int  mode_value = 0;
    char mbuf[32] = {0};

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Raed LED mode */
    sprintf(led_fullpath, devfiles__[local_id]);

    mret = onlp_chassis_led_read(led_fullpath, mbuf, 32);
    if (mret < 0) {
 	DEBUG_PRINT("%s(%d): mret = %d\r\n", __FUNCTION__, __LINE__, mret);
	info->mode = ONLP_LED_MODE_OFF;
	info->status = ONLP_LED_STATUS_FAILED;
	return ONLP_STATUS_OK;
    }
    mode_value = mbuf[0] - '0';
    /* get fullpath */
    switch (local_id) {
	case LED_SYS:
	case LED_POWER:
	case LED_FAN:
	case LED_STACKING:
	    info->status = ONLP_LED_STATUS_PRESENT;
	    if (mode_value == 0) {
		info->mode = ONLP_LED_MODE_OFF;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (mode_value == 1) {
		info->mode = ONLP_LED_MODE_ON;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (mode_value == 2) {
		info->mode = ONLP_LED_MODE_BLINKING;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (mode_value == 3) {
		info->mode = ONLP_LED_MODE_BLINKING;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else {
		info->mode = ONLP_LED_MODE_OFF;
		info->status &= ~ONLP_LED_STATUS_PRESENT;
		info->status |= ONLP_LED_STATUS_FAILED;
	    }
	    break;
	case LED_FAN1:
	case LED_FAN2:
	case LED_FAN3:
	case LED_FAN4:
	case LED_FAN5:
	    info->status = ONLP_LED_STATUS_PRESENT;
	    if (mode_value == 0) {
		info->mode = ONLP_LED_MODE_OFF;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (mode_value == 1) {
		info->mode = ONLP_LED_MODE_GREEN;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (mode_value == 2) {
		info->mode = ONLP_LED_MODE_RED;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else
	    if (mode_value == 3) {
		info->mode = ONLP_LED_MODE_AUTO;
		info->status |= ONLP_LED_STATUS_ON;
	    }
	    else {
		info->mode = ONLP_LED_MODE_OFF;
		info->status &= ~ONLP_LED_STATUS_PRESENT;
		info->status |= ONLP_LED_STATUS_FAILED;
	    }
	    break;
	default:
	    DEBUG_PRINT("%s(%d) Invalid led id %d\r\n", __FUNCTION__, __LINE__, local_id);
	    break;
    }
		
    return ONLP_STATUS_OK;
}

int onlp_ledi_status_get(onlp_oid_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_led_info_t linfo;

    result = onlp_ledi_info_get(id, &linfo);
    *rv = linfo.status;

    return result;
}

/*
 * Turn an LED on or off.
 *
 * This function will only be called if the LED OID supports the ONOFF
 * capability.
 *
 * What 'on' means in terms of colors or modes for multimode LEDs is
 * up to the platform to decide. This is intended as baseline toggle mechanism.
 */
int
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    VALIDATE(id);
    switch (on_or_off) {
    case ONLP_LED_MODE_OFF:
    case ONLP_LED_MODE_ON:
    case ONLP_LED_MODE_BLUE:
    case ONLP_LED_MODE_BLUE_BLINKING:
    case ONLP_LED_MODE_GREEN:
    case ONLP_LED_MODE_GREEN_BLINKING:
        return onlp_ledi_mode_set(id, on_or_off);
	break;
    default:
	break;
    }

    return ONLP_STATUS_E_UNSUPPORTED;
}

#define HWMON_CPLD_CTRL_PATH    "/sys/bus/i2c/devices/i2c-0/0-0077/diag"
#define HWMON_DEVICE_DIAG_PATH  HWMON_CPLD_CTRL_PATH

#define MIN_ACC_SIZE	(32)

/*
 * Store attr Section
 */
static int onlp_chassis_led_diag_enable(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = onlp_chassis_led_read(HWMON_DEVICE_DIAG_PATH, &tmp[0], MIN_ACC_SIZE);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '1') {
	return ret;
    }

    pthread_mutex_lock(&diag_mutex);
    ret = onlp_chassis_led_write(HWMON_DEVICE_DIAG_PATH, "1");
    pthread_mutex_unlock(&diag_mutex);

    return ret;
}

static int onlp_chassis_led_diag_disable(void)
{
    char tmp[MIN_ACC_SIZE];
    ssize_t ret;

    ret = onlp_chassis_led_read(HWMON_DEVICE_DIAG_PATH, &tmp[0], MIN_ACC_SIZE);
    if (ret <= 0) {
        return ret;
    }

    if (tmp[0] == '0') {
	return ret;
    }

    pthread_mutex_lock(&diag_mutex);
    ret = onlp_chassis_led_write(HWMON_DEVICE_DIAG_PATH, "0");
    pthread_mutex_unlock(&diag_mutex);

    return ret;
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    char fullpath[MIN_ACC_SIZE*2] = {0};
    char sys_buf[MIN_ACC_SIZE] = {0};
    onlp_led_info_t linfo;
    onlp_led_info_t linfo1;
    int ret = -1;
    int local_id;

    VALIDATE(id);
	
    local_id = ONLP_OID_ID_GET(id);

    switch (local_id) {
	case LED_SYS:
	case LED_POWER:
	case LED_FAN:
	case LED_STACKING:
	    ret = onlp_chassis_led_diag_enable();
	    if (ret != 0) {
		return ret;
	    }
	    break;
	default:
	    break;
    }
    sleep(1);

    ret = onlp_ledi_info_get(id, &linfo);
    switch (mode) {
        case ONLP_LED_MODE_ON:
	    if ((ret == ONLP_STATUS_OK) && (linfo.status & ONLP_LED_STATUS_ON) &&
		(linfo.mode == ONLP_LED_MODE_ON)) {
		return ONLP_STATUS_OK;
	    }

	    sprintf(sys_buf, "%d", 0x1);	// LED OFF
	    sprintf(fullpath, devfiles__[local_id]);
	    ret = onlp_chassis_led_write(fullpath, sys_buf);

            break;
        case ONLP_LED_MODE_OFF:
	    if ((ret == ONLP_STATUS_OK) && (linfo.status & ONLP_LED_STATUS_ON) &&
		(linfo.mode == ONLP_LED_MODE_OFF)) {
		return ONLP_STATUS_OK;
	    }

	    sprintf(sys_buf, "%d", 0x0);	// LED OFF
	    sprintf(fullpath, devfiles__[local_id]);
	    ret = onlp_chassis_led_write(fullpath, sys_buf);

            break;
        case ONLP_LED_MODE_BLUE:
	    if (local_id != LED_SYS) {		// Only service LED
		return ONLP_STATUS_OK;
	    }

	    if ((ret == ONLP_STATUS_OK) && (linfo.status & ONLP_LED_STATUS_ON) &&
		(linfo.mode == ONLP_LED_MODE_BLUE)) {
		return ONLP_STATUS_OK;
	    }

	    sprintf(sys_buf, "%d", 0x1);	// BLUE ON
	    sprintf(fullpath, devfiles__[local_id]);
	    ret = onlp_chassis_led_write(fullpath, sys_buf);

    onlp_ledi_info_get(id, &linfo1);
            break;
        case ONLP_LED_MODE_BLUE_BLINKING:
	    if (local_id != LED_SYS) {		// Only service LED
		return ONLP_STATUS_OK;
	    }

	    if ((ret == ONLP_STATUS_OK) && (linfo.status & ONLP_LED_STATUS_ON) &&
		(linfo.mode == ONLP_LED_MODE_BLUE_BLINKING)) {
		return ONLP_STATUS_OK;
	    }

	    sprintf(sys_buf, "%d", 0x2);	// BLUE BLINK
	    sprintf(fullpath, devfiles__[local_id]);
	    ret = onlp_chassis_led_write(fullpath, sys_buf);

            break;
        case ONLP_LED_MODE_GREEN:
	    if (local_id == 1) {		// Except service LED
		return ONLP_STATUS_OK;
	    }

	    if ((ret == ONLP_STATUS_OK) && (linfo.status & ONLP_LED_STATUS_ON) &&
		(linfo.mode == ONLP_LED_MODE_GREEN)) {
		return ONLP_STATUS_OK;
	    }

	    sprintf(sys_buf, "%d", 0x1);	// GREEN ON
	    sprintf(fullpath, devfiles__[local_id]);
	    ret = onlp_chassis_led_write(fullpath, sys_buf);
            break;
        case ONLP_LED_MODE_GREEN_BLINKING:
	    if (local_id == 1) {		// Except service LED
		return ONLP_STATUS_OK;
	    }

	    if ((ret == ONLP_STATUS_OK) && (linfo.status & ONLP_LED_STATUS_ON) &&
		(linfo.mode == ONLP_LED_MODE_GREEN_BLINKING)) {
		return ONLP_STATUS_OK;
	    }

	    sprintf(sys_buf, "%d", 0x2);	// GREEN BLINK
	    sprintf(fullpath, devfiles__[local_id]);
	    ret = onlp_chassis_led_write(fullpath, sys_buf);
            break;
        default:
	    DEBUG_PRINT("%s(%d) Invalid led mode %d\r\n", __FUNCTION__, __LINE__, mode);
	    return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}
