/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *        Copyright 2014, 2015 Big Switch Networks, Inc.
 *        Copyright 2018 Alpha Networks Incorporation.
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
 * LED Management
 *
 ***********************************************************/
#include <onlp/platformi/ledi.h>
#include <onlp/platformi/fani.h>
#include <sys/mman.h>
#include <stdio.h>
#include "platform_lib.h"

#include <onlplib/mmap.h>
//#include "onlpie_int.h"

#include <pthread.h>

#define CPLD_PSU_LED_ADDRESS_OFFSET     0x0A //PSU0 [1:0], PSU1 [3:2]
#define CPLD_POWER_LED_ADDRESS_OFFSET     0x0B //power [1:0]

#define PCA9539_NUM2_I2C_BUS_ID           0
#define PCA9539_NUM2_I2C_ADDR              0x75  /* PCA9539#2 Physical Address in the I2C */

#define PCA9539_NUM2_IO0_INPUT_OFFSET   0x00 /* used to read data */
#define PCA9539_NUM2_IO0_OUTPUT_OFFSET   0x02 /* used to write data*/

/* configures the directions of the I/O pin. 1:input, 0:output */
#define PCA9539_NUM2_IO0_DIRECTION_OFFSET   0x06 
#define PCA9539_NUM2_IO1_DIRECTION_OFFSET   0x07 

#define PCA9539_NUM2_SYSTEM_LED_Y_BIT    0
#define PCA9539_NUM2_SYSTEM_LED_G_BIT    1
#define PCA9539_NUM2_FAN_LED_Y_BIT    2
#define PCA9539_NUM2_FAN_LED_G_BIT    3


/* 
  * reference DCGS_TYPE1_ Power_CPLD_Spec_v05_20190820, chap 3.13. 
  * 1. Bit[0] status is got from signal  V3P3_SB_PG and V2P5_SB_PG and V1P2_SB_PG and V1P15_SB_PG and V1P8_SB_PG. 
  * 2. Bit[1] when set to 1 ,PWR_LED_Y is blinking;
  * 3. Bit[1] when set to 0 ,PWR_LED_Y is depended by Bit[0] -- > Bit[0] = 0, PWR_LED_Y is blinking
  *                                                                                             -- > Bit[0] = 1, PWR_LED_Y is off
  */
#define POWER_LED_OFF                     0x00
#define POWER_LED_GREEN_SOLID             0x01
#define POWER_LED_AMBER_BLINKING_BIT0_L          0x02
#define POWER_LED_AMBER_BLINKING_BIT0_H          0x03 

/*
  * Bit[0] status is got from (PSU0_PRESENT_L = '0') and (PSU0_PWOK_L = '0'). 
  * Bit[1] when set to 1 , PSU0_LED_Y is blinking;
  * Bit[1] when set to 0 , PSU0_LED_Y is depended by Bit[0] -- > Bit[0] = 0, PSU0_LED_Y is blinking
  *                                                                                            -- > Bit[0] = 1, PSU0_LED_Y is off
  */
#define PSU_LED_OFF                     0x00
#define PSU_LED_GREEN_SOLID             0x01
#define PSU_LED_AMBER_BLINKING_BIT0_L          0x02
#define PSU_LED_AMBER_BLINKING_BIT0_H          0x03

#define SYSTEM_LED_OFF                     0x00
#define SYSTEM_LED_AMBER_SOLID             0x01
#define SYSTEM_LED_GREEN_SOLID             0x02
#define SYSTEM_LED_GREEN_BLINKING          0x03 

#define FAN_LED_OFF                     0x00
#define FAN_LED_AMBER_SOLID             0x01
#define FAN_LED_GREEN_SOLID             0x02

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_LED(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)

struct led_id_mode
{
    enum onlp_led_id led_id;
    onlp_led_mode_t mode;
    int hw_led_light_mode;
};

static struct led_id_mode led_id_mode_data[] = {
    { LED_POWER, ONLP_LED_MODE_OFF, POWER_LED_OFF },
    { LED_POWER, ONLP_LED_MODE_GREEN, POWER_LED_GREEN_SOLID },
    { LED_POWER, ONLP_LED_MODE_ORANGE_BLINKING, POWER_LED_AMBER_BLINKING_BIT0_L},
    { LED_POWER, ONLP_LED_MODE_ORANGE_BLINKING, POWER_LED_AMBER_BLINKING_BIT0_H},    
    { LED_POWER, ONLP_LED_MODE_ON, POWER_LED_GREEN_SOLID },

    { LED_PSU1, ONLP_LED_MODE_OFF, PSU_LED_OFF },
    { LED_PSU1, ONLP_LED_MODE_GREEN, PSU_LED_GREEN_SOLID },
    { LED_PSU1, ONLP_LED_MODE_ORANGE_BLINKING, PSU_LED_AMBER_BLINKING_BIT0_L},
    { LED_PSU1, ONLP_LED_MODE_ORANGE_BLINKING, PSU_LED_AMBER_BLINKING_BIT0_H},    
    { LED_PSU1, ONLP_LED_MODE_ON, PSU_LED_GREEN_SOLID },

    { LED_PSU2, ONLP_LED_MODE_OFF, PSU_LED_OFF },
    { LED_PSU2, ONLP_LED_MODE_GREEN, PSU_LED_GREEN_SOLID },
    { LED_PSU2, ONLP_LED_MODE_ORANGE_BLINKING, PSU_LED_AMBER_BLINKING_BIT0_L},
    { LED_PSU2, ONLP_LED_MODE_ORANGE_BLINKING, PSU_LED_AMBER_BLINKING_BIT0_H},    
    { LED_PSU2, ONLP_LED_MODE_ON, PSU_LED_GREEN_SOLID },

    { LED_SYSTEM, ONLP_LED_MODE_OFF, SYSTEM_LED_OFF },
    { LED_SYSTEM, ONLP_LED_MODE_GREEN, SYSTEM_LED_GREEN_SOLID },
    { LED_SYSTEM, ONLP_LED_MODE_GREEN_BLINKING, SYSTEM_LED_GREEN_BLINKING },
    { LED_SYSTEM, ONLP_LED_MODE_ORANGE, SYSTEM_LED_AMBER_SOLID },
    { LED_SYSTEM, ONLP_LED_MODE_ON, SYSTEM_LED_GREEN_SOLID },

    { LED_FAN, ONLP_LED_MODE_OFF, FAN_LED_OFF },
    { LED_FAN, ONLP_LED_MODE_GREEN, FAN_LED_GREEN_SOLID },
    { LED_FAN, ONLP_LED_MODE_ORANGE, FAN_LED_AMBER_SOLID },
    { LED_FAN, ONLP_LED_MODE_ON, FAN_LED_GREEN_SOLID },   
};

typedef union
{
    unsigned char val;
    struct
    {
        unsigned power :2;
        unsigned char :6;  /* reserved */
    }bit;

}_CPLD_POWER_LED_REG_T;

typedef union
{
    unsigned char val;
    struct
    {
        unsigned char psu0 :2;
        unsigned char psu1 :2;
        unsigned char :4;  /* reserved */
    }bit;

}_CPLD_PSU_LED_REG_T;

typedef union
{
    unsigned char val;
    struct
    {
        unsigned system :2;
        unsigned char fan_err :2;
        unsigned char :4;  /* reserved */
    }bit;

}_PCA9539_NUM2_SYS_FAN_LED_REG_T;


/*
 * Get the information for the given LED OID.
 */
static onlp_led_info_t linfo[] =
{
    { }, /* Not used */
    {
        { ONLP_LED_ID_CREATE(LED_POWER), "Chassis LED 1 (POWER LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU1), "Chassis LED 2 (PSU1 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_PSU2), "Chassis LED 3 (PSU2 LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE_BLINKING,
    },
    {
        { ONLP_LED_ID_CREATE(LED_SYSTEM), "Chassis LED 4 (SYSTEM LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN | ONLP_LED_CAPS_GREEN_BLINKING | ONLP_LED_CAPS_ORANGE,
    },
    {
        { ONLP_LED_ID_CREATE(LED_FAN), "Chassis LED 5 (FAN LED)", 0 },
        ONLP_LED_STATUS_PRESENT,
        ONLP_LED_CAPS_ON_OFF | ONLP_LED_CAPS_GREEN| ONLP_LED_CAPS_ORANGE,
    },
};

static int convert_hw_led_light_mode_to_onlp(enum onlp_led_id id, int hw_mode)
{
    int i, nsize = sizeof(led_id_mode_data) / sizeof(led_id_mode_data[0]);
    for (i = 0; i < nsize; i++)
    {
        if ((led_id_mode_data[i].led_id == id) &&
            (led_id_mode_data[i].hw_led_light_mode == hw_mode))
        {
            DIAG_PRINT("%s, id:%d, hw_mode:%d mode:%d", __FUNCTION__, id, hw_mode, led_id_mode_data[i].mode);
            return (int)led_id_mode_data[i].mode;
        }
    }

    return -1;
}

static int convert_onlp_led_light_mode_to_hw(enum onlp_led_id id, onlp_led_mode_t mode)
{
    int i, nsize = sizeof(led_id_mode_data) / sizeof(led_id_mode_data[0]);
    for (i = 0; i < nsize; i++)
    {
        if ((led_id_mode_data[i].led_id == id) &&
            (led_id_mode_data[i].mode == mode))
        {
            DIAG_PRINT("%s, id:%d, mode:%d hw_mode:%d", __FUNCTION__, id, mode, led_id_mode_data[i].hw_led_light_mode);
            return led_id_mode_data[i].hw_led_light_mode;
        }
    }

    return -1;
}

pthread_t green_blinking_thread;
int thread_is_running = 0;

static void *set_system_led_green_blinking(void *arg)
{
    int ret = 0, led_status = 1; //0:off, 1:green
    char data = 0;
    _PCA9539_NUM2_SYS_FAN_LED_REG_T sys_fan_led_reg;

    //printf("%s:%d \n", __FUNCTION__, __LINE__);

    do {
      ret = i2c_read_byte(PCA9539_NUM2_I2C_BUS_ID, PCA9539_NUM2_I2C_ADDR, PCA9539_NUM2_IO0_INPUT_OFFSET, &data);
      if (ret < 0)
          {
              AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
              return NULL;
          }
      sys_fan_led_reg.val = data;

      if (led_status == 0)
      {
          sys_fan_led_reg.bit.system = SYSTEM_LED_GREEN_SOLID;
          led_status = 1;
      }
      else
      {
          sys_fan_led_reg.bit.system = SYSTEM_LED_OFF;
          led_status = 0;
      }

      ret = i2c_write_byte(PCA9539_NUM2_I2C_BUS_ID, PCA9539_NUM2_I2C_ADDR, PCA9539_NUM2_IO0_OUTPUT_OFFSET, sys_fan_led_reg.val);
      if (ret < 0)
      {
          AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
          return NULL;
      }

      usleep (300000); //change led status for each 300ms
    } while(1);
    
    //pthread_exit(NULL);
}

static int
stx60d0_led_pca9539_direction_set(int IO_port)
{
    int offset = 0;
    int pca9539_bus = PCA9539_NUM2_I2C_BUS_ID;
    int addr = PCA9539_NUM2_I2C_ADDR;
    char data = 0;
    int ret = 0;

    if (IO_port == 0)
        offset = PCA9539_NUM2_IO0_DIRECTION_OFFSET;
    else
        offset = PCA9539_NUM2_IO1_DIRECTION_OFFSET;

    ret = i2c_read_byte(pca9539_bus, addr, offset, &data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

//debug
//printf("[%s(%d)] pca9539_bus:%d, addr:0x%X, offset:0x%X, data:0x%2X \n", 
//    __FUNCTION__, __LINE__,  pca9539_bus, addr, offset, (unsigned char)data);

    /* configuration direction to output, input:1, output:0.  */
    data &= ~(1 << PCA9539_NUM2_SYSTEM_LED_Y_BIT);
    data &= ~(1 << PCA9539_NUM2_SYSTEM_LED_G_BIT);
    data &= ~(1 << PCA9539_NUM2_FAN_LED_Y_BIT);
    data &= ~(1 << PCA9539_NUM2_FAN_LED_G_BIT);
    
//debug
//printf("[%s(%d)] set direction! data:0x%2X ====\n", __FUNCTION__, __LINE__, (unsigned char)data);

    ret = i2c_write_byte(pca9539_bus, addr, offset, data);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

    return 0;      
}

/*
 * This function will be called prior to any other onlp_ledi_* functions.
 */
int
onlp_ledi_init(void)
{
    DIAG_PRINT("%s", __FUNCTION__);
    int ret = 0;

    /* config pca9539#2 IO port0 direction */
    ret = stx60d0_led_pca9539_direction_set(0);
    if (ret < 0)
    {
        AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
        return ret;
    }

#if 0
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_SERVICE), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_STACKING), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_PWR), ONLP_LED_MODE_OFF);

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_FAN6), ONLP_LED_MODE_OFF);

    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN1), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN2), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN3), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN4), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN5), ONLP_LED_MODE_OFF);
    onlp_ledi_mode_set(ONLP_LED_ID_CREATE(LED_REAR_FAN6), ONLP_LED_MODE_OFF);
#endif
    return ONLP_STATUS_OK;
}

int
onlp_ledi_info_get(onlp_oid_t id, onlp_led_info_t *info)
{
    DIAG_PRINT("%s, id=%d", __FUNCTION__, id);
    _CPLD_POWER_LED_REG_T power_led_reg;
    _CPLD_PSU_LED_REG_T psu_led_reg;
    _PCA9539_NUM2_SYS_FAN_LED_REG_T sys_fan_led_reg;
    char data = 0;
    int ret = 0, local_id = 0;

    VALIDATE(id);

    /* Set the onlp_oid_hdr_t and capabilities */
    *info = linfo[ONLP_OID_ID_GET(id)];

    local_id = ONLP_OID_ID_GET(id);

    switch (local_id)
    {
        case LED_POWER:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_POWER_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            power_led_reg.val = data;
            break;

        case LED_PSU1:
        case LED_PSU2:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_PSU_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            psu_led_reg.val = data;
            break;         

        case LED_SYSTEM:
        case LED_FAN:
            ret = i2c_read_byte(PCA9539_NUM2_I2C_BUS_ID, PCA9539_NUM2_I2C_ADDR, PCA9539_NUM2_IO0_INPUT_OFFSET, &data);
            if (ret < 0)
	            {
	                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
	                return ret;
	            }
            sys_fan_led_reg.val = data;
            break;
            
        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    /* Get LED status */
    switch (local_id)
    {
        case LED_POWER:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, power_led_reg.bit.power);
            break;
        case LED_PSU1:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, psu_led_reg.bit.psu0);
            break;
        case LED_PSU2:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, psu_led_reg.bit.psu1);
            break;
        case LED_SYSTEM:
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, sys_fan_led_reg.bit.system);
            break;
        case LED_FAN:
            //debug
            //printf("[%s]  fan_led_reg.val:0x%x \n", __FUNCTION__,  system_led_reg.val);
            info->mode = convert_hw_led_light_mode_to_onlp(local_id, sys_fan_led_reg.bit.fan_err);
            break;

        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    /* Set the on/off status */
    if (info->mode != ONLP_LED_MODE_OFF)
    {
        info->status |= ONLP_LED_STATUS_ON;
    }

//debug
//printf("[%s] local_id:%d info->mode:%d info->status:%d\n", __FUNCTION__, local_id, info->mode, info->status);

    return ONLP_STATUS_OK;
}

/*
 * Turn an LED on or off.
 *
 * This function will only be called if the LED OID supports the ONOFF
 * capability.
 *
 * What 'on' means in terms of colors or modes for multimode LEDs is
 * up to the platform to decide. This is intended as baseline toggle mechanism.
 */
int
onlp_ledi_set(onlp_oid_t id, int on_or_off)
{
    DIAG_PRINT("%s, id=%d, on_or_off=%d", __FUNCTION__, id, on_or_off);
    VALIDATE(id);

    if (!on_or_off)
    {
        return onlp_ledi_mode_set(id, ONLP_LED_MODE_OFF);
    }

    return onlp_ledi_mode_set(id, ONLP_LED_MODE_ON);
}

/*
 * This function puts the LED into the given mode. It is a more functional
 * interface for multimode LEDs.
 *
 * Only modes reported in the LED's capabilities will be attempted.
 */
int
onlp_ledi_mode_set(onlp_oid_t id, onlp_led_mode_t mode)
{
    DIAG_PRINT("%s, id=%d, mode=%d", __FUNCTION__, id, mode);
    _CPLD_POWER_LED_REG_T power_led_reg;
    _CPLD_PSU_LED_REG_T psu_led_reg;
    _PCA9539_NUM2_SYS_FAN_LED_REG_T sys_fan_led_reg;
    char data = 0;
    int ret = 0, local_id = 0, hw_led_mode = 0;

    VALIDATE(id);

    local_id = ONLP_OID_ID_GET(id);

    switch (local_id)
    {
        case LED_POWER:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_POWER_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            power_led_reg.val = data;
            break;

        case LED_PSU1:
        case LED_PSU2:
            ret = bmc_i2c_read_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_PSU_LED_ADDRESS_OFFSET, &data);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            psu_led_reg.val = data;
            break;

        case LED_SYSTEM:
        case LED_FAN:                  
            ret = i2c_read_byte(PCA9539_NUM2_I2C_BUS_ID, PCA9539_NUM2_I2C_ADDR, PCA9539_NUM2_IO0_INPUT_OFFSET, &data);
            if (ret < 0)
	            {
	                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
	                return ret;
	            }
            sys_fan_led_reg.val = data;
            break;

        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    hw_led_mode = convert_onlp_led_light_mode_to_hw(local_id, mode);
    if (hw_led_mode < 0)
        return ONLP_STATUS_E_PARAM;

    /* Set LED light mode */
    switch (local_id)
    {
        case LED_POWER:
            power_led_reg.bit.power = hw_led_mode;
            break;
        case LED_PSU1:
            psu_led_reg.bit.psu0 = hw_led_mode;
            break;
        case LED_PSU2:
            psu_led_reg.bit.psu1 = hw_led_mode;
            break;
        case LED_SYSTEM:
            sys_fan_led_reg.bit.system = hw_led_mode;
            break;
        case LED_FAN:
            sys_fan_led_reg.bit.fan_err = hw_led_mode;
            break;
        default:
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    switch (local_id)
    {
        case LED_POWER:
            //printf("[debug]service_led_reg.val:0x%x\n", service_led_reg.val);
            ret = bmc_i2c_write_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_POWER_LED_ADDRESS_OFFSET, power_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;

        case LED_PSU1:
        case LED_PSU2:
            ret = bmc_i2c_write_byte(BMC_CPLD_I2C_BUS_ID, BMC_CPLD_I2C_ADDR, CPLD_PSU_LED_ADDRESS_OFFSET, psu_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;

        case LED_SYSTEM:
            if (hw_led_mode == SYSTEM_LED_GREEN_BLINKING && thread_is_running == 0)
            {
                pthread_create(&green_blinking_thread, NULL, &set_system_led_green_blinking, NULL);
                thread_is_running = 1;
            }
            else
            {
                if (thread_is_running)
                {
                    pthread_cancel(green_blinking_thread);
                    thread_is_running = 0;
                }

                ret = i2c_write_byte(PCA9539_NUM2_I2C_BUS_ID, PCA9539_NUM2_I2C_ADDR, PCA9539_NUM2_IO0_OUTPUT_OFFSET, sys_fan_led_reg.val);
                if (ret < 0)
                {
                    AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                    return ret;
                }
            }
            break;

        case LED_FAN:            
            //printf("[debug]service_led_reg.val:0x%x\n", service_led_reg.val);
            ret = i2c_write_byte(PCA9539_NUM2_I2C_BUS_ID, PCA9539_NUM2_I2C_ADDR, PCA9539_NUM2_IO0_OUTPUT_OFFSET, sys_fan_led_reg.val);
            if (ret < 0)
            {
                AIM_LOG_INFO("%s:%d fail[%d]\n", __FUNCTION__, __LINE__, ret);
                return ret;
            }
            break;

        default :
            AIM_LOG_INFO("%s:%d %d is a wrong ID\n", __FUNCTION__, __LINE__, local_id);
            return ONLP_STATUS_E_PARAM;
    }

    return ONLP_STATUS_OK;
}

/*
 * Generic LED ioctl interface.
 */
int
onlp_ledi_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

