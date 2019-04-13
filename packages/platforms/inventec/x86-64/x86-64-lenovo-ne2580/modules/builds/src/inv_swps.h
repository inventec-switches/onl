#ifndef INV_SWPS_H
#define INV_SWPS_H

#include "transceiver.h"
#include "io_expander.h"
#include "inv_mux.h"

/* Module settings */
#define SWP_CLS_NAME          "swps"
#define SWP_DEV_PORT          "port"
#define SWP_DEV_MODCTL        "module"
#define SWP_RESET_PWD         "inventec"
#define SWP_POLLING_PERIOD    (300)  /* msec */
#define SWP_POLLING_ENABLE    (1)
#define SWP_AUTOCONFIG_ENABLE (1)
static int block_polling = 0;

/* Module information */
#define SWP_AUTHOR            "Neil <liao.neil@inventec.com>"
#define SWP_DESC              "Inventec port and transceiver driver"
#define SWP_VERSION           "A1-4.3.4"
#define SWP_LICENSE           "GPL"

/* Module status define */
#define SWP_STATE_NORMAL      (0)
#define SWP_STATE_I2C_DIE     (-91)

/* [Note]:
 *  Functions and mechanism for auto-detect platform type is ready,
 *  But HW and BIOS not ready! We need to wait them.
 *  So, please do not use PLATFORM_TYPE_AUTO until they are ready.
 *  (2016.06.13)
 */
#define PLATFORM_TYPE_AUTO                (100)
#define PLATFORM_TYPE_PEONY_SFP1_GA       (203)
#define PLATFORM_TYPE_PEONY_SFP2_GA       (204)
#define PLATFORM_TYPE_PEONY_COPPER_GA     (205)
#define PLATFORM_TYPE_PEONY_AUTO          (206)
/* Current running platfrom */
#define PLATFORM_SETTINGS                 PLATFORM_TYPE_PEONY_AUTO

/* Define platform flag and kernel version */
#if (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_SFP1_GA)
  #define SWPS_PEONY_SFP1
  #define SWPS_KERN_VER_AF_3_10
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_SFP2_GA)
  #define SWPS_PEONY_SFP2
  #define SWPS_KERN_VER_AF_3_10
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_COPPER_GA)
  #define SWPS_PEONY_COPPER
  #define SWPS_KERN_VER_AF_3_10
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_AUTO)
  #define SWPS_PEONY_SFP1
  #define SWPS_PEONY_SFP2
  #define SWPS_PEONY_COPPER
  #define SWPS_KERN_VER_AF_3_10
#endif


struct inv_platform_s {
    int  id;
    char name[64];
};

struct inv_ioexp_layout_s {
    int ioexp_id;
    int ioexp_type;
    struct ioexp_addr_s addr[4];
};

struct inv_port_layout_s {
    int port_id;
    int chan_id;
    int ioexp_id;
    int ioexp_offset;
    int transvr_type;
    int chipset_type;
    int lane_id[8];
};

enum {
    PEONY_COPPER_ONE_CPLD,
    PEONY_COPPER_TWO_CPLD,
    PEONY_SFP_EVT,
    PEONY_SFP_DVT
};

/* ==========================================
 *   Inventec Platform Settings
 * ==========================================
 */
struct inv_platform_s platform_map[] = {
    {PLATFORM_TYPE_PEONY_SFP1_GA,        "Peony_SFP1_GA"       },
    {PLATFORM_TYPE_PEONY_SFP2_GA,        "Peony_SFP2_GA"       },
    {PLATFORM_TYPE_PEONY_COPPER_GA,      "Peony_Copper_GA"     },
    {PLATFORM_TYPE_PEONY_AUTO,           "Peony_Auto_Detect"   },
};


/* ===========================================================
 *   Peony-SFP1 Layout configuration
 * ===========================================================
 */
#ifdef SWPS_PEONY_SFP1
unsigned peony_sfp1_gpio_rest_mux = MUX_RST_CPLD_C0_A77_70_74_RST_ALL;

struct inv_ioexp_layout_s peony_sfp1_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_QSFP_6P_LAYOUT_1, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 0 A */
                                        {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                        {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {9, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {9, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {9, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
};

struct inv_port_layout_s peony_sfp1_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  20,  1,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  1} },
    { 1,  21,  1,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  2} },
    { 2,  22,  1,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  3} },
    { 3,  23,  1,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  4} },
    { 4,  24,  1,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  5} },
    { 5,  25,  1,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  6} },
    { 6,  26,  1,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  7} },
    { 7,  27,  1,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  8} },
    { 8,  28,  2,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 13} },
    { 9,  29,  2,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 14} },
    {10,  30,  2,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 15} },
    {11,  31,  2,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 16} },
    {12,  32,  2,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 21} },
    {13,  33,  2,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 22} },
    {14,  34,  2,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 23} },
    {15,  35,  2,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 24} },
    {16,  36,  3,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 29} },
    {17,  37,  3,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 30} },
    {18,  38,  3,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 31} },
    {19,  39,  3,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 32} },
    {20,  40,  3,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 33} },
    {21,  41,  3,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 34} },
    {22,  42,  3,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 35} },
    {23,  43,  3,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 36} },
    {24,  44,  4,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 65} },
    {25,  45,  4,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 66} },
    {26,  46,  4,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 67} },
    {27,  47,  4,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 68} },
    {28,  48,  4,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 69} },
    {29,  49,  4,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 70} },
    {30,  50,  4,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 71} },
    {31,  51,  4,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 72} },
    {32,  52,  5,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 97} },
    {33,  53,  5,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 98} },
    {34,  54,  5,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 99} },
    {35,  55,  5,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {100} },
    {36,  56,  5,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {105} },
    {37,  57,  5,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {106} },
    {38,  58,  5,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {107} },
    {39,  59,  5,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {108} },
    {40,  60,  6,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {113} },
    {41,  61,  6,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {114} },
    {42,  62,  6,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {115} },
    {43,  63,  6,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {116} },
    {44,  64,  6,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {121} },
    {45,  65,  6,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {122} },
    {46,  66,  6,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {123} },
    {47,  67,  6,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {124} },
    {48,  12,  0,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 49, 50, 51, 52} },
    {49,  13,  0,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 57, 58, 59, 60} },
    {50,  14,  0,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 61, 62, 63, 64} },
    {51,  15,  0,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 77, 78, 79, 80} },
    {52,  16,  0,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 85, 86, 87, 88} },
    {53,  17,  0,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 93, 94, 95, 96} },
};
#endif


/* ===========================================================
 *   Peony-SFP2 Layout configuration
 * ===========================================================
 */
#ifdef SWPS_PEONY_SFP2
unsigned peony_sfp2_gpio_rest_mux = MUX_RST_GPIO_69_PAC9548;

struct inv_ioexp_layout_s peony_sfp2_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_QSFP_8P_LAYOUT_1, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                        {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                        {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {9, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {9, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {9, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_SFP_8P_LAYOUT_1,  { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                        {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                        {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
};

struct inv_port_layout_s peony_sfp2_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    { 0,  20,  1,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  1} },
    { 1,  21,  1,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  2} },
    { 2,  22,  1,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  3} },
    { 3,  23,  1,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  4} },
    { 4,  24,  1,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  5} },
    { 5,  25,  1,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  6} },
    { 6,  26,  1,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  7} },
    { 7,  27,  1,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, {  8} },
    { 8,  28,  2,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 13} },
    { 9,  29,  2,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 14} },
    {10,  30,  2,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 15} },
    {11,  31,  2,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 16} },
    {12,  32,  2,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 21} },
    {13,  33,  2,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 22} },
    {14,  34,  2,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 23} },
    {15,  35,  2,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 24} },
    {16,  36,  3,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 29} },
    {17,  37,  3,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 30} },
    {18,  38,  3,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 31} },
    {19,  39,  3,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 32} },
    {20,  40,  3,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 33} },
    {21,  41,  3,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 34} },
    {22,  42,  3,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 35} },
    {23,  43,  3,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 36} },
    {24,  44,  4,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 41} },
    {25,  45,  4,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 42} },
    {26,  46,  4,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 43} },
    {27,  47,  4,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 44} },
    {28,  48,  4,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 49} },
    {29,  49,  4,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 50} },
    {30,  50,  4,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 51} },
    {31,  51,  4,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 52} },
    {32,  52,  5,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 57} },
    {33,  53,  5,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 58} },
    {34,  54,  5,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 59} },
    {35,  55,  5,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 60} },
    {36,  56,  5,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 61} },
    {37,  57,  5,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 62} },
    {38,  58,  5,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 63} },
    {39,  59,  5,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 64} },
    {40,  60,  6,  0, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 65} },
    {41,  61,  6,  1, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 66} },
    {42,  62,  6,  2, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 67} },
    {43,  63,  6,  3, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 68} },
    {44,  64,  6,  4, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 69} },
    {45,  65,  6,  5, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 70} },
    {46,  66,  6,  6, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 71} },
    {47,  67,  6,  7, TRANSVR_TYPE_SFP,     BCM_CHIP_TYPE_TRIDENT_3, { 72} },
    {48,  12,  0,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 85, 86, 87, 88} },
    {49,  13,  0,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 77, 78, 79, 80} },
    {50,  14,  0,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 93, 94, 95, 96} },
    {51,  15,  0,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 97, 98, 99,100} },
    {52,  16,  0,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, {113,114,115,116} },
    {53,  17,  0,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, {105,106,107,108} },
    {54,  18,  0,  6, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, {121,122,123,124} },
    {55,  19,  0,  7, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, {125,126,127,128} },
};
#endif


/* ===========================================================
 *   Peony-Copper Layout configuration
 * ===========================================================
 */
#ifdef SWPS_PEONY_COPPER
unsigned peony_copper_gpio_rest_mux = MUX_RST_GPIO_69_PAC9548;

struct inv_ioexp_layout_s peony_copper_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_QSFP_6P_LAYOUT_1, { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 0 A */
                                        {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                        {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
};

struct inv_port_layout_s peony_copper_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / BCM_CHIP_TYPE / LANE_ID */
    {48,  4,  0,  0, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 49, 50, 51, 52} },
    {49,  5,  0,  1, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 57, 58, 59, 60} },
    {50,  6,  0,  2, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 61, 62, 63, 64} },
    {51,  7,  0,  3, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 77, 78, 79, 80} },
    {52,  8,  0,  4, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 85, 86, 87, 88} },
    {53,  9,  0,  5, TRANSVR_TYPE_QSFP_28, BCM_CHIP_TYPE_TRIDENT_3, { 93, 94, 95, 96} },
};
#endif



#endif /* INV_SWPS_H */










