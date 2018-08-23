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
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define PREFIX_PATH "/sys/bus/i2c/devices/0-0066/"
#define filename    "brightness"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */
enum onlp_led_id
{
    LED_RESERVED = 0,
    LED_DIAG,
    LED_LOC,
    LED_FAN,
    LED_PSU1,
    LED_PSU2
};
        
enum led_light_mode {
	LED_MODE_OFF = 0,
	LED_MODE_GREEN,
	LED_MODE_AMBER,
	LED_MODE_RED,
	LED_MODE_BLUE,
	LED_MODE_GREEN_BLINK,
	LED_MODE_AMBER_BLINK,
	LED_MODE_RED_BLINK,
	LED_MODE_BLUE_BLINK,
	LED_MODE_AUTO,
	LED_MODE_UNKNOWN
};

typedef struct led_light_mode_map {
    enum onlp_led_id id;
    enum led_light_mode driver_led_mode;
    enum onlp_led_mode_e onlp_led_mode;
} led_light_mode_map_t;

led_light_mode_map_t led_map[] = {
{LED_DIAG, LED_MODE_OFF,   ONLP_LED_MODE_OFF},
{LED_DIAG, LED_MODE_GREEN, ONLP_LED_MODE_GREEN},
{LED_DIAG, LED_MODE_AMBER, ONLP_LED_MODE_ORANGE},
{LED_DIAG, LED_MODE_RED,   ONLP_LED_MODE_RED},
{LED_LOC,  LED_MODE_OFF,   ONLP_LED_MODE_OFF},
{LED_LOC,  LED_MODE_BLUE,  ONLP_LED_MODE_BLUE},
{LED_FAN,  LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
{LED_PSU1, LED_MODE_AUTO,  ONLP_LED_MODE_AUTO},
{LED_PSU2, LED_MODE_AUTO,  ONLP_LED_MODE_AUTO}
};

static char last_path[][10] =  /* must map with onlp_led_id */
{
    "reserved",
    "diag",
    "loc",
    "fan",
    "psu1",
    "psu2"
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_DIAG), "Chassis LED 1 (DIAG LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_LOC), "Chassis LED 2 (LOC LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 3 (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "Chassis LED 4 (PSU1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "Chassis LED 4 (PSU2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_AUTO,
    },
};

#if 0
static int driver_to_onlp_led_mode(enum onlp_led_id id, enum led_light_mode driver_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);
    
    for (i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && driver_led_mode == led_map[i].driver_led_mode)
        {
            return led_map[i].onlp_led_mode;
        }
    }
    
    return 0;
}
#endif

static int onlp_to_driver_led_mode(enum onlp_led_id id, onlp_led_mode_t onlp_led_mode)
{
    int i, nsize = sizeof(led_map)/sizeof(led_map[0]);
    
    for(i = 0; i < nsize; i++)
    {
        if (id == led_map[i].id && onlp_led_mode == led_map[i].onlp_led_mode)
        {
            return led_map[i].driver_led_mode;
        }
    }
    
    return 0;
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    /*
     * Diag LED Off
     */
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_DIAG), ONLP_LED_MODE_OFF);

    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  local_id;
    int  gvalue, rvalue;
    char fullpath[50] = {0};
		
    VALIDATE(id);
	
    local_id = ONLP_OID_ID_GET(id);
    		
    /* get fullpath */
    sprintf(fullpath, "%s%s%d", PREFIX_PATH, "fan_led_grn", local_id);
		
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Set LED mode */
    if (onlp_file_read_int(&gvalue, "%s%s%d", PREFIX_PATH, "fan_led_grn", local_id) != 0) {
        DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (onlp_file_read_int(&rvalue, "%s%s%d", PREFIX_PATH, "fan_led_red", local_id) != 0) {
        DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (gvalue == 1 && rvalue == 0) {
	info->mode = ONLP_LED_MODE_GREEN;
    }
    else
    if (gvalue == 0 && rvalue == 1) {
	info->mode = ONLP_LED_MODE_RED;
    }
    else
    if (gvalue == 1 && rvalue == 1) {
	info->mode = ONLP_LED_MODE_ORANGE;
    }
    else
    if (gvalue == 0 && rvalue == 0) {
	info->mode = ONLP_LED_MODE_OFF;
    }
    else {
        return ONLP_STATUS_E_INTERNAL;
    }

#if 0
    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) {
        info->status |= ONLP_LED_STATUS_ON;
    }
#endif

    return ONLP_STATUS_OK;
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

    if (!on_or_off) {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }

    return ONLP_STATUS_E_UNSUPPORTED;
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
    int  local_id;
    char fullpath[50] = {0};		

    VALIDATE(id);
	
    local_id = ONLP_OID_ID_GET(id);
    sprintf(fullpath, "%s%s/%s", PREFIX_PATH, last_path[local_id], filename);	
    
    if (onlp_file_write_int(onlp_to_driver_led_mode(local_id, mode), fullpath, NULL) != 0)
    {
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}
