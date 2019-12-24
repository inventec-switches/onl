/************************************************************
 * psui.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/psui.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "platform_lib.h"

#define PSU_STATUS_PRESENT	(0)
#define PSU_STATUS_POWER_GOOD	(1)
#define PSU_STATUS_UNPOWERED	(2)
#define PSU_STATUS_FAULT	(4)
#define PSU_STATUS_UNINSTALLED	(7)

typedef enum hwmon_psu_state_e {
    HWMON_PSU_NORMAL = 0,
    HWMON_PSU_UNPOWERED = 2,       //010
    HWMON_PSU_FAULT = 4,           //100
    HWMON_PSU_NOT_INSTALLED = 7    //111
} hwmon_psu_state_t;

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_PSU(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* __psu_status_path[PSU_MAX] =  /* must map with onlp_psu_id */
{
    "reserved",
    INV_SYSLED_PREFIX"psu0",
    INV_SYSLED_PREFIX"psu1",
};

static char* __psu_attr_path[PSU_MAX] =  /* must map with onlp_psu_id */
{
    "reserved",
    INV_DEVICE_PREFIX"psoc_psu1_%s",
    INV_DEVICE_PREFIX"psoc_psu2_%s",
};

static int
psu_status_info_get(int id, char *node, int *value)
{
    int ret = 0;
    char node_path[ONLP_NODE_MAX_PATH_LEN] = {0};

    *value = 0;
    if (PSU1_ID == id) {
        sprintf(node_path, __psu_status_path[id]);
    } else if (PSU2_ID == id) {
        sprintf(node_path, __psu_status_path[id]);
    }
    ret = onlp_file_read_int(value, node_path);

    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ONLP_STATUS_OK;
}


int
onlp_psui_init(void)
{
    DEBUG_PRINT("%s(%d): %s\r\n", __FUNCTION__, __LINE__, INV_PLATFORM_NAME);

    return ONLP_STATUS_OK;
}

static void
psu_module_name_get(int id, onlp_psu_info_t* info)
{
    char node_path[ONLP_NODE_MAX_PATH_LEN] = {0};
    uint8_t temp[ONLP_CONFIG_INFO_STR_MAX] = {0};
    int ret, len;

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "model");
    ret = onlp_file_read(temp, ONLP_CONFIG_INFO_STR_MAX, &len, node_path);
    if (ret == 0) {
        /*remove the '\n'*/
        temp[strlen((char*)temp)-1] = 0;
        snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "%s", temp);
    } else {
        AIM_LOG_ERROR("Unable to read model name from file(%s)\r\n", node_path);
        strncpy(info->model, "N/A", 3);
    }

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "sn");
    ret = onlp_file_read(temp, ONLP_CONFIG_INFO_STR_MAX, &len, node_path);
    if (ret == 0) {
        /*remove the '\n'*/
        temp[strlen((char*)temp)-1] = 0;
        snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "%s", temp);
    } else {
        AIM_LOG_ERROR("Unable to read model name from file(%s)\r\n", node_path);
        strncpy(info->serial, "N/A", 3);
    }
}

static int
psu_module_info_get(int id, onlp_psu_info_t* info)
{
    int ret = 0;
    char node_path[ONLP_NODE_MAX_PATH_LEN] = {0};
    int value = 0;

    info->caps |= ONLP_PSU_CAPS_DC12;

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "vout");
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read vout from file(%s)\r\n", node_path);
    } else {
        info->mvout = value;
        info->caps |= ONLP_PSU_CAPS_VOUT;
    }

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "iout");
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read iout from file(%s)\r\n", node_path);
    } else {
        info->miout = value;
        info->caps |= ONLP_PSU_CAPS_IOUT;
    }

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "pout");
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read pout from file(%s)\r\n", node_path);
    } else {
        info->mpout = value;
        info->caps |= ONLP_PSU_CAPS_POUT;
    }

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "vin");
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read vin from file(%s)\r\n", node_path);
    } else {
        info->mvin = value;
        info->caps |= ONLP_PSU_CAPS_VIN;
    }

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "iin");
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read iin from file(%s)\r\n", node_path);
    } else {
        info->miin = value;
        info->caps |= ONLP_PSU_CAPS_IIN;
    }

    memset(node_path, 0, ONLP_NODE_MAX_PATH_LEN);
    sprintf(node_path, __psu_attr_path[id], "pin");
    ret = onlp_file_read_int(&value, node_path);
    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read pin from file(%s)\r\n", node_path);
    } else {
        info->mpin = value;
        info->caps |= ONLP_PSU_CAPS_PIN;
    }

    psu_module_name_get(id, info);
    return ONLP_STATUS_OK;
}

/*
 * Get all information about the given PSU oid.
 */
static onlp_psu_info_t __onlp_psu_info[] = {
    { }, /* Not used */
    {
        {
            ONLP_PSU_ID_CREATE(PSU1_ID), "PSU-1", 0,
            {
                ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1),
                ONLP_FAN_ID_CREATE(FAN_1_ON_PSU1)
            }
        },
    },
    {
        {
            ONLP_PSU_ID_CREATE(PSU2_ID), "PSU-2", 0,
            {
                ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2),
                ONLP_FAN_ID_CREATE(FAN_1_ON_PSU2)
            }
        },
    }
};

int onlp_psui_status_get(onlp_oid_t id, uint32_t* rv)
{
    int val=0;
    int index = ONLP_OID_ID_GET(id);
    int ret;

    VALIDATE(id);

    /* Get the present state */
    if ((ret = psu_status_info_get(index, "psu", &val)) == ONLP_STATUS_E_INTERNAL) {
        printf("Unable to read PSU(%d) node(psu)\r\n", index);
        return ret;
    } 
    if(index>=PSU_MAX) return ONLP_STATUS_E_INVALID;

    if (val == HWMON_PSU_NORMAL) {
        *rv = ONLP_PSU_STATUS_PRESENT;
    } else if (val == HWMON_PSU_UNPOWERED ) {
        *rv = ONLP_PSU_STATUS_PRESENT|ONLP_PSU_STATUS_UNPLUGGED;
        return ret;
    } else if(val==HWMON_PSU_FAULT) {
        *rv = ONLP_PSU_STATUS_PRESENT| ONLP_PSU_STATUS_FAILED;
        return ret;
    } else if(val== HWMON_PSU_NOT_INSTALLED) {  
        *rv=0;
    } else {
        return ONLP_STATUS_E_INVALID;
    } 
    return  ONLP_STATUS_OK;
}

int
onlp_psui_info_get(onlp_oid_t id, onlp_psu_info_t* info)
{
    int ret   = ONLP_STATUS_OK;
    int index = ONLP_OID_ID_GET(id);

    VALIDATE(id);
    if(index>=PSU_MAX) return ONLP_STATUS_E_INVALID;

    memset(info, 0, sizeof(onlp_psu_info_t));
    *info = __onlp_psu_info[index]; /* Set the onlp_oid_hdr_t */

    onlp_psui_status_get(id, &info->status);

    if ((ret = psu_module_info_get(index, info)) != ONLP_STATUS_OK) {
        printf("Unable to read PSU(%d) module information\r\n", index);
    }

    return ret;
}

