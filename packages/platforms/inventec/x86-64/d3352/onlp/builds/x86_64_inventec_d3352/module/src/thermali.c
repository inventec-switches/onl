/************************************************************
 * thermali.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 * Thermal Sensor Platform Implementation.
 *
 ***********************************************************/
#include <unistd.h>
#include <onlplib/mmap.h>
#include <onlplib/file.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/psui.h>
#include <fcntl.h>
#include "platform_lib.h"
#include <sys/types.h>
#include <dirent.h>

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

extern int onlp_psui_status_get(onlp_oid_t id, uint32_t* rv);

static char* __thermal_path_list[THERMAL_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    NULL,
    NULL,
    NULL,
    NULL,
    INV_DEVICE_PREFIX"/temp1_input",
    INV_DEVICE_PREFIX"/temp2_input",
    INV_DEVICE_PREFIX"/temp3_input",
    INV_DEVICE_PREFIX"/temp4_input",
    INV_DEVICE_PREFIX"/temp5_input",
    INV_DEVICE_PREFIX"/thermal_psu1",
    INV_DEVICE_PREFIX"/thermal_psu2",
};

/* Static values */
static onlp_thermal_info_t __onlp_thermal_info[THERMAL_MAX] = {
    { }, /* Not used */
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_FIRST), "CPU Core 0", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_2), "CPU Core 1", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_3), "CPU Core 2", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_LAST), "CPU Core 3", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "Chassis Thermal Sensor 1", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "Chassis Thermal Sensor 2", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "Chassis Thermal Sensor 3", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "Chassis Thermal Sensor 4", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "Chassis Thermal Sensor 5", 0},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
    {   { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
        ONLP_THERMAL_STATUS_PRESENT,
        ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
    },
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    DEBUG_PRINT("%s(%d): %s\r\n", __FUNCTION__, __LINE__, INV_PLATFORM_NAME);

    return ONLP_STATUS_OK;
}

static int _get_hwmon_path( char* parent_dir, char* target_path)
{

    DIR * dir;
    struct dirent * ptr;
    char* buf=NULL;
    int ret=ONLP_STATUS_E_INVALID;

    dir = opendir(parent_dir);
    if(dir) {
        ret=ONLP_STATUS_OK;
    }
    if(ret==ONLP_STATUS_OK) {
        ret=ONLP_STATUS_E_INVALID;
        while( (ptr = readdir(dir))!=NULL ) {
            buf=ptr->d_name;
            if( strncmp(buf,"hwmon",5)==0 ) {
                ret=ONLP_STATUS_OK;
                break;
            }
        }
        closedir(dir);
    }
    if(ret==ONLP_STATUS_OK){
        snprintf(target_path, ONLP_CONFIG_INFO_STR_MAX, "%s%s/", parent_dir , buf);
    }else {
        printf("[ERROR] Can't find valid path\n");
    }

    return ret;

}

/*
 * Retrieve the information structure for the given thermal OID.
 *
 * If the OID is invalid, return ONLP_E_STATUS_INVALID.
 * If an unexpected error occurs, return ONLP_E_STATUS_INTERNAL.
 * Otherwise, return ONLP_STATUS_OK with the OID's information.
 *
 * Note -- it is expected that you fill out the information
 * structure even if the sensor described by the OID is not present.
 */
int
onlp_thermali_info_get(onlp_oid_t id, onlp_thermal_info_t* info)
{
    int local_id;
    int ret=ONLP_STATUS_OK;
    char base[ONLP_CONFIG_INFO_STR_MAX];

    local_id = ONLP_OID_ID_GET(id);
    *info = __onlp_thermal_info[local_id];

    if( (local_id<THERMAL_CPU_CORE_FIRST)||(local_id>THERMAL_1_ON_PSU2) ){
        ret=ONLP_STATUS_E_INVALID;
    }

    /* Set the onlp_oid_hdr_t and capabilities */
    if(ret==ONLP_STATUS_OK){
        ret=onlp_thermali_status_get(id,&info->status);
    }

    if( (info->status & ONLP_THERMAL_STATUS_PRESENT)&&(ret==ONLP_STATUS_OK) ){
        /* Set the onlp_oid_hdr_t and capabilities */
        switch(local_id){
            case THERMAL_CPU_CORE_FIRST:
            case THERMAL_CPU_CORE_2:
            case THERMAL_CPU_CORE_3:
            case THERMAL_CPU_CORE_LAST:
                ret=_get_hwmon_path(INV_CTMP_BASE,base);
                if(ret==ONLP_STATUS_OK){
                    __thermal_path_list[local_id]= malloc(sizeof(char)*ONLP_CONFIG_INFO_STR_MAX);
                    snprintf(__thermal_path_list[local_id],ONLP_CONFIG_INFO_STR_MAX,"%stemp%d_input",base,local_id-THERMAL_CPU_CORE_FIRST+2); /*core thermal id start from 2*/
                }
                ret= onlp_file_read_int(&info->mcelsius, __thermal_path_list[local_id]);
                if(__thermal_path_list[local_id]){
                    free(__thermal_path_list[local_id]);
                    __thermal_path_list[local_id]=NULL;
                }
                break;
            default:
                ret= onlp_file_read_int(&info->mcelsius, __thermal_path_list[local_id]);
                break;
        }
    }
    return ret;
}

int onlp_thermali_status_get(onlp_oid_t id, uint32_t* rv)
{
    int ret = ONLP_STATUS_OK;
    int thermal_id;
    onlp_thermal_info_t* info;

    VALIDATE(id);
    uint32_t psu_status;

    thermal_id= ONLP_OID_ID_GET(id);
    info = &__onlp_thermal_info[thermal_id];
    
    switch(thermal_id) {
    case THERMAL_1_ON_PSU1:
    case THERMAL_1_ON_PSU2:
        ret = onlp_psui_status_get((&info->hdr)->poid, &psu_status);
        if(ret != ONLP_STATUS_OK) {
            return ret;
        }
        if(psu_status & ONLP_PSU_STATUS_PRESENT) {
            info->status &= ONLP_PSU_STATUS_PRESENT;
        } else {
            info->status = 0;
        }
        break;
    default:
        break;
    }

    *rv = info->status;

    return ret;
}