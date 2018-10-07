/************************************************************
 * platform_lib.h
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_inventec_d7264q28b_log.h"

#define D7264Q28B_CPLD_COUNT	(2)

#define CHASSIS_LED_COUNT	(5)
#define CHASSIS_PSU_COUNT	(2)
#define CHASSIS_FAN_COUNT	(10)
#define CHASSIS_THERMAL_COUNT	(14)

enum onlp_d7264q28b_psu_id
{
    PSU_RESERVED = 0,
    PSU1_ID,
    PSU2_ID
};

#define INV_CTMP_PREFIX		"/sys/devices/platform/coretemp.0/hwmon/hwmon0/"
#define INV_PSOC_PREFIX		"/sys/devices/virtual/hwmon/hwmon1/"
#define INV_CPLD_PREFIX		"/sys/bus/i2c/devices/0-0055/"
#define INV_CPLD2_PREFIX	"/sys/bus/i2c/devices/0-0077/"
#define INV_EPRM_PREFIX		"/sys/bus/i2c/devices/0-0053/"

#define PSU_HWMON_PSOC_PREFIX	INV_PSOC_PREFIX
#define PSU_HWMON_CPLD_PREFIX	INV_CPLD_PREFIX

#define PSU1_AC_PMBUS_PREFIX	PSU_HWMON_PSOC_PREFIX
#define PSU2_AC_PMBUS_PREFIX	PSU_HWMON_PSOC_PREFIX

#define PSU1_AC_PMBUS_NODE(node) PSU1_AC_PMBUS_PREFIX#node
#define PSU2_AC_PMBUS_NODE(node) PSU2_AC_PMBUS_PREFIX#node

#define PSU1_AC_HWMON_PREFIX	PSU_HWMON_CPLD_PREFIX
#define PSU2_AC_HWMON_PREFIX	PSU_HWMON_CPLD_PREFIX

#define PSU1_AC_HWMON_NODE(node) PSU1_AC_HWMON_PREFIX#node
#define PSU2_AC_HWMON_NODE(node) PSU2_AC_HWMON_PREFIX#node

#define FAN_BOARD_PATH	"/sys/devices/platform/fan/"
#define FAN_NODE(node)	FAN_BOARD_PATH#node

#define IDPROM_PATH     INV_EPRM_PREFIX"/eeprom"

int onlp_file_read_binary(char *filename, char *buffer, int buf_size, int data_len);
int onlp_file_read_string(char *filename, char *buffer, int buf_size, int data_len);

int psu_pmbus_info_get(int id, char *node, int *value);
int psu_pmbus_info_set(int id, char *node, int value);

int inv_get_psoc_id(void);
int inv_get_cpld_id(void);
int inv_get_cpld2_id(void);
int inv_get_coretemp_id(void);

typedef enum psu_type {
    PSU_TYPE_UNKNOWN,
    PSU_TYPE_AC_F2B,
    PSU_TYPE_AC_B2F,
    PSU_TYPE_DC_48V_F2B,
    PSU_TYPE_DC_48V_B2F,
    PSU_TYPE_DC_12V_FANLESS,
    PSU_TYPE_DC_12V_F2B,
    PSU_TYPE_DC_12V_B2F
} psu_type_t;

psu_type_t get_psu_type(int id, char* modelname, int modelname_len);

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
    #define DEBUG_PRINT(format, ...)   printf(format, __VA_ARGS__)
#else
    #define DEBUG_PRINT(format, ...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

