/*
 * ports_device.c
 * Aug 5, 2016 Initial
 * Jan 26, 2017 To support Redwood
 *
 * Inventec Platform Proxy Driver for nic ports device.
 *
 */
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/types.h>
#include <linux/slab.h>
#include <linux/string.h>
#include <linux/version.h>
#include <linux/delay.h>
#include <linux/inventec/common/inventec_sysfs.h>

/* inventec_sysfs.c */
static struct class *inventec_class_p = NULL;

/* ports_device.c */
static struct device *ports_device_p = NULL;
static dev_t ports_device_devt = 0;

typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} port_dev_t;

typedef struct {
	const char		*port_name;
	int			port_major;
        dev_t			port_devt;
	struct device		*port_dev_p;
	port_dev_t		*port_dev_namep;
	int			port_dev_total;
	char			*port_inv_pathp;
	void			*port_tracking;
} port_dev_group_t;

typedef struct tranceiver_type_s {
        char    *info_code_p;
        char    *transceiver_type;
} tranceiver_type_t;

tranceiver_type_t      transceiver_table[] = {
        { "-26001",     "error type" },
        { "26001",      "1G" },
        { "26011",      "10G" },
        { "26021",      "25G" },
        { "26041",      "40G" },
        { "26101",      "100G" },
        { "27000",      "Optical" },
        { "28000",      "Copper" },

        { "27001",      "SFP 100 Base" },
        { "27002",      "SFP 1G Base" },
        { "27003",      "SFP 1G AOC" },
        { "27004",      "SFP 1G SX" },
        { "27005",      "SFP 1G LX" },
        { "27006",      "SFP 1G EX" },
        { "28001",      "SFP 1G DAC" },

        { "29001",      "SFP 1G Copper" },
        { "29002",      "SFP 1G Copper" },

        { "27010",      "SFP 10G Optical" },
        { "27011",      "SFP 10G AOC" },
        { "27012",      "SFP 10G SR" },
        { "27013",      "SFP 10G LR" },
        { "27014",      "SFP 10G ER" },
        { "28011",      "SFP 10G DAC" },

        { "27020",      "SFP 25G Optical" },
        { "27021",      "SFP 25G AOC" },
        { "27022",      "SFP 25G SR" },
        { "27023",      "SFP 25G LR" },
        { "27024",      "SFP 25G ER" },
        { "28021",      "SFP 25G DAC" },

        { "27040",      "QSFP 40G" },
        { "27041",      "QSFP 40G AOC" },
        { "27042",      "QSFP 40G SR4" },
        { "27043",      "QSFP 40G LR4" },
        { "27044",      "QSFP 40G ER4" },
        { "28041",      "QSFP 40G DAC" },

        { "27100",      "QSFP 100G" },
        { "27101",      "QSFP 100G AOC" },
        { "27102",      "QSFP 100G SR4" },
        { "27103",      "QSFP 100G LR4" },
        { "27104",      "QSFP 100G ER4" },
        { "27105",      "QSFP 100G PSM4" },
        { "28101",      "QSFP 100G DAC" },
};
#define TRANSCEIVER_TABLE_TOTAL  ( sizeof(transceiver_table)/ sizeof(const tranceiver_type_t) )

static ssize_t port_transceiver_type(const char *info_code, char *buf_p)
{
    int i;

    for (i = 0; i < TRANSCEIVER_TABLE_TOTAL; i++) {
        if (strncmp(info_code, transceiver_table[i].info_code_p,strlen(transceiver_table[i].info_code_p)) == 0) {
	    return sprintf(buf_p, "%s\n",transceiver_table[i].transceiver_type);
      	}
    }
    //return sprintf(buf_p, "unknown type: transceiver code = %s\n", info_code);
    return sprintf(buf_p, "%s\n", info_code);
}

#if defined PLATFORM_MAGNOLIA
static port_dev_t port_sfp_dev_name[] = {
	{ "plugged_in",		"present" },	//present 0 -> plugged_in 1
	{ "identifier",		"id" },
	{ "connector_type",	"connector" },
	{ "transceiver_code",	"info" },
	{ "length_om1",		"len_om1" },
	{ "length_om2",		"len_om2" },
	{ "length_om3",		"len_om3" },
	{ "length_om4",		"len_om4" },
	{ "length_sm",		"len_sm" },
	{ "length_smf",		"len_smf" },
	{ "vendor_name",	"vendor_name" },
	{ "vendor_pn",		"vendor_pn" },
	{ "vendor_sn",		"vendor_sn" },
	{ "wavelength",		"wavelength" },
	{ "temperature",	"temperature" },
	{ "supply_voltage",	"voltage" },
	{ "tx_bias",		"tx_bias" },
	{ "tx_bias_max",	"tx_bias_max" },
	{ "tx_bias_max_th",	"tx_bias_max_th" },
	{ "tx_bias_min",	"tx_bias_min" },
	{ "tx_bias_min_th",	"tx_bias_min_th" },
	{ "tx_bias_hi_warn",	"tx_bias_hi_warn" },
	{ "tx_bias_hi_warn_th",	"tx_bias_hi_warn_th" },
	{ "tx_bias_low_warn","tx_bias_low_warn" },
	{ "tx_bias_low_warn_th","tx_bias_low_warn_th" },
	{ "tx_power",		"tx_power" },
	{ "tx_power_max",	"tx_power_max" },
	{ "tx_power_min",	"tx_power_min" },
	{ "tx_power_hi_warn",	"tx_power_hi_warn" },
	{ "tx_power_lo_warn_",	"tx_power_low_warn" },
	{ "tx_power_max_th",	"tx_power_max_th" },
	{ "tx_power_min_th",	"tx_power_min_th" },
	{ "tx_power_hi_warn_th","tx_power_hi_warn_th" },
	{ "tx_power_lo_warn_th","tx_power_low_warn_th" },
	{ "rx_power",		"rx_power" },
	{ "rx_power_max",	"rx_power_max" },
	{ "rx_power_min",	"rx_power_min" },
	{ "rx_power_hi_warn",	"rx_power_hi_warn" },
	{ "rx_power_lo_warn",	"rx_power_low_warn" },
	{ "rx_power_max_th",	"rx_power_max_th" },
	{ "rx_power_min_th",	"rx_power_min_th" },
	{ "rx_power_hi_warn_th","rx_power_hi_warn_th" },
	{ "rx_power_lo_warn_th","rx_power_low_warn_th" },
	{ "tx_fault",		"tx_fault" },
	{ "tx_enable",		"tx_disable" },
	{ "phy_speed",		"phy_spec_status_reg" },
	{ "rxlos",		"rxlos" },
	{ "comp_eth",		"comp_eth" },
	{ "br",			"br" },
	{ "br_max",		"br_max" },
	{ "br_min",		"br_min" },
	{ "date_code",		"date_code" },
	{ "vendor_spec",	"vendor_spec" },
	{ "ctrl_reg",		"ctrl_reg" },
	{ "status_reg",		"status_reg" },
	{ "phy_ident1_reg",	"phy_ident1_reg" },
	{ "phy_ident2_reg",	"phy_ident2_reg" },
	{ "auto_neg_adv_reg",	"auto_neg_adv_reg" },
	{ "link_partner_ability_reg",		"link_partner_ability_reg" },
	{ "autoneg_expan_reg",	"autoneg_expan_reg" },
	{ "np_tx_reg",		"np_tx_reg" },
	{ "link_partner_np_reg","link_partner_np_reg" },
	{ "g_baset_ctrl_reg",	"g_baset_ctrl_reg" },
	{ "g_baset_status_reg",	"g_baset_status_reg" },
	{ "ext_status_reg",	"ext_status_reg" },
	{ "phy_spec_ctrl_reg",	"phy_spec_ctrl_reg" },
	{ "int_status_reg",		"int_status_reg" },
	{ "ext_phy_spec_ctrl_reg",	"ext_phy_spec_ctrl_reg" },
	{ "rc_err_cnt_reg",	"rc_err_cnt_reg" },
	{ "ext_addr_reg",	"ext_addr_reg" },
	{ "global_status_reg",	"global_status_reg" },
	{ "led_ctrl_reg",	"led_ctrl_reg" },
	{ "man_led_reg",	"man_led_reg" },
	{ "ext_phy_spec_ctrl2_reg",	"ext_phy_spec_ctrl2_reg" },
	{ "ext_phy_status_reg",	"ext_phy_status_reg" },
	{ "ext_addr",		"ext_addr" },
	{ "ext_reg",		"ext_reg" },
};

static port_dev_t port_qsfp_dev_name[] = {
	{ "plugged_in",		"present" },	//present 0 -> plugged_in 1
	{ "identifier",		"id" },
	{ "ext_identifier",	"ext_id" },
	{ "connector_type",	"connector" },
	{ "transceiver_code",	"info" },
	{ "length_om1",		"len_om1" },
	{ "length_om2",		"len_om2" },
	{ "length_om3",		"len_om3" },
	{ "length_om4",		"len_om4" },
	{ "length_smf",		"len_smf" },
	{ "vendor_name",	"vendor_name" },
	{ "vendor_pn",		"vendor_pn" },
	{ "vendor_sn",		"vendor_sn" },
	{ "wavelength",		"wavelength" },
	{ "temperature",	"temperature" },
	{ "supply_voltage",	"voltage" },
	{ "comp_eth",		"comp_eth" },
	{ "br",			"br" },
	{ "tx_enable",		"soft_tx_disable" },
#if 0
	{ "rx1_power_max_th",	 "rx_power_max_th" },
	{ "rx1_power_min_th",	 "rx_power_min_th" },
	{ "rx1_power_hi_warn_th","rx_power_hi_warn_th" },
	{ "rx1_power_lo_warn_th","rx_power_low_warn_th" },
	{ "rx2_power_max_th",	 "rx_power_max_th" },
	{ "rx2_power_min_th",	 "rx_power_min_th" },
	{ "rx2_power_hi_warn_th","rx_power_hi_warn_th" },
	{ "rx2_power_lo_warn_th","rx_power_low_warn_th" },
	{ "rx3_power_max_th",	 "rx_power_max_th" },
	{ "rx3_power_min_th",	 "rx_power_min_th" },
	{ "rx3_power_hi_warn_th","rx_power_hi_warn_th" },
	{ "rx3_power_lo_warn_th","rx_power_low_warn_th" },
	{ "rx4_power_max_th",	 "rx_power_max_th" },
	{ "rx4_power_min_th",	 "rx_power_min_th" },
	{ "rx4_power_hi_warn_th","rx_power_hi_warn_th" },
	{ "rx4_power_lo_warn_th","rx_power_low_warn_th" },
	{ "rxlos",		"rxlos" },
	{ "status_reg",		"status_reg" },
	{ "phy_ident1_reg",	"phy_ident1_reg" },
	{ "phy_ident2_reg",	"phy_ident2_reg" },
	{ "auto_neg_adv_reg",	"auto_neg_adv_reg" },
	{ "link_partner_ability_reg",		"link_partner_ability_reg" },
	{ "autoneg_expan_reg",	"autoneg_expan_reg" },
	{ "np_tx_reg",		"np_tx_reg" },
	{ "link_partner_np_reg","link_partner_np_reg" },
	{ "g_baset_status_reg",	"g_baset_status_reg" },
	{ "ext_status_reg",	"ext_status_reg" },
	{ "phy_spec_ctrl_reg",	"phy_spec_ctrl_reg" },
	{ "int_status_reg",		"int_status_reg" },
	{ "ext_phy_spec_ctrl_reg",	"ext_phy_spec_ctrl_reg" },
	{ "rc_err_cnt_reg",	"rc_err_cnt_reg" },
	{ "global_status_reg",	"global_status_reg" },
	{ "led_ctrl_reg",	"led_ctrl_reg" },
	{ "man_led_reg",	"man_led_reg" },
	{ "ext_phy_spec_ctrl2_reg",	"ext_phy_spec_ctrl2_reg" },
	{ "ext_phy_status_reg",	"ext_phy_status_reg" },
	{ "tx_bias",	"tx_bias" },
	{ "rx1_power",		"rx_power" },
	{ "rx2_power",		"rx_power" },
	{ "rx3_power",		"rx_power" },
	{ "rx4_power",		"rx_power" },
	{ "tx1_power",		"tx_power" },
	{ "tx2_power",		"tx_power" },
	{ "tx3_power",		"tx_power" },
	{ "tx4_power",		"tx_power" },
	{ "tx1_power_max_th",	 "tx_power_max_th" },
	{ "tx1_power_min_th",	 "tx_power_min_th" },
	{ "tx1_power_hi_warn_th","tx_power_hi_warn_th" },
	{ "tx1_power_lo_warn_th","tx_power_low_warn_th" },
	{ "tx2_power_max_th",	 "tx_power_max_th" },
	{ "tx2_power_min_th",	 "tx_power_min_th" },
	{ "tx2_power_hi_warn_th","tx_power_hi_warn_th" },
	{ "tx2_power_lo_warn_th","tx_power_low_warn_th" },
	{ "tx3_power_max_th",	 "tx_power_max_th" },
	{ "tx3_power_min_th",	 "tx_power_min_th" },
	{ "tx3_power_hi_warn_th","tx_power_hi_warn_th" },
	{ "tx3_power_lo_warn_th","tx_power_low_warn_th" },
	{ "tx4_power_max_th",	 "tx_power_max_th" },
	{ "tx4_power_min_th",	 "tx_power_min_th" },
	{ "tx4_power_hi_warn_th","tx_power_hi_warn_th" },
	{ "tx4_power_lo_warn_th","tx_power_low_warn_th" },
	{ "br_max",		"br_max" },
	{ "br_min",		"br_min" },
	{ "date_code",		"date_code" },
	{ "vendor_spec",	"vendor_spec" },
	{ "ctrl_reg",		"ctrl_reg" },
	{ "g_baset_ctrl_reg",	"g_baset_ctrl_reg" },
	{ "ext_addr_reg",	"ext_addr_reg" },
	{ "ext_addr",		"ext_addr" },
	{ "ext_reg",		"ext_reg" },
#endif
};
#elif defined PLATFORM_REDWOOD
static port_dev_t port_qsfp_dev_name[] = {
	{ "plugged_in",		"present" },	//present 0 -> plugged_in 1
	{ "identifier",		"id" },
	{ "br",			"br" },
	{ "comp_eth",		"comp_eth" },
	{ "connector_type",	"connector" },
	{ "ext_identifier",	"ext_id" },
	{ "transceiver_code",	"info" },
	{ "length_om1",		"len_om1" },
	{ "length_om2",		"len_om2" },
	{ "length_om3",		"len_om3" },
	{ "length_om4",		"len_om4" },
	{ "length_smf",		"len_smf" },
	{ "rx1_power",		"rx_power" },
	{ "rx2_power",		"rx_power" },
	{ "rx3_power",		"rx_power" },
	{ "rx4_power",		"rx_power" },
	{ "tx_enable",		"soft_tx_disable" },
	{ "temperature",	"temperature" },
	{ "tx_bias",		"tx_bias" },
	{ "tx1_power",		"tx_power" },
	{ "tx2_power",		"tx_power" },
	{ "tx3_power",		"tx_power" },
	{ "tx4_power",		"tx_power" },
	{ "vendor_name",	"vendor_name" },
	{ "vendor_pn",		"vendor_pn" },
	{ "vendor_sn",		"vendor_sn" },
	{ "supply_voltage",	"voltage" },
	{ "wavelength",		"wavelength" },
};
#endif

#if defined PLATFORM_MAGNOLIA
static port_dev_group_t port_dev_group[] = {
	{
	  .port_name = "port1",
	},
	{
	  .port_name = "port2",
	},
	{
	  .port_name = "port3",
	},
	{
	  .port_name = "port4",
	},
	{
	  .port_name = "port5",
	},
	{
	  .port_name = "port6",
	},
	{
	  .port_name = "port7",
	},
	{
	  .port_name = "port8",
	},
	{
	  .port_name = "port9",
	},
	{
	  .port_name = "port10",
	},
	{
	  .port_name = "port11",
	},
	{
	  .port_name = "port12",
	},
	{
	  .port_name = "port13",
	},
	{
	  .port_name = "port14",
	},
	{
	  .port_name = "port15",
	},
	{
	  .port_name = "port16",
	},
	{
	  .port_name = "port17",
	},
	{
	  .port_name = "port18",
	},
	{
	  .port_name = "port19",
	},
	{
	  .port_name = "port20",
	},
	{
	  .port_name = "port21",
	},
	{
	  .port_name = "port22",
	},
	{
	  .port_name = "port23",
	},
	{
	  .port_name = "port24",
	},
	{
	  .port_name = "port25",
	},
	{
	  .port_name = "port26",
	},
	{
	  .port_name = "port27",
	},
	{
	  .port_name = "port28",
	},
	{
	  .port_name = "port29",
	},
	{
	  .port_name = "port30",
	},
	{
	  .port_name = "port31",
	},
	{
	  .port_name = "port32",
	},
	{
	  .port_name = "port33",
	},
	{
	  .port_name = "port34",
	},
	{
	  .port_name = "port35",
	},
	{
	  .port_name = "port36",
	},
	{
	  .port_name = "port37",
	},
	{
	  .port_name = "port38",
	},
	{
	  .port_name = "port39",
	},
	{
	  .port_name = "port40",
	},
	{
	  .port_name = "port41",
	},
	{
	  .port_name = "port42",
	},
	{
	  .port_name = "port43",
	},
	{
	  .port_name = "port44",
	},
	{
	  .port_name = "port45",
	},
	{
	  .port_name = "port46",
	},
	{
	  .port_name = "port47",
	},
	{
	  .port_name = "port48",
	},
	{
	  .port_name = "port49",
	},
	{
	  .port_name = "port50",
	},
	{
	  .port_name = "port51",
	},
	{
	  .port_name = "port52",
	},
	{
	  .port_name = "port53",
	},
	{
	  .port_name = "port54",
	},
};
#elif defined PLATFORM_REDWOOD
static port_dev_group_t port_dev_group[] = {
	{
	  .port_name = "port1",
	},
	{
	  .port_name = "port2",
	},
	{
	  .port_name = "port3",
	},
	{
	  .port_name = "port4",
	},
	{
	  .port_name = "port5",
	},
	{
	  .port_name = "port6",
	},
	{
	  .port_name = "port7",
	},
	{
	  .port_name = "port8",
	},
	{
	  .port_name = "port9",
	},
	{
	  .port_name = "port10",
	},
	{
	  .port_name = "port11",
	},
	{
	  .port_name = "port12",
	},
	{
	  .port_name = "port13",
	},
	{
	  .port_name = "port14",
	},
	{
	  .port_name = "port15",
	},
	{
	  .port_name = "port16",
	},
	{
	  .port_name = "port17",
	},
	{
	  .port_name = "port18",
	},
	{
	  .port_name = "port19",
	},
	{
	  .port_name = "port20",
	},
	{
	  .port_name = "port21",
	},
	{
	  .port_name = "port22",
	},
	{
	  .port_name = "port23",
	},
	{
	  .port_name = "port24",
	},
	{
	  .port_name = "port25",
	},
	{
	  .port_name = "port26",
	},
	{
	  .port_name = "port27",
	},
	{
	  .port_name = "port28",
	},
	{
	  .port_name = "port29",
	},
	{
	  .port_name = "port30",
	},
	{
	  .port_name = "port31",
	},
	{
	  .port_name = "port32",
	},
};
#elif defined PLATFORM_CYPRESS
#endif
#define PORT_DEV_TOTAL	( sizeof(port_dev_group)/ sizeof(const port_dev_group_t) )

static char *port_get_invpath(const char *attr_namep, const int devmajor)
{
    port_dev_t *devnamep;
    int i;
    int port_id = -1;

    for (i = 0; i < PORT_DEV_TOTAL; i++) {
        if (devmajor == port_dev_group[i].port_major) {
            port_id = i;
            break;
	}
    }
    if (port_id == -1) {
        return NULL;
    }

    devnamep = port_dev_group[port_id].port_dev_namep;
    for (i = 0; i < port_dev_group[port_id].port_dev_total; i++, devnamep++) {
        if (strcmp(devnamep->proxy_dev_name, attr_namep) == 0) {
            return devnamep->inv_dev_name;
	}
    }
    return NULL;
}

/* INV drivers mapping */
#define SWPS_ACC_PATH_BASE "/sys/class/swps/port%d/%s"
#define PHY_SPEED_NLOOP	20
#define PHY_SPEED_MSLEEP	100

static ssize_t
show_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    char acc_path[64] = "ERR";
    char *invpathp = port_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));
    int portno = simple_strtol(&dev_p->kobj.name[4], NULL, 10);
    ssize_t count;
    char buf[256];

    if (!invpathp) {
        return sprintf(buf_p, "%s\n", SHOW_ATTR_WARNING);
    }

    /* port number in inventec /sys/class/swps is 0 based. We convert it to 1 based in proxy */
    sprintf(acc_path, SWPS_ACC_PATH_BASE, portno-1, invpathp);
    count = inventec_show_attr(buf, acc_path);
    if (count < 0 ) {
        return sprintf(buf_p, "%s\n",SHOW_ATTR_WARNING);
    }

    if (strncmp(buf, "-205", 4) == 0) {
        return sprintf(buf_p, "%s\n",SHOW_ATTR_WARNING);
    }

    if (strncmp(attr_p->attr.name, "phy_speed", 8) == 0) {
	int spec_status;
	int nloop = PHY_SPEED_NLOOP;

phy_speed_nloop_1:
	if (count == 0) {
	    goto phy_speed_nloop_2;
	}
	if (strncmp(buf, "0x", 2) != 0 && strncmp(buf, "0X", 2) != 0) {
	    goto phy_speed_nloop_2;
	}

	spec_status = simple_strtol(&buf[2], NULL, 16);
	if (spec_status >> 14 == 2) {
	    if (spec_status & 0x2000) {
		return sprintf(buf_p, "0x%04X: 1000 Mbps full-duplex\n", spec_status);
	    }
	    else {
		return sprintf(buf_p, "0x%04X: 1000 Mbps half-duplex\n", spec_status);
	    }
	}
	else
	if (spec_status >> 14 == 1) {
	    if (spec_status & 0x2000) {
		return sprintf(buf_p, "0x%04X: 100 Mbps full-duplex\n", spec_status);
	    }
	    else {
		return sprintf(buf_p, "0x%04X: 100 Mbps half-duplex\n", spec_status);
	    }
	}
	else
	if (spec_status >> 14 == 0) {
	    if (spec_status & 0x2000) {
		return sprintf(buf_p, "0x%04X: 10 Mbps full-duplex\n", spec_status);
	    }
	    else {
		return sprintf(buf_p, "0x%04X: 10 Mbps half-duplex\n", spec_status);
	    }
	}

	if (nloop <= 0) {
	    return sprintf(buf_p, "%s: Read phy_speed failed, tried %d times\n", buf, nloop);
	}

phy_speed_nloop_2:
	nloop--;
	msleep(PHY_SPEED_MSLEEP);
	count = inventec_show_attr(buf, acc_path);
	if (count < 0 ) {
	    return sprintf(buf_p, "%s\n",SHOW_ATTR_WARNING);
	}
	if (strncmp(buf, "-205", 4) == 0) {
	    return sprintf(buf_p, "%s\n",SHOW_ATTR_WARNING);
	}
	goto phy_speed_nloop_1;
    }

    if (strncmp(attr_p->attr.name, "plugged_in", 10) == 0) {
        if (buf[0] == '0') {
            return sprintf(buf_p, "1\n");
        }
        else
        if (buf[0] == '1') {
            return sprintf(buf_p, "0\n");
        }
        else {
            return sprintf(buf_p, "Invalid plugged_in value %s\n",buf);
        }
    }

    if (strncmp(attr_p->attr.name, "transceiver_code", 16) == 0) {
        return port_transceiver_type((const char *)&buf[0], buf_p);
    }

    if (strncmp(attr_p->attr.name, "tx_enable", 9) == 0) {
        if (portno >= SYSFS_SFP_DEV_START && portno <= SYSFS_SFP_DEV_END) {
	    int txv = simple_strtol(&buf[0], NULL, 10);
	    if (txv == 0) {
                return sprintf(buf_p, "1\n");
	    }
	    else
	    if (txv == 1) {
                return sprintf(buf_p, "0\n");
	    }
            else {
                return sprintf(buf_p, "Invalid tx_enable value %s\n",buf);
            }
        }
        else
        if (portno >= SYSFS_QSFP_DEV_START && portno <= SYSFS_QSFP_DEV_END) {
            if (strncmp(buf, "0x0", 3) != 0) {
                return sprintf(buf_p, "Invalid tx_enable value %s\n", buf);
            }

            if (buf[3] == '0') return sprintf(buf_p, "1111\n"); else
            if (buf[3] == '1') return sprintf(buf_p, "1110\n"); else
            if (buf[3] == '2') return sprintf(buf_p, "1101\n"); else
            if (buf[3] == '3') return sprintf(buf_p, "1100\n"); else
            if (buf[3] == '4') return sprintf(buf_p, "1011\n"); else
            if (buf[3] == '5') return sprintf(buf_p, "1010\n"); else
            if (buf[3] == '6') return sprintf(buf_p, "1001\n"); else
            if (buf[3] == '7') return sprintf(buf_p, "1000\n"); else
            if (buf[3] == '8') return sprintf(buf_p, "0111\n"); else
            if (buf[3] == '9') return sprintf(buf_p, "0110\n"); else
            if (buf[3] == 'a') return sprintf(buf_p, "0101\n"); else
            if (buf[3] == 'A') return sprintf(buf_p, "0101\n"); else
            if (buf[3] == 'b') return sprintf(buf_p, "0100\n"); else
            if (buf[3] == 'B') return sprintf(buf_p, "0100\n"); else
            if (buf[3] == 'c') return sprintf(buf_p, "0011\n"); else
            if (buf[3] == 'C') return sprintf(buf_p, "0011\n"); else
            if (buf[3] == 'd') return sprintf(buf_p, "0010\n"); else
            if (buf[3] == 'D') return sprintf(buf_p, "0010\n"); else
            if (buf[3] == 'e') return sprintf(buf_p, "0001\n"); else
            if (buf[3] == 'E') return sprintf(buf_p, "0001\n"); else
            if (buf[3] == 'f') return sprintf(buf_p, "0000\n"); else
            if (buf[3] == 'F') return sprintf(buf_p, "0000\n"); else
            return sprintf(buf_p, "Invalid tx_enable value %s\n", buf);
        }
        else {
            return sprintf(buf_p, "Invalid port number %d\n", portno);
        }
    }

    if (strncmp(attr_p->attr.name, "vendor_spec", 11) == 0) {
	int i;

	for( i = 0; i < count ; i++ ){
       	    if( i%80 == 79 ) {
        	buf[i] = '\n';
	    }
	}
        return sprintf(buf_p, "%s", buf);
    }

    return sprintf(buf_p, "%s", buf);
}

static ssize_t
show_attr_tx_bias(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_max(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_min(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_hi_warn(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_low_warn(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_max_th(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_min_th(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_hi_warn_th(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_bias_low_warn_th(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}


static ssize_t
show_attr_connector_type(struct device *dev_p,
                     struct device_attribute *attr_p,
                     char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_identifier(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_identifier(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_length_om1(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_length_om2(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_length_om3(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_length_om4(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_length_sm(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_length_smf(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_plugged_in(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_max(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_min(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_hi_warn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_lo_warn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_supply_voltage(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_temperature(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_transceiver_code(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_fault(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_enable(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_phy_speed(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_max(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_min(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_hi_warn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_lo_warn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_vendor_name(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_vendor_pn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_vendor_sn(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_wavelength(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

#if 0
static ssize_t
show_attr_tx1_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx2_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx3_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx4_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx1_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx1_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx1_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx1_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx1_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx2_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx2_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx2_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx2_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx3_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx3_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx3_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx3_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx4_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx4_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx4_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_tx4_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx2_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx3_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx4_power(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx1_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx1_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx1_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx1_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx2_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx2_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx2_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx2_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx3_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx3_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx3_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx3_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx4_power_max_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx4_power_min_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx4_power_hi_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rx4_power_lo_warn_th(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}
#endif

static ssize_t
show_attr_rxlos(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_comp_eth(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_br(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_br_max(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_br_min(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_date_code(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_vendor_spec(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ctrl_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_status_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_phy_ident1_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_phy_ident2_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_auto_neg_adv_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_link_partner_ability_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_autoneg_expan_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_np_tx_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_link_partner_np_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_g_baset_ctrl_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_g_baset_status_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_status_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_phy_spec_ctrl_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_int_status_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_phy_spec_ctrl_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_rc_err_cnt_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_addr_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_global_status_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_led_ctrl_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_man_led_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_phy_spec_ctrl2_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_phy_status_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_addr(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
show_attr_ext_reg(struct device *dev_p,
			struct device_attribute *attr_p,
			char *buf_p)
{
    return show_attr_common(dev_p, attr_p, buf_p);
}

static ssize_t
port_set_phy_100m(int portno, ssize_t count)
{
    char acc_path[64] = "ERR";
    ssize_t ret;
    int nloop;

	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_addr_reg");
	    while ((ret = inventec_store_attr("0x0000", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_phy_status_reg");
	    while ((ret = inventec_store_attr("0x9084", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"g_baset_ctrl_reg");
	    while ((ret = inventec_store_attr("0x0F00", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0x8140", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"auto_neg_adv_reg");
	    while ((ret = inventec_store_attr("0x0DE1", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0x8140", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_phy_status_reg");
	    while ((ret = inventec_store_attr("0x808C", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0xA100", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_addr_reg");
	    while ((ret = inventec_store_attr("0x0001", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0xA100", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_addr_reg");
	    while ((ret = inventec_store_attr("0x0000", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }

	    return 6;
}

static ssize_t
port_set_phy_1000m(int portno, ssize_t count)
{
    char acc_path[64] = "ERR";
    ssize_t ret;
    int nloop;

	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_addr_reg");
	    while ((ret = inventec_store_attr("0x0000", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_phy_status_reg");
	    while ((ret = inventec_store_attr("0x9084", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"g_baset_ctrl_reg");
	    while ((ret = inventec_store_attr("0x0E00", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0x8140", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"auto_neg_adv_reg");
	    while ((ret = inventec_store_attr("0x0C01", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0x9140", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_phy_status_reg");
	    while ((ret = inventec_store_attr("0x9088", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0x0140", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_addr_reg");
	    while ((ret = inventec_store_attr("0x0001", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ctrl_reg");
	    while ((ret = inventec_store_attr("0x1140", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }
	    nloop = PHY_SPEED_NLOOP;
	    sprintf(acc_path, SWPS_ACC_PATH_BASE,portno-1,"ext_addr_reg");
	    while ((ret = inventec_store_attr("0x0000", 6, acc_path)) != 6 && nloop-- > 0) {
		msleep(PHY_SPEED_MSLEEP);
	    }
	    if (ret != 6) {
		SYSFS_LOG("Write %s failed %d %d\n", acc_path, ret, nloop);
		return ret;
	    }

	    return count;
}

static ssize_t
store_attr_common(struct device *dev_p, struct device_attribute *attr_p, char *buf_p, size_t count)
{
    char acc_path[64] = "ERR";
    char *invpathp = port_get_invpath(attr_p->attr.name, MAJOR(dev_p->devt));
    int portno = simple_strtol(&dev_p->kobj.name[4], NULL, 10);

    if (strncmp(attr_p->attr.name, "phy_speed", 8) == 0) {
	int value = simple_strtol(&buf_p[0], NULL, 10);
	if (value != 1 && value != 2) {
	    return count;
	}

        if (portno >= SYSFS_SFP_DEV_START && portno <= SYSFS_SFP_DEV_END) {
	    if (value == 1) {
		return port_set_phy_100m(portno, count);
	    }
	    else
	    if (value == 2) {
		return port_set_phy_1000m(portno, count);
	    }
	    else {
		return count;
	    }
	}
	printk(KERN_ERR "Invalid port number %d\n",portno);
	return count;
    }

    if (!invpathp) {
        printk(KERN_INFO "%s\n", SHOW_ATTR_WARNING);
        return count;
    }

    if (strncmp(attr_p->attr.name, "tx_enable", 9) == 0) {
        if (portno >= SYSFS_SFP_DEV_START && portno <= SYSFS_SFP_DEV_END) {
	    int txv = simple_strtol(&buf_p[0], NULL, 10);
	    if (txv == 0) {
                strcpy(&buf_p[0], "1");
	    }
	    else
	    if (txv == 1) {
                strcpy(&buf_p[0], "0");
	    }
            else {
                printk(KERN_ERR "Invalid tx_enable value %s\n",buf_p);
                return count;
            }
        }
        else
        if (portno >= SYSFS_QSFP_DEV_START && portno <= SYSFS_QSFP_DEV_END) {
            if (memcmp(buf_p, "1111", 4) == 0) memcpy(buf_p, "0x00", 4); else
            if (memcmp(buf_p, "1110", 4) == 0) memcpy(buf_p, "0x01", 4); else
            if (memcmp(buf_p, "1101", 4) == 0) memcpy(buf_p, "0x02", 4); else
            if (memcmp(buf_p, "1100", 4) == 0) memcpy(buf_p, "0x03", 4); else
            if (memcmp(buf_p, "1011", 4) == 0) memcpy(buf_p, "0x04", 4); else
            if (memcmp(buf_p, "1010", 4) == 0) memcpy(buf_p, "0x05", 4); else
            if (memcmp(buf_p, "1001", 4) == 0) memcpy(buf_p, "0x06", 4); else
            if (memcmp(buf_p, "1000", 4) == 0) memcpy(buf_p, "0x07", 4); else
            if (memcmp(buf_p, "0111", 4) == 0) memcpy(buf_p, "0x08", 4); else
            if (memcmp(buf_p, "0110", 4) == 0) memcpy(buf_p, "0x09", 4); else
            if (memcmp(buf_p, "0101", 4) == 0) memcpy(buf_p, "0x0a", 4); else
            if (memcmp(buf_p, "0100", 4) == 0) memcpy(buf_p, "0x0b", 4); else
            if (memcmp(buf_p, "0011", 4) == 0) memcpy(buf_p, "0x0c", 4); else
            if (memcmp(buf_p, "0010", 4) == 0) memcpy(buf_p, "0x0d", 4); else
            if (memcmp(buf_p, "0001", 4) == 0) memcpy(buf_p, "0x0e", 4); else
            if (memcmp(buf_p, "0000", 4) == 0) memcpy(buf_p, "0x0f", 4);
            else {
                printk(KERN_ERR "Invalid tx_enable value for QSFP port %s\n", buf_p);
                return count;
            }
        }
        else {
            printk(KERN_INFO "Invalid port number %d\n", portno);
            return count;
        }
    }

    sprintf(acc_path, SWPS_ACC_PATH_BASE, portno-1, invpathp);
    return inventec_store_attr(buf_p, count, acc_path);
}

static ssize_t
store_attr_tx_enable(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_phy_speed(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

#if 0
static ssize_t
store_attr_ctrl_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_auto_neg_adv_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_np_tx_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_g_baset_ctrl_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_phy_spec_ctrl_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_ext_addr_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_led_ctrl_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_man_led_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_ext_phy_spec_ctrl2_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_ext_phy_status_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_ext_addr(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}

static ssize_t
store_attr_ext_reg(struct device *dev_p,
                      struct device_attribute *attr_p,
                      const char *buf_p,
                      size_t count)
{
    return store_attr_common(dev_p, attr_p, (char*)buf_p, count);
}
#endif

static DEVICE_ATTR(tx_bias,		S_IRUGO,	 show_attr_tx_bias,		NULL);
static DEVICE_ATTR(tx_bias_max,		S_IRUGO,	 show_attr_tx_bias_max,		NULL);
static DEVICE_ATTR(tx_bias_min,		S_IRUGO,	 show_attr_tx_bias_min,		NULL);
static DEVICE_ATTR(tx_bias_hi_warn,	S_IRUGO,	 show_attr_tx_bias_hi_warn,	NULL);
static DEVICE_ATTR(tx_bias_low_warn,	S_IRUGO,	 show_attr_tx_bias_low_warn,	NULL);
static DEVICE_ATTR(tx_bias_max_th,	S_IRUGO,	 show_attr_tx_bias_max_th,	NULL);
static DEVICE_ATTR(tx_bias_min_th,	S_IRUGO,	 show_attr_tx_bias_min_th,	NULL);
static DEVICE_ATTR(tx_bias_hi_warn_th,	S_IRUGO,	 show_attr_tx_bias_hi_warn_th,	NULL);
static DEVICE_ATTR(tx_bias_low_warn_th,	S_IRUGO,	 show_attr_tx_bias_low_warn_th,	NULL);
static DEVICE_ATTR(connector_type,	S_IRUGO,	 show_attr_connector_type,	NULL);
static DEVICE_ATTR(ext_identifier,	S_IRUGO,	 show_attr_ext_identifier,	NULL);
static DEVICE_ATTR(identifier,		S_IRUGO,	 show_attr_identifier,		NULL);
static DEVICE_ATTR(length_om1,		S_IRUGO,	 show_attr_length_om1,		NULL);
static DEVICE_ATTR(length_om2,		S_IRUGO,	 show_attr_length_om2,		NULL);
static DEVICE_ATTR(length_om3,		S_IRUGO,	 show_attr_length_om3,		NULL);
static DEVICE_ATTR(length_om4,		S_IRUGO,	 show_attr_length_om4,		NULL);
static DEVICE_ATTR(length_sm,		S_IRUGO,	 show_attr_length_sm,		NULL);
static DEVICE_ATTR(length_smf,		S_IRUGO,	 show_attr_length_smf,		NULL);
static DEVICE_ATTR(plugged_in,		S_IRUGO,	 show_attr_plugged_in,		NULL);
static DEVICE_ATTR(rx_power,		S_IRUGO,	 show_attr_rx_power,		NULL);
static DEVICE_ATTR(rx_power_max,	S_IRUGO,	 show_attr_rx_power_max,	NULL);
static DEVICE_ATTR(rx_power_min,	S_IRUGO,	 show_attr_rx_power_min,	NULL);
static DEVICE_ATTR(rx_power_hi_warn,	S_IRUGO,	 show_attr_rx_power_hi_warn,	NULL);
static DEVICE_ATTR(rx_power_lo_warn,	S_IRUGO,	 show_attr_rx_power_lo_warn,	NULL);
static DEVICE_ATTR(rx_power_max_th,	S_IRUGO,	 show_attr_rx_power_max_th,	NULL);
static DEVICE_ATTR(rx_power_min_th,	S_IRUGO,	 show_attr_rx_power_min_th,	NULL);
static DEVICE_ATTR(rx_power_hi_warn_th,	S_IRUGO,	 show_attr_rx_power_hi_warn_th,	NULL);
static DEVICE_ATTR(rx_power_lo_warn_th,	S_IRUGO,	 show_attr_rx_power_lo_warn_th,	NULL);
static DEVICE_ATTR(supply_voltage,	S_IRUGO,	 show_attr_supply_voltage,	NULL);
static DEVICE_ATTR(temperature,		S_IRUGO,	 show_attr_temperature,		NULL);
static DEVICE_ATTR(transceiver_code,	S_IRUGO,	 show_attr_transceiver_code,	NULL);
static DEVICE_ATTR(tx_fault,		S_IRUGO,	 show_attr_tx_fault,		NULL);
static DEVICE_ATTR(tx_power,		S_IRUGO,	 show_attr_tx_power,		NULL);
static DEVICE_ATTR(tx_power_max,	S_IRUGO,	 show_attr_tx_power_max,	NULL);
static DEVICE_ATTR(tx_power_min,	S_IRUGO,	 show_attr_tx_power_min,	NULL);
static DEVICE_ATTR(tx_power_hi_warn,	S_IRUGO,	 show_attr_tx_power_hi_warn,	NULL);
static DEVICE_ATTR(tx_power_lo_warn,	S_IRUGO,	 show_attr_tx_power_lo_warn,	NULL);
static DEVICE_ATTR(tx_power_max_th,	S_IRUGO,	 show_attr_tx_power_max_th,	NULL);
static DEVICE_ATTR(tx_power_min_th,	S_IRUGO,	 show_attr_tx_power_min_th,	NULL);
static DEVICE_ATTR(tx_power_hi_warn_th,	S_IRUGO,	 show_attr_tx_power_hi_warn_th,	NULL);
static DEVICE_ATTR(tx_power_lo_warn_th,	S_IRUGO,	 show_attr_tx_power_lo_warn_th,	NULL);
static DEVICE_ATTR(vendor_name,		S_IRUGO,	 show_attr_vendor_name,		NULL);
static DEVICE_ATTR(vendor_pn,		S_IRUGO,	 show_attr_vendor_pn,		NULL);
static DEVICE_ATTR(vendor_sn,		S_IRUGO,	 show_attr_vendor_sn,		NULL);
static DEVICE_ATTR(wavelength,		S_IRUGO,	 show_attr_wavelength,		NULL);
#if 0
static DEVICE_ATTR(tx1_power,		S_IRUGO,	 show_attr_tx1_power,		NULL);
static DEVICE_ATTR(tx2_power,		S_IRUGO,	 show_attr_tx2_power,		NULL);
static DEVICE_ATTR(tx3_power,		S_IRUGO,	 show_attr_tx3_power,		NULL);
static DEVICE_ATTR(tx4_power,		S_IRUGO,	 show_attr_tx4_power,		NULL);
static DEVICE_ATTR(rx1_power,		S_IRUGO,	 show_attr_rx1_power,		NULL);
static DEVICE_ATTR(rx2_power,		S_IRUGO,	 show_attr_rx2_power,		NULL);
static DEVICE_ATTR(rx3_power,		S_IRUGO,	 show_attr_rx3_power,		NULL);
static DEVICE_ATTR(rx4_power,		S_IRUGO,	 show_attr_rx4_power,		NULL);
static DEVICE_ATTR(tx1_power_max_th,	S_IRUGO,	 show_attr_tx1_power_max_th,	NULL);
static DEVICE_ATTR(tx2_power_max_th,	S_IRUGO,	 show_attr_tx2_power_max_th,	NULL);
static DEVICE_ATTR(tx3_power_max_th,	S_IRUGO,	 show_attr_tx3_power_max_th,	NULL);
static DEVICE_ATTR(tx4_power_max_th,	S_IRUGO,	 show_attr_tx4_power_max_th,	NULL);
static DEVICE_ATTR(rx1_power_max_th,	S_IRUGO,	 show_attr_rx1_power_max_th,	NULL);
static DEVICE_ATTR(rx2_power_max_th,	S_IRUGO,	 show_attr_rx2_power_max_th,	NULL);
static DEVICE_ATTR(rx3_power_max_th,	S_IRUGO,	 show_attr_rx3_power_max_th,	NULL);
static DEVICE_ATTR(rx4_power_max_th,	S_IRUGO,	 show_attr_rx4_power_max_th,	NULL);
static DEVICE_ATTR(tx1_power_min_th,	S_IRUGO,	 show_attr_tx1_power_min_th,	NULL);
static DEVICE_ATTR(tx2_power_min_th,	S_IRUGO,	 show_attr_tx2_power_min_th,	NULL);
static DEVICE_ATTR(tx3_power_min_th,	S_IRUGO,	 show_attr_tx3_power_min_th,	NULL);
static DEVICE_ATTR(tx4_power_min_th,	S_IRUGO,	 show_attr_tx4_power_min_th,	NULL);
static DEVICE_ATTR(rx1_power_min_th,	S_IRUGO,	 show_attr_rx1_power_min_th,	NULL);
static DEVICE_ATTR(rx2_power_min_th,	S_IRUGO,	 show_attr_rx2_power_min_th,	NULL);
static DEVICE_ATTR(rx3_power_min_th,	S_IRUGO,	 show_attr_rx3_power_min_th,	NULL);
static DEVICE_ATTR(rx4_power_min_th,	S_IRUGO,	 show_attr_rx4_power_min_th,	NULL);
static DEVICE_ATTR(tx1_power_hi_warn_th,S_IRUGO,	 show_attr_tx1_power_hi_warn_th,NULL);
static DEVICE_ATTR(tx2_power_hi_warn_th,S_IRUGO,	 show_attr_tx2_power_hi_warn_th,NULL);
static DEVICE_ATTR(tx3_power_hi_warn_th,S_IRUGO,	 show_attr_tx3_power_hi_warn_th,NULL);
static DEVICE_ATTR(tx4_power_hi_warn_th,S_IRUGO,	 show_attr_tx4_power_hi_warn_th,NULL);
static DEVICE_ATTR(rx1_power_hi_warn_th,S_IRUGO,	 show_attr_rx1_power_hi_warn_th,NULL);
static DEVICE_ATTR(rx2_power_hi_warn_th,S_IRUGO,	 show_attr_rx2_power_hi_warn_th,NULL);
static DEVICE_ATTR(rx3_power_hi_warn_th,S_IRUGO,	 show_attr_rx3_power_hi_warn_th,NULL);
static DEVICE_ATTR(rx4_power_hi_warn_th,S_IRUGO,	 show_attr_rx4_power_hi_warn_th,NULL);
static DEVICE_ATTR(tx1_power_lo_warn_th,S_IRUGO,	 show_attr_tx1_power_lo_warn_th,NULL);
static DEVICE_ATTR(tx2_power_lo_warn_th,S_IRUGO,	 show_attr_tx2_power_lo_warn_th,NULL);
static DEVICE_ATTR(tx3_power_lo_warn_th,S_IRUGO,	 show_attr_tx3_power_lo_warn_th,NULL);
static DEVICE_ATTR(tx4_power_lo_warn_th,S_IRUGO,	 show_attr_tx4_power_lo_warn_th,NULL);
static DEVICE_ATTR(rx1_power_lo_warn_th,S_IRUGO,	 show_attr_rx1_power_lo_warn_th,NULL);
static DEVICE_ATTR(rx2_power_lo_warn_th,S_IRUGO,	 show_attr_rx2_power_lo_warn_th,NULL);
static DEVICE_ATTR(rx3_power_lo_warn_th,S_IRUGO,	 show_attr_rx3_power_lo_warn_th,NULL);
static DEVICE_ATTR(rx4_power_lo_warn_th,S_IRUGO,	 show_attr_rx4_power_lo_warn_th,NULL);
#endif
static DEVICE_ATTR(rxlos,		S_IRUGO,	show_attr_rxlos,		NULL);
static DEVICE_ATTR(comp_eth,		S_IRUGO,	show_attr_comp_eth,		NULL);
static DEVICE_ATTR(br,			S_IRUGO,	show_attr_br,			NULL);
static DEVICE_ATTR(br_max,		S_IRUGO,	show_attr_br_max,		NULL);
static DEVICE_ATTR(br_min,		S_IRUGO,	show_attr_br_min,		NULL);
static DEVICE_ATTR(date_code,		S_IRUGO,	show_attr_date_code,		NULL);
static DEVICE_ATTR(vendor_spec,		S_IRUGO,	show_attr_vendor_spec,		NULL);
static DEVICE_ATTR(status_reg,		S_IRUGO,	show_attr_status_reg,		NULL);
static DEVICE_ATTR(phy_ident1_reg,	S_IRUGO,	show_attr_phy_ident1_reg,	NULL);
static DEVICE_ATTR(phy_ident2_reg,	S_IRUGO,	show_attr_phy_ident2_reg,	NULL);
static DEVICE_ATTR(link_partner_ability_reg,	S_IRUGO,show_attr_link_partner_ability_reg,	NULL);
static DEVICE_ATTR(autoneg_expan_reg,	S_IRUGO,	show_attr_autoneg_expan_reg,	NULL);
static DEVICE_ATTR(link_partner_np_reg,	S_IRUGO,	show_attr_link_partner_np_reg,	NULL);
static DEVICE_ATTR(g_baset_status_reg,	S_IRUGO,	show_attr_g_baset_status_reg,	NULL);
static DEVICE_ATTR(ext_status_reg,	S_IRUGO,	show_attr_ext_status_reg,	NULL);
static DEVICE_ATTR(int_status_reg,S_IRUGO,	show_attr_int_status_reg,	NULL);
static DEVICE_ATTR(ext_phy_spec_ctrl_reg,	S_IRUGO,	show_attr_ext_phy_spec_ctrl_reg,	NULL);
static DEVICE_ATTR(rc_err_cnt_reg,	S_IRUGO,	show_attr_rc_err_cnt_reg,	NULL);
static DEVICE_ATTR(global_status_reg,	S_IRUGO,	show_attr_global_status_reg,	NULL);

static DEVICE_ATTR(tx_enable,		S_IRUGO|S_IWUSR, show_attr_tx_enable,		store_attr_tx_enable);
static DEVICE_ATTR(phy_speed,		S_IRUGO|S_IWUSR, show_attr_phy_speed,		store_attr_phy_speed);
#if 0
static DEVICE_ATTR(ctrl_reg,		S_IRUGO|S_IWUSR,show_attr_ctrl_reg,		store_attr_ctrl_reg);
static DEVICE_ATTR(auto_neg_adv_reg,	S_IRUGO|S_IWUSR,show_attr_auto_neg_adv_reg,	store_attr_auto_neg_adv_reg);
static DEVICE_ATTR(np_tx_reg,		S_IRUGO|S_IWUSR,show_attr_np_tx_reg,		store_attr_np_tx_reg);
static DEVICE_ATTR(g_baset_ctrl_reg,	S_IRUGO|S_IWUSR,show_attr_g_baset_ctrl_reg,	store_attr_g_baset_ctrl_reg);
static DEVICE_ATTR(phy_spec_ctrl_reg,	S_IRUGO|S_IWUSR,show_attr_phy_spec_ctrl_reg,	store_attr_phy_spec_ctrl_reg);
static DEVICE_ATTR(ext_addr_reg,	S_IRUGO|S_IWUSR,show_attr_ext_addr_reg,		store_attr_ext_addr_reg);
static DEVICE_ATTR(led_ctrl_reg,	S_IRUGO|S_IWUSR,show_attr_led_ctrl_reg,		store_attr_led_ctrl_reg);
static DEVICE_ATTR(man_led_reg,		S_IRUGO|S_IWUSR,show_attr_man_led_reg,		store_attr_man_led_reg);
static DEVICE_ATTR(ext_phy_spec_ctrl2_reg,	S_IRUGO|S_IWUSR,show_attr_ext_phy_spec_ctrl2_reg,	store_attr_ext_phy_spec_ctrl2_reg);
static DEVICE_ATTR(ext_phy_status_reg,	S_IRUGO|S_IWUSR,show_attr_ext_phy_status_reg,	store_attr_ext_phy_status_reg);
static DEVICE_ATTR(ext_addr,		S_IRUGO|S_IWUSR,show_attr_ext_addr,		store_attr_ext_addr);
static DEVICE_ATTR(ext_reg,		S_IRUGO|S_IWUSR,show_attr_ext_reg,		store_attr_ext_reg);
#else
static DEVICE_ATTR(ext_addr_reg,	S_IRUGO,	show_attr_ext_addr_reg,		NULL);
static DEVICE_ATTR(phy_spec_ctrl_reg,	S_IRUGO,	show_attr_phy_spec_ctrl_reg,	NULL);
static DEVICE_ATTR(g_baset_ctrl_reg,	S_IRUGO,	show_attr_g_baset_ctrl_reg,	NULL);
static DEVICE_ATTR(np_tx_reg,		S_IRUGO,	show_attr_np_tx_reg,		NULL);
static DEVICE_ATTR(auto_neg_adv_reg,	S_IRUGO,	show_attr_auto_neg_adv_reg,	NULL);
static DEVICE_ATTR(ctrl_reg,		S_IRUGO,	show_attr_ctrl_reg,		NULL);
static DEVICE_ATTR(led_ctrl_reg,	S_IRUGO,	show_attr_led_ctrl_reg,		NULL);
static DEVICE_ATTR(man_led_reg,		S_IRUGO,	show_attr_man_led_reg,		NULL);
static DEVICE_ATTR(ext_phy_spec_ctrl2_reg,	S_IRUGO,show_attr_ext_phy_spec_ctrl2_reg,	NULL);
static DEVICE_ATTR(ext_phy_status_reg,	S_IRUGO,	show_attr_ext_phy_status_reg,	NULL);
static DEVICE_ATTR(ext_addr,		S_IRUGO,	show_attr_ext_addr,		NULL);
static DEVICE_ATTR(ext_reg,		S_IRUGO,	show_attr_ext_reg,		NULL);
#endif

static int
port_sfp_register_attrs(struct device *device_p)
{
	char *err_msg = "ERR";

	if (device_create_file(device_p, &dev_attr_tx_bias) < 0) {
		err_msg = "reg_attr tx_bias fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_max) < 0) {
		err_msg = "reg_attr tx_bias_max fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_min) < 0) {
		err_msg = "reg_attr tx_bias_min fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_hi_warn) < 0) {
		err_msg = "reg_attr tx_bias_hi_warn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_low_warn) < 0) {
		err_msg = "reg_attr tx_bias_low_warn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_max_th) < 0) {
		err_msg = "reg_attr tx_bias_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_min_th) < 0) {
		err_msg = "reg_attr tx_bias_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_hi_warn_th) < 0) {
		err_msg = "reg_attr tx_bias_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias_low_warn_th) < 0) {
		err_msg = "reg_attr tx_bias_low_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_connector_type) < 0) {
		err_msg = "reg_attr connector_type fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_identifier) < 0) {
		err_msg = "reg_attr ext_identifier fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_identifier) < 0) {
		err_msg = "reg_attr identifier fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om1) < 0) {
		err_msg = "reg_attr length_om1 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om2) < 0) {
		err_msg = "reg_attr length_om2 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om3) < 0) {
		err_msg = "reg_attr length_om3 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om4) < 0) {
		err_msg = "reg_attr length_om4 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_sm) < 0) {
		err_msg = "reg_attr length_sm fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_smf) < 0) {
		err_msg = "reg_attr length_smf fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_plugged_in) < 0) {
		err_msg = "reg_attr plugged_in fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power) < 0) {
		err_msg = "reg_attr rx_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_max) < 0) {
		err_msg = "reg_attr rx_power_max fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_min) < 0) {
		err_msg = "reg_attr rx_power_min fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_hi_warn) < 0) {
		err_msg = "reg_attr rx_power_hi_warn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_lo_warn) < 0) {
		err_msg = "reg_attr rx_power_lo_warn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_max_th) < 0) {
		err_msg = "reg_attr rx_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_min_th) < 0) {
		err_msg = "reg_attr rx_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_hi_warn_th) < 0) {
		err_msg = "reg_attr rx_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx_power_lo_warn_th) < 0) {
		err_msg = "reg_attr rx_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_supply_voltage) < 0) {
		err_msg = "reg_attr supply_voltage fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_temperature) < 0) {
		err_msg = "reg_attr temperature fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_transceiver_code) < 0) {
		err_msg = "reg_attr transceiver_code fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_fault) < 0) {
		err_msg = "reg_attr tx_fault fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_enable) < 0) {
		err_msg = "reg_attr tx_enable fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_speed) < 0) {
		err_msg = "reg_attr phy_speed fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power) < 0) {
		err_msg = "reg_attr tx_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_max) < 0) {
		err_msg = "reg_attr tx_power_max fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_min) < 0) {
		err_msg = "reg_attr tx_power_min fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_hi_warn) < 0) {
		err_msg = "reg_attr tx_power_hi_warn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_lo_warn) < 0) {
		err_msg = "reg_attr tx_power_lo_warn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_max_th) < 0) {
		err_msg = "reg_attr tx_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_min_th) < 0) {
		err_msg = "reg_attr tx_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_hi_warn_th) < 0) {
		err_msg = "reg_attr tx_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_power_lo_warn_th) < 0) {
		err_msg = "reg_attr tx_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_name) < 0) {
		err_msg = "reg_attr vendor_name fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_pn) < 0) {
		err_msg = "reg_attr vendor_pn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_sn) < 0) {
		err_msg = "reg_attr vendor_sn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_wavelength) < 0) {
		err_msg = "reg_attr wavelength fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rxlos) < 0) {
		err_msg = "reg_attr rxlos fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_comp_eth) < 0) {
		err_msg = "reg_attr comp_eth fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_br) < 0) {
		err_msg = "reg_attr br fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_br_max) < 0) {
		err_msg = "reg_attr br_max fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_br_min) < 0) {
		err_msg = "reg_attr br_min fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_date_code) < 0) {
		err_msg = "reg_attr date_code fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_spec) < 0) {
		err_msg = "reg_attr vendor_spec fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ctrl_reg) < 0) {
		err_msg = "reg_attr ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_status_reg) < 0) {
		err_msg = "reg_attr status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_ident1_reg) < 0) {
		err_msg = "reg_attr phy_ident1_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_ident2_reg) < 0) {
		err_msg = "reg_attr phy_ident2_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_auto_neg_adv_reg) < 0) {
		err_msg = "reg_attr auto_neg_adv_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_link_partner_ability_reg) < 0) {
		err_msg = "reg_attr link_partner_ability_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_autoneg_expan_reg) < 0) {
		err_msg = "reg_attr autoneg_expan_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_np_tx_reg) < 0) {
		err_msg = "reg_attr np_tx_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_link_partner_np_reg) < 0) {
		err_msg = "reg_attr link_partner_np_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_g_baset_ctrl_reg) < 0) {
		err_msg = "reg_attr g_baset_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_g_baset_status_reg) < 0) {
		err_msg = "reg_attr g_baset_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_status_reg) < 0) {
		err_msg = "reg_attr ext_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_spec_ctrl_reg) < 0) {
		err_msg = "reg_attr phy_spec_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_int_status_reg) < 0) {
		err_msg = "reg_attr int_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_phy_spec_ctrl_reg) < 0) {
		err_msg = "reg_attr ext_phy_spec_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rc_err_cnt_reg) < 0) {
		err_msg = "reg_attr rc_err_cnt_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_addr_reg) < 0) {
		err_msg = "reg_attr ext_addr_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_global_status_reg) < 0) {
		err_msg = "reg_attr global_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_led_ctrl_reg) < 0) {
		err_msg = "reg_attr led_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_man_led_reg) < 0) {
		err_msg = "reg_attr man_led_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_phy_spec_ctrl2_reg) < 0) {
		err_msg = "reg_attr ext_phy_spec_ctrl2_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_phy_status_reg) < 0) {
		err_msg = "reg_attr ext_phy_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_addr) < 0) {
		err_msg = "reg_attr ext_addr fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_reg) < 0) {
		err_msg = "reg_attr ext_reg fail";
		goto err_port_register_attrs;
	}
	return 0;

err_port_register_attrs:
	printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
	return -1;
}

static int
port_qsfp_register_attrs(struct device *device_p)
{
	char *err_msg = "ERR";

	if (device_create_file(device_p, &dev_attr_plugged_in) < 0) {
		err_msg = "reg_attr plugged_in fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_connector_type) < 0) {
		err_msg = "reg_attr connector_type fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_identifier) < 0) {
		err_msg = "reg_attr ext_identifier fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_identifier) < 0) {
		err_msg = "reg_attr identifier fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om1) < 0) {
		err_msg = "reg_attr length_om1 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om2) < 0) {
		err_msg = "reg_attr length_om2 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om3) < 0) {
		err_msg = "reg_attr length_om3 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_om4) < 0) {
		err_msg = "reg_attr length_om4 fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_length_smf) < 0) {
		err_msg = "reg_attr length_smf fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_supply_voltage) < 0) {
		err_msg = "reg_attr supply_voltage fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_temperature) < 0) {
		err_msg = "reg_attr temperature fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_transceiver_code) < 0) {
		err_msg = "reg_attr transceiver_code fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_name) < 0) {
		err_msg = "reg_attr vendor_name fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_pn) < 0) {
		err_msg = "reg_attr vendor_pn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_sn) < 0) {
		err_msg = "reg_attr vendor_sn fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_wavelength) < 0) {
		err_msg = "reg_attr wavelength fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_comp_eth) < 0) {
		err_msg = "reg_attr comp_eth fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_br) < 0) {
		err_msg = "reg_attr br fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_enable) < 0) {
		err_msg = "reg_attr tx_enable fail";
		goto err_port_register_attrs;
	}
#if 0
	if (device_create_file(device_p, &dev_attr_rx1_power_max_th) < 0) {
		err_msg = "reg_attr rx1_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx2_power_max_th) < 0) {
		err_msg = "reg_attr rx2_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx3_power_max_th) < 0) {
		err_msg = "reg_attr rx3_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx4_power_max_th) < 0) {
		err_msg = "reg_attr rx4_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx1_power_min_th) < 0) {
		err_msg = "reg_attr rx1_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx2_power_min_th) < 0) {
		err_msg = "reg_attr rx2_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx3_power_min_th) < 0) {
		err_msg = "reg_attr rx3_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx4_power_min_th) < 0) {
		err_msg = "reg_attr rx4_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx1_power_hi_warn_th) < 0) {
		err_msg = "reg_attr rx1_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx2_power_hi_warn_th) < 0) {
		err_msg = "reg_attr rx2_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx3_power_hi_warn_th) < 0) {
		err_msg = "reg_attr rx3_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx4_power_hi_warn_th) < 0) {
		err_msg = "reg_attr rx4_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx1_power_lo_warn_th) < 0) {
		err_msg = "reg_attr rx1_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx2_power_lo_warn_th) < 0) {
		err_msg = "reg_attr rx2_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx3_power_lo_warn_th) < 0) {
		err_msg = "reg_attr rx3_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx4_power_lo_warn_th) < 0) {
		err_msg = "reg_attr rx4_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rxlos) < 0) {
		err_msg = "reg_attr rxlos fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_status_reg) < 0) {
		err_msg = "reg_attr status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_ident1_reg) < 0) {
		err_msg = "reg_attr phy_ident1_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_ident2_reg) < 0) {
		err_msg = "reg_attr phy_ident2_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_auto_neg_adv_reg) < 0) {
		err_msg = "reg_attr auto_neg_adv_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_link_partner_ability_reg) < 0) {
		err_msg = "reg_attr link_partner_ability_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_autoneg_expan_reg) < 0) {
		err_msg = "reg_attr autoneg_expan_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_np_tx_reg) < 0) {
		err_msg = "reg_attr np_tx_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_link_partner_np_reg) < 0) {
		err_msg = "reg_attr link_partner_np_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_g_baset_status_reg) < 0) {
		err_msg = "reg_attr g_baset_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_status_reg) < 0) {
		err_msg = "reg_attr ext_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_phy_spec_ctrl_reg) < 0) {
		err_msg = "reg_attr phy_spec_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_int_status_reg) < 0) {
		err_msg = "reg_attr int_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_phy_spec_ctrl_reg) < 0) {
		err_msg = "reg_attr ext_phy_spec_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rc_err_cnt_reg) < 0) {
		err_msg = "reg_attr rc_err_cnt_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_global_status_reg) < 0) {
		err_msg = "reg_attr global_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_led_ctrl_reg) < 0) {
		err_msg = "reg_attr led_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_man_led_reg) < 0) {
		err_msg = "reg_attr man_led_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_phy_spec_ctrl2_reg) < 0) {
		err_msg = "reg_attr ext_phy_spec_ctrl2_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_phy_status_reg) < 0) {
		err_msg = "reg_attr ext_phy_status_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx_bias) < 0) {
		err_msg = "reg_attr tx_bias fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx1_power) < 0) {
		err_msg = "reg_attr rx1_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx2_power) < 0) {
		err_msg = "reg_attr rx2_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx3_power) < 0) {
		err_msg = "reg_attr rx3_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_rx4_power) < 0) {
		err_msg = "reg_attr rx4_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx1_power) < 0) {
		err_msg = "reg_attr tx1_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx2_power) < 0) {
		err_msg = "reg_attr tx2_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx3_power) < 0) {
		err_msg = "reg_attr tx3_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx4_power) < 0) {
		err_msg = "reg_attr tx4_power fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx1_power_max_th) < 0) {
		err_msg = "reg_attr tx1_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx2_power_max_th) < 0) {
		err_msg = "reg_attr tx2_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx3_power_max_th) < 0) {
		err_msg = "reg_attr tx3_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx4_power_max_th) < 0) {
		err_msg = "reg_attr tx4_power_max_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx1_power_min_th) < 0) {
		err_msg = "reg_attr tx1_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx2_power_min_th) < 0) {
		err_msg = "reg_attr tx2_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx3_power_min_th) < 0) {
		err_msg = "reg_attr tx3_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx4_power_min_th) < 0) {
		err_msg = "reg_attr tx4_power_min_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx1_power_hi_warn_th) < 0) {
		err_msg = "reg_attr tx1_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx2_power_hi_warn_th) < 0) {
		err_msg = "reg_attr tx2_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx3_power_hi_warn_th) < 0) {
		err_msg = "reg_attr tx3_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx4_power_hi_warn_th) < 0) {
		err_msg = "reg_attr tx4_power_hi_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx1_power_lo_warn_th) < 0) {
		err_msg = "reg_attr tx1_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx2_power_lo_warn_th) < 0) {
		err_msg = "reg_attr tx2_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx3_power_lo_warn_th) < 0) {
		err_msg = "reg_attr tx3_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_tx4_power_lo_warn_th) < 0) {
		err_msg = "reg_attr tx4_power_lo_warn_th fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_br_max) < 0) {
		err_msg = "reg_attr br_max fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_br_min) < 0) {
		err_msg = "reg_attr br_min fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_date_code) < 0) {
		err_msg = "reg_attr date_code fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_vendor_spec) < 0) {
		err_msg = "reg_attr vendor_spec fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ctrl_reg) < 0) {
		err_msg = "reg_attr ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_g_baset_ctrl_reg) < 0) {
		err_msg = "reg_attr g_baset_ctrl_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_addr_reg) < 0) {
		err_msg = "reg_attr ext_addr_reg fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_addr) < 0) {
		err_msg = "reg_attr ext_addr fail";
		goto err_port_register_attrs;
	}
	if (device_create_file(device_p, &dev_attr_ext_reg) < 0) {
		err_msg = "reg_attr ext_reg fail";
		goto err_port_register_attrs;
	}
#endif
	return 0;

err_port_register_attrs:
	printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
	return -1;
}

static void
port_sfp_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_tx_bias);
	device_remove_file(device_p, &dev_attr_tx_bias_max);
	device_remove_file(device_p, &dev_attr_tx_bias_min);
	device_remove_file(device_p, &dev_attr_tx_bias_hi_warn);
	device_remove_file(device_p, &dev_attr_tx_bias_low_warn);
	device_remove_file(device_p, &dev_attr_tx_bias_max_th);
	device_remove_file(device_p, &dev_attr_tx_bias_min_th);
	device_remove_file(device_p, &dev_attr_tx_bias_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx_bias_low_warn_th);
	device_remove_file(device_p, &dev_attr_connector_type);
	device_remove_file(device_p, &dev_attr_ext_identifier);
	device_remove_file(device_p, &dev_attr_identifier);
	device_remove_file(device_p, &dev_attr_length_om1);
	device_remove_file(device_p, &dev_attr_length_om2);
	device_remove_file(device_p, &dev_attr_length_om3);
	device_remove_file(device_p, &dev_attr_length_om4);
	device_remove_file(device_p, &dev_attr_length_sm);
	device_remove_file(device_p, &dev_attr_length_smf);
	device_remove_file(device_p, &dev_attr_plugged_in);
	device_remove_file(device_p, &dev_attr_rx_power);
	device_remove_file(device_p, &dev_attr_rx_power_max);
	device_remove_file(device_p, &dev_attr_rx_power_min);
	device_remove_file(device_p, &dev_attr_rx_power_hi_warn);
	device_remove_file(device_p, &dev_attr_rx_power_lo_warn);
	device_remove_file(device_p, &dev_attr_rx_power_max_th);
	device_remove_file(device_p, &dev_attr_rx_power_min_th);
	device_remove_file(device_p, &dev_attr_rx_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_rx_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_supply_voltage);
	device_remove_file(device_p, &dev_attr_temperature);
	device_remove_file(device_p, &dev_attr_transceiver_code);
	device_remove_file(device_p, &dev_attr_tx_fault);
	device_remove_file(device_p, &dev_attr_tx_enable);
	device_remove_file(device_p, &dev_attr_phy_speed);
	device_remove_file(device_p, &dev_attr_tx_power);
	device_remove_file(device_p, &dev_attr_tx_power_max);
	device_remove_file(device_p, &dev_attr_tx_power_min);
	device_remove_file(device_p, &dev_attr_tx_power_hi_warn);
	device_remove_file(device_p, &dev_attr_tx_power_lo_warn);
	device_remove_file(device_p, &dev_attr_tx_power_max_th);
	device_remove_file(device_p, &dev_attr_tx_power_min_th);
	device_remove_file(device_p, &dev_attr_tx_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_vendor_name);
	device_remove_file(device_p, &dev_attr_vendor_pn);
	device_remove_file(device_p, &dev_attr_vendor_sn);
	device_remove_file(device_p, &dev_attr_wavelength);
	device_remove_file(device_p, &dev_attr_rxlos);
	device_remove_file(device_p, &dev_attr_comp_eth);
	device_remove_file(device_p, &dev_attr_br);
	device_remove_file(device_p, &dev_attr_br_max);
	device_remove_file(device_p, &dev_attr_br_min);
	device_remove_file(device_p, &dev_attr_date_code);
	device_remove_file(device_p, &dev_attr_vendor_spec);
	device_remove_file(device_p, &dev_attr_ctrl_reg);
	device_remove_file(device_p, &dev_attr_status_reg);
	device_remove_file(device_p, &dev_attr_phy_ident1_reg);
	device_remove_file(device_p, &dev_attr_phy_ident2_reg);
	device_remove_file(device_p, &dev_attr_auto_neg_adv_reg);
	device_remove_file(device_p, &dev_attr_link_partner_ability_reg);
	device_remove_file(device_p, &dev_attr_autoneg_expan_reg);
	device_remove_file(device_p, &dev_attr_np_tx_reg);
	device_remove_file(device_p, &dev_attr_link_partner_np_reg);
	device_remove_file(device_p, &dev_attr_g_baset_ctrl_reg);
	device_remove_file(device_p, &dev_attr_g_baset_status_reg);
	device_remove_file(device_p, &dev_attr_ext_status_reg);
	device_remove_file(device_p, &dev_attr_phy_spec_ctrl_reg);
	device_remove_file(device_p, &dev_attr_int_status_reg);
	device_remove_file(device_p, &dev_attr_ext_phy_spec_ctrl_reg);
	device_remove_file(device_p, &dev_attr_rc_err_cnt_reg);
	device_remove_file(device_p, &dev_attr_ext_addr_reg);
	device_remove_file(device_p, &dev_attr_global_status_reg);
	device_remove_file(device_p, &dev_attr_led_ctrl_reg);
	device_remove_file(device_p, &dev_attr_man_led_reg);
	device_remove_file(device_p, &dev_attr_ext_phy_spec_ctrl2_reg);
	device_remove_file(device_p, &dev_attr_ext_phy_status_reg);
	device_remove_file(device_p, &dev_attr_ext_addr);
	device_remove_file(device_p, &dev_attr_ext_reg);
}

static void
port_qsfp_unregister_attrs(struct device *device_p)
{
	device_remove_file(device_p, &dev_attr_plugged_in);
	device_remove_file(device_p, &dev_attr_connector_type);
	device_remove_file(device_p, &dev_attr_ext_identifier);
	device_remove_file(device_p, &dev_attr_identifier);
	device_remove_file(device_p, &dev_attr_length_om1);
	device_remove_file(device_p, &dev_attr_length_om2);
	device_remove_file(device_p, &dev_attr_length_om3);
	device_remove_file(device_p, &dev_attr_length_om4);
	device_remove_file(device_p, &dev_attr_length_smf);
	device_remove_file(device_p, &dev_attr_supply_voltage);
	device_remove_file(device_p, &dev_attr_temperature);
	device_remove_file(device_p, &dev_attr_transceiver_code);
	device_remove_file(device_p, &dev_attr_vendor_name);
	device_remove_file(device_p, &dev_attr_vendor_pn);
	device_remove_file(device_p, &dev_attr_vendor_sn);
	device_remove_file(device_p, &dev_attr_wavelength);
	device_remove_file(device_p, &dev_attr_comp_eth);
	device_remove_file(device_p, &dev_attr_br);
	device_remove_file(device_p, &dev_attr_tx_enable);
#if 0
	device_remove_file(device_p, &dev_attr_rx1_power_max_th);
	device_remove_file(device_p, &dev_attr_rx2_power_max_th);
	device_remove_file(device_p, &dev_attr_rx3_power_max_th);
	device_remove_file(device_p, &dev_attr_rx4_power_max_th);
	device_remove_file(device_p, &dev_attr_rx1_power_min_th);
	device_remove_file(device_p, &dev_attr_rx2_power_min_th);
	device_remove_file(device_p, &dev_attr_rx3_power_min_th);
	device_remove_file(device_p, &dev_attr_rx4_power_min_th);
	device_remove_file(device_p, &dev_attr_rx1_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_rx2_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_rx3_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_rx4_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_rxlos);
	device_remove_file(device_p, &dev_attr_status_reg);
	device_remove_file(device_p, &dev_attr_phy_ident1_reg);
	device_remove_file(device_p, &dev_attr_phy_ident2_reg);
	device_remove_file(device_p, &dev_attr_auto_neg_adv_reg);
	device_remove_file(device_p, &dev_attr_link_partner_ability_reg);
	device_remove_file(device_p, &dev_attr_autoneg_expan_reg);
	device_remove_file(device_p, &dev_attr_np_tx_reg);
	device_remove_file(device_p, &dev_attr_link_partner_np_reg);
	device_remove_file(device_p, &dev_attr_g_baset_status_reg);
	device_remove_file(device_p, &dev_attr_ext_status_reg);
	device_remove_file(device_p, &dev_attr_phy_spec_ctrl_reg);
	device_remove_file(device_p, &dev_attr_int_status_reg);
	device_remove_file(device_p, &dev_attr_ext_phy_spec_ctrl_reg);
	device_remove_file(device_p, &dev_attr_rc_err_cnt_reg);
	device_remove_file(device_p, &dev_attr_global_status_reg);
	device_remove_file(device_p, &dev_attr_led_ctrl_reg);
	device_remove_file(device_p, &dev_attr_man_led_reg);
	device_remove_file(device_p, &dev_attr_ext_phy_spec_ctrl2_reg);
	device_remove_file(device_p, &dev_attr_ext_phy_status_reg);
	device_remove_file(device_p, &dev_attr_rx1_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_rx2_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_rx3_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_rx4_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx_bias);
	device_remove_file(device_p, &dev_attr_tx1_power);
	device_remove_file(device_p, &dev_attr_tx2_power);
	device_remove_file(device_p, &dev_attr_tx3_power);
	device_remove_file(device_p, &dev_attr_tx4_power);
	device_remove_file(device_p, &dev_attr_rx1_power);
	device_remove_file(device_p, &dev_attr_rx2_power);
	device_remove_file(device_p, &dev_attr_rx3_power);
	device_remove_file(device_p, &dev_attr_rx4_power);
	device_remove_file(device_p, &dev_attr_tx1_power_max_th);
	device_remove_file(device_p, &dev_attr_tx2_power_max_th);
	device_remove_file(device_p, &dev_attr_tx3_power_max_th);
	device_remove_file(device_p, &dev_attr_tx4_power_max_th);
	device_remove_file(device_p, &dev_attr_tx1_power_min_th);
	device_remove_file(device_p, &dev_attr_tx2_power_min_th);
	device_remove_file(device_p, &dev_attr_tx3_power_min_th);
	device_remove_file(device_p, &dev_attr_tx4_power_min_th);
	device_remove_file(device_p, &dev_attr_tx1_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx2_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx3_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx4_power_hi_warn_th);
	device_remove_file(device_p, &dev_attr_tx1_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_tx2_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_tx3_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_tx4_power_lo_warn_th);
	device_remove_file(device_p, &dev_attr_br_max);
	device_remove_file(device_p, &dev_attr_br_min);
	device_remove_file(device_p, &dev_attr_date_code);
	device_remove_file(device_p, &dev_attr_vendor_spec);
	device_remove_file(device_p, &dev_attr_ctrl_reg);
	device_remove_file(device_p, &dev_attr_g_baset_ctrl_reg);
	device_remove_file(device_p, &dev_attr_ext_addr_reg);
	device_remove_file(device_p, &dev_attr_ext_addr);
	device_remove_file(device_p, &dev_attr_ext_reg);
#endif
}

static int
port_register_device(int port_id)
{
    char *err_msg	= "ERR";
    int path_len	= 64;
    void		*curr_tracking;
    port_dev_group_t *group = &port_dev_group[port_id];

    /* Prepare INV driver access path */
    group->port_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->port_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_port_register_device_1;
    }
    curr_tracking = group->port_tracking;
    /* Creatr Sysfs device object */
    group->port_dev_p = inventec_child_device_create(ports_device_p,
                                       group->port_devt,
                                       group->port_inv_pathp,
                                       group->port_name,
                                       curr_tracking);

    if (IS_ERR(group->port_dev_p)){
        err_msg = "device_create fail";
        goto err_port_register_device_2;
    }

    if (port_id < PORT_SFP_DEV_TOTAL) {
        if (port_sfp_register_attrs(group->port_dev_p) < 0){
            goto err_port_register_device_3;
        }
    }
    else {
        if (port_qsfp_register_attrs(group->port_dev_p) < 0){
            goto err_port_register_device_3;
        }
    }
    return 0;

err_port_register_device_3:
    if (port_id < PORT_SFP_DEV_TOTAL) {
	port_sfp_unregister_attrs(group->port_dev_p);
    }
    else {
	port_qsfp_unregister_attrs(group->port_dev_p);
    }
    inventec_child_device_destroy(group->port_tracking);
err_port_register_device_2:
    kfree(group->port_inv_pathp);
err_port_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
port_unregister_device(int port_id)
{
    port_dev_group_t *group = &port_dev_group[port_id];

    if (port_id < PORT_SFP_DEV_TOTAL) {
        port_sfp_unregister_attrs(group->port_dev_p);
    }
    else {
        port_qsfp_unregister_attrs(group->port_dev_p);
    }
    inventec_child_device_destroy(group->port_tracking);
    kfree(group->port_inv_pathp);
}

static int
ports_device_create(void)
{
    char *err_msg  = "ERR";
    port_dev_group_t *group;
    int port_id;

    if (alloc_chrdev_region(&ports_device_devt, 0, 1, PORTS_DEVICE_NAME) < 0) {
        err_msg = "alloc_chrdev_region fail!";
        goto err_ports_device_create_1;
    }

    /* Creatr Sysfs device object */
    ports_device_p = device_create(inventec_class_p,	/* struct class *cls     */
                             NULL,			/* struct device *parent */
			     ports_device_devt,		/* dev_t devt */
                             NULL,			/* void *private_data    */
                             PORTS_DEVICE_NAME);		/* const char *fmt       */
    if (IS_ERR(ports_device_p)){
        err_msg = "device_create fail";
        goto err_ports_device_create_2;
    }

    /* portX interface tree create */
    for (port_id = 0; port_id < PORT_DEV_TOTAL; port_id++) {
        group = &port_dev_group[port_id];

	if (alloc_chrdev_region(&group->port_devt, 0, group->port_dev_total, group->port_name) < 0) {
            err_msg = "portX alloc_chrdev_region fail!";
            goto err_ports_device_create_4;
        }
	group->port_major = MAJOR(group->port_devt);
    }

    for (port_id = 0; port_id < PORT_DEV_TOTAL; port_id++) {
        group = &port_dev_group[port_id];

        if (port_register_device(port_id) < 0){
            err_msg = "register_test_device fail";
            goto err_ports_device_create_5;
        }
    }
    return 0;

err_ports_device_create_5:
    while (--port_id >= 0) {
        port_unregister_device(port_id);
    }
    port_id = PORT_DEV_TOTAL;
err_ports_device_create_4:
    while (--port_id >= 0) {
        group = &port_dev_group[port_id];
        unregister_chrdev_region(group->port_devt, group->port_dev_total);
    }
    device_destroy(inventec_class_p, ports_device_devt);
err_ports_device_create_2:
    unregister_chrdev_region(ports_device_devt, 1);
err_ports_device_create_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
ports_device_destroy(void)
{
    port_dev_group_t *group;
    int port_id;

    for (port_id = 0; port_id < PORT_DEV_TOTAL; port_id++) {
        port_unregister_device(port_id);
    }
    for (port_id = 0; port_id < PORT_DEV_TOTAL; port_id++) {
        group = &port_dev_group[port_id];
        unregister_chrdev_region(group->port_devt, group->port_dev_total);
    }
    device_destroy(inventec_class_p, ports_device_devt);
    unregister_chrdev_region(ports_device_devt, 1);
}

/*
 * Start of portsinfo_device Jul 5, 2016
 *
 * Inventec Platform Proxy Driver for platform portsinfo devices.
 */
typedef struct {
	char *proxy_dev_name;
	char *inv_dev_name;
} portsinfo_dev_t;

typedef struct {
	const char	*portsinfo_name;
	int		portsinfo_major;
        dev_t		portsinfo_devt;
	struct device	*portsinfo_dev_p;
	void		*portsinfo_tracking;
	portsinfo_dev_t	*portsinfo_dev_namep;
	int		portsinfo_dev_total;
	char		*portsinfo_inv_pathp;
} portsinfo_dev_group_t;

static portsinfo_dev_t portsinfo_dev_name[] = {
	{ "ports_number",	NULL },
	{ "ports_map",		"present" },
};

static portsinfo_dev_group_t portsinfo_dev_group[] = {
	{
		.portsinfo_name		= PORTSINFO_DEVICE_NAME,
		.portsinfo_major	= 0,
		.portsinfo_devt		= 0L,
		.portsinfo_dev_p	= NULL,
		.portsinfo_tracking	= NULL,
		.portsinfo_dev_namep	= &portsinfo_dev_name[0],
		.portsinfo_dev_total	= sizeof(portsinfo_dev_name) / sizeof(const portsinfo_dev_t),
		.portsinfo_inv_pathp	= NULL,
	},
};
#define PORTINFO_DEV_TOTAL	( sizeof(portsinfo_dev_group)/ sizeof(const portsinfo_dev_group_t) )

static ssize_t
show_attr_ports_number(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    return sprintf(buf_p, "%lu\n", PORT_DEV_TOTAL);
}

/*
 * For ports_map output format
 */
#if defined PLATFORM_MAGNOLIA
/* Do not EDIT the next 2 lines */
#define PORTS_MAP_LINE1   "1     15  17    31  33    47 49 53"
#define PORTS_MAP_LINE4   "2     16  18    32  34    48 50 54"

#define PORT_GAP_SPACES	8
#elif defined PLATFORM_REDWOOD
/* Do not EDIT the next 2 lines */
#define PORTS_MAP_LINE1   "1   5   9   13  17  21  25   31"
#define PORTS_MAP_LINE4   "2   6   10  14  18  22  26   32"

#define PORT_GAP_SPACES	2
#elif defined PLATFORM_CYPRESS
TBD
#endif

static char portmap[PORTS_NUMBER_TOTAL+1];

static void ports_map_init(void)
{
    char pathbuf[64], tmpbuf[8];
    int port_id;

    for (port_id = PORT_SFP_DEV_START; port_id < PORT_SFP_DEV_TOTAL; port_id++) {
        portmap[port_id] = 'S';
    }
    for (port_id = PORT_QSFP_DEV_START; port_id < PORT_QSFP_DEV_TOTAL; port_id++) {
        portmap[port_id] = 'Q';
    }

    for (port_id = 0; port_id < PORTS_NUMBER_TOTAL; port_id++) {
        sprintf(&pathbuf[0], SWPS_ACC_PATH_BASE, port_id, "present");
        inventec_show_attr(&tmpbuf[0], &pathbuf[0]);
        if (tmpbuf[0] == '1') {
            portmap[port_id] = '.';
        }
    }
}

static ssize_t
show_attr_ports_map(struct device *dev_p, struct device_attribute *attr_p, char *buf_p)
{
    int port_id, map_id, space_id;
    char mapbuf1[64], mapbuf2[64];

    ports_map_init();

    for (port_id = 0, map_id = 0, space_id = 0; port_id < PORTS_NUMBER_TOTAL; port_id+=2, map_id++, space_id++) {
        if (space_id == PORT_GAP_SPACES) {
            mapbuf1[map_id++] = ' ';
            mapbuf1[map_id++] = ' ';
            space_id = 0;
        }
        mapbuf1[map_id] = portmap[port_id];
    }
    mapbuf1[map_id++] = '\0';
    for (port_id = 1, map_id = 0, space_id = 0; port_id < PORTS_NUMBER_TOTAL; port_id+=2, map_id++, space_id++) {
        if (space_id == PORT_GAP_SPACES) {
            mapbuf2[map_id++] = ' ';
            mapbuf2[map_id++] = ' ';
            space_id = 0;
        }
        mapbuf2[map_id] = portmap[port_id];
    }
    mapbuf2[map_id++] = '\0';

    return sprintf(buf_p, "%s\n%s\n%s\n%s\n", PORTS_MAP_LINE1, mapbuf1, mapbuf2, PORTS_MAP_LINE4);
}

static DEVICE_ATTR(ports_number,	S_IRUGO,	show_attr_ports_number,	NULL);
static DEVICE_ATTR(ports_map,		S_IRUGO,	show_attr_ports_map,	NULL);

static int
portsinfo_register_attrs(struct device *device_p)
{
    if (device_create_file(device_p, &dev_attr_ports_number) < 0) {
        goto err_portsinfo_register_attrs;
    }
    if (device_create_file(device_p, &dev_attr_ports_map) < 0) {
        goto err_portsinfo_register_attrs;
    }
    return 0;

err_portsinfo_register_attrs:
    printk(KERN_ERR "[%s/%d] device_create_file() fail\n",__func__,__LINE__);
    return -1;
}

static void
portsinfo_unregister_attrs(struct device *device_p)
{
    device_remove_file(device_p, &dev_attr_ports_number);
    device_remove_file(device_p, &dev_attr_ports_map);
}

static int
portsinfo_register_device(portsinfo_dev_group_t *group)
{
    int path_len	= 64;
    char *err_msg	= "ERR";

    /* Prepare INV driver access path */
    group->portsinfo_inv_pathp = kzalloc((path_len * sizeof(char)), GFP_KERNEL);
    if (!group->portsinfo_inv_pathp) {
        err_msg = "kernel memory allocate fail";
        goto err_portsinfo_register_device_1;
    }
    /* Get Sysfs device object */
    group->portsinfo_dev_p = ports_device_p;
    if (!group->portsinfo_dev_p) {
        err_msg = "ports_get_ports_devp() fail";
        goto err_portsinfo_register_device_2;
    }
    if (portsinfo_register_attrs(group->portsinfo_dev_p) < 0){
        goto err_portsinfo_register_device_3;
    }
    return 0;

err_portsinfo_register_device_3:
    device_destroy(inventec_class_p, group->portsinfo_devt);
err_portsinfo_register_device_2:
    kfree(group->portsinfo_inv_pathp);
err_portsinfo_register_device_1:
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void
portsinfo_unregister_device(portsinfo_dev_group_t *group)
{
    portsinfo_unregister_attrs(group->portsinfo_dev_p);
    device_destroy(inventec_class_p, group->portsinfo_devt);
    kfree(group->portsinfo_inv_pathp);
}

static int
portsinfo_device_create(void)
{
    char *err_msg  = "ERR";
    portsinfo_dev_group_t *group;
    int portsinfo_id;

    for (portsinfo_id = 0; portsinfo_id < PORTINFO_DEV_TOTAL; portsinfo_id++) {
        group = &portsinfo_dev_group[portsinfo_id];

        if (alloc_chrdev_region(&group->portsinfo_devt, 0, group->portsinfo_dev_total, group->portsinfo_name) < 0) {
            err_msg = "alloc_chrdev_region fail!";
            goto err_portsinfo_device_init_1;
        }
        group->portsinfo_major = MAJOR(group->portsinfo_devt);
    }

    for (portsinfo_id = 0; portsinfo_id < PORTINFO_DEV_TOTAL; portsinfo_id++) {
        group = &portsinfo_dev_group[portsinfo_id];

        /* Register device number */
        if (portsinfo_register_device(group) < 0){
            err_msg = "portsinfo_register_device fail";
            goto err_portsinfo_device_init_2;
        }
    }
    return 0;

err_portsinfo_device_init_2:
    while (--portsinfo_id >= 0) {
        group = &portsinfo_dev_group[portsinfo_id];
        portsinfo_unregister_device(group);
    }
    portsinfo_id = PORTINFO_DEV_TOTAL;
err_portsinfo_device_init_1:
    while (--portsinfo_id >= 0) {
        group = &portsinfo_dev_group[portsinfo_id];
        unregister_chrdev_region(group->portsinfo_devt, group->portsinfo_dev_total);
    }
    printk(KERN_ERR "[%s/%d] %s\n",__func__,__LINE__, err_msg);
    return -1;
}

static void portsinfo_device_destroy(void)
{
    int portsinfo_id;
    portsinfo_dev_group_t *group;

    for (portsinfo_id = 0; portsinfo_id < PORTINFO_DEV_TOTAL; portsinfo_id++) {
        group = &portsinfo_dev_group[portsinfo_id];
        portsinfo_unregister_device(group);
    }
    for (portsinfo_id = 0; portsinfo_id < PORTINFO_DEV_TOTAL; portsinfo_id++) {
        group = &portsinfo_dev_group[portsinfo_id];
        unregister_chrdev_region(group->portsinfo_devt, group->portsinfo_dev_total);
    }
}

static int
portsinfo_device_init(void)
{
    char *err_msg  = "ERR";

    if (portsinfo_device_create() < 0){
        err_msg = "portsinfo_device_create() fail!";
        goto err_portsinfo_device_init;
    }
    printk(KERN_INFO "Module initial success.\n");
    return 0;

err_portsinfo_device_init:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void
portsinfo_device_exit(void)
{
    portsinfo_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}
/* End of portsinfo_device */

static int __init
ports_device_init(void)
{
    char *err_msg  = "ERR";
    int port_id;

    inventec_class_p = inventec_get_class();
    if (!inventec_class_p) {
        err_msg = "inventec_get_class() fail!";
        goto err_ports_device_init_1;
    }

    for (port_id = 0; port_id < PORT_DEV_TOTAL; port_id++) {
        port_dev_group[port_id].port_major = 0;
        port_dev_group[port_id].port_devt = 0L;
        port_dev_group[port_id].port_dev_p = NULL;
        port_dev_group[port_id].port_inv_pathp = NULL;
        port_dev_group[port_id].port_tracking = NULL;
    }
#if defined PLATFORM_MAGNOLIA
    for (port_id = PORT_SFP_DEV_START; port_id < PORT_SFP_DEV_TOTAL; port_id++) {
	port_dev_group[port_id].port_dev_namep = &port_sfp_dev_name[0];
	port_dev_group[port_id].port_dev_total = sizeof(port_sfp_dev_name) / sizeof(const port_dev_t);
    }

    for (port_id = PORT_QSFP_DEV_START; port_id < PORT_QSFP_DEV_TOTAL; port_id++) {
	port_dev_group[port_id].port_dev_namep = &port_qsfp_dev_name[0];
	port_dev_group[port_id].port_dev_total = sizeof(port_qsfp_dev_name) / sizeof(const port_dev_t);
    }
#elif defined PLATFORM_REDWOOD
    for (port_id = PORT_QSFP_DEV_START; port_id < PORT_QSFP_DEV_TOTAL; port_id++) {
	port_dev_group[port_id].port_dev_namep = &port_qsfp_dev_name[0];
	port_dev_group[port_id].port_dev_total = sizeof(port_qsfp_dev_name) / sizeof(const port_dev_t);
    }
#elif defined PLATFORM_CYPRESS
TBD
#endif

    if (ports_device_create() < 0){
        err_msg = "ports_device_create() fail!";
        goto err_ports_device_init_1;
    }

    if (portsinfo_device_init() < 0) {
        err_msg = "portsinfo_device_init() fail!";
        goto err_ports_device_init_2;
    }
    printk(KERN_INFO "Module initial success.\n");

    return 0;

err_ports_device_init_2:
    ports_device_destroy();
err_ports_device_init_1:
    printk(KERN_ERR "[%s/%d] Module initial failure. %s\n",__func__,__LINE__,err_msg);
    return -1;
}


static void __exit
ports_device_exit(void)
{
    portsinfo_device_exit();
    ports_device_destroy();
    printk(KERN_INFO "Remove module.\n");
}


MODULE_AUTHOR(INV_SYSFS_AUTHOR);
MODULE_DESCRIPTION(INV_SYSFS_DESC);
MODULE_VERSION(INV_SYSFS_VERSION);
MODULE_LICENSE(INV_SYSFS_LICENSE);

module_init(ports_device_init);
module_exit(ports_device_exit);
