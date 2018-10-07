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

enum onlp_thermal_id
{
    THERMAL_RESERVED = 0,
    THERMAL_CPU_CORE_FIRST,
    THERMAL_CPU_CORE_2,
    THERMAL_CPU_CORE_3,
    THERMAL_CPU_CORE_LAST,
    THERMAL_1_ON_MAIN_BROAD,
    THERMAL_2_ON_MAIN_BROAD,
    THERMAL_3_ON_MAIN_BROAD,
    THERMAL_4_ON_MAIN_BROAD,
    THERMAL_5_ON_MAIN_BROAD,
    THERMAL_1_ON_PSU1,
    THERMAL_1_ON_PSU2,
};

static char* cpu_coretemp_files[] =
    {
	"reserved",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp2_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp3_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp4_input",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp5_input",
        NULL,
    };

static char* cpu_coretemp_lable[] =
    {
	"reserved",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp2_label",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp3_label",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp4_label",
        "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/temp5_label",
        NULL,
    };

static char* devfiles__[] =  /* must map with onlp_thermal_id */
{
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "reserved",
    "/sys/bus/i2c/devices/1-0066/temp1_input",
    "/sys/bus/i2c/devices/1-0066/temp2_input",
    "/sys/bus/i2c/devices/1-0066/temp3_input",
    "/sys/bus/i2c/devices/1-0066/temp4_input",
    "/sys/bus/i2c/devices/1-0066/temp5_input",
    "/sys/bus/i2c/devices/1-0066/thermal_psu1",
    "/sys/bus/i2c/devices/1-0066/thermal_psu2",
};

/* Static values */
static onlp_thermal_info_t linfo[] = {
	{ }, /* Not used */
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_FIRST), "CPU Core 2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_2), "CPU Core 3", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_3), "CPU Core 4", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_CPU_CORE_LAST), "CPU Core 5", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },

	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_MAIN_BROAD), "Chassis Thermal Sensor 1", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_2_ON_MAIN_BROAD), "Chassis Thermal Sensor 2", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_3_ON_MAIN_BROAD), "Chassis Thermal Sensor 3", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_4_ON_MAIN_BROAD), "Chassis Thermal Sensor 4", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_5_ON_MAIN_BROAD), "Chassis Thermal Sensor 5", 0},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU1), "PSU-1 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU1_ID)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        },
	{ { ONLP_THERMAL_ID_CREATE(THERMAL_1_ON_PSU2), "PSU-2 Thermal Sensor 1", ONLP_PSU_ID_CREATE(PSU2_ID)},
            ONLP_THERMAL_STATUS_PRESENT,
            ONLP_THERMAL_CAPS_ALL, 0, ONLP_THERMAL_THRESHOLD_INIT_DEFAULTS
        }
};

static int coretemp_id = 0;

/*
 * This will be called to intiialize the thermali subsystem.
 */
int
onlp_thermali_init(void)
{
    char coret[32], *ctp = &coret[0];

    if (ctp) {
	int i, rv;
	for (i = 0; i < 10; i++) {
	    rv = onlp_file_read_str(&ctp, "/sys/devices/platform/coretemp.0/hwmon/hwmon%d/name", i);
            if (rv == 8 && strncmp("coretemp", ctp, rv) == 0) {
		coretemp_id = i;
		break;
	    }
	}
    }

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
    char desc[32], *dp = &desc[0];
    int local_id;
    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);
    *info = linfo[local_id];

    if(local_id >= THERMAL_CPU_CORE_FIRST && local_id <= THERMAL_CPU_CORE_LAST) {
	int rv;
	if (dp) {
	    rv = onlp_file_read_str(&dp, cpu_coretemp_lable[local_id], coretemp_id);
	    if (rv > 0) {
		memset (info->hdr.description, 0, ONLP_OID_DESC_SIZE);
		strncpy(info->hdr.description, dp, rv);
	    }
	}

	/* Set the onlp_oid_hdr_t and capabilities */
        rv = onlp_file_read_int(&info->mcelsius, cpu_coretemp_files[local_id], coretemp_id);
        return rv;
    }

    return onlp_file_read_int(&info->mcelsius, devfiles__[local_id]);
}