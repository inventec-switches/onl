/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017 Delta Networks, Inc.
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
#include <onlp/platformi/ledi.h>
#include <sys/mman.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <onlplib/mmap.h>
#include <onlplib/i2c.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_FAN), "FRONT LED (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_SYS), "FRONT LED  (SYS LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_RED,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR1), "FRONT LED  (PWR1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE_BLINKING | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FRONT_PWR2), "FRONT LED (PWR2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_ORANGE_BLINKING | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_1), "FAN TRAY 1 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_2), "FAN TRAY 2 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_3), "FAN TRAY 3 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_4), "FAN TRAY 4 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
    },
    {
        { ONLP_LED_ID_CREATE(LED_REAR_FAN_TRAY_5), "FAN TRAY 5 LED", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_RED | ONLP_LED_CAPS_GREEN,
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
    int r_data = 0, fantray_present = -1;
    VALIDATE(id);
    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    /* Set front panel's  mode of leds */
    r_data = dni_lock_swpld_read_attribute(LED_REG);
    int local_id = ONLP_OID_ID_GET(id);
    
    dev_info_t dev_info;
    dev_info.bus = I2C_BUS_3;
    dev_info.offset = 0x00;
    dev_info.flags = DEFAULT_FLAG;

    mux_info_t mux_info;
    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.flags = DEFAULT_FLAG;

    switch(local_id)
    {
        case LED_FRONT_FAN:
            if((r_data & 0x03) == 0x01)
	        info->mode = ONLP_LED_MODE_GREEN;
	    else if((r_data & 0x03) == 0x02)
	        info->mode = ONLP_LED_MODE_ORANGE;
            else if((r_data & 0x03) == 0x03 || (r_data & 0x03) == 0x00)
                info->mode = ONLP_LED_MODE_OFF;
            else
                return ONLP_STATUS_E_INTERNAL;
            break;
        case LED_FRONT_SYS:
            if((r_data & 0x0C) == 0x04)
	        info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x0C) == 0x0C)
	        info->mode = ONLP_LED_MODE_RED;
	    else if((r_data & 0x0C) == 0x08)
		info->mode = ONLP_LED_MODE_GREEN_BLINKING;
	    else if((r_data & 0x0C) == 0x00)
                info->mode = ONLP_LED_MODE_OFF;
            else
		return ONLP_STATUS_E_INTERNAL;
            break;
        case LED_FRONT_PWR1:
            if((r_data & 0x40) == 0x40)
	        info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x80) == 0x80)
	        info->mode = ONLP_LED_MODE_ORANGE_BLINKING;
	    else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_FRONT_PWR2:
            if((r_data & 0x10) == 0x10)
	        info->mode = ONLP_LED_MODE_GREEN;
            else if((r_data & 0x20) == 0x20)
	        info->mode = ONLP_LED_MODE_ORANGE_BLINKING;
	    else
                info->mode = ONLP_LED_MODE_OFF;
            break;
        case LED_REAR_FAN_TRAY_1:
           /* Select fan tray 1 */
           dev_info.addr = FAN_TRAY_1;
           mux_info.channel = 0x00;
           r_data = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG);
           fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
           if(fantray_present >= 0)
           {
               if((r_data & 0x40) == 0x40)
                  info->mode = ONLP_LED_MODE_GREEN;
               else if((r_data & 0x80) == 0x80)
                  info->mode = ONLP_LED_MODE_RED;
               else
                  info->mode = ONLP_LED_MODE_OFF;
           }
           else
               info->status = ONLP_LED_STATUS_FAILED;
           break;
        case LED_REAR_FAN_TRAY_2:
           /* Select fan tray 2 */
           dev_info.addr = FAN_TRAY_2;
           mux_info.channel = 0x01;
           r_data = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG);
           fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
           if(fantray_present >= 0)
           {
               if((r_data & 0x10) == 0x10)
                  info->mode = ONLP_LED_MODE_GREEN;
               else if((r_data & 0x20) == 0x20)
                  info->mode = ONLP_LED_MODE_RED;
               else
                  info->mode = ONLP_LED_MODE_OFF;
           }
           else
               info->status = ONLP_LED_STATUS_FAILED;
           break;
        case LED_REAR_FAN_TRAY_3:
           /* Select fan tray 3 */
           dev_info.addr = FAN_TRAY_3;
           mux_info.channel = 0x02;
           r_data = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG);
           fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
           if(fantray_present >= 0)
           {
               if((r_data & 0x04) == 0x04)
                  info->mode = ONLP_LED_MODE_GREEN;
               else if((r_data & 0x08) == 0x08)
                  info->mode = ONLP_LED_MODE_RED;
               else
                  info->mode = ONLP_LED_MODE_OFF;
           }
           else
               info->status = ONLP_LED_STATUS_FAILED;
           break;
        case LED_REAR_FAN_TRAY_4:
           /* Select fan tray 4 */
           dev_info.addr = FAN_TRAY_4;
           mux_info.channel = 0x03;
           r_data = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG);
           fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
           if(fantray_present >= 0)
           {
               if((r_data & 0x01) == 0x01)
                  info->mode = ONLP_LED_MODE_GREEN;
               else if((r_data & 0x02) == 0x02)
                  info->mode = ONLP_LED_MODE_RED;
               else
                  info->mode = ONLP_LED_MODE_OFF;
           }
           else
               info->status = ONLP_LED_STATUS_FAILED;
           break;
        case LED_REAR_FAN_TRAY_5:
           /* Select fan tray 5 */
           dev_info.addr = FAN_TRAY_5;
           mux_info.channel = 0x04;
           r_data = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG_2);
           fantray_present = dni_i2c_lock_read(&mux_info, &dev_info);
           if(fantray_present >= 0)
           {
               if((r_data & 0x40) == 0x40)
                  info->mode = ONLP_LED_MODE_GREEN;
               else if((r_data & 0x80) == 0x80)
                  info->mode = ONLP_LED_MODE_RED;
               else
                  info->mode = ONLP_LED_MODE_OFF;
           }
           else
               info->status = ONLP_LED_STATUS_FAILED;
	   break; 
        default:
            break;
    }

    /* Set the on/off status */
    if (info->mode == ONLP_LED_MODE_OFF) 
        info->status |= ONLP_LED_STATUS_FAILED;
    else
	info->status |=ONLP_LED_STATUS_PRESENT;


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
    if (!on_or_off)
    {
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
    VALIDATE(id);
    int local_id = ONLP_OID_ID_GET(id);
    uint8_t front_panel_led_value, fan_tray_led_reg_value,fan_tray_led_reg_2_value;

    fan_tray_led_reg_value = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG);
    fan_tray_led_reg_2_value = dni_lock_swpld_read_attribute(FAN_TRAY_LED_REG_2);      
    switch(local_id)
    {   
        case LED_FRONT_FAN:
            /* Clean the bit 1,0 */
            front_panel_led_value = dni_lock_swpld_read_attribute(LED_REG);
            front_panel_led_value &= ~0x3;
            if(mode == ONLP_LED_MODE_GREEN)
            {
               front_panel_led_value |= 0x01;
               dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if(mode == ONLP_LED_MODE_ORANGE)
            {
                front_panel_led_value |= 0x02;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            break;
         case LED_FRONT_PWR1: 
            /* Clean bit 7,6 */
            front_panel_led_value = dni_lock_swpld_read_attribute(LED_REG);
            front_panel_led_value &= ~0xC0;
            if(mode == ONLP_LED_MODE_GREEN)
            {
                /* Green */
                front_panel_led_value |= 0x40;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if(mode == ONLP_LED_MODE_ORANGE_BLINKING)
            {
                /* BLINKING ORANGE */
                front_panel_led_value |= 0x80;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if(mode == ONLP_LED_MODE_OFF)
            {
                front_panel_led_value &= ~0xC0;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else
                return ONLP_STATUS_E_UNSUPPORTED;
            break;
        case LED_FRONT_PWR2:
            /* Clean bit 5,4 */
            front_panel_led_value = dni_lock_swpld_read_attribute(LED_REG);
            front_panel_led_value &= ~0x30;
            if(mode == ONLP_LED_MODE_GREEN)
            {
                /* Green */
                front_panel_led_value |= 0x10;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if(mode == ONLP_LED_MODE_ORANGE_BLINKING)
            { 
                /* BLINKING ORANGE */
                front_panel_led_value |= 0x20;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if(mode == ONLP_LED_MODE_OFF)
            {
                front_panel_led_value &= ~0x30;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else
                return ONLP_STATUS_E_UNSUPPORTED;
            break;
 
        case LED_FRONT_SYS:
            front_panel_led_value = dni_lock_swpld_read_attribute(LED_REG);
            front_panel_led_value &= ~0x0C;
            if(mode == ONLP_LED_MODE_GREEN_BLINKING)
            {
                front_panel_led_value |= 0x08;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if (mode == ONLP_LED_MODE_GREEN)
            {
                front_panel_led_value |= 0x04;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else if (mode == ONLP_LED_MODE_RED) 
            {
                front_panel_led_value |= 0x0c;
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            }
            else
                dni_lock_swpld_write_attribute(LED_REG, front_panel_led_value);
            break;
 
        case LED_REAR_FAN_TRAY_1:
             fan_tray_led_reg_value &= ~0xC0;
             if(mode == ONLP_LED_MODE_GREEN)
             {/* Green light */
                 fan_tray_led_reg_value |= 0x40;
                 dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
             }
             else if(mode == ONLP_LED_MODE_RED)
             {/* Red light */
                fan_tray_led_reg_value |= 0x80;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
             }
             else
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
             break;
        case LED_REAR_FAN_TRAY_2:
            fan_tray_led_reg_value &= ~0x30;
            if(mode == ONLP_LED_MODE_GREEN)
            {/* Green light */
                fan_tray_led_reg_value |= 0x10;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else if(mode == ONLP_LED_MODE_RED)
            {/* Red light */
               fan_tray_led_reg_value |= 0x20;
               dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            break;
        case LED_REAR_FAN_TRAY_3:
            fan_tray_led_reg_value &= ~0x0c;
            if(mode == ONLP_LED_MODE_GREEN)
            {/* Green light */
                fan_tray_led_reg_value |= 0x04;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else if(mode == ONLP_LED_MODE_RED)
            {/* Red light */
               fan_tray_led_reg_value |= 0x08;
               dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            break;
        case LED_REAR_FAN_TRAY_4:
            fan_tray_led_reg_value &= ~0x03;
            if(mode == ONLP_LED_MODE_GREEN)
            {/* Green light */
                fan_tray_led_reg_value |= 0x01;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else if(mode == ONLP_LED_MODE_RED)
            {/* Red light */
                fan_tray_led_reg_value |= 0x02;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            }
            else
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
            break;
         case LED_REAR_FAN_TRAY_5:
             fan_tray_led_reg_2_value &= ~0xC0;
             if(mode == ONLP_LED_MODE_GREEN)
             {/* Green light */
                fan_tray_led_reg_2_value |= 0x40;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG_2, fan_tray_led_reg_2_value);
             }
             else if(mode == ONLP_LED_MODE_RED)
             {/* Red light */
                fan_tray_led_reg_2_value |= 0x80;
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG_2, fan_tray_led_reg_2_value);
             }
             else
                dni_lock_swpld_write_attribute(FAN_TRAY_LED_REG, fan_tray_led_reg_value);
             break;
    }
    return ONLP_STATUS_OK;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
