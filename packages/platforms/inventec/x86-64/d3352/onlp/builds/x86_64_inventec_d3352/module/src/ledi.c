/************************************************************
 * ledi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/fani.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

typedef enum sys_led_mode_e {
    SYS_LED_MODE_OFF = 0,
    SYS_LED_MODE_ON = 1,
    SYS_LED_MODE_1_HZ = 2,
    SYS_LED_MODE_0_5_HZ = 3
} sys_led_mode_t;

static char* __led_path_list[LED_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_SYSLED_PREFIX"/stack_led",
    INV_SYSLED_PREFIX"/fan_led",
    INV_SYSLED_PREFIX"/power_led",
    INV_SYSLED_PREFIX"/service_led",
    INV_DEVICE_PREFIX"/fan1_led_%s",
    INV_DEVICE_PREFIX"/fan2_led_%s",
};

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[LED_MAX] = {
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_STK), "Chassis LED (STACKING LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING ,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_PWR), "Chassis LED (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "Chassis LED (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_BLUE| ONLP_LED_CAPS_BLUE_BLINKING,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN1), "Fan LED 1 (FAN1 LED)", ONLP_FAN_ID_CREATE(FAN_1_ON_MAIN_BOARD) },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
        ONLP_LED_MODE_ON, '0',
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN2), "Fan LED 2 (FAN2 LED)", ONLP_FAN_ID_CREATE(FAN_2_ON_MAIN_BOARD) },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_RED,
        ONLP_LED_MODE_ON, '0',
    },
};


/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    return ONLP_STATUS_OK;
}


int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  local_id = ONLP_OID_ID_GET(id);
    char fullpath_grn[ONLP_CONFIG_INFO_STR_MAX] = {0};
    char fullpath_red[ONLP_CONFIG_INFO_STR_MAX] = {0};
    int  gvalue = 0, rvalue = 0;
    int  len,led_state;
    char buf[ONLP_CONFIG_INFO_STR_MAX] = {0};
    uint32_t fan_status;

    VALIDATE(id);

    memset(info, 0, sizeof(onlp_led_info_t));
    *info = linfo[local_id]; /* Set the onlp_oid_hdr_t */

    /* get fullpath */
    switch (local_id) {
    case LED_STK:
        if( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, __led_path_list[local_id]) == ONLP_STATUS_OK ) {
            led_state=(int)(buf[0]-'0');
            if(led_state==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_state==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if(led_state==SYS_LED_MODE_1_HZ) {
                info->mode =ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                printf("[ONLP][ERROR] Not defined LED behavior detected on led@%d\n", local_id);
                return ONLP_STATUS_E_INTERNAL;
            }
        }
        info->status &= ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_OK;
        break;
    case LED_SYS:
        if( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, __led_path_list[local_id]) == ONLP_STATUS_OK ) {
            led_state=(int)(buf[0]-'0');
            if(led_state==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_state==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if(led_state==SYS_LED_MODE_0_5_HZ) {
                info->mode =ONLP_LED_MODE_BLUE_BLINKING;
            } else {
                printf("[ONLP][ERROR] Not defined LED behavior detected on led@%d\n", local_id);
                return ONLP_STATUS_E_INTERNAL;
            }
        }

        info->status &= ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_OK;
        break;
    case LED_FAN:
    case LED_PWR:
        if( onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, __led_path_list[local_id]) == ONLP_STATUS_OK ) {
            led_state=(int)(buf[0]-'0');
            if(led_state==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_state==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if( (led_state==SYS_LED_MODE_1_HZ)||(led_state==SYS_LED_MODE_0_5_HZ) ) {
                info->mode =ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                printf("[ONLP][ERROR] Not defined LED behavior detected on led@%d\n", local_id);
                return ONLP_STATUS_E_INTERNAL;
            }
        }
        info->status &= ONLP_LED_STATUS_PRESENT;
        return ONLP_STATUS_OK;
        break;
    case LED_FAN1:
    case LED_FAN2:
        if( onlp_fani_status_get(info->hdr.poid, &fan_status)!= ONLP_STATUS_OK ) {
            return ONLP_STATUS_E_INTERNAL;
        }
        if( !(fan_status & ONLP_FAN_STATUS_PRESENT) ) {
            info->status=0;
            return ONLP_STATUS_OK;
        }

        sprintf(fullpath_grn, __led_path_list[local_id], "grn");
        sprintf(fullpath_red, __led_path_list[local_id], "red");

        /* Set LED mode */
        if (onlp_file_read_int(&gvalue, fullpath_grn) != 0) {
            DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
            gvalue = 0;
        }
        if (onlp_file_read_int(&rvalue, fullpath_red) != 0) {
            DEBUG_PRINT("%s(%d)\r\n", __FUNCTION__, __LINE__);
            rvalue = 0;
        }

        if (gvalue == 1 && rvalue == 0) {
            info->mode = ONLP_LED_MODE_GREEN;
            info->status |= ONLP_LED_STATUS_PRESENT;
            info->status |= ONLP_LED_STATUS_ON;
        } else if (gvalue == 0 && rvalue == 1) {
            info->mode = ONLP_LED_MODE_RED;
            info->status |= ONLP_LED_STATUS_PRESENT;
            info->status |= ONLP_LED_STATUS_ON;
        } else {
            info->mode = ONLP_LED_MODE_OFF;
            info->status &= ~ONLP_LED_STATUS_PRESENT;
            info->status &= ~ONLP_LED_STATUS_ON;
        }
        break;
    default:
        DEBUG_PRINT("%s(%d) Invalid led id %d\r\n", __FUNCTION__, __LINE__, local_id);
        return ONLP_STATUS_E_INVALID;
        break;
    }

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];
    if (gvalue == 1 && rvalue == 0) {
        info->mode = ONLP_LED_MODE_GREEN;
        info->status |= ONLP_LED_STATUS_ON;
    } else if (gvalue == 0 && rvalue == 1) {
        info->mode = ONLP_LED_MODE_RED;
        info->status |= ONLP_LED_STATUS_ON;
    } else if (gvalue == 1 && rvalue == 1) {
        info->mode = ONLP_LED_MODE_ORANGE;
        info->status |= ONLP_LED_STATUS_ON;
    } else if (gvalue == 0 && rvalue == 0) {
        info->mode = ONLP_LED_MODE_OFF;
        info->status |= ONLP_LED_STATUS_ON;
    } else {
        info->mode = ONLP_LED_MODE_OFF;
        info->status &= ~ONLP_LED_STATUS_PRESENT;
    }
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
    /*the led behavior is handled by another driver on D3352. Therefore, we're not supporting this attribute*/
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
    /*the led behavior is handled by another driver on D3352. Therefore, we're not supporting this attribute*/
    return ONLP_STATUS_E_UNSUPPORTED;
}
