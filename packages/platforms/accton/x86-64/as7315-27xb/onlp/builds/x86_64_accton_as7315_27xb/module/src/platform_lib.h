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
#ifndef __PLATFORM_LIB_H__
#define __PLATFORM_LIB_H__

#include "x86_64_accton_as7315_27xb_log.h"

#define CHASSIS_FAN_COUNT		5
#define CHASSIS_THERMAL_COUNT	4
#define CHASSIS_LED_COUNT		3
#define CHASSIS_PSU_COUNT		2

#define PSU1_ID 1
#define PSU2_ID 2

#define PSU_SYSFS_PATH  "/sys/bus/i2c/devices/%d-00%02x/"



#define IDPROM_PATH "/sys/bus/i2c/devices/4-0057/eeprom"

#define DEBUG_MODE 0

#if (DEBUG_MODE == 1)
#define DEBUG_PRINT(fmt, args...)                                        \
		printf("%s:%s[%d]: " fmt "\r\n", __FILE__, __FUNCTION__, __LINE__, ##args)
#else
#define DEBUG_PRINT(fmt, args...)
#endif

#endif  /* __PLATFORM_LIB_H__ */

