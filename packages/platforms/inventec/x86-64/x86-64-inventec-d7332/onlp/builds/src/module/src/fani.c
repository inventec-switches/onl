/************************************************************
 * fani.c
 *
 *           Copyright 2018 Inventec Technology Corporation.
 *
 ************************************************************
 *
 * Fan Platform Implementation Defaults.
 *
 ***********************************************************/
#include <onlp/platformi/fani.h>
#include <onlplib/file.h>
#include <fcntl.h>
#include <unistd.h>
#include <onlp/platformi/psui.h>
#include "platform_lib.h"

#define VALIDATE(_id)                           \
    do {                                        \
        if(!ONLP_OID_IS_FAN(_id)) {             \
            return ONLP_STATUS_E_INVALID;       \
        }                                       \
    } while(0)


#define SLOW_PWM 100
#define NORMAL_PWM 175
#define MAX_PWM 255
#define STEP_SIZE 100
#define FAN_ID_TO_PSU_ID(id) (id-ONLP_FAN_PSU_1+1)

#define FAN_CAPS ONLP_FAN_CAPS_GET_RPM|ONLP_FAN_CAPS_GET_PERCENTAGE

static int _fani_status_failed_check(uint32_t* status, int fan_id);
static int _fani_status_present_check(uint32_t* status, int fan_id);
extern char* psu_i2c_addr[ONLP_PSU_MAX];

#define MAKE_FAN_INFO_NODE_ON_FAN_BOARD(id)			\
    {								\
        {							\
            ONLP_FAN_ID_CREATE(ONLP_FAN_##id), "Fan "#id, 0,	\
            {							\
                ONLP_LED_ID_CREATE(ONLP_LED_FAN##id),		\
            }							\
        },							\
        0, FAN_CAPS							\
    }

#define MAKE_FAN_INFO_NODE_ON_PSU(psu_id)		\
    {							\
        { ONLP_FAN_ID_CREATE(ONLP_FAN_PSU_##psu_id), "PSU-"#psu_id" Fan", ONLP_PSU_ID_CREATE(ONLP_PSU_##psu_id)}, \
        0, FAN_CAPS						\
    }


/* Static values */
static onlp_fan_info_t __onlp_fan_info[ONLP_FAN_COUNT] = {
    {},
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(1),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(2),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(3),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(4),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(5),
    MAKE_FAN_INFO_NODE_ON_FAN_BOARD(6),
    MAKE_FAN_INFO_NODE_ON_PSU(1),
    MAKE_FAN_INFO_NODE_ON_PSU(2),
};


/*
 * This function will be called prior to all of onlp_fani_* functions.
 */
int
onlp_fani_init(void)
{
    return ONLP_STATUS_OK;
}

int
onlp_fani_info_get(onlp_oid_t id, onlp_fan_info_t* info)
{
    int rv = ONLP_STATUS_OK;
    int lrpm=0, rrpm=0, pwm=0, psu_id;
    char path[ONLP_CONFIG_INFO_STR_MAX];
    int fan_id;
    VALIDATE(id);

    fan_id=ONLP_OID_ID_GET(id);
    if(fan_id>=ONLP_FAN_MAX) {
        rv=ONLP_STATUS_E_INVALID;
    }

    if(rv==ONLP_STATUS_OK) {
        *info=__onlp_fan_info[fan_id];
        rv=onlp_fani_status_get(id, &info->status);
    }
    if(rv == ONLP_STATUS_OK) {
        if(info->status & ONLP_FAN_STATUS_PRESENT) {
            switch(fan_id) {
            case ONLP_FAN_1:
            case ONLP_FAN_2:
            case ONLP_FAN_3:
            case ONLP_FAN_4:
            case ONLP_FAN_5:
            case ONLP_FAN_6:
                if(info->status & ONLP_FAN_STATUS_F2B) {
                    info->caps = ADD_STATE(info->caps,ONLP_FAN_CAPS_F2B);
                } else if(info->status & ONLP_FAN_STATUS_B2F) {
                    info->caps = ADD_STATE(info->caps,ONLP_FAN_CAPS_B2F) ;
                }

                rv = onlp_file_read_int(&lrpm, INV_FAN_PREFIX"fan%d_input", fan_id*2-1);
                if(rv != ONLP_STATUS_OK ) {
                    return rv;
                }
                rv = onlp_file_read_int(&rrpm, INV_FAN_PREFIX"fan%d_input", fan_id*2);
                if(rv != ONLP_STATUS_OK ) {
                    return rv;
                }
                rv = onlp_file_read_int(&pwm,INV_FAN_PREFIX"pwm%d", fan_id);
                if(rv != ONLP_STATUS_OK ) {
                    return rv;
                }

                if(lrpm <=0 && rrpm <=0) {
                    info->rpm = 0;
                } else if(lrpm <= 0) {
                    info->rpm = rrpm;
                } else if(rrpm <= 0) {
                    info->rpm = lrpm;
                } else {
                    info->rpm = (lrpm+rrpm)/2;
                }

                break;
            case ONLP_FAN_PSU_1:
            case ONLP_FAN_PSU_2:
                info->caps = FAN_CAPS|ONLP_FAN_CAPS_F2B;
                psu_id = FAN_ID_TO_PSU_ID(fan_id);

                snprintf(path,ONLP_CONFIG_INFO_STR_MAX,"%s%d-00%s/fan1_input",INV_PSU_BASE,PSU_I2C_CHAN,psu_i2c_addr[psu_id]);
                rv = onlp_file_read_int(&info->rpm, path);
                if(rv != ONLP_STATUS_OK) {
                    return rv;
                }
                snprintf(path,ONLP_CONFIG_INFO_STR_MAX,"%s%d-00%s/pwm1",INV_PSU_BASE,PSU_I2C_CHAN,psu_i2c_addr[psu_id]);
                rv = onlp_file_read_int(&pwm, path);
                if(rv != ONLP_STATUS_OK) {
                    return rv;
                }
                break;
            default:
                rv = ONLP_STATUS_E_INVALID;
                break;
            }
            if(rv == ONLP_STATUS_OK) {
                if(info->rpm <= 0) {
                    info->mode = ONLP_FAN_MODE_OFF;
                    info->percentage = 0;
                } else {
                    info->percentage = (pwm*100)/MAX_PWM;
                    if(pwm < SLOW_PWM) {
                        info->mode = ONLP_FAN_MODE_SLOW;
                    } else if(pwm < NORMAL_PWM) {
                        info->mode = ONLP_FAN_MODE_NORMAL;
                    } else if(pwm < MAX_PWM) {
                        info->mode = ONLP_FAN_MODE_FAST;
                    } else {
                        info->mode = ONLP_FAN_MODE_MAX;
                    }
                }
            }
        } else {
            info->caps = 0;
            info->rpm = 0;
            info->percentage = 0;
            info->mode = ONLP_FAN_MODE_OFF;
        }
        snprintf(info->model, ONLP_CONFIG_INFO_STR_MAX, "NA");
        snprintf(info->serial, ONLP_CONFIG_INFO_STR_MAX, "NA");
    }
    return rv;
}
static int _fani_status_failed_check(uint32_t* status, int fan_id)
{
    int rv;
    int lrpm, rrpm, rpm, pwm, psu_id;
    char path[ONLP_CONFIG_INFO_STR_MAX];

    switch(fan_id) {
    case ONLP_FAN_1:
    case ONLP_FAN_2:
    case ONLP_FAN_3:
    case ONLP_FAN_4:
    case ONLP_FAN_5:
    case ONLP_FAN_6:
        rv = onlp_file_read_int(&lrpm, INV_FAN_PREFIX"fan%d_input", fan_id*2-1);
        if(rv != ONLP_STATUS_OK ) {
            return rv;
        }
        rv = onlp_file_read_int(&rrpm, INV_FAN_PREFIX"fan%d_input", fan_id*2);
        if(rv != ONLP_STATUS_OK ) {
            return rv;
        }
        rv = onlp_file_read_int(&pwm,INV_FAN_PREFIX"pwm%d", fan_id);
        if(rv != ONLP_STATUS_OK ) {
            return rv;
        }

        if(lrpm<=0 || rrpm<=0 || pwm<=0 || pwm > MAX_PWM) {
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_FAILED);
            *status=REMOVE_STATE(*status,ONLP_FAN_STATUS_B2F);
            *status=REMOVE_STATE(*status,ONLP_FAN_STATUS_F2B);
        } else {
            *status=REMOVE_STATE(*status,ONLP_FAN_STATUS_FAILED);
        }
        break;
    case ONLP_FAN_PSU_1:
    case ONLP_FAN_PSU_2:
        psu_id = FAN_ID_TO_PSU_ID(fan_id);

        snprintf(path,ONLP_CONFIG_INFO_STR_MAX,"%s%d-00%s/fan1_input",INV_PSU_BASE,PSU_I2C_CHAN,psu_i2c_addr[psu_id]);
        rv = onlp_file_read_int(&rpm, path);
        if(rv != ONLP_STATUS_OK ) {
            return rv;
        }
        snprintf(path,ONLP_CONFIG_INFO_STR_MAX,"%s%d-00%s/pwm1",INV_PSU_BASE,PSU_I2C_CHAN,psu_i2c_addr[psu_id]);
        rv = onlp_file_read_int(&pwm, path);
        if(rv != ONLP_STATUS_OK ) {
            return rv;
        }

        if( rpm <= 0  || pwm <=0 || pwm > MAX_PWM) {
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_FAILED);
            *status=REMOVE_STATE(*status,ONLP_FAN_STATUS_B2F);
            *status=REMOVE_STATE(*status,ONLP_FAN_STATUS_F2B);
        } else {
            *status=REMOVE_STATE(*status,ONLP_FAN_STATUS_FAILED);
        }
        break;
    default:
        rv = ONLP_STATUS_E_INVALID;
        break;
    }
    return rv;
}

static int _fani_status_present_check(uint32_t* status, int fan_id)
{
    int rv;
    int len;
    char buf[ONLP_CONFIG_INFO_STR_MAX];
    if(fan_id >= ONLP_FAN_1 && fan_id <= ONLP_FAN_6) {
        rv = onlp_file_read((uint8_t*)buf,ONLP_CONFIG_INFO_STR_MAX, &len, INV_FAN_PREFIX"fanmodule%d_type",fan_id);
    } else {
        rv = ONLP_STATUS_E_INVALID;
    }
    if( rv == ONLP_STATUS_OK ) {
        switch(buf[0]) {
        case '0':
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_PRESENT);
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_F2B);
            break;
        case '1':
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_PRESENT);
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_B2F);
            break;
        case '3':
        case '4':
            *status=ADD_STATE(*status,ONLP_FAN_STATUS_FAILED);
            break;
        default:
            rv = ONLP_STATUS_E_INVALID;
            break;
        }
    }
    return rv;
}

/**
 * @brief Retrieve the fan's operational status.
 * @param id The fan OID.
 * @param rv [out] Receives the fan's operations status flags.
 * @notes Only operational state needs to be returned -
 *        PRESENT/FAILED
 */
int onlp_fani_status_get(onlp_oid_t id, uint32_t* rv)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;
    int fan_id=ONLP_OID_ID_GET(id);
    uint32_t psu_status;

    VALIDATE(id);


    if(fan_id >= ONLP_FAN_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {

        info = &__onlp_fan_info[fan_id];
        switch(fan_id) {
        case ONLP_FAN_1:
        case ONLP_FAN_2:
        case ONLP_FAN_3:
        case ONLP_FAN_4:
        case ONLP_FAN_5:
        case ONLP_FAN_6:
            result = _fani_status_present_check(&info->status, fan_id);
            if (result == ONLP_STATUS_OK ) {
                if (info->status & ONLP_FAN_STATUS_PRESENT) {
                    result = _fani_status_failed_check(&info->status, fan_id);
                }
            }
            break;
        case ONLP_FAN_PSU_1:
        case ONLP_FAN_PSU_2:
            result = onlp_psui_status_get((&info->hdr)->poid, &psu_status);
            if(result != ONLP_STATUS_OK) {
                return result;
            }
            if(psu_status & ONLP_PSU_STATUS_PRESENT) {
                info->status = ADD_STATE(info->status, ONLP_FAN_STATUS_PRESENT);
                result = _fani_status_failed_check(&info->status,  fan_id);
            } else {
                info->status = 0;
            }
            break;
        default:
            result = ONLP_STATUS_E_INVALID;
            break;
        }
        *rv = info->status;
    }
    return result;
}

/**
 * @brief Retrieve the fan's OID hdr.
 * @param id The fan OID.
 * @param rv [out] Receives the OID header.
 */
int onlp_fani_hdr_get(onlp_oid_t id, onlp_oid_hdr_t* hdr)
{
    int result = ONLP_STATUS_OK;
    onlp_fan_info_t* info;
    int fan_id;
    VALIDATE(id);

    fan_id = ONLP_OID_ID_GET(id);
    if(fan_id >= ONLP_FAN_MAX) {
        result = ONLP_STATUS_E_INVALID;
    } else {
        info = &__onlp_fan_info[fan_id];
        *hdr = info->hdr;
    }
    return result;
}


/*
 * This function sets the speed of the given fan in RPM.
 *
 * This function will only be called if the fan supprots the RPM_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_rpm_set(onlp_oid_t id, int rpm)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan speed of the given OID as a percentage.
 *
 * This will only be called if the OID has the PERCENTAGE_SET
 * capability.
 *
 * It is optional if you have no fans at all with this feature.
 */
int
onlp_fani_percentage_set(onlp_oid_t id, int p)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}


/*
 * This function sets the fan speed of the given OID as per
 * the predefined ONLP fan speed modes: off, slow, normal, fast, max.
 *
 * Interpretation of these modes is up to the platform.
 *
 */
int
onlp_fani_mode_set(onlp_oid_t id, onlp_fan_mode_t mode)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * This function sets the fan direction of the given OID.
 *
 * This function is only relevant if the fan OID supports both direction
 * capabilities.
 *
 * This function is optional unless the functionality is available.
 */
int
onlp_fani_dir_set(onlp_oid_t id, onlp_fan_dir_t dir)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

/*
 * Generic fan ioctl. Optional.
 */
int
onlp_fani_ioctl(onlp_oid_t id, va_list vargs)
{
    return ONLP_STATUS_E_UNSUPPORTED;
}

