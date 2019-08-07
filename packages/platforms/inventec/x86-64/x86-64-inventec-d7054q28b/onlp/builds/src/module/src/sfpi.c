/************************************************************
 * sfpi.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 ***********************************************************/
#include <onlp/platformi/sfpi.h>

#include <fcntl.h> /* For O_RDWR && open */
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <onlplib/i2c.h>
#include <onlplib/file.h>
#include "platform_lib.h"

static char sfp_node_path[ONLP_NODE_MAX_PATH_LEN] = {0};

#define NUM_OF_SFP_PORT	(CHASSIS_SFP_COUNT)
static const int sfp_mux_index[NUM_OF_SFP_PORT] = {
#ifdef SWPS_CYPRESS_GA1
10, 11, 12, 13, 14, 15, 16, 17,
18, 19, 20, 21, 22, 23, 24, 25,
26, 27, 28, 29, 30, 31, 32, 33,
34, 35, 36, 37, 38, 39, 40, 41,
42, 43, 44, 45, 46, 47, 48, 49,
50, 51, 52, 53, 54, 55, 56, 57,
58, 59, 60, 61, 62, 63
#endif
#ifdef SWPS_CYPRESS_GA2
11, 10, 13, 12, 15, 14, 17, 16,
19, 18, 21, 20, 23, 22, 25, 24,
27, 26, 29, 28, 31, 30, 33, 32,
35, 34, 37, 36, 39, 38, 41, 40,
43, 42, 45, 44, 47, 46, 49, 48,
51, 50, 53, 52, 55, 54, 57, 56,
59, 58, 61, 60, 63, 62
#endif
#ifdef SWPS_CYPRESS_BAI
11, 10, 13, 12, 15, 14, 17, 16,
19, 18, 21, 20, 23, 22, 25, 24,
27, 26, 29, 28, 31, 30, 33, 32,
35, 34, 37, 36, 39, 38, 41, 40,
43, 42, 45, 44, 47, 46, 49, 48,
51, 50, 53, 52, 55, 54, 57, 56,
59, 58, 61, 60, 63, 62
#endif
};

#define FRONT_PORT_TO_MUX_INDEX(port)	(sfp_mux_index[port])

static int
sfp_node_read_int(char *node_path, int *value, int data_len)
{
    int ret = 0;
    *value = 0;

    ret = onlp_file_read_int(value, node_path);

    if (ret < 0) {
        AIM_LOG_ERROR("Unable to read status from file(%s)\r\n", node_path);
        return ONLP_STATUS_E_INTERNAL;
    }

    return ret;
}

static char*
sfp_get_port_path(int port, char *node_name)
{
    sprintf(sfp_node_path, "/sys/class/swps/port%d/%s", port, node_name);

    return sfp_node_path;
}

/************************************************************
 *
 * SFPI Entry Points
 *
 ***********************************************************/
int
onlp_sfpi_init(void)
{
    /* Called at initialization time */
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_bitmap_get(onlp_sfp_bitmap_t* bmap)
{
    /*
     * Ports {0, 54}
     */
    int p;
    AIM_BITMAP_CLR_ALL(bmap);

    for(p = 0; p < NUM_OF_SFP_PORT; p++) {
        AIM_BITMAP_SET(bmap, p);
    }

    return ONLP_STATUS_OK;
}

int
onlp_sfpi_is_present(int port)
{
    /*
     * Return 1 if present.
     * Return 0 if not present.
     * Return < 0 if error.
     */
    int present;
    char* path = sfp_get_port_path(port, "present");
    if (sfp_node_read_int(path, &present, 0) != 0) {
        AIM_LOG_ERROR("Unable to read present status from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    if (present == 0) {
	present = 1;
    }
    else
    if (present == 1) {
	present = 0;
    }
    else {
        AIM_LOG_ERROR("Unvalid present status %d from port(%d)\r\n",present,port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return present;
}

int
onlp_sfpi_presence_bitmap_get(onlp_sfp_bitmap_t* dst)
{
    uint32_t presence_all[2] = {0};
    int port, ret, index;

    for (port = 0, index = 0; port < NUM_OF_SFP_PORT; port++) {
	if (port == 32) {
	    index = 1;
	}

	ret = onlp_sfpi_is_present(port);
	if (ret == 1) {
	    presence_all[index] |= (1<<port);
	}
	else
	if (ret == 0) {
	    presence_all[index] &= ~(1<<port);
	}
	else {
            AIM_LOG_ERROR("Unable to read present status of port(%d).", port);
	}
    }
    /* Populate bitmap */
    for(port = 0, index = 0; port < NUM_OF_SFP_PORT; port++) {
	if (port == 32) {
	    index = 1;
	}

        AIM_BITMAP_MOD(dst, port, (presence_all[index] & 1));
        presence_all[index] >>= 1;
    }
    return ONLP_STATUS_OK;
}

#define TRANSCEIVER_EEPROM_I2C_ADDR		(0x50)
#define TRANSCEIVER_DOM_I2C_ADDR		(0x51)
#define QSFP28_EEPROM_PAGE_SELECT_OFFSET	(127)
#define QSFP_START_INDEX			(48)

int
onlp_sfpi_eeprom_read(int port, uint8_t data[256])
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);

    memset(data, 0, 256);
    /* Read eeprom information into data[] */
    if (onlp_i2c_read(bus, TRANSCEIVER_EEPROM_I2C_ADDR, 0x00, 256, data, 0) != 0)
    {
        AIM_LOG_ERROR("Unable to read eeprom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dom_read(int port, uint8_t data[256])
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);

    memset(data, 0, 256);
    /* Read eeprom information into data[] */
    if (port > QSFP_START_INDEX) {
        if (onlp_i2c_writeb(bus, TRANSCEIVER_EEPROM_I2C_ADDR, QSFP28_EEPROM_PAGE_SELECT_OFFSET, 1, ONLP_I2C_F_FORCE) < 0)
	{
	    AIM_LOG_INFO("%s:%d set EEPROM byte 127 for QSFP DOM read fail\n", __FUNCTION__, __LINE__);
	    return ONLP_STATUS_E_INTERNAL;
	}
    }

    if (onlp_i2c_read(bus, TRANSCEIVER_DOM_I2C_ADDR, 0x00, 256, data, 0) != 0)
    {
        AIM_LOG_ERROR("Unable to read dom from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
}

int
onlp_sfpi_dev_readb(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    uint8_t data = 0;

    /* Read eeprom information into data[] */
    if (onlp_i2c_read(bus, devaddr, addr, 1, &data, 0) != 0)
    {
        AIM_LOG_ERROR("Unable to read byte from port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return (int)data;
}

int
onlp_sfpi_dev_writeb(int port, uint8_t devaddr, uint8_t addr, uint8_t value)
{
#if 0
    int bus = FRONT_PORT_TO_MUX_INDEX(port);

    /* Read eeprom information into data[] */
    if (onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE) != 0)
    {
        AIM_LOG_ERROR("Unable to write byte to port(%d)\r\n", port);
        return ONLP_STATUS_E_INTERNAL;
    }
    return ONLP_STATUS_OK;
#else
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    int ret = onlp_i2c_writeb(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);

    if (ret == 0)  {
	unsigned char buf[8] = { 0 };
	char str[20];

	buf[port/8] = (1 << port%8);
	sprintf(str, "0x%02x%02x%02x%02x%02x%02x%02x%02x", buf[7],buf[6],buf[5],buf[4],buf[3],buf[2],buf[1],buf[0]);
	if (onlp_file_write((uint8_t*)str, 20, INV_SFP_EEPROM_UPDATE) < 0) {
	    AIM_LOG_ERROR("Unable to update eeprom_update for port(%d)\r\n", port);
	    return ONLP_STATUS_E_INTERNAL;
	}
	AIM_LOG_INFO("%s has be updated for port(%d)\r\n",INV_SFP_EEPROM_UPDATE,port);
        return ONLP_STATUS_OK;
    }
    AIM_LOG_ERROR("Unable to modify eeprom for port(%d)\r\n", port);
    return ONLP_STATUS_E_INTERNAL;
#endif
}

int
onlp_sfpi_dev_readw(int port, uint8_t devaddr, uint8_t addr)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    return onlp_i2c_readw(bus, devaddr, addr, ONLP_I2C_F_FORCE);
}

int
onlp_sfpi_dev_writew(int port, uint8_t devaddr, uint8_t addr, uint16_t value)
{
    int bus = FRONT_PORT_TO_MUX_INDEX(port);
    int ret = onlp_i2c_writew(bus, devaddr, addr, value, ONLP_I2C_F_FORCE);

    if (ret == 0)  {
	unsigned char buf[8] = { 0 };
	char str[20];

	buf[port/8] = (1 << port%8);
	sprintf(str, "0x%02x%02x%02x%02x%02x%02x%02x%02x", buf[7],buf[6],buf[5],buf[4],buf[3],buf[2],buf[1],buf[0]);
	if (onlp_file_write((uint8_t*)str, 20, INV_SFP_EEPROM_UPDATE) < 0) {
	    AIM_LOG_ERROR("Unable to write eeprom_update for port(%d)\r\n", port);
	    return ONLP_STATUS_E_INTERNAL;
	}
        return ONLP_STATUS_OK;
    }
    AIM_LOG_ERROR("Unable to read eeprom_update from port(%d)\r\n", port);
    return ret;
}

int
onlp_sfpi_denit(void)
{
    return ONLP_STATUS_OK;
}
