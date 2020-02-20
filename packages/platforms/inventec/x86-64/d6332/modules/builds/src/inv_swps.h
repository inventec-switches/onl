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

/* Module information */
#define SWP_AUTHOR            "Neil <liao.neil@inventec.com>"
#define SWP_DESC              "Inventec port and transceiver driver"
#define SWP_VERSION           "4.3.10"
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
#define PLATFORM_TYPE_MAGNOLIA            (111)
#define PLATFORM_TYPE_MAGNOLIA_FNC        (112)
#define PLATFORM_TYPE_REDWOOD             (121)
#define PLATFORM_TYPE_REDWOOD_FSL         (122)
#define PLATFORM_TYPE_HUDSON32I_GA        (131)
#define PLATFORM_TYPE_SPRUCE              (141)
#define PLATFORM_TYPE_CYPRESS_GA1         (151) /* Up -> Down */
#define PLATFORM_TYPE_CYPRESS_GA2         (152) /* Down -> Up */
#define PLATFORM_TYPE_CYPRESS_BAI         (153) /* Down -> Up */
#define PLATFORM_TYPE_TAHOE               (161)
#define PLATFORM_TYPE_SEQUOIA_GA          (171)
#define PLATFORM_TYPE_LAVENDER_GA         (181)
#define PLATFORM_TYPE_LAVENDER_ONL        (182)
#define PLATFORM_TYPE_COTTONWOOD_RANGELEY (191)
#define PLATFORM_TYPE_MAPLE_GA            (201)
#define PLATFORM_TYPE_GULMOHAR_GA         (202)
#define PLATFORM_TYPE_GULMOHAR_2T_EVT1_GA (203)
#define PLATFORM_TYPE_PEONY_SFP_GA        (204)
#define PLATFORM_TYPE_PEONY_COPPER_GA     (205)
#define PLATFORM_TYPE_PEONY_AUTO          (206)
#define PLATFORM_TYPE_MAPLE_B             (207)
#define PLATFORM_TYPE_MAPLE_J             (208)
#define PLATFORM_TYPE_CEDAR_GA            (209)
/* Current running platfrom */
#define PLATFORM_SETTINGS                 PLATFORM_TYPE_CEDAR_GA

/* Define platform flag and kernel version */
#if (PLATFORM_SETTINGS == PLATFORM_TYPE_MAGNOLIA)
  #define SWPS_MAGNOLIA             (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_MAGNOLIA_FNC)
  #define SWPS_MAGNOLIA             (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_REDWOOD)
  #define SWPS_REDWOOD              (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_REDWOOD_FSL)
  #define SWPS_REDWOOD_FSL          (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_HUDSON32I_GA)
  #define SWPS_HUDSON32I_GA         (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_SPRUCE)
  #define SWPS_SPRUCE               (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_CYPRESS_GA1)
  #define SWPS_CYPRESS_GA1          (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_CYPRESS_GA2)
  #define SWPS_CYPRESS_GA2          (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_CYPRESS_BAI)
  #define SWPS_CYPRESS_BAI          (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_TAHOE)
  #define SWPS_TAHOE                (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_SEQUOIA_GA)
  #define SWPS_SEQUOIA              (1)
  #define SWPS_KERN_VER_BF_3_8      (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_LAVENDER_GA)
  #define SWPS_LAVENDER             (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_LAVENDER_ONL)
  #define SWPS_LAVENDER             (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_COTTONWOOD_RANGELEY)
  #define SWPS_COTTONWOOD_RANGELEY  (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_MAPLE_GA)
  #define SWPS_MAPLE_GA             (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_MAPLE_B)
  #define SWPS_MAPLE_B              (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_MAPLE_J)
  #define SWPS_MAPLE_J              (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_GULMOHAR_GA)
  #define SWPS_GULMOHAR             (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_GULMOHAR_2T_EVT1_GA)
  #define SWPS_GULMOHAR_2T_EVT1     (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_SFP_GA)
  #define SWPS_PEONY_SFP            (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_COPPER_GA)
  #define SWPS_PEONY_COPPER         (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_PEONY_AUTO)
  #define SWPS_PEONY_SFP            (1)
  #define SWPS_PEONY_COPPER         (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_CEDAR_GA)
  #define SWPS_CEDAR_GA             (1)
  #define SWPS_KERN_VER_AF_3_10     (1)
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


/* ==========================================
 *   Inventec Platform Settings
 * ==========================================
 */
struct inv_platform_s platform_map[] = {
    {PLATFORM_TYPE_AUTO,                 "Auto-Detect"         },
    {PLATFORM_TYPE_MAGNOLIA,             "Magnolia"            },
    {PLATFORM_TYPE_MAGNOLIA_FNC,         "Magnolia_FNC"        },
    {PLATFORM_TYPE_REDWOOD,              "Redwood"             },
    {PLATFORM_TYPE_REDWOOD_FSL,          "Redwood_FSL"         },
    {PLATFORM_TYPE_HUDSON32I_GA,         "Hudson32i"           },
    {PLATFORM_TYPE_SPRUCE,               "Spruce"              },
    {PLATFORM_TYPE_CYPRESS_GA1,          "Cypress_GA1"         },
    {PLATFORM_TYPE_CYPRESS_GA2,          "Cypress_GA2"         },
    {PLATFORM_TYPE_CYPRESS_BAI,          "Cypress_BAI"         },
    {PLATFORM_TYPE_TAHOE,                "Tahoe"               },
    {PLATFORM_TYPE_SEQUOIA_GA,           "Sequoia_GA"          },
    {PLATFORM_TYPE_LAVENDER_GA,          "Lavender_GA"         },
    {PLATFORM_TYPE_LAVENDER_ONL,         "Lavender_ONL"        },
    {PLATFORM_TYPE_COTTONWOOD_RANGELEY,  "Cottonwood_RANGELEY" },
    {PLATFORM_TYPE_MAPLE_GA,             "Maple_GA"            },
    {PLATFORM_TYPE_MAPLE_B,              "Maple_B"             },
    {PLATFORM_TYPE_MAPLE_J,              "Maple_J"             },
    {PLATFORM_TYPE_GULMOHAR_GA,          "Gulmohar_GA"         },
    {PLATFORM_TYPE_GULMOHAR_2T_EVT1_GA,  "Gulmohar_2T_EVT1_GA" },
    {PLATFORM_TYPE_PEONY_SFP_GA,         "Peony_SFP_GA"        },
    {PLATFORM_TYPE_PEONY_COPPER_GA,      "Peony_Copper_GA"     },
    {PLATFORM_TYPE_PEONY_AUTO,           "Peony_Auto_Detect"   },
    {PLATFORM_TYPE_CEDAR_GA,             "Cedar_GA"            },
};


/* ==========================================
 *   Magnolia Layout configuration
 * ==========================================
 */
#ifdef SWPS_MAGNOLIA
unsigned magnolia_gpio_rest_mux = MUX_RST_GPIO_48_PCA9548;

struct inv_ioexp_layout_s magnolia_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_MAGINOLIA_NAB, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {1,  IOEXP_TYPE_MAGINOLIA_NAB, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {2,  IOEXP_TYPE_MAGINOLIA_NAB, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {3,  IOEXP_TYPE_MAGINOLIA_4AB, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander 4 A */
                                     {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf0, 0xff}, {0xf0, 0xff}, }, }, /* addr[1] = I/O Expander 4 B */
    },
    {4,  IOEXP_TYPE_MAGINOLIA_NAB, { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {5,  IOEXP_TYPE_MAGINOLIA_NAB, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                     {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, }, }, /* addr[1] = I/O Expander N B */
    },
    {6,  IOEXP_TYPE_MAGINOLIA_7AB, { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xf0, 0x33}, }, }, /* addr[1] = I/O Expander 7 B */
    },
};

struct inv_port_layout_s magnolia_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  10,  0,  0, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 16} },
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 15} },
    { 2,  12,  0,  2, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 14} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 13} },
    { 4,  14,  0,  4, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 24} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 23} },
    { 6,  16,  0,  6, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 22} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 21} },
    { 8,  18,  1,  0, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 28} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 27} },
    {10,  20,  1,  2, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 26} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 25} },
    {12,  22,  1,  4, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 32} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 31} },
    {14,  24,  1,  6, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 30} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 29} },
    {16,  26,  2,  0, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 48} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 47} },
    {18,  28,  2,  2, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 46} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 45} },
    {20,  30,  2,  4, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 52} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 51} },
    {22,  32,  2,  6, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 50} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 49} },
    {24,  34,  3,  0, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 56} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 55} },
    {26,  36,  3,  2, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 54} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 53} },
    {28,  38,  3,  4, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 60} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 59} },
    {30,  40,  3,  6, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 58} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 57} },
    {32,  42,  4,  0, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 64} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 63} },
    {34,  44,  4,  2, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 62} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 61} },
    {36,  46,  4,  4, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 68} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 67} },
    {38,  48,  4,  6, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 66} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 65} },
    {40,  50,  5,  0, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 72} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 71} },
    {42,  52,  5,  2, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 70} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 69} },
    {44,  54,  5,  4, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 76} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 75} },
    {46,  56,  5,  6, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 74} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,  CHIP_TYPE_MAGNOLIA, { 73} },
    {48,  58,  6,  0, TRANSVR_TYPE_QSFP, CHIP_TYPE_MAGNOLIA, { 81, 82, 83, 84} },
    {49,  59,  6,  1, TRANSVR_TYPE_QSFP, CHIP_TYPE_MAGNOLIA, { 77, 78, 79, 80} },
    {50,  60,  6,  2, TRANSVR_TYPE_QSFP, CHIP_TYPE_MAGNOLIA, { 97, 98, 99,100} },
    {51,  61,  6,  3, TRANSVR_TYPE_QSFP, CHIP_TYPE_MAGNOLIA, {101,102,103,104} },
    {52,  62,  6,  4, TRANSVR_TYPE_QSFP, CHIP_TYPE_MAGNOLIA, {105,106,107,108} },
    {53,  63,  6,  5, TRANSVR_TYPE_QSFP, CHIP_TYPE_MAGNOLIA, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Redwood Layout configuration
 * ==========================================
 */
#ifdef SWPS_REDWOOD
unsigned redwood_gpio_rest_mux = MUX_RST_GPIO_48_PCA9548;

struct inv_ioexp_layout_s redwood_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_REDWOOD_P01P08, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {1,  IOEXP_TYPE_REDWOOD_P09P16, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {2,  IOEXP_TYPE_REDWOOD_P01P08, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {3,  IOEXP_TYPE_REDWOOD_P09P16, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
};

struct inv_port_layout_s redwood_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  22,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  1,  2,  3,  4} },
    { 1,  23,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  5,  6,  7,  8} },
    { 2,  24,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  9, 10, 11, 12} },
    { 3,  25,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 13, 14, 15, 16} },
    { 4,  26,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 17, 18, 19, 20} },
    { 5,  27,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 21, 22, 23, 24} },
    { 6,  28,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 25, 26, 27, 28} },
    { 7,  29,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 29, 30, 31, 32} },
    { 8,  30,  1,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 33, 34, 35, 36} },
    { 9,  31,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 37, 38, 39, 40} },
    {10,  32,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 41, 42, 43, 44} },
    {11,  33,  1,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 45, 46, 47, 48} },
    {12,  34,  1,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 49, 50, 51, 52} },
    {13,  35,  1,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 53, 54, 55, 56} },
    {14,  36,  1,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 57, 58, 59, 60} },
    {15,  37,  1,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 61, 62, 63, 64} },
    {16,   6,  2,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 65, 66, 67, 68} },
    {17,   7,  2,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 69, 70, 71, 72} },
    {18,   8,  2,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 73, 74, 75, 76} },
    {19,   9,  2,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 77, 78, 79, 80} },
    {20,  10,  2,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 81, 82, 83, 84} },
    {21,  11,  2,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85, 86, 87, 88} },
    {22,  12,  2,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 89, 90, 91, 92} },
    {23,  13,  2,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 93, 94, 95, 96} },
    {24,  14,  3,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97, 98, 99,100} },
    {25,  15,  3,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101,102,103,104} },
    {26,  16,  3,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105,106,107,108} },
    {27,  17,  3,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109,110,111,112} },
    {28,  18,  3,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {113,114,115,116} },
    {29,  19,  3,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {117,118,119,120} },
    {30,  20,  3,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {121,122,123,124} },
    {31,  21,  3,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {125,126,127,128} },
};
#endif


/* ==========================================
 *   Hudson32i Layout configuration
 * ==========================================
 */
#ifdef SWPS_HUDSON32I_GA
unsigned hudsin32iga_gpio_rest_mux = MUX_RST_GPIO_48_PCA9548;

struct inv_ioexp_layout_s hudson32iga_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_HUDSON32IGA_P01P08, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x24 */
    },
    {1,  IOEXP_TYPE_HUDSON32IGA_P09P16, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x24 */
    },
    {2,  IOEXP_TYPE_HUDSON32IGA_P01P08, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x25 */
    },
    {3,  IOEXP_TYPE_HUDSON32IGA_P09P16, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander A    */
                                          {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander B    */
                                          {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0x25 */
    },
};

struct inv_port_layout_s hudson32iga_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,   6,  0,  0, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {  1,  2,  3,  4} },
    { 1,   7,  0,  1, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {  5,  6,  7,  8} },
    { 2,   8,  0,  2, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {  9, 10, 11, 12} },
    { 3,   9,  0,  3, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 13, 14, 15, 16} },
    { 4,  10,  0,  4, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 17, 18, 19, 20} },
    { 5,  11,  0,  5, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 21, 22, 23, 24} },
    { 6,  12,  0,  6, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 25, 26, 27, 28} },
    { 7,  13,  0,  7, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 29, 30, 31, 32} },
    { 8,  14,  1,  0, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 33, 34, 35, 36} },
    { 9,  15,  1,  1, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 37, 38, 39, 40} },
    {10,  16,  1,  2, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 41, 42, 43, 44} },
    {11,  17,  1,  3, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 45, 46, 47, 48} },
    {12,  18,  1,  4, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 49, 50, 51, 52} },
    {13,  19,  1,  5, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 53, 54, 55, 56} },
    {14,  20,  1,  6, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 57, 58, 59, 60} },
    {15,  21,  1,  7, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 61, 62, 63, 64} },
    {16,  22,  2,  0, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 65, 66, 67, 68} },
    {17,  23,  2,  1, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 69, 70, 71, 72} },
    {18,  24,  2,  2, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 73, 74, 75, 76} },
    {19,  25,  2,  3, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 77, 78, 79, 80} },
    {20,  26,  2,  4, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 81, 82, 83, 84} },
    {21,  27,  2,  5, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 85, 86, 87, 88} },
    {22,  28,  2,  6, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 89, 90, 91, 92} },
    {23,  29,  2,  7, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 93, 94, 95, 96} },
    {24,  30,  3,  0, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 97, 98, 99,100} },
    {25,  31,  3,  1, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {101,102,103,104} },
    {26,  32,  3,  2, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {105,106,107,108} },
    {27,  33,  3,  3, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {109,110,111,112} },
    {28,  34,  3,  4, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {113,114,115,116} },
    {29,  35,  3,  5, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {117,118,119,120} },
    {30,  36,  3,  6, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {121,122,123,124} },
    {31,  37,  3,  7, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {125,126,127,128} },
};
#endif


/* ==========================================
 *   Spruce Layout configuration
 * ==========================================
 */
#ifdef SWPS_SPRUCE
unsigned spruce_gpio_rest_mux = MUX_RST_GPIO_48_PCA9548;

struct inv_ioexp_layout_s spruce_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SPRUCE_7AB,  { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 7A  */
                                   {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xf0, 0x33}, }, }, /* addr[2] = I/O Expander 7B  */
    },
};

struct inv_port_layout_s spruce_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,   6,  0,  0, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 81, 82, 83, 84} },
    { 1,   7,  0,  1, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 77, 78, 79, 80} },
    { 2,   8,  0,  2, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, { 97, 98, 99,100} },
    { 3,   9,  0,  3, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {101,102,103,104} },
    { 4,  10,  0,  4, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {105,106,107,108} },
    { 5,  11,  0,  5, TRANSVR_TYPE_QSFP_PLUS, CHIP_TYPE_MAGNOLIA, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Cypress Layout configuration (Inventec version [Up->Down])
 * ==========================================
 */
#ifdef SWPS_CYPRESS_GA1
unsigned cypress_ga1_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;

struct inv_ioexp_layout_s cypress_ga1_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_CYPRESS_7ABC,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xc0}, {0xff, 0xc0}, },    /* addr[1] = I/O Expander 7 B */
                                     {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};

struct inv_port_layout_s cypress_ga1_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  10,  0,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  1} },
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  2} },
    { 2,  12,  0,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  3} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  4} },
    { 4,  14,  0,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  5} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  6} },
    { 6,  16,  0,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  7} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  8} },
    { 8,  18,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  9} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 10} },
    {10,  20,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 11} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 12} },
    {12,  22,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 21} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 22} },
    {14,  24,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 23} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 24} },
    {16,  26,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 33} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 34} },
    {18,  28,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 35} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 36} },
    {20,  30,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 37} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 38} },
    {22,  32,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 39} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 40} },
    {24,  34,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 41} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 42} },
    {26,  36,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 43} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 44} },
    {28,  38,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 49} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 50} },
    {30,  40,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 51} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 52} },
    {32,  42,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 53} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 54} },
    {34,  44,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 55} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 56} },
    {36,  46,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 65} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 66} },
    {38,  48,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 67} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 68} },
    {40,  50,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 69} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 70} },
    {42,  52,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 71} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 72} },
    {44,  54,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 81} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 82} },
    {46,  56,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 83} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 84} },
    {48,  58,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97, 98, 99,100} },
    {49,  59,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85, 86, 87, 88} },
    {50,  60,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101,102,103,104} },
    {51,  61,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105,106,107,108} },
    {52,  62,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109,110,111,112} },
    {53,  63,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {117,118,119,120} },
};
#endif


/* ==========================================
 *   Cypress Layout configuration (Inventec version [Down->Up])
 * ==========================================
 */
#ifdef SWPS_CYPRESS_GA2
unsigned cypress_ga2_gpio_rest_mux = MUX_RST_GPIO_FORCE_HEDERA;

struct inv_ioexp_layout_s cypress_ga2_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_CYPRESS_7ABC,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xc0}, {0xff, 0xc0}, },    /* addr[1] = I/O Expander 7 B */
                                     {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};

struct inv_port_layout_s cypress_ga2_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  11,  0,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  2} },
    { 1,  10,  0,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  1} },
    { 2,  13,  0,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  4} },
    { 3,  12,  0,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  3} },
    { 4,  15,  0,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  6} },
    { 5,  14,  0,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  5} },
    { 6,  17,  0,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  8} },
    { 7,  16,  0,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  7} },
    { 8,  19,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 10} },
    { 9,  18,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  9} },
    {10,  21,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 12} },
    {11,  20,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 11} },
    {12,  23,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 22} },
    {13,  22,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 21} },
    {14,  25,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 24} },
    {15,  24,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 23} },
    {16,  27,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 34} },
    {17,  26,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 33} },
    {18,  29,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 36} },
    {19,  28,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 35} },
    {20,  31,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 38} },
    {21,  30,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 37} },
    {22,  33,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 40} },
    {23,  32,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 39} },
    {24,  35,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 42} },
    {25,  34,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 41} },
    {26,  37,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 44} },
    {27,  36,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 43} },
    {28,  39,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 50} },
    {29,  38,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 49} },
    {30,  41,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 52} },
    {31,  40,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 51} },
    {32,  43,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 54} },
    {33,  42,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 53} },
    {34,  45,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 56} },
    {35,  44,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 55} },
    {36,  47,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 66} },
    {37,  46,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 65} },
    {38,  49,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 68} },
    {39,  48,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 67} },
    {40,  51,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 70} },
    {41,  50,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 69} },
    {42,  53,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 72} },
    {43,  52,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 71} },
    {44,  55,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 82} },
    {45,  54,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 81} },
    {46,  57,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 84} },
    {47,  56,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 83} },
    {48,  59,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85, 86, 87, 88} },
    {49,  58,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97, 98, 99,100} },
    {50,  61,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105,106,107,108} },
    {51,  60,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101,102,103,104} },
    {52,  63,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {117,118,119,120} },
    {53,  62,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Cypress Layout configuration (C1 version)
 * ==========================================
 */
#ifdef SWPS_CYPRESS_BAI
unsigned cypress_b_gpio_rest_mux = MUX_RST_GPIO_FORCE_HEDERA;

struct inv_ioexp_layout_s cypress_b_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_SFP_8P_LAYOUT_1, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                       {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                       {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_CYPRESS_7ABC,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 7 A */
                                     {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xc0}, {0xff, 0xc0}, },    /* addr[1] = I/O Expander 7 B */
                                     {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};

struct inv_port_layout_s cypress_b_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  2} },
    { 2,  10,  0,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  1} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  4} },
    { 4,  12,  0,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  3} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  6} },
    { 6,  14,  0,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  5} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  8} },
    { 8,  16,  0,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  7} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 10} },
    {10,  18,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, {  9} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 12} },
    {12,  20,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 11} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 22} },
    {14,  22,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 21} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 24} },
    {16,  24,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 23} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 34} },
    {18,  26,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 33} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 36} },
    {20,  28,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 35} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 38} },
    {22,  30,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 37} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 40} },
    {24,  32,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 39} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 42} },
    {26,  34,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 41} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 44} },
    {28,  36,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 43} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 50} },
    {30,  38,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 49} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 52} },
    {32,  40,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 51} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 54} },
    {34,  42,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 53} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 56} },
    {36,  44,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 55} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 66} },
    {38,  46,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 65} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 68} },
    {40,  48,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 67} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 70} },
    {42,  50,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 69} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 72} },
    {44,  52,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 71} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 82} },
    {46,  54,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 81} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 84} },
    {48,  56,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_REDWOOD, { 83} },
    {49,  59,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85, 86, 87, 88} },
    {50,  58,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97, 98, 99,100} },
    {51,  61,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105,106,107,108} },
    {52,  60,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101,102,103,104} },
    {53,  63,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {117,118,119,120} },
    {54,  62,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Redwood_fsl Layout configuration
 * ==========================================
 */
#ifdef SWPS_REDWOOD_FSL
unsigned redwood_fsl_gpio_rest_mux = MUX_RST_GPIO_48_PCA9548;

struct inv_ioexp_layout_s redwood_fsl_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_REDWOOD_P01P08, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {1,  IOEXP_TYPE_REDWOOD_P09P16, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x25, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {2,  IOEXP_TYPE_REDWOOD_P01P08, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
    {3,  IOEXP_TYPE_REDWOOD_P09P16, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[0] = I/O Expander 1-4 A */
                                      {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0x0f}, },    /* addr[1] = I/O Expander 1-4 B */
                                      {0, 0x24, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 B   */
    },
};


struct inv_port_layout_s redwood_fsl_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  22,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  1,  2,  3,  4} },
    { 1,  23,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  5,  6,  7,  8} },
    { 2,  24,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  9, 10, 11, 12} },
    { 3,  25,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 13, 14, 15, 16} },
    { 4,  26,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 17, 18, 19, 20} },
    { 5,  27,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 21, 22, 23, 24} },
    { 6,  28,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 25, 26, 27, 28} },
    { 7,  29,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 29, 30, 31, 32} },
    { 8,  30,  1,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 33, 34, 35, 36} },
    { 9,  31,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 37, 38, 39, 40} },
    {10,  32,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 41, 42, 43, 44} },
    {11,  33,  1,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 45, 46, 47, 48} },
    {12,  34,  1,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 49, 50, 51, 52} },
    {13,  35,  1,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 53, 54, 55, 56} },
    {14,  36,  1,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 57, 58, 59, 60} },
    {15,  37,  1,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 61, 62, 63, 64} },
    {16,   6,  2,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 65, 66, 67, 68} },
    {17,   7,  2,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 69, 70, 71, 72} },
    {18,   8,  2,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 73, 74, 75, 76} },
    {19,   9,  2,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 77, 78, 79, 80} },
    {20,  10,  2,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 81, 82, 83, 84} },
    {21,  11,  2,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85, 86, 87, 88} },
    {22,  12,  2,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 89, 90, 91, 92} },
    {23,  13,  2,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 93, 94, 95, 96} },
    {24,  14,  3,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97, 98, 99,100} },
    {25,  15,  3,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101,102,103,104} },
    {26,  16,  3,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105,106,107,108} },
    {27,  17,  3,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109,110,111,112} },
    {28,  18,  3,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {113,114,115,116} },
    {29,  19,  3,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {117,118,119,120} },
    {30,  20,  3,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {121,122,123,124} },
    {31,  21,  3,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {125,126,127,128} },
};
#endif


/* ==========================================
 *   Tahoe Layout configuration
 * ==========================================
 */
#ifdef SWPS_TAHOE
unsigned tahoe_gpio_rest_mux = MUX_RST_GPIO_249_PCA9548;

struct inv_ioexp_layout_s tahoe_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_TAHOE_6ABC, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xce, 0xb9}, {0x18, 0xe3}, },    /* addr[0] = I/O Expander 6 A */
                                  {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xce, 0xb9}, {0x18, 0xe3}, },    /* addr[1] = I/O Expander 6 B */
                                  {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xce, 0xb9}, {0x18, 0xe3}, }, }, /* addr[2] = I/O Expander 6 C */
    },
    {1,  IOEXP_TYPE_TAHOE_5A,   { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xce, 0xb9}, {0x18, 0xe3}, }, }, /* addr[0] = I/O Expander 5 A */
    },
};


struct inv_port_layout_s tahoe_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  12,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 65,  66,  67,  68} },
    { 1,  11,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 53,  54,  55,  56} },
    { 2,  22,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 69,  70,  71,  72} },
    { 3,  21,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 81,  82,  83,  84} },
    { 4,  24,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97,  98,  99, 100} },
    { 5,  23,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85,  86,  87,  88} },
    { 6,  18,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101, 102, 103, 104} },
    { 7,  17,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105, 106, 107, 108} },
    { 8,  20,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {113, 114, 115, 116} },
    { 9,  19,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109, 110, 111, 112} },
};
#endif


/* ==========================================
 *   Sequoia Layout configuration
 * ==========================================
 */
#ifdef SWPS_SEQUOIA
unsigned sequoia_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;

struct inv_ioexp_layout_s sequoia_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SEQUOIA_NABC, { {1, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    {1, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                    {1, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_SEQUOIA_NABC, { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1 A */
                                    {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1 B */
                                    {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 1 C */
    },
    {2,  IOEXP_TYPE_SEQUOIA_NABC, { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 2 A */
                                    {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 2 B */
                                    {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 2 C */
    },
    {3,  IOEXP_TYPE_SEQUOIA_NABC, { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 3 A */
                                    {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 3 B */
                                    {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 3 C */
    },
    {4,  IOEXP_TYPE_SEQUOIA_NABC, { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 4 A */
                                    {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 4 B */
                                    {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 4 C */
    },
    {5,  IOEXP_TYPE_SEQUOIA_NABC, { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 5 A */
                                    {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 5 B */
                                    {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 C */
    },
    {6,  IOEXP_TYPE_SEQUOIA_NABC, { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 6 A */
                                    {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 6 B */
                                    {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 6 C */
    },
    {7,  IOEXP_TYPE_SEQUOIA_NABC, { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 7 A */
                                    {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 7 B */
                                    {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};


struct inv_port_layout_s sequoia_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,   9,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  9,  10,  11,  12} },
    { 1,  10,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  1,   2,   3,   4} },
    { 2,  11,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 25,  26,  27,  28} },
    { 3,  12,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 17,  18,  19,  20} },
    { 4,  13,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 41,  42,  43,  44} },
    { 5,  14,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 33,  34,  35,  36} },
    { 6,  15,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 57,  58,  59,  60} },
    { 7,  16,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 49,  50,  51,  52} },
    { 8,  17,  1,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 73,  74,  75,  76} },
    { 9,  18,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 65,  66,  67,  68} },
    {10,  19,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 89,  90,  91,  92} },
    {11,  20,  1,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 81,  82,  83,  84} },
    {12,  21,  1,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {105, 106, 107, 108} },
    {13,  22,  1,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 97,  98,  99, 100} },
    {14,  23,  1,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {121, 122, 123, 124} },
    {15,  24,  1,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {113, 114, 115, 116} },
    {16,  25,  2,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {137, 138, 139, 140} },
    {17,  26,  2,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {129, 130, 131, 132} },
    {18,  27,  2,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {153, 154, 155, 156} },
    {19,  28,  2,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {145, 146, 147, 148} },
    {20,  29,  2,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {169, 170, 171, 172} },
    {21,  30,  2,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {161, 162, 163, 164} },
    {22,  31,  2,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {185, 186, 187, 188} },
    {23,  32,  2,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {177, 178, 179, 180} },
    {24,  33,  3,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {201, 202, 203, 204} },
    {25,  34,  3,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {193, 194, 195, 196} },
    {26,  35,  3,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {217, 218, 219, 220} },
    {27,  36,  3,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {209, 210, 211, 212} },
    {28,  37,  3,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {233, 234, 235, 236} },
    {29,  38,  3,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {225, 226, 227, 228} },
    {30,  39,  3,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {249, 250, 251, 252} },
    {31,  40,  3,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {241, 242, 243, 244} },
    {32,  44,  4,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 13,  14,  15,  16} },
    {33,  43,  4,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {  5,   6,   7,   8} },
    {34,  42,  4,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 29,  30,  31,  32} },
    {35,  41,  4,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 21,  22,  23,  24} },
    {36,  48,  4,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 45,  46,  47,  48} },
    {37,  47,  4,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 37,  38,  39,  40} },
    {38,  46,  4,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 61,  62,  63,  64} },
    {39,  45,  4,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 53,  54,  55,  56} },
    {40,  52,  5,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 77,  78,  79,  80} },
    {41,  51,  5,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 69,  70,  71,  72} },
    {42,  50,  5,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 93,  94,  95,  96} },
    {43,  49,  5,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, { 85,  86,  87,  88} },
    {44,  56,  5,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {109, 110, 111, 112} },
    {45,  55,  5,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {101, 102, 103, 104} },
    {46,  54,  5,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {125, 126, 127, 128} },
    {47,  53,  5,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {117, 118, 119, 120} },
    {48,  60,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {141, 142, 143, 144} },
    {49,  59,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {133, 134, 135, 136} },
    {50,  58,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {157, 158, 159, 160} },
    {51,  57,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {149, 150, 151, 152} },
    {52,  64,  6,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {173, 174, 175, 176} },
    {53,  63,  6,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {165, 166, 167, 168} },
    {54,  62,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {189, 190, 191, 192} },
    {55,  61,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {181, 182, 183, 184} },
    {56,  68,  7,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {205, 206, 207, 208} },
    {57,  67,  7,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {197, 198, 199, 200} },
    {58,  66,  7,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {221, 222, 223, 224} },
    {59,  65,  7,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {213, 214, 215, 216} },
    {60,  72,  7,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {237, 238, 239, 240} },
    {61,  71,  7,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {229, 230, 231, 232} },
    {62,  70,  7,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {253, 254, 255, 256} },
    {63,  69,  7,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_REDWOOD, {245, 246, 247, 248} },
};
#endif


/* ==========================================
 *   Lavender Layout configuration
 * ==========================================
 */
#if (PLATFORM_SETTINGS == PLATFORM_TYPE_LAVENDER_GA)
unsigned lavender_gpio_rest_mux = MUX_RST_GPIO_505_PCA9548;
#elif (PLATFORM_SETTINGS == PLATFORM_TYPE_LAVENDER_ONL)
unsigned lavender_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;
#endif

#ifdef SWPS_LAVENDER
struct inv_ioexp_layout_s lavender_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_SEQUOIA_NABC, { { 1, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 1, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                    { 1, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_SEQUOIA_NABC, { { 2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 1 A */
                                    { 2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 1 B */
                                    { 2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 1 C */
    },
    {2,  IOEXP_TYPE_SEQUOIA_NABC, { { 3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 2 A */
                                    { 3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 2 B */
                                    { 3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 2 C */
    },
    {3,  IOEXP_TYPE_SEQUOIA_NABC, { { 4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 3 A */
                                    { 4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 3 B */
                                    { 4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 3 C */
    },
    {4,  IOEXP_TYPE_SEQUOIA_NABC, { { 9, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 4 A */
                                    { 9, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 4 B */
                                    { 9, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 4 C */
    },
    {5,  IOEXP_TYPE_SEQUOIA_NABC, { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 5 A */
                                    {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 5 B */
                                    {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 5 C */
    },
    {6,  IOEXP_TYPE_SEQUOIA_NABC, { {11, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 6 A */
                                    {11, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 6 B */
                                    {11, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 6 C */
    },
    {7,  IOEXP_TYPE_SEQUOIA_NABC, { {12, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 7 A */
                                    {12, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0x00}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 7 B */
                                    {12, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
    {8,  IOEXP_TYPE_LAVENDER_P65, { { 5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xF6, 0xff}, {0xF8, 0xff}, }, }, /* addr[0] = I/O Expander CPU */
    },
};


struct inv_port_layout_s lavender_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  17,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {188, 189, 190, 191} },
    { 1,  18,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {184, 185, 186, 187} },
    { 2,  19,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {180, 181, 182, 183} },
    { 3,  20,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {176, 177, 178, 179} },
    { 4,  21,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {172, 173, 174, 175} },
    { 5,  22,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {168, 169, 170, 171} },
    { 6,  23,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {164, 165, 166, 167} },
    { 7,  24,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {160, 161, 162, 163} },
    { 8,  25,  1,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {156, 157, 158, 159} },
    { 9,  26,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {152, 153, 154, 155} },
    {10,  27,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {148, 149, 150, 151} },
    {11,  28,  1,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {144, 145, 146, 147} },
    {12,  29,  1,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {140, 141, 142, 143} },
    {13,  30,  1,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {136, 137, 138, 139} },
    {14,  31,  1,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {132, 133, 134, 135} },
    {15,  32,  1,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {128, 129, 130, 131} },
    {16,  33,  2,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {  0,   1,   2,   3} },
    {17,  34,  2,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {  4,   5,   6,   7} },
    {18,  35,  2,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {  8,   9,  10,  11} },
    {19,  36,  2,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 12,  13,  14,  15} },
    {20,  37,  2,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 16,  17,  18,  19} },
    {21,  38,  2,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 20,  21,  22,  23} },
    {22,  39,  2,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 24,  25,  26,  27} },
    {23,  40,  2,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 28,  29,  30,  31} },
    {24,  41,  3,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 32,  33,  34,  35} },
    {25,  42,  3,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 36,  37,  38,  39} },
    {26,  43,  3,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 40,  41,  42,  43} },
    {27,  44,  3,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 44,  45,  46,  47} },
    {28,  45,  3,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 48,  49,  50,  51} },
    {29,  46,  3,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 52,  53,  54,  55} },
    {30,  47,  3,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 56,  57,  58,  59} },
    {31,  48,  3,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 60,  61,  62,  63} },
    {32,  49,  4,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {256, 257, 258, 259} },
    {33,  50,  4,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {260, 261, 262, 263} },
    {34,  51,  4,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {264, 265, 266, 267} },
    {35,  52,  4,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {268, 269, 270, 271} },
    {36,  53,  4,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {272, 273, 274, 275} },
    {37,  54,  4,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {276, 277, 278, 279} },
    {38,  55,  4,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {280, 281, 282, 283} },
    {39,  56,  4,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {284, 285, 286, 287} },
    {40,  57,  5,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {288, 289, 290, 291} },
    {41,  58,  5,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {292, 293, 294, 295} },
    {42,  59,  5,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {296, 297, 298, 299} },
    {43,  60,  5,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {300, 301, 302, 303} },
    {44,  61,  5,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {304, 305, 306, 307} },
    {45,  62,  5,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {308, 309, 310, 311} },
    {46,  63,  5,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {312, 313, 314, 315} },
    {47,  64,  5,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {316, 317, 318, 319} },
    {48,  65,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {444, 445, 446, 447} },
    {49,  66,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {440, 441, 442, 443} },
    {50,  67,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {436, 437, 438, 439} },
    {51,  68,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {432, 433, 434, 435} },
    {52,  69,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {428, 429, 430, 431} },
    {53,  70,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {424, 425, 426, 427} },
    {54,  71,  6,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {420, 421, 422, 423} },
    {55,  72,  6,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {416, 417, 418, 419} },
    {56,  73,  7,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {412, 413, 414, 415} },
    {57,  74,  7,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {408, 409, 410, 411} },
    {58,  75,  7,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {404, 405, 406, 407} },
    {59,  76,  7,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {400, 401, 402, 403} },
    {60,  77,  7,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {396, 397, 398, 399} },
    {61,  78,  7,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {392, 393, 394, 395} },
    {62,  79,  7,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {388, 389, 390, 391} },
    {63,  80,  7,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {384, 385, 386, 387} },
    {64,   5,  8,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 64,  65,  66,  67} },
};
#endif

/* ===========================================================
 *   Cottonwood Layout configuration Rangeley (Rangeley CPU board)
 * ===========================================================
 */
#ifdef SWPS_COTTONWOOD_RANGELEY
unsigned cottonwood_rangeley_gpio_rest_mux = MUX_RST_GPIO_500_PCA9548;

struct inv_ioexp_layout_s cottonwood_rangeley_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  CPLD_TYPE_COTTONWOOD,{ {1, 0x55, {22, 23, 24, 25}, {22, 23, 24, 25}, {-1, -1, -1, -1}, {0xee, 0xee, 0x99, 0x99}, {0x00, 0x00, 0x00, 0x00}, },
                              },
    },
};


struct inv_port_layout_s cottonwood_rangeley_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  2,  0,  0, TRANSVR_TYPE_SFP, CHIP_TYPE_REDWOOD, { 75} },
    { 1,  3,  0,  1, TRANSVR_TYPE_SFP, CHIP_TYPE_REDWOOD, { 77} },
    { 2,  4,  0,  2, TRANSVR_TYPE_SFP, CHIP_TYPE_REDWOOD, { 79} },
    { 3,  5,  0,  3, TRANSVR_TYPE_SFP, CHIP_TYPE_REDWOOD, { 81} },
};
#endif

/* ===========================================================
 *   Maple Layout configuration (Old)
 * ===========================================================
 */
#ifdef SWPS_MAPLE_GA
unsigned maple_ga_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;

struct inv_ioexp_layout_s maple_ga_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_MAPLE_0ABC,   { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                    {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_MAPLE_NABC,   { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_MAPLE_NABC,   { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_MAPLE_NABC,   { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_MAPLE_NABC,   { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_MAPLE_NABC,   { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_MAPLE_NABC,   { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
};

struct inv_port_layout_s maple_ga_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  18,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  1} },
    { 1,  19,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  2} },
    { 2,  20,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  3} },
    { 3,  21,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  4} },
    { 4,  22,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  5} },
    { 5,  23,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  6} },
    { 6,  24,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  7} },
    { 7,  25,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  8} },
    { 8,  26,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 13} },
    { 9,  27,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 14} },
    {10,  28,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 15} },
    {11,  29,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 16} },
    {12,  30,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 21} },
    {13,  31,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 22} },
    {14,  32,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 23} },
    {15,  33,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 24} },
    {16,  34,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 29} },
    {17,  35,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 30} },
    {18,  36,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 31} },
    {19,  37,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 32} },
    {20,  38,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 33} },
    {21,  39,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 34} },
    {22,  40,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 35} },
    {23,  41,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 36} },
    {24,  42,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 41} },
    {25,  43,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 42} },
    {26,  44,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 43} },
    {27,  45,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 44} },
    {28,  46,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 49} },
    {29,  47,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 50} },
    {30,  48,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 51} },
    {31,  49,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 52} },
    {32,  50,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 57} },
    {33,  51,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 58} },
    {34,  52,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 59} },
    {35,  53,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 60} },
    {36,  54,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 61} },
    {37,  55,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 62} },
    {38,  56,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 63} },
    {39,  57,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 64} },
    {40,  58,  6,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 65} },
    {41,  59,  6,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 66} },
    {42,  60,  6,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 67} },
    {43,  61,  6,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 68} },
    {44,  62,  6,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 69} },
    {45,  63,  6,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 70} },
    {46,  64,  6,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 71} },
    {47,  65,  6,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 72} },
    {48,  10,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 77, 78, 79, 80} },
    {49,  11,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 85, 86, 87, 88} },
    {50,  12,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 93, 94, 95, 96} },
    {51,  13,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 97, 98, 99,100} },
    {52,  14,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {105,106,107,108} },
    {53,  15,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {113,114,115,116} },
    {54,  16,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {121,122,123,124} },
    {55,  17,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {125,126,127,128} },
};
#endif

/* ===========================================================
 *   Maple Layout configuration (B version)
 * ===========================================================
 */
#ifdef SWPS_MAPLE_B
unsigned maple_b_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;

struct inv_ioexp_layout_s maple_b_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_MAPLE_0ABC,   { { 6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                    { 6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_MAPLE_NABC,   { { 7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    { 7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    { 7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_MAPLE_NABC,   { { 8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    { 8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    { 8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_MAPLE_NABC,   { { 9, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    { 9, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    { 9, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_MAPLE_NABC,   { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_MAPLE_NABC,   { {11, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {11, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {11, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_MAPLE_NABC,   { {12, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {12, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {12, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
};

struct inv_port_layout_s maple_b_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 1,  23,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  2} },
    { 2,  22,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  1} },
    { 3,  25,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  4} },
    { 4,  24,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  3} },
    { 5,  27,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  6} },
    { 6,  26,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  5} },
    { 7,  29,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  8} },
    { 8,  28,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  7} },
    { 9,  31,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 14} },
    {10,  30,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 13} },
    {11,  33,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 16} },
    {12,  32,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 15} },
    {13,  35,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 22} },
    {14,  34,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 21} },
    {15,  37,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 24} },
    {16,  36,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 23} },
    {17,  39,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 30} },
    {18,  38,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 29} },
    {19,  41,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 32} },
    {20,  40,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 31} },
    {21,  43,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 34} },
    {22,  42,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 33} },
    {23,  45,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 36} },
    {24,  44,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 35} },
    {25,  47,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 42} },
    {26,  46,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 41} },
    {27,  49,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 44} },
    {28,  48,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 43} },
    {29,  51,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 50} },
    {30,  50,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 49} },
    {31,  53,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 52} },
    {32,  52,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 51} },
    {33,  55,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 58} },
    {34,  54,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 57} },
    {35,  57,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 60} },
    {36,  56,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 59} },
    {37,  59,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 62} },
    {38,  58,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 61} },
    {39,  61,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 64} },
    {40,  60,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 63} },
    {41,  63,  6,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 66} },
    {42,  62,  6,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 65} },
    {43,  65,  6,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 68} },
    {44,  64,  6,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 67} },
    {45,  67,  6,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 70} },
    {46,  66,  6,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 69} },
    {47,  69,  6,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 72} },
    {48,  68,  6,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 71} },
    {49,  15,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 77, 78, 79, 80} },
    {50,  14,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 85, 86, 87, 88} },
    {51,  17,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 97, 98, 99,100} },
    {52,  16,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 93, 94, 95, 96} },
    {53,  19,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {105,106,107,108} },
    {54,  18,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {113,114,115,116} },
    {55,  21,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {125,126,127,128} },
    {56,  20,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {121,122,123,124} },
};
#endif

/* ===========================================================
 *   Maple Layout configuration (J version)
 * ===========================================================
 */
#ifdef SWPS_MAPLE_J
unsigned maple_j_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;

struct inv_ioexp_layout_s maple_j_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_MAPLE_0ABC,   { { 6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                    { 6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_MAPLE_NABC,   { { 7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    { 7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    { 7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_MAPLE_NABC,   { { 8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    { 8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    { 8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_MAPLE_NABC,   { { 9, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    { 9, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    { 9, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_MAPLE_NABC,   { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_MAPLE_NABC,   { {11, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {11, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {11, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_MAPLE_NABC,   { {12, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[0] = I/O Expander N A */
                                    {12, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xf0}, {0xff, 0xf0}, },    /* addr[1] = I/O Expander N B */
                                    {12, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
};

struct inv_port_layout_s maple_j_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  22,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  1} },
    { 1,  23,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  2} },
    { 2,  24,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  3} },
    { 3,  25,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  4} },
    { 4,  26,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  5} },
    { 5,  27,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  6} },
    { 6,  28,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  7} },
    { 7,  29,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  8} },
    { 8,  30,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 13} },
    { 9,  31,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 14} },
    {10,  32,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 15} },
    {11,  33,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 16} },
    {12,  34,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 21} },
    {13,  35,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 22} },
    {14,  36,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 23} },
    {15,  37,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 24} },
    {16,  38,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 29} },
    {17,  39,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 30} },
    {18,  40,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 31} },
    {19,  41,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 32} },
    {20,  42,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 33} },
    {21,  43,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 34} },
    {22,  44,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 35} },
    {23,  45,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 36} },
    {24,  46,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 41} },
    {25,  47,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 42} },
    {26,  48,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 43} },
    {27,  49,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 44} },
    {28,  50,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 49} },
    {29,  51,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 50} },
    {30,  52,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 51} },
    {31,  53,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 52} },
    {32,  54,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 57} },
    {33,  55,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 58} },
    {34,  56,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 59} },
    {35,  57,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 60} },
    {36,  58,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 61} },
    {37,  59,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 62} },
    {38,  60,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 63} },
    {39,  61,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 64} },
    {40,  62,  6,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 65} },
    {41,  63,  6,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 66} },
    {42,  64,  6,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 67} },
    {43,  65,  6,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 68} },
    {44,  66,  6,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 69} },
    {45,  67,  6,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 70} },
    {46,  68,  6,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 71} },
    {47,  69,  6,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 72} },
    {48,  14,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 85, 86, 87, 88} },
    {49,  15,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 77, 78, 79, 80} },
    {50,  16,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 93, 94, 95, 96} },
    {51,  17,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 97, 98, 99,100} },
    {52,  18,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {113,114,115,116} },
    {53,  19,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {105,106,107,108} },
    {54,  20,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {121,122,123,124} },
    {55,  21,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {125,126,127,128} },
};
#endif

/* ==========================================
 *   Gulmohar Layout configuration
 * ==========================================
 */
#ifdef SWPS_GULMOHAR
unsigned gulmohar_gpio_rest_mux = MUX_RST_GPIO_505_PCA9548;

struct inv_ioexp_layout_s gulmohar_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_GULMOHAR_NABC,  { {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                      {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                      {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_GULMOHAR_NABC,  { {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                      {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                      {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_GULMOHAR_NABC,  { {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                      {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                      {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_GULMOHAR_NABC,  { {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                      {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                      {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_GULMOHAR_NABC,  { {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                      {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                      {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_GULMOHAR_NABC,  { {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                      {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                      {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_GULMOHAR_7ABC,  { {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 7 A */
                                      {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xff}, },    /* addr[1] = I/O Expander 7 B */
                                      {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};



struct inv_port_layout_s gulmohar_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  10,  0,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  1} },
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  2} },
    { 2,  12,  0,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  3} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  4} },
    { 4,  14,  0,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  5} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  6} },
    { 6,  16,  0,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  7} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  8} },
    { 8,  18,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {  9} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 10} },
    {10,  20,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 11} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 12} },
    {12,  22,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 21} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 22} },
    {14,  24,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 23} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 24} },
    {16,  26,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 33} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 34} },
    {18,  28,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 35} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 36} },
    {20,  30,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 37} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 38} },
    {22,  32,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 39} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 40} },
    {24,  34,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 41} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 42} },
    {26,  36,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 43} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 44} },
    {28,  38,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 49} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 50} },
    {30,  40,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 51} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 52} },
    {32,  42,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 53} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 54} },
    {34,  44,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 55} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 56} },
    {36,  46,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 65} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 66} },
    {38,  48,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 67} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 68} },
    {40,  50,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 69} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 70} },
    {42,  52,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 71} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 72} },
    {44,  54,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 81} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 82} },
    {46,  56,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 83} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, { 84} },
    {48,  58,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 97, 98, 99,100} },
    {49,  59,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, { 85, 86, 87, 88} },
    {50,  60,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {101,102,103,104} },
    {51,  61,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {105,106,107,108} },
    {52,  62,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {109,110,111,112} },
    {53,  63,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {109,110,111,112} },
};
#endif


/* ==========================================
 *   Gulmohar_2T EVT1 Layout configuration
 * ==========================================
 */
#ifdef SWPS_GULMOHAR_2T_EVT1
unsigned gulmohar_2t_evt1_gpio_rest_mux = MUX_RST_GPIO_505_PCA9548;

struct inv_ioexp_layout_s gulmohar_2t_evt1_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_GULMOHAR_2T_EVT1_NABC,{ {2, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[0] = I/O Expander N A */
                                            {2, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[1] = I/O Expander N B */
                                            {2, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {1,  IOEXP_TYPE_GULMOHAR_2T_EVT1_1ABC,{ {3, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[0] = I/O Expander N A */
                                            {3, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[1] = I/O Expander N B */
                                            {3, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {2,  IOEXP_TYPE_GULMOHAR_2T_EVT1_NABC,{ {4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[0] = I/O Expander N A */
                                            {4, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[1] = I/O Expander N B */
                                            {4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {3,  IOEXP_TYPE_GULMOHAR_2T_EVT1_3ABC,{ {5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xf3, 0xf3}, {0xf3, 0xf3}, },    /* addr[0] = I/O Expander N A */
                                            {5, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[1] = I/O Expander N B */
                                            {5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {4,  IOEXP_TYPE_GULMOHAR_2T_EVT1_NABC,{ {6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[0] = I/O Expander N A */
                                            {6, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[1] = I/O Expander N B */
                                            {6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {5,  IOEXP_TYPE_GULMOHAR_2T_EVT1_NABC,{ {7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[0] = I/O Expander N A */
                                            {7, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xdd, 0xdd}, {0xdd, 0xdd}, },    /* addr[1] = I/O Expander N B */
                                            {7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0x00, 0x00}, }, }, /* addr[2] = I/O Expander N C */
    },
    {6,  IOEXP_TYPE_GULMOHAR_2T_EVT1_7ABC,{ {8, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xd6, 0xda}, {0x18, 0xe3}, },    /* addr[0] = I/O Expander 7 A */
                                            {8, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xd6, 0xda}, {0x18, 0xe3}, },    /* addr[1] = I/O Expander 7 B */
                                            {8, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xd6, 0xff}, {0x18, 0xff}, }, }, /* addr[2] = I/O Expander 7 C */
    },
};



struct inv_port_layout_s gulmohar_2t_evt1_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  10,  0,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 1,  11,  0,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 2,  12,  0,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 3,  13,  0,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 4,  14,  0,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 5,  15,  0,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 6,  16,  0,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 7,  17,  0,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 8,  18,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    { 9,  19,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {10,  20,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {11,  21,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {12,  22,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {13,  23,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {14,  24,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {15,  25,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {16,  26,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {17,  27,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {18,  28,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {19,  29,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {20,  30,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {21,  31,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {22,  32,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {23,  33,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {24,  34,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {25,  35,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {26,  36,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {27,  37,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {28,  38,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {29,  39,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {30,  40,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {31,  41,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {32,  42,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {33,  43,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {34,  44,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {35,  45,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {36,  46,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {37,  47,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {38,  48,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {39,  49,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {40,  50,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {41,  51,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {42,  52,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {43,  53,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {44,  54,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {45,  55,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {46,  56,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {47,  57,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_LAVENDER, {-99} },
    {48,  59,  6,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {49,  58,  6,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {50,  61,  6,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {51,  60,  6,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {52,  63,  6,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {53,  62,  6,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {54,  65,  6,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
    {55,  64,  6,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_LAVENDER, {-99,-99,-99,-99} },
};
#endif


/* ===========================================================
 *   Peony-SFP Layout configuration
 * ===========================================================
 */
#ifdef SWPS_PEONY_SFP
unsigned peony_sfp_gpio_rest_mux = MUX_RST_CPLD_C0_A77_70_74_RST_ALL;

struct inv_ioexp_layout_s peony_sfp_ioexp_layout[] = {
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

struct inv_port_layout_s peony_sfp_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  20,  1,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  1} },
    { 1,  21,  1,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  2} },
    { 2,  22,  1,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  3} },
    { 3,  23,  1,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  4} },
    { 4,  24,  1,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  5} },
    { 5,  25,  1,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  6} },
    { 6,  26,  1,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  7} },
    { 7,  27,  1,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {  8} },
    { 8,  28,  2,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 13} },
    { 9,  29,  2,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 14} },
    {10,  30,  2,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 15} },
    {11,  31,  2,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 16} },
    {12,  32,  2,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 21} },
    {13,  33,  2,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 22} },
    {14,  34,  2,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 23} },
    {15,  35,  2,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 24} },
    {16,  36,  3,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 29} },
    {17,  37,  3,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 30} },
    {18,  38,  3,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 31} },
    {19,  39,  3,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 32} },
    {20,  40,  3,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 33} },
    {21,  41,  3,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 34} },
    {22,  42,  3,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 35} },
    {23,  43,  3,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 36} },
    {24,  44,  4,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 65} },
    {25,  45,  4,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 66} },
    {26,  46,  4,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 67} },
    {27,  47,  4,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 68} },
    {28,  48,  4,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 69} },
    {29,  49,  4,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 70} },
    {30,  50,  4,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 71} },
    {31,  51,  4,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 72} },
    {32,  52,  5,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 97} },
    {33,  53,  5,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 98} },
    {34,  54,  5,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, { 99} },
    {35,  55,  5,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {100} },
    {36,  56,  5,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {105} },
    {37,  57,  5,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {106} },
    {38,  58,  5,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {107} },
    {39,  59,  5,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {108} },
    {40,  60,  6,  0, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {113} },
    {41,  61,  6,  1, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {114} },
    {42,  62,  6,  2, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {115} },
    {43,  63,  6,  3, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {116} },
    {44,  64,  6,  4, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {121} },
    {45,  65,  6,  5, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {122} },
    {46,  66,  6,  6, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {123} },
    {47,  67,  6,  7, TRANSVR_TYPE_SFP,     CHIP_TYPE_MAPLE, {124} },
    {48,  12,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 49, 50, 51, 52} },
    {49,  13,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 57, 58, 59, 60} },
    {50,  14,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 61, 62, 63, 64} },
    {51,  15,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 77, 78, 79, 80} },
    {52,  16,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 85, 86, 87, 88} },
    {53,  17,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 93, 94, 95, 96} },
};
#endif


/* ===========================================================
 *   Peony-Copper Layout configuration
 * ===========================================================
 */
#ifdef SWPS_PEONY_COPPER
unsigned peony_copper_gpio_rest_mux = MUX_RST_CPLD_C0_A77_70_74_RST_ALL;

struct inv_ioexp_layout_s peony_copper_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_QSFP_6P_LAYOUT_1, { {10, 0x20, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xc0, 0xc0}, },    /* addr[0] = I/O Expander 0 A */
                                        {10, 0x21, {0, 1}, {2, 3}, {6, 7}, {0xff, 0xff}, {0xc0, 0xff}, },    /* addr[1] = I/O Expander 0 B */
                                        {10, 0x22, {0, 1}, {2, 3}, {6, 7}, {0xc0, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
};

struct inv_port_layout_s peony_copper_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    {48,  4,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 49, 50, 51, 52} },
    {49,  5,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 57, 58, 59, 60} },
    {50,  6,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 61, 62, 63, 64} },
    {51,  7,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 77, 78, 79, 80} },
    {52,  8,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 85, 86, 87, 88} },
    {53,  9,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 93, 94, 95, 96} },
};
#endif


/* ===========================================================
 *   Cedar Layout configuration 
 * ===========================================================
 */
#ifdef SWPS_CEDAR_GA
unsigned cedar_ga_gpio_rest_mux = MUX_RST_GPIO_69_PCA9548;

struct inv_ioexp_layout_s cedar_ga_ioexp_layout[] = {
    /* IOEXP_ID / IOEXP_TYPE / { Chan_ID, Chip_addr, Read_offset, Write_offset, config_offset, data_default, conf_default } */
    {0,  IOEXP_TYPE_CEDAR_0ABC,   { { 4, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 4, 0x21, {0,-1}, {1,-1}, {3,-1}, {0xff      }, {0x00      }, },    /* addr[1] = I/O Expander 0 B */
                                    { 4, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {1,  IOEXP_TYPE_CEDAR_0ABC,   { { 5, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 5, 0x21, {0,-1}, {1,-1}, {3,-1}, {0xff      }, {0x00      }, },    /* addr[1] = I/O Expander 0 B */
                                    { 5, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {2,  IOEXP_TYPE_CEDAR_0ABC,   { { 6, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 6, 0x21, {0,-1}, {1,-1}, {3,-1}, {0xff      }, {0x00      }, },    /* addr[1] = I/O Expander 0 B */
                                    { 6, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
    {3,  IOEXP_TYPE_CEDAR_0ABC,   { { 7, 0x20, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0x00, 0x00}, },    /* addr[0] = I/O Expander 0 A */
                                    { 7, 0x21, {0,-1}, {1,-1}, {3,-1}, {0xff      }, {0x00      }, },    /* addr[1] = I/O Expander 0 B */
                                    { 7, 0x22, {0, 1}, {2, 3}, {6, 7}, {0x00, 0xff}, {0xff, 0xff}, }, }, /* addr[2] = I/O Expander 0 C */
    },
};

struct inv_port_layout_s cedar_ga_port_layout[] = {
    /* Port_ID / Chan_ID / IOEXP_ID / IOEXP_VIRT_OFFSET / TRANSCEIVER_TYPE / CHIP_TYPE / LANE_ID */
    { 0,  12,  0,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 21, 22, 23, 24} },
    { 1,  13,  0,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 17, 18, 19, 20} },
    { 2,  14,  0,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 25, 26, 27, 28} },
    { 3,  15,  0,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 29, 30, 31, 32} },
    { 4,  16,  0,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 37, 38, 39, 40} },
    { 5,  17,  0,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 33, 34, 35, 36} },
    { 6,  18,  0,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 41, 42, 43, 44} },
    { 7,  19,  0,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 45, 46, 47, 48} },
    { 8,  20,  1,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {  5,  6,  7,  8} },
    { 9,  21,  1,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {  1,  2,  3,  4} },
    {10,  22,  1,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {  9, 10, 11, 12} },
    {11,  23,  1,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 13, 14, 15, 16} },
    {12,  24,  1,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 53, 54, 55, 56} },
    {13,  25,  1,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 49, 50, 51, 52} },
    {14,  26,  1,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 57, 58, 59, 60} },
    {15,  27,  1,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 61, 62, 63, 64} },
    {16,  28,  2,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 71, 72, 73, 74} },
    {17,  29,  2,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 67, 68, 69, 70} },
    {18,  30,  2,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 75, 76, 77, 78} },
    {19,  31,  2,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 79, 80, 81, 82} },
    {20,  32,  2,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 119, 120, 121, 122} },
    {21,  33,  2,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 115, 116, 117, 118} },
    {22,  34,  2,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 123, 124, 125, 126} },
    {23,  35,  2,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 127, 128, 129, 130} },
    {24,  36,  3,  0, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 87, 88, 89, 90} },
    {25,  37,  3,  1, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 83, 84, 85, 86} },
    {26,  38,  3,  2, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 91, 92, 93, 94} },
    {27,  39,  3,  3, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 95, 96, 97, 98} },
    {28,  40,  3,  4, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 103, 104, 105, 106} },
    {29,  41,  3,  5, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, {  99, 100, 101, 102} },
    {30,  42,  3,  6, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 107, 108, 109, 110} },
    {31,  43,  3,  7, TRANSVR_TYPE_QSFP_28, CHIP_TYPE_MAPLE, { 111, 112, 113, 114} },
};
#endif


#endif /* INV_SWPS_H */









