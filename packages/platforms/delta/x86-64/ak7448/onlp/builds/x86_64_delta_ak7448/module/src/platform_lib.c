/************************************************************
 * <bsn.cl fy=2014 v=onl>
 *
 *           Copyright 2014 Big Switch Networks, Inc.
 *           Copyright (C) 2017  Delta Networks, Inc.
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
#include <sys/mman.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <AIM/aim.h>
#include "platform_lib.h"
#include <onlplib/i2c.h>
#include <onlplib/mmap.h>
#include <pthread.h>

int dni_i2c_read_attribute_binary(char *filename, char *buffer, int buf_size, int data_len)
{
    int fd;
    int len;

    if ((buffer == NULL) || (buf_size < 0)) {
        return -1;
    }

    if ((fd = open(filename, O_RDONLY)) == -1) {
        return -1;
    }

    if ((len = read(fd, buffer, buf_size)) < 0) { 
        close(fd);
        return -1;
    }

    if ((close(fd) == -1)) {
        return -1;
    }

    if ((len > buf_size) || (data_len != 0 && len != data_len)) {
        return -1;
    }

    return 0;
}

int dni_i2c_read_attribute_string(char *filename, char *buffer, int buf_size, int data_len)
{
    int ret;

    if (data_len >= buf_size) {
	    return -1;
    }

    ret = dni_i2c_read_attribute_binary(filename, buffer, buf_size-1, data_len);

    if (ret == 0) {
        buffer[buf_size-1] = '\0';
    }

    return ret;
}

/* Lock function */
int dni_i2c_lock_read( mux_info_t * mux_info, dev_info_t * dev_info)
{
    int r_data=0;

    pthread_mutex_lock(&mutex);
    if(mux_info != NULL){
        char cpld_path[100] = {0};
        sprintf(cpld_path, "%s", CPLD_B_PLATFORM_PATH); 
        dni_lock_cpld_write_attribute(cpld_path, mux_info->offset, mux_info->channel);
    }
    if(dev_info->size == 1)
        r_data = onlp_i2c_readb(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);
    else
        r_data = onlp_i2c_readw(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->flags);
 
    pthread_mutex_unlock(&mutex);
    return r_data;
}

int dni_i2c_lock_write( mux_info_t * mux_info, dev_info_t * dev_info)
{
    pthread_mutex_lock(&mutex);
    int ret;
    if(mux_info != NULL){
        char cpld_path[100] = {0};
        sprintf(cpld_path, "%s", CPLD_B_PLATFORM_PATH);
        dni_lock_cpld_write_attribute(cpld_path, mux_info->offset, mux_info->channel);
    }
    /* Write size */
    if(dev_info->size == 1)
        ret = onlp_i2c_write(dev_info->bus, dev_info->addr, dev_info->offset, 1, &dev_info->data_8, dev_info->flags);
    else
        ret = onlp_i2c_writew(dev_info->bus, dev_info->addr, dev_info->offset, dev_info->data_16, dev_info->flags);

    pthread_mutex_unlock(&mutex);
    if (ret != 0)
    {
        return -1;
    }
    return 0;
}

int dni_i2c_lock_read_attribute(mux_info_t * mux_info, char * fullpath)
{
    int fd, len, nbytes = 10;
    char  r_data[10]   = {0};

    pthread_mutex_lock(&mutex);
    if(mux_info != NULL){
        char cpld_path[100] = {0};
        sprintf(cpld_path, "%s", CPLD_B_PLATFORM_PATH);
        dni_lock_cpld_write_attribute(cpld_path, mux_info->offset, mux_info->channel);
    }
    if ((fd = open(fullpath, O_RDONLY)) == -1){
        goto ERROR;
    }
    if ((len = read(fd, r_data, nbytes)) <= 0){
        goto ERROR;
    }
    close(fd);
    pthread_mutex_unlock(&mutex);
    return atoi(r_data);
ERROR:
    close(fd);
    pthread_mutex_unlock(&mutex);
    return -1;
}

int dni_i2c_lock_write_attribute(mux_info_t * mux_info, char * data,char * fullpath)
{
    int fd, len, nbytes = 10;
    pthread_mutex_lock(&mutex);
    if(mux_info!=NULL){
        char cpld_path[100] = {0};
        sprintf(cpld_path, "%s", CPLD_B_PLATFORM_PATH);
        dni_lock_cpld_write_attribute(cpld_path, mux_info->offset, mux_info->channel);
    }
    /* Create output file descriptor */
    fd = open(fullpath, O_WRONLY,  0644);
    if (fd == -1){
        goto ERROR;
    }
    len = write (fd, data, (ssize_t) nbytes);
    if (len != nbytes){
        goto ERROR;
    }
    close(fd);
    pthread_mutex_unlock(&mutex);
    return 0;
ERROR:
    close(fd);
    pthread_mutex_unlock(&mutex);
    return -1;
}

/* Use this function to select MUX and read data on CPLD */
int dni_lock_cpld_read_attribute(char *cpld_path, int addr)
{
    int fd, len, nbytes = 10,data = 0;
    char r_data[10]   = {0};
    char address[10] = {0};
    char cpld_data_path[100] = {0};
    char cpld_addr_path[100] = {0};

    sprintf(cpld_data_path, "%s/data", cpld_path);
    sprintf(cpld_addr_path, "%s/addr", cpld_path);
    sprintf(address, "%02x", addr);

    pthread_mutex_lock(&mutex1);
    /* Create output file descriptor */
    fd = open(cpld_addr_path, O_WRONLY, 0644);
    if (fd == -1){
        goto ERR_HANDLE; 
    }

    len = write (fd, address, 2);
    if (len <= 0){
      goto ERR_HANDLE;
    }
    close(fd);

    if ((fd = open(cpld_data_path, O_RDONLY, 0644)) == -1){
        goto ERR_HANDLE;
    }

    if ((len = read(fd, r_data, nbytes)) <= 0){
        goto ERR_HANDLE;
    }
    close(fd);
    pthread_mutex_unlock(&mutex1);

    sscanf(r_data, "%x", &data);
    return data;

ERR_HANDLE:
    close(fd);
    pthread_mutex_unlock(&mutex1);
    return -1;
}

/* Use this function to select MUX and write data on CPLD */
int dni_lock_cpld_write_attribute(char *cpld_path, int addr, int data)
{
    int fd, len;
    char address[10] = {0};
    char cpld_data_path[100] = {0};
    char cpld_addr_path[100] = {0};

    sprintf(cpld_data_path, "%s/data", cpld_path);
    sprintf(cpld_addr_path, "%s/addr", cpld_path);
    sprintf(address, "%02x", addr);

    pthread_mutex_lock(&mutex1);
    /* Create output file descriptor */
    fd = open(cpld_addr_path, O_WRONLY,  0644);
    if (fd == -1){
        goto ERR_HANDLE;
    }
    len = write(fd, address, 2);
    if(len <= 0){
        goto ERR_HANDLE;
    }
    close(fd);

    fd = open(cpld_data_path, O_WRONLY,  0644);
    if (fd == -1){
        goto ERR_HANDLE;
    }
    sprintf(address, "%02x", data);
    len = write (fd, address, 2);
    if(len <= 0){
        goto ERR_HANDLE;
    }
    close(fd);
    pthread_mutex_unlock(&mutex1);

    return 0;

ERR_HANDLE:
    close(fd);
    pthread_mutex_unlock(&mutex1);
    return -1;
}


int dni_fan_speed_good()
{
    int rpm = 0, rpm1 = 0, speed_good = 0;
    mux_info_t mux_info;

    mux_info.offset = FAN_I2C_MUX_SEL_REG;
    mux_info.channel = FAN_I2C_SEL_FAN_CTRL;

    rpm = dni_i2c_lock_read_attribute(&mux_info, FAN1_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(&mux_info, FAN1_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;

    rpm = dni_i2c_lock_read_attribute(&mux_info, FAN2_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(&mux_info, FAN2_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;

    rpm = dni_i2c_lock_read_attribute(&mux_info, FAN3_FRONT);
    rpm1 = dni_i2c_lock_read_attribute(&mux_info, FAN3_REAR);
    if(rpm != 0 && rpm != FAN_ZERO_RPM && rpm1 != 0 && rpm1 != FAN_ZERO_RPM)
        speed_good++;

    return speed_good;
}

