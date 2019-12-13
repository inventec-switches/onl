#ifndef INV_PTHREAD_H
#define INV_PTHREAD_H

/*
 * onlp_led_id should be same as in onlp/platform_lib.h
 */
enum led_id {
    LED_STK=0,
    LED_FAN,
    LED_PWR,
    LED_SYS,
    LED_FAN1,
    LED_FAN2,
};

enum psu_id {
    PSU1,
    PSU2
};

enum ctl_state_list{
    CTL_NOT_READY=0,
    CTL_SWITCH_READY=0x1,
    CTL_BIOS_READY=0x10,
    CTL_BOTH_READY=0x11
};

typedef enum sys_led_mode_e {
    SYS_LED_MODE_OFF = 0,
    SYS_LED_MODE_ON = 1,
    SYS_LED_MODE_1_HZ = 2,
    SYS_LED_MODE_0_5_HZ = 3
} sys_led_mode_t;

ssize_t cpld_show_psu(u8 *psu_state, int index);

ssize_t cpld_set_ctl(u8 ctl_state);
ssize_t cpld_set_cottonwood_led(u8 led_mode,int index);

ssize_t psoc_show_psu_state(char *buf, int index);
ssize_t psoc_show_fan_input(char *buf, int index);
ssize_t psoc_show_fan_state(char *buf);
ssize_t psoc_show_psu_vin(char *buf, int index);
ssize_t psoc_show_diag(char *buf);
ssize_t psoc_set_diag(const char *buf, size_t count);

#endif /* INV_PTHREAD_H */
