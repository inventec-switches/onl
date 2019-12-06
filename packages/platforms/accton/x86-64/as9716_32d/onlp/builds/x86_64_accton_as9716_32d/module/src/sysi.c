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
#include <limits.h>

#include <onlplib/i2c.h>
#include <onlp/platformi/sysi.h>
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/thermali.h>
#include <onlp/platformi/fani.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"
#include "x86_64_accton_as9716_32d_int.h"
#include "x86_64_accton_as9716_32d_log.h"

#define NUM_OF_FAN_ON_MAIN_BROAD      6
#define PREFIX_PATH_ON_CPLD_DEV          "/sys/bus/i2c/devices/"
#define NUM_OF_CPLD                      3
#define FAN_DUTY_CYCLE_MAX         (100)
#define FAN_DUTY_CYCLE_75     (75)
#define FAN_DUTY_CYCLE_50     (50)

static char arr_cplddev_name[NUM_OF_CPLD][10] =
{
 "4-0060",
 "5-0062",
 "6-0064"
};

const char*
onlp_sysi_platform_get(void)
{
    return "x86-64-accton-as9716-32d-r0";
}

int
onlp_sysi_onie_data_get(uint8_t** data, int* size)
{
    int ret = ONLP_STATUS_OK;
    int i = 0;
    uint8_t* rdata = aim_zmalloc(256);
  
    for (i = 0; i < 128; i++) {
        ret = onlp_i2c_readw(0, 0x56, i*2, ONLP_I2C_F_FORCE);
        if (ret < 0) {
            aim_free(rdata);
            *size = 0;
            return ret;
        }

        rdata[i*2]   = ret & 0xff;
        rdata[i*2+1] = (ret >> 8) & 0xff;
    }

    *size = 256;
    *data = rdata;

    return ONLP_STATUS_OK;
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

    /* 5 LEDs on the chassis */
    for (i = 1; i <= CHASSIS_LED_COUNT; i++) {
        *e++ = ONLP_LED_ID_CREATE(i);
    }

    /* 2 PSUs on the chassis */
    for (i = 1; i <= CHASSIS_PSU_COUNT; i++) {
        *e++ = ONLP_PSU_ID_CREATE(i);
    }

    /* 6 Fans on the chassis */
    for (i = 1; i <= CHASSIS_FAN_COUNT; i++) {
        *e++ = ONLP_FAN_ID_CREATE(i);
    }

    return 0;
}

int
onlp_sysi_platform_info_get(onlp_platform_info_t* pi)
{
    int   i, v[NUM_OF_CPLD]={0};
	
    for (i = 0; i < NUM_OF_CPLD; i++) {
        v[i] = 0;
		
        if(onlp_file_read_int(v+i, "%s%s/version", PREFIX_PATH_ON_CPLD_DEV, arr_cplddev_name[i]) < 0) {
            return ONLP_STATUS_E_INTERNAL;
        }
    }
    pi->cpld_versions = aim_fstrdup("%d.%d.%d", v[0], v[1], v[2]);
	
    return 0;
}

void
onlp_sysi_platform_info_free(onlp_platform_info_t* pi)
{
    aim_free(pi->cpld_versions);
}

/*Read fanN_direction=1: The air flow of Fan6 is ��AFI-Back to Front��
 *                    0: The air flow of Fan6 is ��AFO-Front to back��
 */
/*
 Thermal policy:
 One fan fail, set to fan pwm=100%
 1. For AFI:
 Default fan pwm will be 75%(LEVEL_FAN_DEF).
 When below case,  Fan pwm will be 100%(LEVEL_FAN_DAX)
 (MB board)
 LM75-1(0X48)��61.5�J
 LM75-2(0X49)��51.5�J
 LM75-3(0X4A)��49.4�J
 LM75-4(0X4C)��49.4�J
 LM75-5(0X4E)��45.1�J
 LM75-6(0X4F)��46.75�J
 (CPU board)
 Core��48 
 LM75-1(0X4B)��38.5�J

 When FAN pwm was 100%(LEVEL_FAN_MAX). If all below case meet with, set to 75%(LEVEL_FAN_DEF).
 MB board
 LM75-1(0X48)��57�J
 LM75-2(0X49)��47.3�J
 LM75-3(0X4A)��45�J
 LM75-4(0X4C)��45.1�J
 LM75-5(0X4E)��40.75�J
 LM75-6(0X4F)��42.1�J
 CPU board
 Core��44 
 LM75-1(0X4B)��35�J
 
 2. For AFO:
 Default fan pwm will be 50%(LEVEL_FAN_DEF).
 When below case, Fan pwm will be 75%(LEVEL_FAN_MID)
 (MB board)
 LM75-1(0X48)��67�J
 LM75-2(0X49)��62.5�J
 LM75-3(0X4A)��65�J
 LM75-4(0X4C)��59�J
 LM75-5(0X4E)��58.5�J
 LM75-6(0X4F)��63�J
 (CPU board)
 Core >=69�J
 LM75-1(0X4B)��49�J
 
 When FAN pwm was 75%(LEVEL_FAN_MID). If all below case meet with, set to 100%(LEVEL_FAN_MAX).
 (MB board)
 LM75-1(0X48)��70�J
 LM75-2(0X49)��66�J
 LM75-3(0X4A)��68�J
 LM75-4(0X4C)��62�J
 LM75-5(0X4E)��62�J
 LM75-6(0X4F)��67�J
 (CPU board)
 Core��77
 LM75-1(0X4B)��50�J

 When FAN pwm was 100%(LEVEL_FAN_MAX). If all below case meet with, set to 75%(LEVEL_FAN_MID).
 (MB board)
 LM75-1(0X48)��59�J
 LM75-2(0X49)��53.5�J
 LM75-3(0X4A)��55.3�J
 LM75-4(0X4C)��50.3�J
 LM75-5(0X4E)��50�J
 LM75-6(0X4F)��52.5�J
 (CPU board)
 Core��59
 LM75-1(0X4B)��41.1�J

 When FAN pwm was 75%(LEVEL_FAN_MID). If all below case meet with, set to 50%(LEVEL_FAN_DEF).
 (MB board)
 LM75-1(0X48)��55.8�J
 LM75-2(0X49)��50.5�J
 LM75-3(0X4A)��51.1�J
 LM75-4(0X4C)��47.6�J
 LM75-5(0X4E)��45.75�J
 LM75-6(0X4F)��50.1�J
 (CPU board)
 Core��57
 LM75-1(0X4B)��36.6�J


 */

typedef struct afi_temp_range{
    int def_to_max_temp[8];
    int max_to_def_temp[8];
}afi_temp_range_t;
 
afi_temp_range_t afi_thermal_spec={
 {61500, 51500, 49400, 49400, 45100, 46750, 48000, 38500},
 {57000, 47300, 45000, 45100, 40750, 42100, 44000, 35000}
};

typedef struct afo_temp_range{
    int def_to_mid_temp[8];
    int mid_to_max_temp[8];
    int max_to_mid_temp[8];
    int mid_to_def_temp[8];    
} afo_temp_range_t; 
  
 
afo_temp_range_t afo_thermal_spec={
 {67000, 62000, 65000, 59000, 58500, 63000, 69000, 49000},
 {70000, 66000, 68000, 62000, 62000, 67000, 77000, 50000},
 {59000, 53500, 55300, 50300, 50000, 52500, 59000, 41100},
 {55800, 50500, 51100, 47600, 45750, 50100, 57000, 36600}
};


typedef struct fan_ctrl_policy {
   int duty_cycle; 
   int pwm;
   int state;
} fan_ctrl_policy_t;
   
   
/*For AFI. 2 state. LEVEL_FAN_DEF(75%), LEVEL_FAN_MAX(100%)
  For AFO. 3 state. LEVEL_FAN_DEF(50%), LEVEL_FAN_MID(75%), LEVEL_FAN_MAX(100%)
 */
enum
{
   LEVEL_FAN_INIT=0,
   LEVEL_FAN_DEF=1,
   LEVEL_FAN_MID,
   LEVEL_FAN_MAX,   
};

fan_ctrl_policy_t  fan_thermal_policy_f2b[] = { /*AFO*/
{50,  0x7, LEVEL_FAN_DEF},
{75,  0xb, LEVEL_FAN_MID},
{100, 0xf, LEVEL_FAN_MAX},
    
};

fan_ctrl_policy_t  fan_thermal_policy_b2f[] = { /*AFI*/
{75,  0xb, LEVEL_FAN_DEF},
{100, 0xf, LEVEL_FAN_MAX}
};
    
int onlp_sysi_get_duty_cycle_by_fan_state(int state, int direction)
{
    int i;
    if(direction)
    {
        for(i=0; i< 2; i++)
        {
            if(state==fan_thermal_policy_b2f[i].state)
            {
                return fan_thermal_policy_b2f[i].duty_cycle;
            }
        }
        
    }
    else
    {
        for(i=0; i< 3; i++)
        {
            if(state==fan_thermal_policy_f2b[i].state)
            {
                return fan_thermal_policy_f2b[i].duty_cycle;
            }
        }
    }
    return 0;     
   
}    

#define FAN_SPEED_CTRL_PATH "/sys/bus/i2c/devices/17-0066/fan_duty_cycle_percentage"
#define FAN_DIRECTION_PATH "/sys/bus/i2c/devices/17-0066/fan1_direction"
    
static int fan_state=LEVEL_FAN_INIT;
static int fan_fail = 0;

int current_duty_cycle, new_duty_cycle;

int onlp_sysi_platform_manage_fans(void)
{
    int i=0,k=0, ori_state=LEVEL_FAN_DEF, current_state=LEVEL_FAN_DEF;
    int fd, len, direction_val=1;
    int max_to_def=0, max_to_mid=0, mid_to_def=0; /* Only this flag equal to 8, otherwise state can not to be down.*/
    onlp_thermal_info_t thermal[8];
    char  buf[10] = {0};
    
    /* Get fan direction
     */
    if (onlp_file_read_int(&direction_val, FAN_DIRECTION_PATH) < 0) {
        AIM_LOG_ERROR("Unable to read status from file (%s)\r\n", FAN_DIRECTION_PATH); 
    }
    
    /* Get current temperature
     */

    for (i=2; i <5; i++)
    {
        
        if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal[k]) != ONLP_STATUS_OK  )
        {
            AIM_LOG_ERROR("Unable to read thermal status, set fans to 75% speed");
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), 75);
        return ONLP_STATUS_E_INTERNAL;
    }
        k++; 
}    
    for (i=6; i <=8; i++)
    {
        if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(i), &thermal[k]) != ONLP_STATUS_OK  )
        {
            AIM_LOG_ERROR("Unable to read thermal status, set fans to 75% speed");
            onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(i), 75);
            return ONLP_STATUS_E_INTERNAL;
        }
        k++; 
    }
    
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(1), &thermal[6]) != ONLP_STATUS_OK  )
    {
        AIM_LOG_ERROR("Unable to read thermal status, set fans to 75% speed");
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), 75);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (onlp_thermali_info_get(ONLP_THERMAL_ID_CREATE(5), &thermal[7]) != ONLP_STATUS_OK  )
    {
        AIM_LOG_ERROR("Unable to read thermal status, set fans to 75% speed");
        onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(5), 75);
        return ONLP_STATUS_E_INTERNAL;
    }
    
    /* Get current fan pwm percent
     */
    fd = open(FAN_SPEED_CTRL_PATH, O_RDONLY);
    if (fd == -1){
        AIM_LOG_ERROR("Unable to open fan speed control node (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
    }
    len = read(fd, buf, sizeof(buf));
    close(fd);    
    if (len <= 0) {
        AIM_LOG_ERROR("Unable to read fan speed from (%s)", FAN_SPEED_CTRL_PATH);
        return ONLP_STATUS_E_INTERNAL;
}

    current_duty_cycle = atoi(buf);

    current_state=ori_state=fan_state;

    /* Check thermal is in which range. */
    max_to_def=0;
    if(direction_val==1) /* AFI */
    {
        if(fan_state==LEVEL_FAN_INIT)
        {
            current_state=LEVEL_FAN_DEF;
        }
        for (i=0; i <CHASSIS_THERMAL_COUNT; i++)
        {
            if (thermal[i].mcelsius >= afi_thermal_spec.def_to_max_temp[i])
            {
               current_state=LEVEL_FAN_MAX;
               break;  
            }
            if (thermal[i].mcelsius <= afi_thermal_spec.def_to_max_temp[i])
            {
                max_to_def++;
            }            
        }
        if(max_to_def==CHASSIS_THERMAL_COUNT && fan_state==LEVEL_FAN_MAX)
        {
            current_state=LEVEL_FAN_DEF;
        }
    }
    else  /* AFO */
    {
        if(fan_state==LEVEL_FAN_INIT)
        {
            current_state=LEVEL_FAN_DEF;
        }
        for (i=0; i <CHASSIS_THERMAL_COUNT; i++)
        {
            if (thermal[i].mcelsius >= afo_thermal_spec.def_to_mid_temp[i])
            {
               current_state=LEVEL_FAN_MID;               
            }
            if (thermal[i].mcelsius >= afo_thermal_spec.mid_to_max_temp[i])
            {
                current_state=LEVEL_FAN_MAX;
        	    break;
            }
            if (thermal[i].mcelsius <= afo_thermal_spec.max_to_mid_temp[i])
            {
                max_to_mid++;
            }
            if (thermal[i].mcelsius <= afo_thermal_spec.mid_to_def_temp[i])
            {
                mid_to_def++;
            }
        }
        if(max_to_mid==CHASSIS_THERMAL_COUNT && fan_state==LEVEL_FAN_MAX)
        {
            current_state=LEVEL_FAN_MID;
        }
        if(mid_to_def==CHASSIS_THERMAL_COUNT && fan_state==LEVEL_FAN_MID)
        {
            current_state=LEVEL_FAN_DEF;
        } 

    }
   // fan_state = current_state;
    /*
    for(i=0; i < sizeof(fan_thermal_policy_f2b)/sizeof(fan_ctrl_policy_t); i++)
    {
        if (temp > fan_thermal_policy[i].temp_down)
        {           
            if (temp <= fan_thermal_policy[i].temp_up)
            {
                current_state =i;
            }
        }
    }
    
    */
  
  
    /* Get each fan status
     */
    for (i = 1; i <= NUM_OF_FAN_ON_MAIN_BROAD; i++)
    {
        onlp_fan_info_t fan_info;

        if (onlp_fani_info_get(ONLP_FAN_ID_CREATE(i), &fan_info) != ONLP_STATUS_OK) {
            AIM_LOG_ERROR("Unable to get fan(%d) status, try to set the other fans as full speed\r\n", i);
            if(current_duty_cycle != FAN_DUTY_CYCLE_MAX)
            {
                onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
    }
            fan_fail=1;
        	    break;
        	}
        /* Decision 1: Set fan as full speed if any fan is failed.
         */
        if (fan_info.status & ONLP_FAN_STATUS_FAILED || !(fan_info.status & ONLP_FAN_STATUS_PRESENT)) {
            AIM_LOG_ERROR("Fan(%d) is not working, set the other fans as full speed\r\n", i);
            if(current_duty_cycle != FAN_DUTY_CYCLE_MAX)
            {
                onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
        }
            fan_fail=1;
        	    break;
            }            	    
    	}
    
    
    if(current_state!=ori_state)
    {
        fan_state=current_state;
        new_duty_cycle=onlp_sysi_get_duty_cycle_by_fan_state(current_state, direction_val);
        if(new_duty_cycle!=current_duty_cycle && !fan_fail)
        {
             onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), new_duty_cycle); 
        }
        if(!new_duty_cycle && !fan_fail)
        {
             onlp_fani_percentage_set(ONLP_FAN_ID_CREATE(1), FAN_DUTY_CYCLE_MAX);
        }

         switch (ori_state)
         {
             case LEVEL_FAN_INIT:                 
                 break;
             case LEVEL_FAN_DEF:
                 break;
             case LEVEL_FAN_MID:
                 break;
             case LEVEL_FAN_MAX:
                 break;
             default:
                AIM_SYSLOG_WARN("onlp_sysi_platform_manage_fans abnormal state", "onlp_sysi_platform_manage_fans  abnormal state", "onlp_sysi_platform_manage_fans at abnormal state\n");
                 break;
        }
    
    }
  
    return 0;
}   



int
onlp_sysi_platform_manage_leds(void)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}
   
