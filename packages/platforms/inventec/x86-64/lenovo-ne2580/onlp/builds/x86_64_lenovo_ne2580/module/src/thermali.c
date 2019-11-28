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
#include <fcntl.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_THERMAL(_id)) {         \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

static char* devfiles__[THERMAL_MAX] =  /* must map with onlp_thermal_id */
{
    "reserved",
    INV_CPLD_PREFIX"/temp0_cpu_%s",
    INV_CPLD_PREFIX"/temp1_ne2580_%s",
    INV_CPLD_PREFIX"/temp2_ne2580_%s",
    INV_CPLD_PREFIX"/temp3_ne2580_%s",
    INV_CPLD_PREFIX"/thermal_psu1",
    INV_CPLD_PREFIX"/thermal2_psu1",
    INV_CPLD_PREFIX"/thermal_psu2",
    INV_CPLD_PREFIX"/thermal2_psu2",
};

/* Static values */
static onlp_thermal_info_t linfo[THERMAL_MAX] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_0), "CPU Core 0", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_SWITCH_BROAD), "NE2580 Thermal Sensor 1", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_SWITCH_BROAD), "NE2580 Thermal Sensor 2", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_SWITCH_BROAD), "NE2580 Thermal Sensor 3", 0},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU1), "PSU-1 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU1_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	},
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_PSU2), "PSU-2 Thermal Sensor 2", ONLP_PSU_ID_CREATE(PSU2_ID)},
	    ONLP_THERMAL_STATUS_PRESENT,
	    ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
	}
};

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    return ONLP_STATUS_OK;
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
    int local_id, ret;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[local_id];

    if(local_id >= THERMAL_CPU_CORE_FIRST && local_id <= THERMAL_CPU_CORE_LAST) {
	char desc[32], *dp = &desc[0];
        int rv = onlp_file_read_str(&dp, devfiles__[local_id], "label");
        if (rv > 0) {
            memset (info->hdr.description, 0, ONLP_OID_DESC_SIZE);
            strncpy(info->hdr.description, dp, rv);
        }

        /* Set the onlp_oid_hdr_t and capabilities */
        ret = onlp_file_read_int(&info->mcelsius, devfiles__[local_id], "input");
	if (ret < 0) {
	    info->mcelsius = 0;
	    ret = 0;
	}
	return ret;
    }
    ret = onlp_file_read_int(&info->mcelsius, devfiles__[local_id]);
    if (ret < 0) {
	info->mcelsius = 0;
	ret = 0;
    }
    return ret;
}
