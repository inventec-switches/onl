/************************************************************
 * ledi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <onlplib/file.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlp/platformi/fani.h>

#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/* LED related data
 */

/* CAPS*/
#define MEMT_LED_CAPS ONLP_LED_CAPS_ON_OFF|ONLP_LED_CAPS_BLUE|ONLP_LED_CAPS_BLUE_BLINKING
#define CHASSIS_LED_CAPS ONLP_LED_CAPS_ON_OFF|ONLP_LED_CAPS_GREEN|ONLP_LED_CAPS_GREEN_BLINKING
#define FAN_LED_CAPS ONLP_LED_CAPS_RED|ONLP_LED_CAPS_GREEN

#define LED_ID_TO_FAN_ID(id) (id-ONLP_LED_PWR)

typedef enum sys_led_mode_e {
    SYS_LED_MODE_OFF = 0,
    SYS_LED_MODE_ON = 1,
    SYS_LED_MODE_1_HZ = 2,
    SYS_LED_MODE_2_HZ = 3
} sys_led_mode_t;

/* function declarations*/
static int _sys_onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info);
static int _fan_onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info);

/*
 * Get the information for the given LED OID.
 */
#define MAKE_MGMT_LED_INFO_NODE			\
    {						\
        { ONLP_LED_ID_CREATE(ONLP_LED_MGMT), "MGMT LED" , 0 }, \
        ONLP_LED_STATUS_PRESENT,		\
        MEMT_LED_CAPS,		\
    }

#define MAKE_CHASSIS_LED_INFO_NODE(led_name)           \
    {                       \
        { ONLP_LED_ID_CREATE(ONLP_LED_##led_name), #led_name" LED" , 0 }, \
        ONLP_LED_STATUS_PRESENT,        \
        CHASSIS_LED_CAPS,      \
    }

#define MAKE_LED_INFO_NODE_ON_FAN(fan_id)	\
    {						\
        { ONLP_LED_ID_CREATE(ONLP_LED_FAN##fan_id), \
          "FAN LED "#fan_id,			\
          ONLP_FAN_ID_CREATE(ONLP_FAN_##fan_id) \
        },					\
        0,	FAN_LED_CAPS,					\
    }

static onlp_led_info_t __onlp_led_info[] = {
    {},
    MAKE_MGMT_LED_INFO_NODE,
    MAKE_CHASSIS_LED_INFO_NODE(STK),
    MAKE_CHASSIS_LED_INFO_NODE(FAN),
    MAKE_CHASSIS_LED_INFO_NODE(PWR),
    MAKE_LED_INFO_NODE_ON_FAN(1),
    MAKE_LED_INFO_NODE_ON_FAN(2),
    MAKE_LED_INFO_NODE_ON_FAN(3),
    MAKE_LED_INFO_NODE_ON_FAN(4),
    MAKE_LED_INFO_NODE_ON_FAN(5),
};

static char* __led_path_list[] = {
    "reserved",
    INV_LED_PREFIX"/service_led",
    INV_LED_PREFIX"/stacking_led",
    INV_LED_PREFIX"/fan_led",
    INV_LED_PREFIX"/power_led",
};

static int _sys_onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int led_mode,len;
    int led_id= ONLP_OID_ID_GET(id);
    *info= __onlp_led_info[led_id];
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len,__led_path_list[led_id]);

    if(rv==ONLP_STATUS_OK) {
        led_mode=(buf[0]-'0');
        switch(led_id) {
        case ONLP_LED_MGMT:
            if(led_mode==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_mode==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if(led_mode==SYS_LED_MODE_1_HZ) {
                info->mode =ONLP_LED_MODE_BLUE_BLINKING;
            } else {
                printf("[ONLP][ERROR] Not defined LED behavior detected on led@%d\n", led_id);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
        case ONLP_LED_STK:
            if(led_mode==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_mode==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if(led_mode==SYS_LED_MODE_1_HZ) {
                info->mode =ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                printf("[ONLP][ERROR] Not defined LED behavior detected on led@%d\n", led_id);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
        case ONLP_LED_FAN:
        case ONLP_LED_PWR:
            if(led_mode==SYS_LED_MODE_OFF) {
                info->mode = ONLP_LED_MODE_OFF;
            } else if(led_mode==SYS_LED_MODE_ON) {
                info->mode = ONLP_LED_MODE_ON;
            } else if( (led_mode==SYS_LED_MODE_1_HZ)||(led_mode==SYS_LED_MODE_2_HZ) ) {
                info->mode =ONLP_LED_MODE_GREEN_BLINKING;
            } else {
                printf("[ONLP][ERROR] Not defined LED behavior detected on led@%d\n", led_id);
                return ONLP_STATUS_E_INTERNAL;
            }
            break;
        default:
            rv=ONLP_STATUS_E_INVALID;
            break;
        }
    }


    if(info->mode==ONLP_LED_MODE_OFF) {
        info->status = REMOVE_STATE(info->status,ONLP_LED_STATUS_ON);
    } else {
        info->status = ADD_STATE(info->status,ONLP_LED_STATUS_ON);
    }

    return rv;
}

static int _fan_onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int rv = ONLP_STATUS_OK, len;
    int led_id=ONLP_OID_ID_GET(id);
    int fan_id=LED_ID_TO_FAN_ID(led_id);
    *info=__onlp_led_info[led_id];
    char buf[ONLP_CONFIG_INFO_STR_MAX];

    rv = onlp_ledi_status_get(id, &info->status);
    if( rv != ONLP_STATUS_OK ) {
        return rv;
    }

    if( info->status & ONLP_LED_STATUS_PRESENT) {
        info->caps = FAN_LED_CAPS;
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_LED_PREFIX"fanmodule%d_led", fan_id);
        if(rv == ONLP_STATUS_OK ) {
            switch(buf[0]) {
            case '0':
                info->mode = ONLP_LED_MODE_OFF;
                break;
            case '1':
                info->mode = ONLP_LED_MODE_GREEN;
                break;
            case '2':
                info->mode = ONLP_LED_MODE_RED;
                break;
            default:
                rv=ONLP_STATUS_E_INVALID;
                break;
            }
        }
    } else {
        info->mode = ONLP_LED_MODE_OFF;
    }
    return rv;
}

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
    int rv = ONLP_STATUS_OK;
    VALIDATE(id);
    int led_id= ONLP_OID_ID_GET(id);
    if( led_id >= ONLP_LED_MAX) {
        return ONLP_STATUS_E_INVALID;
    }

    switch(led_id) {
    case ONLP_LED_MGMT:
    case ONLP_LED_STK:
    case ONLP_LED_FAN:
    case ONLP_LED_PWR:
        rv = _sys_onlp_ledi_info_get(id, info);
        break;
    case ONLP_LED_FAN1:
    case ONLP_LED_FAN2:
    case ONLP_LED_FAN3:
    case ONLP_LED_FAN4:
    case ONLP_LED_FAN5:
        rv = _fan_onlp_ledi_info_get(id, info);
        break;
    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }
    return rv;
}

/**
 * @brief Get the LED operational status.
 * @param id The LED OID
 * @param rv [out] Receives the operational status.
 */
int onlp_ledi_status_get(onlp_oid_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_led_info_t* info;

    VALIDATE(id);
    int led_id = ONLP_OID_ID_GET(id);
    int fan_id = LED_ID_TO_FAN_ID( led_id);
    char mode[ONLP_CONFIG_INFO_STR_MAX];
    uint32_t fan_status;

    if( led_id >= ONLP_LED_MAX) {
        result = ONLP_STATUS_E_INVALID;
    }
    if(result == ONLP_STATUS_OK) {
        info = &__onlp_led_info[led_id];
        switch(led_id) {
        case ONLP_LED_MGMT:
        case ONLP_LED_STK:
        case ONLP_LED_FAN:
        case ONLP_LED_PWR:
            /*The status setting of these 4 leds is handled by onlp_ledi_info_get and _sys_onlp_ledi_info_get()*/
            result=ONLP_STATUS_E_UNSUPPORTED;
            break;
        case ONLP_LED_FAN1:
        case ONLP_LED_FAN2:
        case ONLP_LED_FAN3:
        case ONLP_LED_FAN4:
        case ONLP_LED_FAN5:
            result = onlp_fani_status_get((&info->hdr)->poid, &fan_status);
            if(result != ONLP_STATUS_OK) {
                return result;
            }

            if(fan_status & ONLP_FAN_STATUS_PRESENT) {
                info->status = ADD_STATE(info->status,ONLP_LED_STATUS_PRESENT);

                result = onlp_file_read_int((int*)&mode, INV_LED_PREFIX"fanmodule%d_led", fan_id);
                if(result != ONLP_STATUS_OK) {
                    return result;
                }

                if(mode[0]=='0') {
                    info->status = REMOVE_STATE(info->status,ONLP_LED_STATUS_ON);
                } else {
                    info->status = ADD_STATE(info->status,ONLP_LED_STATUS_ON);
                }
            } else {
                info->status = 0;
            }
            *rv = info->status;
            break;
        default:
            result = ONLP_STATUS_E_INVALID;
            break;
        }
    }

    return result;
}

/**
 * @brief Get the LED header.
 * @param id The LED OID
 * @param rv [out] Receives the header.
 */
int onlp_ledi_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_led_info_t* info;

    VALIDATE(id);

    int led_id = ONLP_OID_ID_GET(id);
    if( led_id >= ONLP_LED_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {

        info = &__onlp_led_info[led_id];
        *rv = info->hdr;
    }
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
    /*the led behavior is handled by another driver on ne2580. Therefore, we're not supporting this attribute*/
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
    /*the led behavior is handled by another driver on ne2580. Therefore, we're not supporting this attribute*/
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

