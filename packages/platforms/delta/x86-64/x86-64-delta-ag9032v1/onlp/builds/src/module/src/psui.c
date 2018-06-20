/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2017 (C) Delta Networks, Inc.
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
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <stdio.h>
#include <string.h>
#include "platform_lib.h"
#include <onlplib/i2c.h>

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int
dni_psu_pmbus_info_get(int id, char *node, int *value)
{
    int  ret = 0;
    char node_path[PSU_NODE_MAX_PATH_LEN] = {0};
    
    *value = 0;

    switch (id) {
    case PSU1_ID:
        sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, node);   	
	break;
    case PSU2_ID:
        sprintf(node_path, "%s%s", PSU2_AC_PMBUS_PREFIX, node);
	break;
    default:
	break;
    }

    /* Read attribute value */
    *value = dni_i2c_lock_read_attribute(NULL, node_path);
    
    return ret;
}

int
onlp_psui_init(void)
{
    return ONLP_STATUS_OK;
}

static int
dni_psu_info_get(onlp_psu_info_t* info)
{
    int val = 0;
    int index = ONLP_OID_ID_GET(info->hdr.id);
    char val_char[16] = {'\0'};
    char node_path[PSU_NODE_MAX_PATH_LEN] = {'\0'};

    /* Set capability */
    info->caps |= ONLP_PSU_CAPS_AC;

    /* Set the associated oid_table
     * Set PSU's fan and thermal to child OID
     */
    info->hdr.coids[0] = ONLP_FAN_ID_CREATE(index + CHASSIS_FAN_COUNT);
    info->hdr.coids[1] = ONLP_THERMAL_ID_CREATE(index + CHASSIS_THERMAL_COUNT);

    /* Read PSU module name from attribute */
    sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, "psu_mfr_model");
    dni_i2c_read_attribute_string(node_path, val_char, sizeof(val_char), 0);
    strcpy(info->model, val_char);
    
    /* Read PSU serial number from attribute */
    sprintf(node_path, "%s%s", PSU1_AC_PMBUS_PREFIX, "psu_mfr_serial");
    dni_i2c_read_attribute_string(node_path, val_char, sizeof(val_char), 0);
    strcpy(info->serial, val_char);

    /* Read voltage, current and power */
    if (dni_psu_pmbus_info_get(index, "psu_v_out", &val) == 0) {
        info->mvout = val;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    if (dni_psu_pmbus_info_get(index, "psu_v_in", &val) == 0) {
        info->mvin = val;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    if (dni_psu_pmbus_info_get(index, "psu_i_out", &val) == 0) {
        info->miout = val;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }
    
    if (dni_psu_pmbus_info_get(index, "psu_i_in", &val) == 0) {
        info->miin = val;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    if (dni_psu_pmbus_info_get(index, "psu_p_out", &val) == 0) {
        info->mpout = val;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }   

    if (dni_psu_pmbus_info_get(index, "psu_p_in", &val) == 0) {
        info->mpin = val;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }   

    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t pinfo[] =
{
    { }, /* Not used */
    {
        { ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0 },
    },
    {
        { ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0 },
    }
};

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int val   = 0;
    int ret   = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);
    uint8_t channel;
    char channel_data[2] = {'\0'};
    mux_info_t mux_info;
    dev_info_t dev_info;

    VALIDATE(id);

    /* Set the onlp_oid_hdr_t */
    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = pinfo[index];

    switch (index) {
    case PSU1_ID:
    	channel = PSU_I2C_SEL_PSU1_EEPROM;
	break;
    case PSU2_ID:
        channel = PSU_I2C_SEL_PSU2_EEPROM;
	break;
    default:
	break;
    }

    mux_info.offset = SWPLD_PSU_FAN_I2C_MUX_REG;
    mux_info.channel = channel;
    mux_info.flags = DEFAULT_FLAG;    
    
    dev_info.bus = I2C_BUS_4;
    dev_info.addr = PSU_EEPROM;
    dev_info.offset = 0x00;	/* In EEPROM address 0x00 */
    dev_info.flags = DEFAULT_FLAG;

    /* Select PSU member(channel) */
    sprintf(channel_data, "%x", channel);
    dni_i2c_lock_write_attribute(NULL, channel_data, PSU_SELECT_MEMBER_PATH);


    /* Check PSU have voltage input or not */
    dni_psu_pmbus_info_get(index, "psu_v_in", &val);

    /* Check PSU is PRESENT or not
     * Read PSU EEPROM 1 byte from adress 0x00
     * if not present, return Negative value.
     */        
    if(val == 0 && dni_i2c_lock_read(&mux_info, &dev_info) < 0)
    {
        /* Unable to read PSU EEPROM */
        /* Able to read PSU VIN(psu_power_not_good) */
        info->status |= ONLP_PSU_STATUS_FAILED;
        return ONLP_STATUS_OK;
    }
    else if(val == 0){
        /* Unable to read PSU VIN(psu_power_good) */
        info->status |= ONLP_PSU_STATUS_UNPLUGGED;
    } 
    else {
        info->status |= ONLP_PSU_STATUS_PRESENT;
    }

    ret = dni_psu_info_get(info);
    return ret;
}

int
onlp_psui_ioctl(onlp_oid_t pid, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}