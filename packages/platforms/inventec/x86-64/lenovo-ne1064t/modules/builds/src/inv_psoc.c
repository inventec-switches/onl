/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 */

#include <linux/module.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/jiffies.h>
#include <linux/i2c.h>
#include <linux/hwmon.h>
#include <linux/hwmon-sysfs.h>
#include <linux/err.h>
#include <linux/mutex.h>
#include <linux/delay.h>
#include <linux/swab.h>

#include <linux/kthread.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#define SWITCH_TEMPERATURE_SOCK     "/proc/switch/temp"

#define USE_SMBUS    1

//#define offsetof(st, m) ((size_t)(&((st *)0)->m))
#define FAN_NUM  5
#define PSU_NUM  2 

#define FAN_CLEI_SUPPORT 1
#define PSU_CLEI_SUPPORT 1
#define CHECK_FAN_ENABLE 0

struct __attribute__ ((__packed__))  psoc_psu_layout {
    u16 psu1_iin;
    u16 psu2_iin;
    u16 psu1_iout;
    u16 psu2_iout;
    
    u16 psu1_pin;
    u16 psu2_pin;
    u16 psu1_pout;
    u16 psu2_pout;
    
    u16 psu1_vin;
    u16 psu2_vin;
    u16 psu1_vout;
    u16 psu2_vout;

    u8  psu1_vendor[16];
    u8  psu2_vendor[16];
    u8  psu1_model[20];
    u8  psu2_model[20];
    u8  psu1_version[8];
    u8  psu2_version[8];
    u8  psu1_date[6];
    u8  psu2_date[6];
    u8  psu1_sn[20];
    u8  psu2_sn[20];
};

struct clei{
    u8 issue_number[3];
    u8 abbreviation_number[9];
    u8 fc_number[10];
    u8 clei_code[10];
    u8 product_year_and_month[5];
    u8 label_location_code[2];
    u8 serial_number[5];
    u8 pcb_revision[5];
    u8 vendor_name[10];
    u8 reserved[5];
};

struct __attribute__ ((__packed__))  psoc_layout {
    u8 ctl;                 //offset: 0
    u16 switch_temp;        //offset: 1
    u8 reserve1;            //offset: 3

    u8 fw_upgrade;          //offset: 4

    //i2c bridge
    u8 i2c_st;              //offset: 5
    u8 i2c_ctl;             //offset: 6
    u8 i2c_addr;            //offset: 7
    u8 i2c_data[27];        //offset: 8

    // BYTE[23:27] - ExtFan 
    u16 ext_rpm[2];         //offset: 23
    u8  gpi_fan2;           //offset: 27
	
    //gpo
    u8 led_grn;             //offset: 28
    u8 led_red;             //offset: 29

    //pwm duty
    u8 pwm[5];              //offset: 2a
    u8 pwm_psu[2];          //offset: 2f

    //fan rpm
    u16 fan[10];            //offset: 31

    //gpi 
    u8 gpi_fan;             //offset: 45

    //psu state
    u8 psu_state;           //offset: 46

    //temperature
    u16 temp[5];            //offset: 47
    u16 temp_psu[2];        //offset: 51

    //version
    u8 version[2];          //offset: 55
    
    u8  reserve2[3];        //offset: 57
    struct psoc_psu_layout psu_info;      //offset: 5a
	
	u8 reserved[0xff-0xfd];
    struct clei clei[7];   //FAN:0~4   PSU:4~5;
};        


/* definition */
#define PSOC_OFF(m)    offsetof(struct psoc_layout, m)
#define PSOC_PSU_OFF(m)    offsetof(struct psoc_psu_layout, m)

#define SWITCH_TMP_OFFSET       PSOC_OFF(switch_temp) //0x01
#define PWM_OFFSET              PSOC_OFF(pwm)
#define THERMAL_OFFSET          PSOC_OFF(temp)
#define RPM_OFFSET              PSOC_OFF(fan)
#define DIAG_FLAG_OFFSET        PSOC_OFF(ctl)
#define FAN_LED_OFFSET          PSOC_OFF(led_grn)
#define FAN_GPI_OFFSET          PSOC_OFF(gpi_fan)
#define PSOC_PSU_OFFSET         PSOC_OFF(psu_state)
#define VERSION_OFFSET          PSOC_OFF(version)
#define PSU_INFO_OFFSET         PSOC_OFF(psu_info)

#define RPM2_OFFSET             PSOC_OFF(ext_rpm)
#define FAN_GPI2_OFFSET         PSOC_OFF(gpi_fan2)

#define CLEI_OFF(m)    offsetof(struct clei, m)
#define FAN1_CLEI_INDEX         0
#define FAN2_CLEI_INDEX         1
#define FAN3_CLEI_INDEX         2
#define FAN4_CLEI_INDEX         3
#define FAN5_CLEI_INDEX			4
#define PSU1_CLEI_INDEX         5
#define PSU2_CLEI_INDEX         6

/* Each client has this additional data */
struct psoc_data {
	struct device		*hwmon_dev;
	struct mutex		update_lock;
	u32                 diag;
	struct task_struct  *auto_update;
};

/*-----------------------------------------------------------------------*/

static ssize_t psoc_i2c_read(struct i2c_client *client, u8 *buf, u8 offset, size_t count)
{
#if USE_SMBUS    
	int i;	
    for(i=0; i<count; i++) {
        i2c_smbus_write_byte_data(client, ((offset+i) >> 8), ((offset+i) & 0xff));
        buf[i] = i2c_smbus_read_byte(client);
    }	

    return count;
#else
	struct i2c_msg msg[2];
	char msgbuf[2];
	int status;

	memset(msg, 0, sizeof(msg));
	
	msgbuf[0] = offset >> 8 ;
	msgbuf[1] = offset & 0xff;
	
	msg[0].addr = client->addr;
	msg[0].buf = msgbuf;
	msg[0].len = 2;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = count;
	
	status = i2c_transfer(client->adapter, msg, 2);
	
	if(status == 2)
	    status = count;
	    
	return status;    
#endif
}

static ssize_t psoc_i2c_write(struct i2c_client *client, char *buf, unsigned offset, size_t count)
{
#if USE_SMBUS    
	int i;
	
    for(i=0; i<count; i++) {
	 i2c_smbus_write_word_data(client, ((offset+i) >> 8), ((offset+i) & 0xff) | (buf[i] << 8));
    }	
    return count;
#else
	struct i2c_msg msg;
	int status;
    u8 writebuf[256];
	
	int i = 0;

	msg.addr = client->addr;
	msg.flags = 0;

	/* msg.buf is u8 and casts will mask the values */
	msg.buf = writebuf;
	
	msg.buf[i++] = offset >> 8;
	msg.buf[i++] = offset & 0xff;
	memcpy(&msg.buf[i], buf, count);
	msg.len = i + count;
	
	status = i2c_transfer(client->adapter, &msg, 1);
	if (status == 1)
		status = count;
	
    return status;	
#endif    
}

#if 0
static u32 psoc_read32(struct i2c_client *client, u8 offset)
{
	u32 value = 0;
	u8 buf[4];
    
    if( psoc_i2c_read(client, buf, offset, 4) == 4)
        value = (buf[0]<<24 | buf[1]<<16 | buf[2]<<8 | buf[3]);
    
	return value;
}
#endif

static u16 psoc_read16(struct i2c_client *client, u8 offset)
{
	u16 value = 0;
	u8 buf[2];
    
    if(psoc_i2c_read(client, buf, offset, 2) == 2)
        value = (buf[0]<<8 | buf[1]<<0);
    
	return value;
}

static u8 psoc_read8(struct i2c_client *client, u8 offset)
{
	u8 value = 0;
	u8 buf = 0;
    
    if(psoc_i2c_read(client, &buf, offset, 1) == 1)
        value = buf;
    
	return value;
}

#if 0
/*
CPLD report the PSU0 status
000 = PSU normal operation
100 = PSU fault
010 = PSU unpowered
111 = PSU not installed

7 6 | 5 4 3 |  2 1 0
----------------------
    | psu1  |  psu0
*/
static char* psu_str[] = {
    "normal",           //000
    "NA",               //001
    "unpowered",        //010
    "NA",               //011
    "fault",            //100
    "NA",               //101
    "NA",               //110
    "not installed",    //111
};

static ssize_t show_psu_st(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u32 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 byte;
	int shift = (attr->index == 0)?3:0;
    
	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, &byte, PSOC_PSU_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
    byte = (byte >> shift) & 0x7;
	
	status = sprintf (buf, "%d : %s\n", byte, psu_str[byte]);
	    
	return strlen(buf);
}
#endif

static ssize_t show_clei(struct device *dev, struct device_attribute *da,
                         char *buf)
{
    u16 status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct psoc_data *data = i2c_get_clientdata(client);
	u8 device_index = attr->index & 0xFF;
    u8 clei_index   = (attr->index >> 8) & 0xFF;
	u16 offset = PSOC_OFF(clei) + sizeof(struct clei)* device_index;
	u8 len = sizeof(struct clei);
    u8 rxbuf[sizeof(struct clei)] = {0};

    mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client,rxbuf,offset,len);
    mutex_unlock(&data->update_lock);

	status = sprintf (buf, "Issue Number: %.3s\n", &rxbuf[CLEI_OFF(issue_number)]);
	status = sprintf (buf, "%sAbbreviation Number: %.9s\n", buf,  &rxbuf[CLEI_OFF(abbreviation_number)]);
	status = sprintf (buf, "%sFC Number: %.10s\n", buf,  &rxbuf[CLEI_OFF(fc_number)]);
	status = sprintf (buf, "%sCLEI Code: %.10s\n", buf,  &rxbuf[CLEI_OFF(clei_code)]);
	status = sprintf (buf, "%sProduct Year and Month: %.5s\n", buf,  &rxbuf[CLEI_OFF(product_year_and_month)]);
    status = sprintf (buf, "%s2D Label Location Code: %.2s\n", buf,  &rxbuf[CLEI_OFF(label_location_code)]);
    status = sprintf (buf, "%sSerial Number: %.5s\n", buf,  &rxbuf[CLEI_OFF(serial_number)]);
    status = sprintf (buf, "%sPCB Revision: %.5s\n", buf,  &rxbuf[CLEI_OFF(pcb_revision)]);
    status = sprintf (buf, "%sVendor Name: %.10s\n", buf,  &rxbuf[CLEI_OFF(vendor_name)]);
	return strlen(buf);
}
/*-----------------------------------------------------------------------*/

/* sysfs attributes for hwmon */

static ssize_t show_thermal(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index * 2 + THERMAL_OFFSET;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read16(client, offset);
	status = __swab16(status);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       (s8)(status>>8) * 1000  );
}


static ssize_t show_pwm(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index + PWM_OFFSET;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read8(client, offset);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       status);
}

static ssize_t set_pwm(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index + PWM_OFFSET;

	u8 pwm = simple_strtol(buf, NULL, 10);
	if(pwm > 255) pwm = 255;
	
	if(data->diag) {    
    	mutex_lock(&data->update_lock);
    	psoc_i2c_write(client, &pwm, offset, 1);
    	mutex_unlock(&data->update_lock);
    }
	
	return count;
}


static ssize_t show_rpm(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read16(client, offset);
	status = __swab16(status);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       status);
}

static ssize_t show_fan_type(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u8 status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 index = attr->index;
	int type = -1;
    
	mutex_lock(&data->update_lock);
	if (attr->index < 4){
		status = psoc_read8(client, FAN_GPI_OFFSET);		
	}
	else {
		status = psoc_read8(client, FAN_GPI2_OFFSET);
		index-=4;
	}
	mutex_unlock(&data->update_lock);

	if( (status & 1<<index) == 0) {
	    if( (status & 1<<(index+4) ) == 0) {
	        type = 1;//NORMAL FAN
	    }
	    else {
	        type = 2;//REVERSAL FAN
	    }
	}
	else {
	    type = 0;//UNPLUGGED
	}
	
	return sprintf(buf, "%d\n", type);
}

static ssize_t show_switch_tmp(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u16 temp = 0;
    
	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, (u8*)&temp, SWITCH_TMP_OFFSET, 2);
	status = __swab16(status);
	mutex_unlock(&data->update_lock);
	
	status = sprintf (buf, "%d\n",  (s8)(temp>>8) * 1000  );
	    
	return strlen(buf);
}

static ssize_t set_switch_tmp(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);

	long temp = simple_strtol(buf, NULL, 10);
    u16 temp2 =  ( (temp/1000) <<8 ) & 0xFF00 ;
    
    //printk("set_switch_tmp temp=%d, temp2=0x%x (%x,%x)\n", temp, temp2, ( ( (temp/1000) <<8 ) & 0xFF00 ),  (( (temp%1000) / 10 ) & 0xFF));
    
	mutex_lock(&data->update_lock);
	psoc_i2c_write(client, (u8*)&temp2, SWITCH_TMP_OFFSET, 2);
	mutex_unlock(&data->update_lock);
	
	return count;
}

static ssize_t show_diag(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 diag_flag = 0;
    
	mutex_lock(&data->update_lock);
    status = psoc_i2c_read(client, (u8*)&diag_flag, DIAG_FLAG_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
	data->diag = (diag_flag & 0x80)?1:0;
	status = sprintf (buf, "%d\n", data->diag);
	    
	return strlen(buf);
}

static ssize_t set_diag(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 value = 0;
	u8 diag = simple_strtol(buf, NULL, 10);
	
	diag = diag?1:0;
	data->diag = diag;
	    
	mutex_lock(&data->update_lock);
	psoc_i2c_read(client, (u8*)&value, DIAG_FLAG_OFFSET, 1);
	if(diag) value |= (1<<7);
	else     value &= ~(1<<7);
	psoc_i2c_write(client, (u8*)&value, DIAG_FLAG_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
	return count;
}

static ssize_t show_version(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u16 status;
	//struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read16(client, VERSION_OFFSET);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "ver: %x.%x\n", (status & 0xFF00)>>8,  (status & 0xFF) );
}


static ssize_t show_fan_led(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 bit = attr->index;
    
	mutex_lock(&data->update_lock);
	status = psoc_read16(client, FAN_LED_OFFSET);
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",
		       (status & (1<<bit))?1:0 );
}

static ssize_t set_fan_led(struct device *dev,
			   struct device_attribute *devattr,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(devattr);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 bit = attr->index;
	u16 led_state = 0;

	u8 v = simple_strtol(buf, NULL, 10);
	
	if(data->diag) {    
    	mutex_lock(&data->update_lock);
    	led_state = psoc_read16(client, FAN_LED_OFFSET);
    	if(v) led_state |=  (1<<bit);
    	else  led_state &= ~(1<<bit);    
    	psoc_i2c_write(client, (char *)&led_state, FAN_LED_OFFSET, 2);
    	mutex_unlock(&data->update_lock);
    }
	
	return count;
}

static ssize_t show_value8(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct psoc_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index;
    
	mutex_lock(&data->update_lock);
	
	status = psoc_read8(client, offset);
	
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "0x%02X\n", status );
}

static long pmbus_reg2data_linear(int data, int linear16)
{
    s16 exponent;
    s32 mantissa;
    long val;

    if (linear16) { /* LINEAR16 */
        exponent = -9;
        mantissa = (u16) data;
    } else {  /* LINEAR11 */
        exponent = ((s16)data) >> 11;
        exponent = ((s16)( data & 0xF800) ) >> 11;
        mantissa = ((s32)((data & 0x7ff) << 5)) >> 5;
    }
    
    //printk("data=%d,  m=%d, e=%d\n", data, exponent, mantissa);
    val = mantissa;

    /* scale result to micro-units for power sensors */
    val = val * 1000L;

    if (exponent >= 0)
        val <<= exponent;
    else
        val >>= -exponent;

    return val;
}

static ssize_t show_psu_psoc(struct device *dev, struct device_attribute *da,
			 char *buf)
{
    u16 status;
    struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
    struct i2c_client *client = to_i2c_client(dev);
    struct psoc_data *data = i2c_get_clientdata(client);
    u8 offset = attr->index + PSU_INFO_OFFSET;
    u8 len = (attr->index >> 8)& 0xFF;
    u8 rxbuf[21] = {0};
    u8 rxbuf1[21] = {0};
    u8 counter = 0;
    u8 index = 0;
	if (len == 2) {
		mutex_lock(&data->update_lock);
		status = psoc_read16(client, offset);
		mutex_unlock(&data->update_lock);
		if(strstr(attr->dev_attr.attr.name, "vout") || strstr(attr->dev_attr.attr.name, "in3") || strstr(attr->dev_attr.attr.name, "in4"))
			index=1;
		return sprintf(buf, "%ld \n", pmbus_reg2data_linear(status, index));	
	}
	
    do {
        mutex_lock(&data->update_lock);
        status = psoc_i2c_read(client,rxbuf,offset,len);
        status = psoc_i2c_read(client,rxbuf1,offset,len);
        mutex_unlock(&data->update_lock);
        counter++;
    } while((strcmp(rxbuf,rxbuf1)!=0) || ((strlen(rxbuf)==0) && (counter < 5)));
	
	if( rxbuf[0] == 0xff || status == 0 ) return sprintf(buf, "ERROR\n");
	if (strlen(rxbuf)==0 ) return sprintf(buf,"N/A\n");
	return sprintf(buf,"%.20s\n",rxbuf);
}

#define PSOC_POLLING_PERIOD 1000
#define PSU_NOT_INSTALLED   7
#define PSOC_WARNING(fmt, args...) printk( KERN_WARNING "[PSOC]" fmt, ##args)
static u8 psu_info[PSU_NUM];
static u8 fan_info[FAN_NUM];

#if CHECK_FAN_ENABLE
static void check_fan_status(struct i2c_client *client)
{
    struct psoc_data *data = i2c_get_clientdata(client);
    u32 status;
    int i = 0;
    u8 rst = 0;
	
    mutex_lock(&data->update_lock);
    status = psoc_read8(client, FAN_GPI_OFFSET) & 0x0f;
    status |= (psoc_read8(client, FAN_GPI2_OFFSET) & 0x0f) << 4;	
    mutex_unlock(&data->update_lock);
	
    for (i = 0; i < FAN_NUM; i++)
    {
		rst = (status >> i) & 0x01;
        if(rst != fan_info[i])
        {
            fan_info[i] = rst;
            if (rst)
              PSOC_WARNING("Detect FAN%d is removed \n",(i+1));
            else
              PSOC_WARNING("Detect FAN%d is present \n",(i+1));
        }
    }
}
#endif

static void check_switch_temp(struct i2c_client * client)
{
	struct psoc_data *data = i2c_get_clientdata(client);
	static struct file *f;
	mm_segment_t old_fs;

	set_fs(get_ds());
	f = filp_open(SWITCH_TEMPERATURE_SOCK,O_RDONLY,0644);
	if(IS_ERR(f)) { 
		return; 
	}
	else { 
		char temp_str[]={0,0,0,0,0,0,0};
		loff_t pos = 0;
		u16 temp2 = 0;
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		vfs_read(f, temp_str,6,&pos);
		temp2 = ((simple_strtoul(temp_str,NULL,10)/1000) <<8 ) & 0xFF00 ; 
                mutex_lock(&data->update_lock);
                psoc_i2c_write(client, (u8*)&temp2, SWITCH_TMP_OFFSET, 2);
                mutex_unlock(&data->update_lock);
	}
	filp_close(f,NULL);
	set_fs(old_fs);
}

static int psoc_polling_thread(void *p)
{
    struct i2c_client *client = p;
    memset(psu_info,7,PSU_NUM);
	memset(fan_info,1,FAN_NUM); 
	
    while (!kthread_should_stop())
    {
#if CHECK_FAN_ENABLE
        check_fan_status(client);
#endif
        check_switch_temp(client);
        set_current_state(TASK_INTERRUPTIBLE);
        if(kthread_should_stop())
            break;
 
        schedule_timeout(msecs_to_jiffies(PSOC_POLLING_PERIOD));
    }
    return 0;
}



static SENSOR_DEVICE_ATTR(temp1_input, S_IRUGO,			show_thermal, 0, 0);
static SENSOR_DEVICE_ATTR(temp2_input, S_IRUGO,			show_thermal, 0, 1);
static SENSOR_DEVICE_ATTR(temp3_input, S_IRUGO,			show_thermal, 0, 2);
static SENSOR_DEVICE_ATTR(temp4_input, S_IRUGO,			show_thermal, 0, 3);
static SENSOR_DEVICE_ATTR(temp5_input, S_IRUGO,			show_thermal, 0, 4);
static SENSOR_DEVICE_ATTR(thermal_psu1, S_IRUGO,		show_thermal, 0, 5);
static SENSOR_DEVICE_ATTR(thermal_psu2, S_IRUGO,		show_thermal, 0, 6);
static SENSOR_DEVICE_ATTR(temp7_input, S_IRUGO,			show_thermal, 0, 5);
static SENSOR_DEVICE_ATTR(temp8_input, S_IRUGO,			show_thermal, 0, 6);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 0);
static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 1);
static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 2);
static SENSOR_DEVICE_ATTR(pwm4, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 3);
#if FAN_NUM > 4
static SENSOR_DEVICE_ATTR(pwm5, S_IWUSR|S_IRUGO,			show_pwm, set_pwm, 4);
#endif
static SENSOR_DEVICE_ATTR(pwm_psu1, S_IWUSR|S_IRUGO,		show_pwm, set_pwm, 5);
static SENSOR_DEVICE_ATTR(pwm_psu2, S_IWUSR|S_IRUGO,		show_pwm, set_pwm, 6);
static SENSOR_DEVICE_ATTR(pwm6, S_IWUSR|S_IRUGO,            	show_pwm, set_pwm, 5);
static SENSOR_DEVICE_ATTR(pwm7, S_IWUSR|S_IRUGO,            	show_pwm, set_pwm, 6);

//static SENSOR_DEVICE_ATTR(psu0,  S_IRUGO,			        show_psu_st, 0, 0);
//static SENSOR_DEVICE_ATTR(psu1,  S_IRUGO,			        show_psu_st, 0, 1);

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO,			show_rpm, 0, 0   + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO,			show_rpm, 0, 2   + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO,			show_rpm, 0, 4   + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan4_input, S_IRUGO,			show_rpm, 0, 6   + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan5_input, S_IRUGO,			show_rpm, 0, 8   + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan6_input, S_IRUGO,			show_rpm, 0, 10  + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan7_input, S_IRUGO,			show_rpm, 0, 12  + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan8_input, S_IRUGO,			show_rpm, 0, 14  + RPM_OFFSET);
#if FAN_NUM > 4
static SENSOR_DEVICE_ATTR(fan9_input, S_IRUGO,			show_rpm, 0, 0   + RPM2_OFFSET);
static SENSOR_DEVICE_ATTR(fan10_input, S_IRUGO,			show_rpm, 0, 2   + RPM2_OFFSET);
#endif
static SENSOR_DEVICE_ATTR(rpm_psu1, S_IRUGO,			show_rpm, 0, 16  + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(rpm_psu2, S_IRUGO,			show_rpm, 0, 18  + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan11_input, S_IRUGO,			show_rpm, 0, 16  + RPM_OFFSET);
static SENSOR_DEVICE_ATTR(fan12_input, S_IRUGO,			show_rpm, 0, 18  + RPM_OFFSET);

static SENSOR_DEVICE_ATTR(switch_tmp, S_IWUSR|S_IRUGO,			show_switch_tmp, set_switch_tmp, 0);
static SENSOR_DEVICE_ATTR(temp6_input, S_IWUSR|S_IRUGO,			show_switch_tmp, 0, 0);

static SENSOR_DEVICE_ATTR(diag, S_IWUSR|S_IRUGO,			show_diag, set_diag, 0);
static SENSOR_DEVICE_ATTR(version, S_IRUGO,			show_version, 0, 0);

static SENSOR_DEVICE_ATTR(fan_led_grn1, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 0 );
static SENSOR_DEVICE_ATTR(fan_led_grn2, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 1 );
static SENSOR_DEVICE_ATTR(fan_led_grn3, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 2 );
static SENSOR_DEVICE_ATTR(fan_led_grn4, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 3 );
static SENSOR_DEVICE_ATTR(fan_led_red1, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 8 );
static SENSOR_DEVICE_ATTR(fan_led_red2, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 9 );
static SENSOR_DEVICE_ATTR(fan_led_red3, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 10);
static SENSOR_DEVICE_ATTR(fan_led_red4, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 11);
#if FAN_NUM > 4
static SENSOR_DEVICE_ATTR(fan_led_grn5, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 4 );
static SENSOR_DEVICE_ATTR(fan_led_red5, S_IWUSR|S_IRUGO,			show_fan_led, set_fan_led, 12);
#endif

static SENSOR_DEVICE_ATTR(fan_gpi,      S_IRUGO,			        show_value8,  0,           FAN_GPI_OFFSET);
static SENSOR_DEVICE_ATTR(fan1_type, S_IRUGO,			show_fan_type, 0, 0);
static SENSOR_DEVICE_ATTR(fan2_type, S_IRUGO,			show_fan_type, 0, 1);
static SENSOR_DEVICE_ATTR(fan3_type, S_IRUGO,			show_fan_type, 0, 2);
static SENSOR_DEVICE_ATTR(fan4_type, S_IRUGO,			show_fan_type, 0, 3);
#if FAN_NUM > 4
static SENSOR_DEVICE_ATTR(fan5_type, S_IRUGO,			show_fan_type, 0, 4);
static SENSOR_DEVICE_ATTR(fan_gpi2,      S_IRUGO,			        show_value8,  0,           FAN_GPI2_OFFSET);
#endif

static SENSOR_DEVICE_ATTR(psoc_psu1_vin,      S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu1_vin));
static SENSOR_DEVICE_ATTR(psoc_psu1_vout,     S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu1_vout));
static SENSOR_DEVICE_ATTR(psoc_psu1_iin,      S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu1_iin));
static SENSOR_DEVICE_ATTR(psoc_psu1_iout,     S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu1_iout));
static SENSOR_DEVICE_ATTR(psoc_psu1_pin,      S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu1_pin));
static SENSOR_DEVICE_ATTR(psoc_psu1_pout,     S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu1_pout));

static SENSOR_DEVICE_ATTR(psoc_psu2_vin,      S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu2_vin)); 
static SENSOR_DEVICE_ATTR(psoc_psu2_vout,     S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu2_vout));
static SENSOR_DEVICE_ATTR(psoc_psu2_iin,      S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu2_iin)); 
static SENSOR_DEVICE_ATTR(psoc_psu2_iout,     S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu2_iout));
static SENSOR_DEVICE_ATTR(psoc_psu2_pin,      S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu2_pin)); 
static SENSOR_DEVICE_ATTR(psoc_psu2_pout,     S_IRUGO,			        show_psu_psoc,  0,           (0x02<<8)|PSOC_PSU_OFF(psu2_pout));

static SENSOR_DEVICE_ATTR(in1_input,          S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_vin));
static SENSOR_DEVICE_ATTR(curr1_input,        S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_iin));
static SENSOR_DEVICE_ATTR(power1_input,       S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_pin));
static SENSOR_DEVICE_ATTR(in3_input,          S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_vout));
static SENSOR_DEVICE_ATTR(curr3_input,        S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_iout));
static SENSOR_DEVICE_ATTR(power3_input,       S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu1_pout));

static SENSOR_DEVICE_ATTR(in2_input,          S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_vin));
static SENSOR_DEVICE_ATTR(curr2_input,        S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_iin));
static SENSOR_DEVICE_ATTR(power2_input,       S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_pin));
static SENSOR_DEVICE_ATTR(in4_input,          S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_vout));
static SENSOR_DEVICE_ATTR(curr4_input,        S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_iout));
static SENSOR_DEVICE_ATTR(power4_input,       S_IRUGO,                  show_psu_psoc,  0,      (0x02<<8)|PSOC_PSU_OFF(psu2_pout));

static SENSOR_DEVICE_ATTR(psoc_psu1_vendor,   S_IRUGO,                  show_psu_psoc,  0,   (0x10<<8)|PSOC_PSU_OFF(psu1_vendor));
static SENSOR_DEVICE_ATTR(psoc_psu1_model,    S_IRUGO,                  show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu1_model));
static SENSOR_DEVICE_ATTR(psoc_psu1_version,  S_IRUGO,                  show_psu_psoc,  0,   (0x08<<8)|PSOC_PSU_OFF(psu1_version));
static SENSOR_DEVICE_ATTR(psoc_psu1_date,     S_IRUGO,                  show_psu_psoc,  0,   (0x06<<8)|PSOC_PSU_OFF(psu1_date));
static SENSOR_DEVICE_ATTR(psoc_psu1_sn,       S_IRUGO,                  show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu1_sn));
static SENSOR_DEVICE_ATTR(psoc_psu2_vendor,   S_IRUGO,                  show_psu_psoc,  0,   (0x10<<8)|PSOC_PSU_OFF(psu2_vendor));
static SENSOR_DEVICE_ATTR(psoc_psu2_model,    S_IRUGO,                  show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu2_model));
static SENSOR_DEVICE_ATTR(psoc_psu2_version,  S_IRUGO,                  show_psu_psoc,  0,   (0x08<<8)|PSOC_PSU_OFF(psu2_version));
static SENSOR_DEVICE_ATTR(psoc_psu2_date,     S_IRUGO,                  show_psu_psoc,  0,   (0x06<<8)|PSOC_PSU_OFF(psu2_date));
static SENSOR_DEVICE_ATTR(psoc_psu2_sn,       S_IRUGO,                  show_psu_psoc,  0,   (0x14<<8)|PSOC_PSU_OFF(psu2_sn));

#if FAN_CLEI_SUPPORT
static SENSOR_DEVICE_ATTR(fan1_clei, S_IRUGO, show_clei, 0, FAN1_CLEI_INDEX );
static SENSOR_DEVICE_ATTR(fan2_clei, S_IRUGO, show_clei, 0, FAN2_CLEI_INDEX );
static SENSOR_DEVICE_ATTR(fan3_clei, S_IRUGO, show_clei, 0, FAN3_CLEI_INDEX );
static SENSOR_DEVICE_ATTR(fan4_clei, S_IRUGO, show_clei, 0, FAN4_CLEI_INDEX );
#if FAN_NUM > 4
static SENSOR_DEVICE_ATTR(fan5_clei, S_IRUGO, show_clei, 0, FAN5_CLEI_INDEX );
#endif
#endif

#if PSU_CLEI_SUPPORT
static SENSOR_DEVICE_ATTR(psu1_clei, S_IRUGO, show_clei, 0, PSU1_CLEI_INDEX );
static SENSOR_DEVICE_ATTR(psu2_clei, S_IRUGO, show_clei, 0, PSU2_CLEI_INDEX );
#endif		
static struct attribute *psoc_attributes[] = {
    //thermal
	&sensor_dev_attr_temp1_input.dev_attr.attr,
	&sensor_dev_attr_temp2_input.dev_attr.attr,
	&sensor_dev_attr_temp3_input.dev_attr.attr,
	&sensor_dev_attr_temp4_input.dev_attr.attr,
	&sensor_dev_attr_temp5_input.dev_attr.attr,
	
	&sensor_dev_attr_thermal_psu1.dev_attr.attr,
	&sensor_dev_attr_thermal_psu2.dev_attr.attr,
	&sensor_dev_attr_temp7_input.dev_attr.attr,
	&sensor_dev_attr_temp8_input.dev_attr.attr,

    //pwm
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm2.dev_attr.attr,
	&sensor_dev_attr_pwm3.dev_attr.attr,
	&sensor_dev_attr_pwm4.dev_attr.attr,
#if FAN_NUM > 4
	&sensor_dev_attr_pwm5.dev_attr.attr,
#endif
	&sensor_dev_attr_pwm_psu1.dev_attr.attr,
	&sensor_dev_attr_pwm_psu2.dev_attr.attr,
	&sensor_dev_attr_pwm6.dev_attr.attr,
	&sensor_dev_attr_pwm7.dev_attr.attr,
	
	//rpm
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,
	&sensor_dev_attr_fan4_input.dev_attr.attr,
	&sensor_dev_attr_fan5_input.dev_attr.attr,
	&sensor_dev_attr_fan6_input.dev_attr.attr,
	&sensor_dev_attr_fan7_input.dev_attr.attr,
	&sensor_dev_attr_fan8_input.dev_attr.attr,
#if FAN_NUM > 4
	&sensor_dev_attr_fan9_input.dev_attr.attr,
	&sensor_dev_attr_fan10_input.dev_attr.attr,
#endif
	&sensor_dev_attr_rpm_psu1.dev_attr.attr,
	&sensor_dev_attr_rpm_psu2.dev_attr.attr,
	&sensor_dev_attr_fan11_input.dev_attr.attr,
	&sensor_dev_attr_fan12_input.dev_attr.attr,   
	
    //switch temperature
	&sensor_dev_attr_switch_tmp.dev_attr.attr,
	&sensor_dev_attr_temp6_input.dev_attr.attr,
	
    //diag flag
	&sensor_dev_attr_diag.dev_attr.attr,
	
	//version
	&sensor_dev_attr_version.dev_attr.attr,
	
	//fan led 
	&sensor_dev_attr_fan_led_grn1.dev_attr.attr,
	&sensor_dev_attr_fan_led_grn2.dev_attr.attr,
	&sensor_dev_attr_fan_led_grn3.dev_attr.attr,
	&sensor_dev_attr_fan_led_grn4.dev_attr.attr,
	&sensor_dev_attr_fan_led_red1.dev_attr.attr,
	&sensor_dev_attr_fan_led_red2.dev_attr.attr,
	&sensor_dev_attr_fan_led_red3.dev_attr.attr,
	&sensor_dev_attr_fan_led_red4.dev_attr.attr,
#if FAN_NUM > 4
	&sensor_dev_attr_fan_led_grn5.dev_attr.attr,
	&sensor_dev_attr_fan_led_red5.dev_attr.attr,
#endif	
	//fan GPI 
	&sensor_dev_attr_fan_gpi.dev_attr.attr,
#if FAN_NUM > 4
	&sensor_dev_attr_fan_gpi2.dev_attr.attr,
#endif

	//fan type
	&sensor_dev_attr_fan1_type.dev_attr.attr,
	&sensor_dev_attr_fan2_type.dev_attr.attr,
	&sensor_dev_attr_fan3_type.dev_attr.attr,
	&sensor_dev_attr_fan4_type.dev_attr.attr,
#if FAN_NUM > 4
	&sensor_dev_attr_fan5_type.dev_attr.attr,
#endif	

	//psu
	&sensor_dev_attr_psoc_psu1_vin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_vout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_iin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_iout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_pin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu1_pout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_vin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_vout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_iin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_iout.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_pin.dev_attr.attr,
	&sensor_dev_attr_psoc_psu2_pout.dev_attr.attr,
//	&sensor_dev_attr_psu0.dev_attr.attr,
//	&sensor_dev_attr_psu1.dev_attr.attr,

    &sensor_dev_attr_in1_input.dev_attr.attr,
    &sensor_dev_attr_curr1_input.dev_attr.attr,
    &sensor_dev_attr_power1_input.dev_attr.attr,
    &sensor_dev_attr_in2_input.dev_attr.attr,
    &sensor_dev_attr_curr2_input.dev_attr.attr,
    &sensor_dev_attr_power2_input.dev_attr.attr,
    &sensor_dev_attr_in3_input.dev_attr.attr,
    &sensor_dev_attr_curr3_input.dev_attr.attr,
    &sensor_dev_attr_power3_input.dev_attr.attr,
    &sensor_dev_attr_in4_input.dev_attr.attr,
    &sensor_dev_attr_curr4_input.dev_attr.attr,
    &sensor_dev_attr_power4_input.dev_attr.attr,

    //psu info
    &sensor_dev_attr_psoc_psu1_vendor.dev_attr.attr,
    &sensor_dev_attr_psoc_psu1_model.dev_attr.attr,
    &sensor_dev_attr_psoc_psu1_version.dev_attr.attr,
    &sensor_dev_attr_psoc_psu1_date.dev_attr.attr,
    &sensor_dev_attr_psoc_psu1_sn.dev_attr.attr,
    &sensor_dev_attr_psoc_psu2_vendor.dev_attr.attr,
    &sensor_dev_attr_psoc_psu2_model.dev_attr.attr,
    &sensor_dev_attr_psoc_psu2_version.dev_attr.attr,
    &sensor_dev_attr_psoc_psu2_date.dev_attr.attr,
    &sensor_dev_attr_psoc_psu2_sn.dev_attr.attr,
	
	//clei
#if FAN_CLEI_SUPPORT
    &sensor_dev_attr_fan1_clei.dev_attr.attr,
    &sensor_dev_attr_fan2_clei.dev_attr.attr,
    &sensor_dev_attr_fan3_clei.dev_attr.attr,
    &sensor_dev_attr_fan4_clei.dev_attr.attr,
#if FAN_NUM > 4
    &sensor_dev_attr_fan5_clei.dev_attr.attr,
#endif
#endif

#if PSU_CLEI_SUPPORT
    &sensor_dev_attr_psu1_clei.dev_attr.attr,
    &sensor_dev_attr_psu2_clei.dev_attr.attr,
#endif	
	NULL
};

static const struct attribute_group psoc_group = {
	.attrs = psoc_attributes,
};

/*-----------------------------------------------------------------------*/

/* device probe and removal */

static int
psoc_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct psoc_data *data;
	int status;

//    printk("+%s\n", __func__);
    
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = kzalloc(sizeof(struct psoc_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);
	data->diag    = 0;
	
	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &psoc_group);
	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}
	
    data->auto_update = kthread_run(psoc_polling_thread,client,"%s",dev_name(data->hwmon_dev));
    if (IS_ERR(data->auto_update))
    {
    	status = PTR_ERR(data->auto_update);
    	goto exit_remove;
    }
	dev_info(&client->dev, "%s: sensor '%s'\n",
		 dev_name(data->hwmon_dev), client->name);

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &psoc_group);
exit_free:
	i2c_set_clientdata(client, NULL);
	kfree(data);
	return status;
}

static int psoc_remove(struct i2c_client *client)
{
	struct psoc_data *data = i2c_get_clientdata(client);

	hwmon_device_unregister(data->hwmon_dev);
	sysfs_remove_group(&client->dev.kobj, &psoc_group);
	i2c_set_clientdata(client, NULL);
	kfree(data);
	return 0;
}

static const struct i2c_device_id psoc_ids[] = {
	{ "inv_psoc", 0, },
	{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, psoc_ids);

static struct i2c_driver psoc_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "inv_psoc",
	},
	.probe		= psoc_probe,
	.remove		= psoc_remove,
	.id_table	= psoc_ids,
};

/*-----------------------------------------------------------------------*/

/* module glue */

static int __init inv_psoc_init(void)
{
	printk("el6661 inv_psoc_init\n");
	return i2c_add_driver(&psoc_driver);
}

static void __exit inv_psoc_exit(void)
{
	i2c_del_driver(&psoc_driver);
}

MODULE_AUTHOR("ting.jack <ting.jack@inventec>");
MODULE_DESCRIPTION("inv psoc5 driver");
MODULE_LICENSE("GPL");

module_init(inv_psoc_init);
module_exit(inv_psoc_exit);
