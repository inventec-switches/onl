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
#include <linux/notifier.h>
#include <linux/reboot.h>
#include <linux/kthread.h>
#include <linux/kernel.h>
#include <linux/fs.h>
#include <asm/uaccess.h>
#include <linux/delay.h>

#define USE_SMBUS    		1
#define CPLD_POLLING_PERIOD 	1000
#define ENABLE_SIMULATE		0
#define ENABLE_AUTOFAN		1

#if ENABLE_SIMULATE
	u8 sim_register[0x90];
#endif

/* definition */
#define CPLD_INFO_OFFSET			0x00
#define CPLD_WATCHDOGCOUNTER_OFFSET	0x05
#define CPLD_WATCHDOGCTL_OFFSET		0x06
#define CPLD_WATCHDOGSETCOUNTER_OFFSET	0x07
#define CPLD_PSUSTATUS_OFFSET		0x08
#define CPLD_CTL_OFFSET				0x0C
#define CPLD_INT_OFFSET				0x11
#define CPLD_OC_OFFSET				0x12
#define CPLD_OPTCTL_OFFSET			0x16
#define CPLD_OPTSTATUS_OFFSET		0x18
#define CPLD_SWRW_OFFSET			0x1A
#define CPLD_PWM_OFFSET				0x1B
#define CPLD_RPM_OFFSET				0x1E
#define CPLD_POELED_OFFSET			0x24
#define CPLD_FANLED_OFFSET			0x25
#define CPLD_SYSLED_OFFSET			0x26
#define CPLD_PSUENABLE_OFFSET		0x27
#define FAN_NUM				3
#define PWM_MIN				30
#define PWM_DEFAULT			150
#define PORT_ENALBE		"/etc/port_enable"

/* Each client has this additional data */
struct cpld_data {
	struct device *hwmon_dev;
	struct mutex update_lock;
	struct task_struct *cpld_thread;
	u8 diag;
};

/*-----------------------------------------------------------------------*/
static ssize_t cpld_i2c_read(struct i2c_client *client, u8 *buf, u8 offset, size_t count)
{
#if ENABLE_SIMULATE
	memcpy(buf,&sim_register[offset],count);
	return count;
#else
#if USE_SMBUS    
	int i;
	
	for(i=0; i<count; i++)
	{
		buf[i] = i2c_smbus_read_byte_data(client, offset+i);
	}
	return count;
#else
	struct i2c_msg msg[2];
	char msgbuf[2];
	int status;

	memset(msg, 0, sizeof(msg));
	
	msgbuf[0] = offset;
	
	msg[0].addr = client->addr;
	msg[0].buf = msgbuf;
	msg[0].len = 1;

	msg[1].addr = client->addr;
	msg[1].flags = I2C_M_RD;
	msg[1].buf = buf;
	msg[1].len = count;
	
	status = i2c_transfer(client->adapter, msg, 2);
	
	if(status == 2)
	    status = count;
	    
	return status;    
#endif	
#endif
}

static ssize_t cpld_i2c_write(struct i2c_client *client, char *buf, unsigned offset, size_t count)
{
#if ENABLE_SIMULATE
	memcpy(&sim_register[offset],buf,count);
	return count;
#else
#if USE_SMBUS    
	int i;
	
	for(i=0; i<count; i++)
	{
		i2c_smbus_write_byte_data(client, offset+i, buf[i]);
	}
	return count;
#else
	struct i2c_msg msg;
	int status;
	u8 writebuf[64];
	
	int i = 0;

	msg.addr = client->addr;
	msg.flags = 0;

	/* msg.buf is u8 and casts will mask the values */
	msg.buf = writebuf;
	
	msg.buf[i++] = offset;
	memcpy(&msg.buf[i], buf, count);
	msg.len = i + count;
	
	status = i2c_transfer(client->adapter, &msg, 1);
	if (status == 1)
		status = count;
	
	return status;	
#endif
#endif
}

static int check_poe_led(void)
{
	char enable_str[6]= {'0','0','0','0', '0', '0'};
	int i;
	static struct file *f;
	mm_segment_t old_fs;

	set_fs(get_ds());
	f = filp_open(PORT_ENALBE,O_RDONLY,0644);
	
	if(IS_ERR(f)) {
		return;
	}
	else {
		loff_t pos = 0;
		old_fs = get_fs();
		set_fs(KERNEL_DS);
		vfs_read(f, enable_str, sizeof(enable_str),&pos);
		#if 0
		for(i=0; i<6; i++)
		{
			printk("enable_str[%d]=%c\n", i, enable_str[i]);
		}
		#endif
	}
	filp_close(f,NULL);
	set_fs(old_fs);

	for(i=0; i<6; i++)
	{
		if(enable_str[i]!='0')
			return 1;
	}
	
	return 0;
}

/*-----------------------------------------------------------------------*/
/* sysfs attributes for hwmon */

static ssize_t show_info(struct device *dev, struct device_attribute *da,
			char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte[4] = {0,0,0,0};

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, byte, CPLD_INFO_OFFSET, 4);
	mutex_unlock(&data->update_lock);

	sprintf (buf, "The CPLD release date is %02d/%02d/%d.\n", 
	byte[2] & 0xf, (byte[3] & 0x1f), 2014+(byte[2] >> 4));	/* mm/dd/yyyy*/
	sprintf (buf, "%sThe PCB  version is %X\n", buf,  byte[0]);
	sprintf (buf, "%sThe CPLD version is %d.%d\n", buf, byte[1]>>4, byte[1]&0xf);
	
	return strlen(buf);
}

static ssize_t show_diag(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	return sprintf (buf, "%d\n", data->diag);
}

static ssize_t set_diag(struct device *dev,
			   struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 diag = simple_strtol(buf, NULL, 10);
	data->diag = diag?1:0;
	return count;
}

static char* interrupt_str[] = {
    "BCM56342_CMIC_INTR_N", 	//0
    "B50292_D00_INTRP_N",      	//1
    "B50292_D01_INTRP_N",  		//2
    "B50292_D02_INTRP_N"		//3
};

static ssize_t show_interrupt(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_INT_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	sprintf (buf, "0x%02X:", byte);
	if(byte==0x0f) sprintf (buf, "%sNone",buf);
	if(!(byte&0x01)) sprintf (buf, "%s%s ",buf,interrupt_str[0]);
	if(!(byte&0x02)) sprintf (buf, "%s%s ",buf,interrupt_str[1]);
	if(!(byte&0x04)) sprintf (buf, "%s%s ",buf,interrupt_str[2]);
	if(!(byte&0x08)) sprintf (buf, "%s%s ",buf,interrupt_str[3]);

	return sprintf (buf, "%s\n", buf);
}

static char* overcurrentstatus_str[] = {
    "SFP+_P01_OC_N",	//0
    "SFP+_P02_OC_N",	//1
    "SFP+_P03_OC_N",	//2
    "SFP+_P04_OC_N"		//3
};

static ssize_t show_overcurrentstatus(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_OC_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	sprintf (buf, "0x%02X:", byte);
	if(byte==0x0f) sprintf (buf, "%sNormal",buf);
	if(!(byte&0x01)) sprintf (buf, "%s%s ",buf,interrupt_str[0]);
	if(!(byte&0x02)) sprintf (buf, "%s%s ",buf,interrupt_str[1]);
	if(!(byte&0x04)) sprintf (buf, "%s%s ",buf,interrupt_str[2]);
	if(!(byte&0x08)) sprintf (buf, "%s%s ",buf,interrupt_str[3]);

	return sprintf (buf, "%s\n", buf);
}

static char* optctl_str[] = {
    "TX_DISABLE",	//0
    "RS",			//1
};

static ssize_t show_optctl(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	
	int opt_index = attr->index;
	u8 offset = (opt_index==0 || opt_index==1)?0:1;
	offset = offset  + CPLD_OPTCTL_OFFSET;
	int shift = (opt_index==0 || opt_index==2)?0:4;
	
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);
	
	sprintf (buf, "0x%02X:", byte);
	sprintf (buf, "%s%s:%d ",buf,optctl_str[0],((byte>>shift)&0x01)?1:0);
	sprintf (buf, "%s%s:%d ",buf,optctl_str[1],((byte>>shift)&0x04)?1:0);

	return sprintf(buf, "%s\n", buf);
}

static ssize_t set_opttxdisable(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	u8 temp = simple_strtol(buf, NULL, 16);
	
	int opt_index = attr->index;
	u8 offset = (opt_index==0 || opt_index==1)?0:1;
	offset = offset  + CPLD_OPTCTL_OFFSET;
	int shift = (opt_index==0 || opt_index==2)?0:4;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	byte &= ~(0x1<<shift);
	byte |= (temp<<shift);
	cpld_i2c_write(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t set_optrs(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	u8 temp = simple_strtol(buf, NULL, 16);
	
	int opt_index = attr->index;
	u8 offset = (opt_index==0 || opt_index==1)?0:1;
	offset = offset  + CPLD_OPTCTL_OFFSET;
	int shift = (opt_index==0 || opt_index==2)?2:6;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	byte &= ~(0x1<<shift);
	byte |= (temp<<shift);
	cpld_i2c_write(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static char* optstatus_str[] = {
    "Module Absent",		//0
    "RX optical lost of lock detected",		//1
    "TX fault detected",		//2
};

static ssize_t show_optstatus(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	
	int opt_index = attr->index;
	u8 offset = (opt_index==0 || opt_index==1)?0:1;
	offset = offset  + CPLD_OPTSTATUS_OFFSET;
	int shift = (opt_index==0 || opt_index==2)?0:4;
	
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);
	
	sprintf (buf, "0x%02X:\n", byte);
	if(((byte>>shift)&0x01)==0x01) sprintf (buf, "%sModule is present\n",buf);
	if(!((byte>>shift)&0x01)) sprintf (buf, "%s%s\n",buf,optstatus_str[0]);
	if((byte>>shift)&0x02) sprintf (buf, "%s%s\n",buf,optstatus_str[1]);
	if((byte>>shift)&0x04) sprintf (buf, "%s%s\n",buf,optstatus_str[2]);
	
	return sprintf(buf, "%s\n", buf);
}

static ssize_t show_psuenable(struct device *dev, struct device_attribute *da,
			 char *buf)
{	
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	int shift = (attr->index == 0)?0:4;
	
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_PSUENABLE_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	
	return sprintf(buf, "%d\n",( (byte>>shift) &0x01));
}

static ssize_t set_psuenable(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 temp = simple_strtol(buf, NULL, 16);
	u8 byte = 0;
	int shift = (attr->index == 0)?0:4;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_PSUENABLE_OFFSET, 1);
	byte &= ~(0x1<<shift);
	byte |= (temp<<shift);
	cpld_i2c_write(client, &byte, CPLD_PSUENABLE_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static char* led_str[] = {
    "OFF",     	//000
    "1 Hz",     //001
    "4 Hz",    	//010
    "ON",    	//011
};

static char* led_color[] = {
    "GRN",     	//0
    "RED",      //1
    "BLU",		//3
};

static ssize_t show_led(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	u8 offset = attr->index + CPLD_POELED_OFFSET;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);
	
	if(attr->index==0)
		sprintf(buf, "%d:%s %s ", (byte>>4) & 0x3, led_color[2], led_str[(byte>>4) & 0x3]);

	if(attr->index==1 || attr->index==2)
	{
		sprintf(buf, "%d:%s %s ", (byte>>4) & 0x3, led_color[0], led_str[(byte>>4) & 0x3]);
		sprintf(buf, "%s%d:%s %s", buf, byte & 0x3, led_color[1], led_str[byte & 0x3]);
	}
	return sprintf (buf, "%s\n", buf);
}

static ssize_t set_led(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);

	u8 temp = simple_strtol(buf, NULL, 16);
	u8 byte = 0;
	u8 mask;
	u8 offset = attr->index + CPLD_POELED_OFFSET;

	temp <<= 4;
	mask = 0x30;
	temp &= mask;
	
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	byte &= ~(mask);
	byte |= (temp);
	cpld_i2c_write(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t show_red_led(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = 0;
	u8 offset = attr->index + CPLD_FANLED_OFFSET;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);
	
	sprintf(buf, "%s%d:%s %s", buf, byte & 0x3, led_color[1], led_str[byte & 0x3]);

	return sprintf (buf, "%s\n", buf);
}

static ssize_t set_red_led(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);

	u8 temp = simple_strtol(buf, NULL, 16);
	u8 byte = 0;
	u8 mask;
	u8 offset = attr->index + CPLD_FANLED_OFFSET;
	
	mask = 0x3;
	temp &= mask;
	
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	byte &= ~(mask);
	byte |= (temp);
	cpld_i2c_write(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static char* psu_str[] = {
	"normal",          //000
	"unpowered",       //010
	"fault",           //100
	"not installed",   //111
};

static ssize_t show_psustatus(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;
	int shift = (attr->index == 0)?0:3;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_PSUSTATUS_OFFSET, 1);
	mutex_unlock(&data->update_lock);
	byte = (byte >> shift) & 0x7;

	sprintf (buf, "0x%02X: ", byte);
	if(byte==0x00) sprintf (buf, "%s%s ", buf, psu_str[0]);
	else if(byte==0x02) sprintf (buf, "%s%s ", buf,psu_str[1]);
	else if(byte==0x04) sprintf (buf, "%s%s ", buf,psu_str[2]);
	else if(byte==0x07) sprintf (buf, "%s%s ", buf,psu_str[3]);
	
	return sprintf (buf, "%s\n", buf);
}

static ssize_t show_pwm(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;
	u8 offset = attr->index  + CPLD_PWM_OFFSET;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%d\n", byte);
}

static ssize_t set_pwm(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index  + CPLD_PWM_OFFSET;
	u8 byte = simple_strtol(buf, NULL, 10);
	mutex_lock(&data->update_lock);
	cpld_i2c_write(client, &byte, offset, 1);
	mutex_unlock(&data->update_lock);
	return count;
}

static ssize_t show_rpm(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index*2  + CPLD_RPM_OFFSET;
	u8 byte[2] = {0,0};
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, byte, offset, 2);
	mutex_unlock(&data->update_lock);
	return sprintf(buf, "%d\n", (byte[0]<<8 | byte[1]));
}

static char* fantype_str[] = {
    "normal",   //00
    "fault", 	//01
};

static ssize_t show_fantype(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	int status;
	struct sensor_device_attribute *attr = to_sensor_dev_attr(da);
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 offset = attr->index*2  + CPLD_RPM_OFFSET;
	u8 byte[2] = {0,0};
	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, byte, offset, 2);
	mutex_unlock(&data->update_lock);

	if(byte[0]==0) status = 1;
	else status = 0;

	return sprintf(buf, "%d:%s\n",status,fantype_str[status]);
}

static ssize_t set_watchdog_feed(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	byte |= 0x04;
	cpld_i2c_write(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t set_watchdog_countbase(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;
	int shift = 1;
	u8 temp = simple_strtol(buf, NULL, 16);

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	byte |= (temp<<shift);
	cpld_i2c_write(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t set_watchdog_enable(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	byte |= 0x01;
	cpld_i2c_write(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static char* watchdog_str[] = {
    "WDT enable",  	//0
	"WDT base sec",	//1
	"WDT base min",	//2
    "WDT clear",  	//3
};

static ssize_t show_watchdog_ctl(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_WATCHDOGCTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	sprintf (buf, "0x%02X:\n", byte);
	sprintf (buf, "%s%s:%d\n", buf, watchdog_str[0], (byte&0x01)?1:0);
	sprintf (buf, "%s%s\n", buf, (byte&0x02)?watchdog_str[1]:watchdog_str[2]);
	sprintf (buf, "%s%s:%d\n", buf, watchdog_str[3], (byte&0x04)?1:0);

	return sprintf(buf, "%s\n", buf);
}

static ssize_t set_watchdog_counter(struct device *dev,
			   struct device_attribute *da,
			   const char *buf, size_t count)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte = simple_strtol(buf, NULL, 10);

	mutex_lock(&data->update_lock);
	cpld_i2c_write(client, &byte, CPLD_WATCHDOGSETCOUNTER_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return count;
}

static ssize_t show_watchdog_set_counter(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;
	u8 countbase=0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_WATCHDOGSETCOUNTER_OFFSET, 1);
	cpld_i2c_read(client, &countbase, CPLD_WATCHDOGCTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%s:%d \n",(countbase&0x2)?watchdog_str[1]:watchdog_str[2],byte);
}

static ssize_t show_watchdog_counter(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	struct i2c_client *client = to_i2c_client(dev);
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 byte=0;
	u8 countbase=0;

	mutex_lock(&data->update_lock);
	cpld_i2c_read(client, &byte, CPLD_WATCHDOGCOUNTER_OFFSET, 1);
	cpld_i2c_read(client, &countbase, CPLD_WATCHDOGCTL_OFFSET, 1);
	mutex_unlock(&data->update_lock);

	return sprintf(buf, "%s:%d \n",(countbase&0x2)?watchdog_str[1]:watchdog_str[2],byte);
}

#if ENABLE_SIMULATE
static ssize_t show_simdump(struct device *dev, struct device_attribute *da,
			 char *buf)
{
	u8 i, j;
	sprintf(buf,"usage: echo 0xAABB > sim_buffer (AA is address, BB is value)\n\n"); 
	sprintf(buf,"%s       00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F\n",buf);
	sprintf(buf,"%s======================================================\n",buf);
	sprintf(buf,"%s0000:  ",buf);
	for (j = 0, i = 0; j < sizeof(sim_register); j++, i++)
	{
		sprintf(buf,"%s%02x ", buf, (int)sim_register[i]);
		if ((i & 0x0F) == 0x0F) sprintf(buf,"%s\n%04x:  ", buf, (i+1));
	}
	return sprintf(buf,"%s\n",buf);
}

static ssize_t set_simbuffer(struct device *dev, struct device_attribute *da,
			   const char *buf, size_t count)
{
	u16 byte = simple_strtol(buf, NULL, 16);
	u8 address = (byte >> 8);
	u8 value = (byte & 0xff);

	sim_register[address]=value;
	
	return count;
}
#endif
static SENSOR_DEVICE_ATTR(info,         S_IRUGO,		show_info, 0, 0);
static SENSOR_DEVICE_ATTR(diag, 	S_IWUSR|S_IRUGO,	show_diag, set_diag, 0);
static SENSOR_DEVICE_ATTR(interrupt, 	S_IRUGO,   		show_interrupt, 0, 0);

static SENSOR_DEVICE_ATTR(poe_led,    	S_IWUSR|S_IRUGO,   show_led, set_led, 0);
static SENSOR_DEVICE_ATTR(fan_led,      S_IWUSR|S_IRUGO,   show_led, set_led, 1);
static SENSOR_DEVICE_ATTR(sys_led, 		S_IWUSR|S_IRUGO,   show_led, set_led, 2);

static SENSOR_DEVICE_ATTR(fan_red_led,      S_IWUSR|S_IRUGO,   show_red_led, set_red_led, 0);
static SENSOR_DEVICE_ATTR(sys_red_led,    	S_IWUSR|S_IRUGO,   show_red_led, set_red_led, 1);

static SENSOR_DEVICE_ATTR(fanmodule1_type, S_IRUGO,	show_fantype, 0, 0);
static SENSOR_DEVICE_ATTR(fanmodule2_type, S_IRUGO,	show_fantype, 0, 1);
static SENSOR_DEVICE_ATTR(fanmodule3_type, S_IRUGO,	show_fantype, 0, 2);

static SENSOR_DEVICE_ATTR(fan1_input, S_IRUGO,	show_rpm, 0, 0);
static SENSOR_DEVICE_ATTR(fan2_input, S_IRUGO,	show_rpm, 0, 1);
static SENSOR_DEVICE_ATTR(fan3_input, S_IRUGO,	show_rpm, 0, 2);

static SENSOR_DEVICE_ATTR(psu1_status,	S_IRUGO,	show_psustatus, 0, 0);
static SENSOR_DEVICE_ATTR(psu2_status,  S_IRUGO, 	show_psustatus, 0, 1);

static SENSOR_DEVICE_ATTR(psu1_enable, S_IWUSR|S_IRUGO,	show_psuenable, set_psuenable, 0);
static SENSOR_DEVICE_ATTR(psu2_enable, S_IWUSR|S_IRUGO,	show_psuenable, set_psuenable, 1);

static SENSOR_DEVICE_ATTR(overcurrentstatus, S_IRUGO,	show_overcurrentstatus, 0, 1);

static SENSOR_DEVICE_ATTR(opt0_ctl,	S_IRUGO,	show_optctl, 0, 0);
static SENSOR_DEVICE_ATTR(opt1_ctl, S_IRUGO,	show_optctl, 0, 1);
static SENSOR_DEVICE_ATTR(opt2_ctl, S_IRUGO,	show_optctl, 0, 2);
static SENSOR_DEVICE_ATTR(opt3_ctl, S_IRUGO,	show_optctl, 0, 3);

static SENSOR_DEVICE_ATTR(opt0_txdisable,	S_IWUSR,	0,	set_opttxdisable, 0);
static SENSOR_DEVICE_ATTR(opt1_txdisable,	S_IWUSR,	0,	set_opttxdisable, 1);
static SENSOR_DEVICE_ATTR(opt2_txdisable,	S_IWUSR,	0,	set_opttxdisable, 2);
static SENSOR_DEVICE_ATTR(opt3_txdisable,	S_IWUSR,	0,	set_opttxdisable, 3);

static SENSOR_DEVICE_ATTR(opt0_rs,	S_IWUSR,	0,	set_optrs,	0);
static SENSOR_DEVICE_ATTR(opt1_rs,	S_IWUSR,	0,	set_optrs,	1);
static SENSOR_DEVICE_ATTR(opt2_rs,	S_IWUSR,	0,	set_optrs,	2);
static SENSOR_DEVICE_ATTR(opt3_rs,	S_IWUSR,	0,	set_optrs,	3);

static SENSOR_DEVICE_ATTR(opt0_status, S_IRUGO,	show_optstatus, 0, 0);
static SENSOR_DEVICE_ATTR(opt1_status, S_IRUGO,	show_optstatus, 0, 1);
static SENSOR_DEVICE_ATTR(opt2_status, S_IRUGO,	show_optstatus, 0, 2);
static SENSOR_DEVICE_ATTR(opt3_status, S_IRUGO,	show_optstatus, 0, 3);

static SENSOR_DEVICE_ATTR(pwm1, S_IWUSR|S_IRUGO,	show_pwm, set_pwm, 0);
static SENSOR_DEVICE_ATTR(pwm2, S_IWUSR|S_IRUGO,	show_pwm, set_pwm, 1);
static SENSOR_DEVICE_ATTR(pwm3, S_IWUSR|S_IRUGO,	show_pwm, set_pwm, 2);

static SENSOR_DEVICE_ATTR(watchdog_feed,		S_IWUSR,			0, set_watchdog_feed, 0);
static SENSOR_DEVICE_ATTR(watchdog_countbase,	S_IWUSR,			0, set_watchdog_countbase, 0);
static SENSOR_DEVICE_ATTR(watchdog_enable,		S_IWUSR,			0, set_watchdog_enable, 0);
static SENSOR_DEVICE_ATTR(watchdog_ctl, 		S_IRUGO,			show_watchdog_ctl, 0, 0);
static SENSOR_DEVICE_ATTR(watchdog_set_counter,		S_IWUSR|S_IRUGO,	show_watchdog_set_counter, set_watchdog_counter, 0);
static SENSOR_DEVICE_ATTR(watchdog_counter,		S_IRUGO,           	show_watchdog_counter,  0, 0);

#if ENABLE_SIMULATE
	static SENSOR_DEVICE_ATTR(sim_buffer,	S_IWUSR|S_IRUGO,	show_simdump, set_simbuffer, 0);
#endif
			
static struct attribute *cpld_attributes[] = {
	&sensor_dev_attr_info.dev_attr.attr,
	&sensor_dev_attr_diag.dev_attr.attr,
	
	&sensor_dev_attr_sys_led.dev_attr.attr,
	&sensor_dev_attr_fan_led.dev_attr.attr,
	&sensor_dev_attr_poe_led.dev_attr.attr,

	&sensor_dev_attr_fan_red_led.dev_attr.attr,
	&sensor_dev_attr_sys_red_led.dev_attr.attr,

	&sensor_dev_attr_interrupt.dev_attr.attr,
	
	&sensor_dev_attr_psu1_status.dev_attr.attr,
	&sensor_dev_attr_psu2_status.dev_attr.attr,
	
	&sensor_dev_attr_psu1_enable.dev_attr.attr,
	&sensor_dev_attr_psu2_enable.dev_attr.attr,
	
	&sensor_dev_attr_overcurrentstatus.dev_attr.attr,
	
	&sensor_dev_attr_opt0_ctl.dev_attr.attr,
	&sensor_dev_attr_opt1_ctl.dev_attr.attr,
	&sensor_dev_attr_opt2_ctl.dev_attr.attr,
	&sensor_dev_attr_opt3_ctl.dev_attr.attr,
	
	&sensor_dev_attr_opt0_txdisable.dev_attr.attr,
	&sensor_dev_attr_opt1_txdisable.dev_attr.attr,
	&sensor_dev_attr_opt2_txdisable.dev_attr.attr,
	&sensor_dev_attr_opt3_txdisable.dev_attr.attr,
	
	&sensor_dev_attr_opt0_rs.dev_attr.attr,
	&sensor_dev_attr_opt1_rs.dev_attr.attr,
	&sensor_dev_attr_opt2_rs.dev_attr.attr,
	&sensor_dev_attr_opt3_rs.dev_attr.attr,
	
	&sensor_dev_attr_opt0_status.dev_attr.attr,
	&sensor_dev_attr_opt1_status.dev_attr.attr,
	&sensor_dev_attr_opt2_status.dev_attr.attr,
	&sensor_dev_attr_opt3_status.dev_attr.attr,
	
	&sensor_dev_attr_pwm1.dev_attr.attr,
	&sensor_dev_attr_pwm2.dev_attr.attr,
	&sensor_dev_attr_pwm3.dev_attr.attr,

	&sensor_dev_attr_fanmodule1_type.dev_attr.attr,
	&sensor_dev_attr_fanmodule2_type.dev_attr.attr,
	&sensor_dev_attr_fanmodule3_type.dev_attr.attr,
	
	&sensor_dev_attr_fan1_input.dev_attr.attr,
	&sensor_dev_attr_fan2_input.dev_attr.attr,
	&sensor_dev_attr_fan3_input.dev_attr.attr,

	&sensor_dev_attr_watchdog_feed.dev_attr.attr,
	&sensor_dev_attr_watchdog_countbase.dev_attr.attr,
	&sensor_dev_attr_watchdog_enable.dev_attr.attr,
	&sensor_dev_attr_watchdog_ctl.dev_attr.attr,
	&sensor_dev_attr_watchdog_set_counter.dev_attr.attr,
	&sensor_dev_attr_watchdog_counter.dev_attr.attr,

#if ENABLE_SIMULATE
	&sensor_dev_attr_sim_buffer.dev_attr.attr,
#endif
	NULL
};

struct i2c_client *client_notifier;

static const struct attribute_group cpld_group = {
	.attrs = cpld_attributes,
};

#if ENABLE_AUTOFAN
#define SWITCH_ADDRESS 0-0049
#define ENV_ADDRESS    0-004f
#define OUTLET_ADDRESS 0-004a
#define PSU1_ADDRESS   1-0058
#define PSU2_ADDRESS   1-0059

#define _STR(s) #s
#define __STR(s) _STR(s)
#define __File_input(__file) __STR(/sys/bus/i2c/devices/__file/hwmon/hwmon%d/temp1_input)
#define __File_max(__file) __STR(/sys/bus/i2c/devices/__file/hwmon/hwmon%d/temp1_max)
#define __File_max_hyst(__file) __STR(/sys/bus/i2c/devices/__file/hwmon/hwmon%d/temp1_max_hyst)
#define __File_pwm(__file) __STR(/sys/bus/i2c/devices/__file/hwmon/hwmon%d/pwm1)

#define SWITCH_TEMPERATURE __File_input(SWITCH_ADDRESS)
#define ENV_TEMPERATURE    __File_input(ENV_ADDRESS)
#define OUTLET_TEMPERATURE    __File_input(OUTLET_ADDRESS)
#define SWITCH_MAX         __File_max(SWITCH_ADDRESS)
#define ENV_MAX            __File_max(ENV_ADDRESS)
#define OUTLET_MAX            __File_max(OUTLET_ADDRESS)
#define SWITCH_MAX_HYST    __File_max_hyst(SWITCH_ADDRESS)
#define ENV_MAX_HYST       __File_max_hyst(ENV_ADDRESS)
#define OUTLET_MAX_HYST       __File_max_hyst(OUTLET_ADDRESS)
#define PSU1_PWM           __File_pwm(PSU1_ADDRESS)
#define PSU2_PWM           __File_pwm(PSU2_ADDRESS)

#define n_entries	14 
#define temp_hysteresis 2
#define MAX_HWMON 9

u8 outlet_thermaltrip  = 64;
u8 asic_thermaltrip = 64;
u8 env_thermaltrip  = 67;

u8 FanTable[3][n_entries][2]={
//Front-to-Rear
{//SWITCH
	{ 62, 255 },  \
	{ 60, 240 },  \
	{ 59, 220 },  \
	{ 58, 201 },  \
	{ 57, 160 },  \
	{ 56, 120 },  \
	{ 55, 101 },  \
	{ 54,  85 },  \
	{ 52,  75 },  \
	{ 49,  70 },  \
	{ 46,  65 },  \
	{ 43,  60 },  \
	{ 40,  55 },  \
	{ 37,  50 }
}
,{//ENV
	{ 60, 250 },  \
	{ 57, 240 },  \
	{ 56, 220 },  \
	{ 55, 201 },  \
	{ 54, 160 },  \
	{ 51, 120 },  \
	{ 49, 101 },  \
	{ 47,  85 },  \
	{ 45,  75 },  \
	{ 40,  70 },  \
	{ 37,  65 },  \
	{ 34,  60 },  \
	{ 25,  55 },  \
	{ 22,  50 }
}
,{//OUTLET
	{ 60, 255 },  \
	{ 56, 240 },  \
	{ 55, 220 },  \
	{ 54, 201 },  \
	{ 52, 160 },  \
	{ 50, 120 },  \
	{ 47, 101 },  \
	{ 44,  85 },  \
	{ 41,  75 },  \
	{ 38,  70 },  \
	{ 34,  65 },  \
	{ 25,  60 },  \
	{ 18,  55 },  \
	{ 15,  50 }
}
};

//type 0:SWITCH,1:ENV,2:OUTLET
static u8 find_duty(u8 type, u8 temp)
{
	static u8 fantable_index[3]={n_entries-1, n_entries-1, n_entries-1};
	while(1)
	{
		if(fantable_index[type] > 1 && temp > FanTable[type][fantable_index[type]-1][0])
			fantable_index[type]--;
		else if(fantable_index[type] < (n_entries-1) && temp < (FanTable[type][fantable_index[type]][0]-temp_hysteresis))
			fantable_index[type]++;
		else 
			break;		
	}
	return FanTable[type][fantable_index[type]][1];
}

static u32 getvalue(char *path)
{
	static struct file *f;
	mm_segment_t old_fs;
	u16 temp = 0;
	char temp_str[]={0,0,0,0,0,0,0,0,0};
	loff_t pos = 0;

	f = filp_open(path,O_RDONLY,0644);
	if(IS_ERR(f)) return -1;

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	vfs_read(f, temp_str,8,&pos);
	temp = simple_strtoul(temp_str,NULL,10);
	filp_close(f,NULL);
	set_fs(old_fs);

	if(temp<0) temp=0;
	return temp;
}

static u8 setvalue(char *path, u32 value)
{
	static struct file *f;
	mm_segment_t old_fs;
	char temp_str[]={0,0,0,0,0,0,0,0,0};
	u8 len=0;
	loff_t pos = 0;

	f = filp_open(path,O_WRONLY,0644);
	if(IS_ERR(f)) return -1;

	len = sprintf(temp_str,"%d",value);

	old_fs = get_fs();
	set_fs(KERNEL_DS);
	vfs_write(f, temp_str, len, &pos);
	filp_close(f,NULL);
	set_fs(old_fs);

	return 0;
}

static u8 probe_gettemp(char *path)
{
	u8 temp_path[64],i;
	u32 value;
	
	for(i=0;i<MAX_HWMON;i++)
	{
		sprintf(temp_path,path,i);
		value=getvalue(temp_path);
		if(value!=-1) return (u8)(value/1000);
	}
	return 0;
}

static u8 probe_setvalue(char *path, u32 value)
{
	u8 temp_path[64],i,status;
	
	for(i=0;i<MAX_HWMON;i++)
	{
		sprintf(temp_path,path,i);
		status=setvalue(temp_path, value);
		if(status==0) break;
	}
	return status;
}

static void autofanspeed(struct i2c_client *client, u8 fanfail, u8 fanturnoff)
{
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 fanpwm[3]={PWM_DEFAULT,PWM_DEFAULT,PWM_DEFAULT};
	u8 i;
	u8 duty = PWM_MIN;
	u8 psu_duty = 0;
	
	if(fanfail==2)
	{
		u8 temp_switch = probe_gettemp(SWITCH_TEMPERATURE);
		u8 temp_env    = probe_gettemp(ENV_TEMPERATURE);
		u8 temp_outlet    = probe_gettemp(OUTLET_TEMPERATURE);

		u8 duty_outlet    = 0;
		u8 duty_switch = 0;
		u8 duty_env    = 0;

		if(temp_outlet!=0xff) duty_outlet=find_duty(0,temp_outlet);
		if(temp_switch!=0xff) duty_switch=find_duty(1,temp_switch);
		if(temp_env!=0xff) duty_env=find_duty(2,temp_env);
		
		if(temp_outlet==0xff && temp_switch==0xff && temp_env==0xff)
		{
			duty=PWM_DEFAULT;
		}
		else
		{
			duty=(duty>duty_outlet)?duty:duty_outlet;	
			duty=(duty>duty_switch)?duty:duty_switch;	
			duty=(duty>duty_env)?duty:duty_env;	
		}
		
		memset(fanpwm,duty,FAN_NUM);
		for(i=0;i<FAN_NUM;i++)
			if(fanturnoff & (1<<i)) fanpwm[i]=PWM_MIN;		
#if ENABLE_SIMULATE
		sim_register[0x80]=temp_outlet;
		sim_register[0x81]=temp_switch;
		sim_register[0x82]=temp_env;
		sim_register[0x83]=duty_outlet;
		sim_register[0x84]=duty_switch;
		sim_register[0x85]=duty_env;
		sim_register[0x86]=duty;
		sim_register[0x87]=psu_duty;	
#endif	
	}
	else
	{
		duty=0xff;
		memset(fanpwm,duty,FAN_NUM);	
	}
	if (duty > 200) 
		psu_duty = 80;
	else if (duty <= 200 && duty > 150)
		psu_duty = 55;
	else if (duty <= 150 && duty > 100)
		psu_duty = 45;
	else
		psu_duty = 30;
	probe_setvalue(PSU1_PWM, psu_duty);
	probe_setvalue(PSU2_PWM, psu_duty);

	mutex_lock(&data->update_lock);
	cpld_i2c_write(client,fanpwm,CPLD_PWM_OFFSET,FAN_NUM);
	mutex_unlock(&data->update_lock);
}
#endif

/*-----------------------------------------------------------------------*/
static int cpld_polling(void *p)
{
	struct i2c_client *client = p;
	struct cpld_data *data = i2c_get_clientdata(client);
	u8 fanrpm[6], sysled, fanled, poeled, byte[2], i;
	u8 fanerror;
	u8 fanfail,fanturnoff,psufail,retry,swrdy;
	u8 psustatus=0;
	u8 initial_thermaltrip[3] = {0,0,0};
	int poe_enable;

    while (!kthread_should_stop())
    {
#if ENABLE_AUTOFAN
		//initial tmp75's thermaltrip value
		if(initial_thermaltrip[0]==0)
		{
			if((probe_setvalue(SWITCH_MAX, asic_thermaltrip*1000)!=0xff)
				&& (probe_setvalue(SWITCH_MAX_HYST, (asic_thermaltrip-temp_hysteresis)*1000)!=0xff))
					initial_thermaltrip[0]=1;
		}
		if(initial_thermaltrip[1]==0)
		{
			if((probe_setvalue(ENV_MAX, env_thermaltrip*1000)!=0xff)
				&& (probe_setvalue(ENV_MAX_HYST, (env_thermaltrip-temp_hysteresis)*1000)!=0xff))
					initial_thermaltrip[1]=1;
		}
		if(initial_thermaltrip[2]==0)
		{
			if((probe_setvalue(OUTLET_MAX, outlet_thermaltrip*1000)!=0xff)
				&& (probe_setvalue(OUTLET_MAX_HYST, (outlet_thermaltrip-temp_hysteresis)*1000)!=0xff))
					initial_thermaltrip[2]=1;
		}
#endif	

		//LED control
		if (data->diag==0 && i2c_smbus_read_byte_data(client, 0)>=0)
		{
			for(retry=0;retry<2;retry++)
			{
				fanfail=0;
				fanerror=0;
				fanturnoff=0;

				//------ Fan Check -------
				cpld_i2c_read(client, fanrpm, CPLD_RPM_OFFSET, 6);

				for (i=0;i<FAN_NUM;i++)
				{
					if(fanrpm[i*2]==0)
						fanerror++;
				}
				if(fanerror==2) break;
			}

			//Fan LED control
			for (i=0;i<FAN_NUM;i++)
			{
				//fan < 256RPM : Red LED
				if(fanrpm[i*2]==0) fanfail++;
			}

			if(fanfail==2) 
			{
				fanled &= 0x0;
				fanled |= 0x30; //turn-on green led
			}
			else
			{
				fanled &= 0x0;
				fanled |= 0x03; //turn-on red led
			}
			
			cpld_i2c_write(client, &fanled, CPLD_FANLED_OFFSET, 1);
#if ENABLE_AUTOFAN
			//Fan PWM control
			autofanspeed(client, fanfail, fanturnoff);
#endif
			//Sys LED control
			cpld_i2c_read(client, &swrdy, CPLD_CTL_OFFSET, 1);
			if(swrdy==0x01)
			{
				sysled &= 0x0;
				sysled |= 0x30; //turn-on green led
			}
			
			cpld_i2c_write(client, &sysled, CPLD_SYSLED_OFFSET, 1);

			poe_enable = check_poe_led();
			if(poe_enable)
			{
				poeled &= 0x0;
				poeled |= 0x30;
				cpld_i2c_write(client, &poeled, CPLD_POELED_OFFSET, 1);
			}
			else
			{
				poeled &= 0x0;
				cpld_i2c_write(client, &poeled, CPLD_POELED_OFFSET, 1);
			}

		}
    	set_current_state(TASK_INTERRUPTIBLE);
    	if(kthread_should_stop()) break;
    	schedule_timeout(msecs_to_jiffies(CPLD_POLLING_PERIOD));
	}
	return 0;
}

/*-----------------------------------------------------------------------*/
/* device probe and removal */

static int
cpld_probe(struct i2c_client *client, const struct i2c_device_id *id)
{
	struct cpld_data *data;
	int status;
	u8 byte[3]={PWM_DEFAULT,PWM_DEFAULT,PWM_DEFAULT};
   
	if (!i2c_check_functionality(client->adapter,
			I2C_FUNC_SMBUS_BYTE_DATA | I2C_FUNC_SMBUS_WORD_DATA))
		return -EIO;

	data = kzalloc(sizeof(struct cpld_data), GFP_KERNEL);
	if (!data)
		return -ENOMEM;

	i2c_set_clientdata(client, data);
	mutex_init(&data->update_lock);

	/* Register sysfs hooks */
	status = sysfs_create_group(&client->dev.kobj, &cpld_group);

	if (status)
		goto exit_free;

	data->hwmon_dev = hwmon_device_register(&client->dev);
	if (IS_ERR(data->hwmon_dev)) {
		status = PTR_ERR(data->hwmon_dev);
		goto exit_remove;
	}

	dev_info(&client->dev, "%s: sensor '%s'\n",
		 dev_name(data->hwmon_dev), client->name);
	
	//initial PWM to 60%
	cpld_i2c_write(client, byte, CPLD_PWM_OFFSET, 3);
	
	//Handle LED control by the driver
	byte[0]=0x01;
	cpld_i2c_write(client, byte, CPLD_CTL_OFFSET, 1);
	
	client_notifier=client;
	
	data->cpld_thread = kthread_run(cpld_polling,client,"%s",dev_name(data->hwmon_dev));

	return 0;

exit_remove:
	sysfs_remove_group(&client->dev.kobj, &cpld_group);
exit_free:
	i2c_set_clientdata(client, NULL);
	kfree(data);
	return status;
}

static int cpld_remove(struct i2c_client *client)
{	
	struct cpld_data *data = i2c_get_clientdata(client);
	//Return LED control to the CPLD
	u8 byte=0x00;
	cpld_i2c_write(client, &byte, CPLD_CTL_OFFSET, 1);
	
	//stop cpld thread
	kthread_stop(data->cpld_thread);
	
	sysfs_remove_group(&client->dev.kobj, &cpld_group);
	hwmon_device_unregister(data->hwmon_dev);
	i2c_set_clientdata(client, NULL);

	kfree(data);
	return 0;
}

static const struct i2c_device_id cpld_ids[] = {
	{ "inv_cpld" , 0, },
	{ /* LIST END */ }
};
MODULE_DEVICE_TABLE(i2c, cpld_ids);

static struct i2c_driver cpld_driver = {
	.class		= I2C_CLASS_HWMON,
	.driver = {
		.name	= "inv_cpld",
	},
	.probe		= cpld_probe,
	.remove		= cpld_remove,
	.id_table	= cpld_ids,
};

/*-----------------------------------------------------------------------*/

/* module glue */

static int __init inv_cpld_init(void)
{
	return i2c_add_driver(&cpld_driver);
}

static void __exit inv_cpld_exit(void)
{
	i2c_del_driver(&cpld_driver);
}

MODULE_AUTHOR("jack.ting <ting.jack@inventec>");
MODULE_DESCRIPTION("cpld driver");
MODULE_LICENSE("GPL");

module_init(inv_cpld_init);
module_exit(inv_cpld_exit);
