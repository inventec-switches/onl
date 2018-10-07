/************************************************************
 * fani.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <stdlib.h>
#include <onlplib/file.h>
#include <onlp/platformi/fani.h>
#include <onlplib/mmap.h>
#include <fcntl.h>
#include "platform_lib.h"

#define PREFIX_PSOC_PATH	INV_PSOC_PREFIX

#define FAN_GPI_ON_MAIN_BOARD	PREFIX_PSOC_PATH"/fan_gpi"

#define MAX_FAN_SPEED     18000
#define MAX_PSU_FAN_SPEED 25500

#define PROJECT_NAME
#define LEN_FILE_NAME 80

enum fan_id {
	FAN_RESERVED,
	FAN_1_ON_MAIN_BOARD,
	FAN_2_ON_MAIN_BOARD,
	FAN_3_ON_MAIN_BOARD,
	FAN_4_ON_MAIN_BOARD,
	FAN_5_ON_MAIN_BOARD,
	FAN_6_ON_MAIN_BOARD,
	FAN_7_ON_MAIN_BOARD,
	FAN_8_ON_MAIN_BOARD,
	FAN_1_ON_PSU1,
	FAN_1_ON_PSU2,
};

static char* devfiles__[CHASSIS_FAN_COUNT+1] =  /* must map with onlp_thermal_id */
{
    "reserved",
    PREFIX_PSOC_PATH"/fan1_input",
    PREFIX_PSOC_PATH"/fan2_input",
    PREFIX_PSOC_PATH"/fan3_input",
    PREFIX_PSOC_PATH"/fan4_input",
    PREFIX_PSOC_PATH"/fan5_input",
    PREFIX_PSOC_PATH"/fan6_input",
    PREFIX_PSOC_PATH"/fan7_input",
    PREFIX_PSOC_PATH"/fan8_input",
    PREFIX_PSOC_PATH"/rpm_psu1",
    PREFIX_PSOC_PATH"/rpm_psu2",
};

#define MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##id##_ON_MAIN_BOARD), "Chassis Fan "#id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id, fan_id) \
    { \
        { ONLP_FAN_ID_CREATE(FAN_##fan_id##_ON_PSU##psu_id), "Chassis PSU-"#psu_id " Fan "#fan_id, 0 }, \
        0x0, \
        (ONLP_FAN_CAPS_SET_PERCENTAGE | ONLP_FAN_CAPS_GET_RPM | ONLP_FAN_CAPS_GET_PERCENTAGE), \
        0, \
        0, \
        ONLP_FAN_MODE_INVALID, \
    }

/* Static fan information */
onlp_fan_info_t linfo[] = {
    { }, /* Not used */
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(6),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(7),
    MAKE_FAN_INFO_NODE_ON_MAIN_BOARD(8),
    MAKE_FAN_INFO_NODE_ON_PSU(1,1),
    MAKE_FAN_INFO_NODE_ON_PSU(2,1),
};

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static int inv_psoc_id = D7264Q28B_PSOC_HWMON_ID;

static int
_onlp_fani_info_get_fan(int fid, onlp_fan_info_t* info)
{
    int   value, ret;
    char  vstr[32], *vstrp = vstr, **vp = &vstrp;

    memset(vstr, 0, 32);
    /* get fan present status */
    ret = onlp_file_read_str(vp, FAN_GPI_ON_MAIN_BOARD, inv_psoc_id);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf(*vp, "%x", &value);
    if (value & (1 << (fid-1))) {
	info->status |= ONLP_FAN_STATUS_FAILED;
    }
    else {
	info->status |= ONLP_FAN_STATUS_PRESENT;
    }

    /* get front fan speed */
    memset(vstr, 0, 32);
    ret = onlp_file_read_str(vp, devfiles__[fid], inv_psoc_id);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf(*vp, "%d", &value);
    info->rpm = value;
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;

    return ONLP_STATUS_OK;
}


static uint32_t
_onlp_get_fan_direction_on_psu(void)
{
    /* Try to read direction from PSU1.
     * If PSU1 is not valid, read from PSU2
     */
    int i = 0;

    for (i = PSU1_ID; i <= PSU2_ID; i++) {
        psu_type_t psu_type;
        psu_type = get_psu_type(i, NULL, 0);

        if (psu_type == PSU_TYPE_UNKNOWN) {
            continue;
        }

        if (PSU_TYPE_AC_F2B == psu_type) {
            return ONLP_FAN_STATUS_F2B;
        }
        else {
            return ONLP_FAN_STATUS_B2F;
        }
    }

    return 0;
}


static int
_onlp_fani_info_get_fan_on_psu(int fid, onlp_fan_info_t* info)
{
    int   value, ret;
    char  vstr[32], *vstrp = vstr, **vp = &vstrp;


    info->status |= ONLP_FAN_STATUS_PRESENT;

    /* get fan direction */
    info->status |= _onlp_get_fan_direction_on_psu();

    /* get front fan speed */
    memset(vstr, 0, 32);
    ret = onlp_file_read_str(vp, devfiles__[fid], inv_psoc_id);
    if (ret < 0) {
        return ONLP_STATUS_E_INTERNAL;
    }
    sscanf(*vp, "%d", &value);
    info->rpm = value;
    info->percentage = (info->rpm * 100) / MAX_PSU_FAN_SPEED;
    info->status |= (value == 0) ? ONLP_FAN_STATUS_FAILED : 0;
    return ONLP_STATUS_OK;
}


/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rc = 0;
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];

    switch (local_id)
    {
	case FAN_1_ON_PSU1:
        case FAN_1_ON_PSU2:
            rc = _onlp_fani_info_get_fan_on_psu(local_id, info);
            break;
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
            rc = _onlp_fani_info_get_fan(local_id, info);
            break;
        default:
            rc = ONLP_STATUS_E_INVALID;
            break;
    }

    return rc;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    int  fid;
    char *path = NULL;

    VALIDATE(id);

    fid = ONLP_OID_ID_GET(id);

    /* reject p=0 (p=0, stop fan) */
    if (p == 0){
        return ONLP_STATUS_E_INVALID;
    }

    switch (fid)
        {
        case FAN_1_ON_PSU1:
                        return psu_pmbus_info_set(PSU1_ID, "rpm_psu", p);
        case FAN_1_ON_PSU2:
                        return psu_pmbus_info_set(PSU2_ID, "rpm_psu", p);
        case FAN_1_ON_MAIN_BOARD:
        case FAN_2_ON_MAIN_BOARD:
        case FAN_3_ON_MAIN_BOARD:
        case FAN_4_ON_MAIN_BOARD:
        case FAN_5_ON_MAIN_BOARD:
        case FAN_6_ON_MAIN_BOARD:
        case FAN_7_ON_MAIN_BOARD:
        case FAN_8_ON_MAIN_BOARD:
                        path = FAN_NODE(fan_duty_cycle_percentage);
                        break;
        default:
            return ONLP_STATUS_E_INVALID;
    }

    if (onlp_file_write_int(p, path, NULL) != 0) {
        AIM_LOG_ERROR("Unable to write data to file (%s)\r\n", path);
        return ONLP_STATUS_E_INTERNAL;
    }

        return ONLP_STATUS_OK;
}