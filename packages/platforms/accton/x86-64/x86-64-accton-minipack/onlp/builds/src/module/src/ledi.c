/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2013 Accton Technology Corporation.
 *
 * Licensed under the Eclipse Public License, Version 1.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *        http://www.eclipse.org/legal/epl-v10.html
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND,
 * either express or implied. See the License for the specific
 * language governing permissions and limitations under the
 * License.
 *
 * </bsn.cl>
 ************************************************************
 *
 *
 *
 ***********************************************************/
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include <onlp/platformi/ledi.h>
#include "platform_lib.h"


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
    LED_SYS,
    LED_ACT
};

typedef struct led_address_s {
    enum onlp_led_id id;
    uint8_t bus;
    uint8_t devaddr;
    uint8_t offset;
} led_address_t;

typedef struct led_mode_info_s {
    onlp_led_mode_t  mode;
    uint8_t regval;
} led_mode_info_t;

static led_address_t led_addr[] = 
{
    { }, /* Not used */
    {LED_SYS, 50, 0x20, 3},
    {LED_ACT, 50, 0x20, 3},
};

struct led_type_mode {
    enum onlp_led_id type;
    onlp_led_mode_t mode;
    int  reg_bit_mask;
    int  mode_value;
};

#define LED_TYPE_SYS_REG_MASK           (0x07)
#define LED_MODE_SYS_OFF_VALUE          (0x07)
#define LED_TYPE_ACT_REG_MASK           (0x38)
#define LED_MODE_ACT_OFF_VALUE          (0x38)

static struct led_type_mode led_type_mode_data[] = {
    {LED_SYS,  ONLP_LED_MODE_OFF,        LED_TYPE_SYS_REG_MASK,   LED_MODE_SYS_OFF_VALUE},
    {LED_SYS,  ONLP_LED_MODE_RED,        LED_TYPE_SYS_REG_MASK,   0x06},
    {LED_SYS,  ONLP_LED_MODE_GREEN,	     LED_TYPE_SYS_REG_MASK,   0x05},
    {LED_SYS,  ONLP_LED_MODE_BLUE,       LED_TYPE_SYS_REG_MASK,   0x03},
    {LED_SYS,  ONLP_LED_MODE_YELLOW,	 LED_TYPE_SYS_REG_MASK,   0x04},
    {LED_SYS,  ONLP_LED_MODE_PURPLE,     LED_TYPE_SYS_REG_MASK,   0x02},

    {LED_ACT,  ONLP_LED_MODE_OFF,        LED_TYPE_ACT_REG_MASK,   LED_MODE_ACT_OFF_VALUE},
    {LED_ACT,  ONLP_LED_MODE_RED,        LED_TYPE_ACT_REG_MASK,   0x30},
    {LED_ACT,  ONLP_LED_MODE_GREEN,      LED_TYPE_ACT_REG_MASK,   0x28},
    {LED_ACT,  ONLP_LED_MODE_BLUE,       LED_TYPE_ACT_REG_MASK,   0x18},
    {LED_ACT,  ONLP_LED_MODE_YELLOW,	 LED_TYPE_ACT_REG_MASK,   0x20},
    {LED_ACT,  ONLP_LED_MODE_PURPLE,     LED_TYPE_ACT_REG_MASK,   0x10},
};


/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_SYS), "SIM LED 1 (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF |
        ONLP_LED_CAPS_GREEN  | ONLP_LED_CAPS_PURPLE |
        ONLP_LED_CAPS_RED    | ONLP_LED_CAPS_YELLOW |
        ONLP_LED_CAPS_BLUE   
    },
    {
        { ONLP_LED_ID_CREATE(LED_ACT), "SIM LED 2 (ACT LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF |
        ONLP_LED_CAPS_GREEN  | ONLP_LED_CAPS_PURPLE |
        ONLP_LED_CAPS_RED    | ONLP_LED_CAPS_YELLOW |
        ONLP_LED_CAPS_BLUE   
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

static int reg_value_to_onlp_led_mode(enum onlp_led_id type, uint8_t reg_val) {
    int i;

    for (i = 0; i < AIM_ARRAYSIZE(led_type_mode_data); i++) {

        if (type != led_type_mode_data[i].type)
            continue;

        if ((led_type_mode_data[i].reg_bit_mask & reg_val) ==
                led_type_mode_data[i].mode_value)
        {
            return led_type_mode_data[i].mode;
        }
    }

    return 0;
}

static uint8_t onlp_led_mode_to_reg_value(enum onlp_led_id type,
                                    onlp_led_mode_t mode, uint8_t reg_val) {
    int i;

    for (i = 0; i < AIM_ARRAYSIZE(led_type_mode_data); i++) {
        if (type != led_type_mode_data[i].type)
            continue;

        if (mode != led_type_mode_data[i].mode)
            continue;

        reg_val = led_type_mode_data[i].mode_value |
                  (reg_val & (~led_type_mode_data[i].reg_bit_mask));
        break;
    }

    return reg_val;
}


int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t* info)
{
    int  lid, value;

    VALIDATE(id);
    lid = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    value = bmc_i2c_readb(led_addr[lid].bus, led_addr[lid].devaddr, led_addr[lid].offset);
    if (value < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }

    info->mode = reg_value_to_onlp_led_mode(lid, value);
    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF) {
        info->status |= ONLP_LED_STATUS_ON;
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
    int  lid, value, i;    

    VALIDATE(id);
    lid = ONLP_OID_ID_GET(id);

    for (i = 1; i <= PLATFOTM_H_TTY_RETRY; i++) {    
        value = bmc_i2c_readb(led_addr[lid].bus, led_addr[lid].devaddr, 
                        led_addr[lid].offset);
        if (value < 0) {
            continue;
        }
        value = onlp_led_mode_to_reg_value(lid, mode, value);
        if (bmc_i2c_writeb(led_addr[lid].bus, led_addr[lid].devaddr, 
                        led_addr[lid].offset, value) < 0) {
            continue;
        } else {
            return ONLP_STATUS_OK;
        }
    }

    return ONLP_STATUS_E_INTERNAL;

}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

