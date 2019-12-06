/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright 2014 Accton Technology Corporation.
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
#include <unistd.h>
#include <fcntl.h>

#include <onlplib/file.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#include "x86_64_accton_minipack_int.h"
#include "x86_64_accton_minipack_log.h"

#define IDPROM_PATH "/sys/bus/i2c/devices/1-0057/eeprom"

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-minipack-r0";
}

#define TLV_START_OFFSET    512
#define TLV_DATA_LENGTH     256

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    FILE* fp;
    uint8_t* rdata = aim_zmalloc(TLV_DATA_LENGTH);

    /* Temporary solution.
     * The very start part of eeprom is in other format.
     * Real TLV info locates at where else.
     */
    fp = fopen(IDPROM_PATH, "r");
    if(fp == NULL) {
        AIM_LOG_ERROR("Unable to open file of (%s)", IDPROM_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    if (fseek(fp, TLV_START_OFFSET, SEEK_CUR) != 0) {
        fclose(fp);
        AIM_LOG_ERROR("Unable to open file of (%s)", IDPROM_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }

    *size = fread(rdata, 1, TLV_DATA_LENGTH, fp);
    fclose(fp);
    if(*size == TLV_DATA_LENGTH) {
        *data = rdata;
        return ONLP_STATUS_OK;
    }

    aim_free(rdata);
    *size = 0;
    return ONLP_STATUS_E_INTERNAL;
}


void
onlp_sysi_onie_data_free(uint8_t* data)
{
    /*If onlp_sysi_onie_data_get() allocated, it has be freed here.*/
    if (data) {
        aim_free(data);
    }
}


int
onlp_sysi_oids_get(onlp_oid_t* table, int max)
{
    int i;
    onlp_oid_t* e = table;
    memset(table, 0, max*sizeof(onlp_oid_t));

    /* 8 Thermal sensors on the chassis */
    for (i = 1; i <= CHASSIS_THERMAL_COUNT; i++) {
        *e++ = ONLP_THERMAL_ID_CREATE(i);
    }

    /* 2 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 5 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    return ONLP_STATUS_OK;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
}

int
onlp_sysi_platform_manage_fans(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

