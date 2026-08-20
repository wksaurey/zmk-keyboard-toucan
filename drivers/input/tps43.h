#pragma once

#include <zephyr/device.h>
#include <zephyr/sys/util.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/i2c.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* TPS43 I2C address */
#define TPS43_I2C_ADDR              0x74

/* TPS43 controller register definitions with IQS572 - Based on IQS5xx-B000 datasheet */
/* https://www.azoteq.com/images/stories/pdf/iqs5xx-b000_trackpad_datasheet.pdf */

/* Gesture events */
// Read-only
#define TPS43_REG_GESTURE_EVENTS_0  0x000D  /* 1 byte */
#define TPS43_REG_GESTURE_EVENTS_1  0x000E  /* 1 byte */

/* System information */
// Read-only
#define TPS43_REG_SYSTEM_INFO_0     0x000F  /* 1 byte - Status flags */
#define TPS43_REG_SYSTEM_INFO_1     0x0010  /* 1 byte - Status flags */

/* Touch data - main coordinates */
// Read-only
#define TPS43_REG_NUM_FINGERS       0x0011  /* 1 byte */
#define TPS43_REG_REL_X             0x0012  /* 2 bytes - relative X movement */
#define TPS43_REG_REL_Y             0x0014  /* 2 bytes - relative Y movement */
#define TPS43_REG_ABS_X             0x0016  /* 2 bytes - absolute X position */
#define TPS43_REG_ABS_Y             0x0018  /* 2 bytes - absolute Y position */
#define TPS43_REG_TOUCH_STRENGTH    0x001A  /* 2 bytes */
#define TPS43_REG_TOUCH_AREA        0x001C  /* 1 byte */


/* System control and configuration */
// Read-write
#define TPS43_REG_SYSTEM_CONTROL_0  0x0431  /* 1 byte - Main control (ACK_RESET, AUTO_ATI) */
#define TPS43_REG_SYSTEM_CONTROL_1  0x0432  /* 1 byte - Event mode control */
// Read-write
#define TPS43_REG_SYSTEM_CONFIG_0   0x058E  /* 1 byte - System configuration */
#define TPS43_REG_SYSTEM_CONFIG_1   0x058F  /* 1 byte - Additional configuration */

#define TPS43_REG_GLOBAL_ATI_C      0x056B  /* 1 byte - Global ATI C value */
#define TPS43_REG_ATI_TARGET        0x056D  /* 2 byte - ATI target value */
#define TPS43_REG_REF_DRIFT_LIMIT   0x0571  /* 1 byte - ATI reference drift limit */
#define TPS43_REG_REATI_LOWER_LIMIT 0x0573 /* 1 byte - ATI reattempt lower limit */
#define TPS43_REG_REATI_UPPER_LIMIT 0x0574 /* 1 byte - ATI reattempt upper limit */
#define TPS43_REG_MAX_COUNT_LIMIT   0x0575  /* 2 byte - ATI max count limit */
#define TPS43_REG_ATI_RETRY_TIME    0x0577  /* 1 byte - ATI retry time [s] */
#define TPS43_REG_REPORT_RATE_ACTIVE 0x057A /* 2 byte - Report rate active [ms]*/
#define TPS43_REG_REPORT_RATE_IDLE_TOUCH 0x057C  /* 2 byte - Report rate idle touch [ms] */
#define TPS43_REG_REPORT_RATE_IDLE  0x057E  /* 2 byte - Report rate idle [ms] */
#define TPS43_REG_REPORT_RATE_LP1   0x0580  /* 2 byte - Report rate lp1 [ms] */
#define TPS43_REG_REPORT_RATE_LP2   0x0582  /* 2 byte - Report rate lp2 [ms] */
#define TPS43_REG_TIMEOUT_ACTIVE    0x0584  /* 1 byte - Timeout active [s] */
#define TPS43_REG_TIMEOUT_IDLE_TOUCH      0x0585  /* 1 byte - Timeout idle touch [s] */
#define TPS43_REG_TIMEOUT_IDLE      0x0586  /* 1 byte - Timeout idle [s] */
#define TPS43_REG_TIMEOUT_LP1       0x0587  /* 1 byte - Timeout lp1 [s] */
#define TPS43_REG_REF_UPDATE_TIME   0x0588  /* 1 byte - Reference update time [s] */

// XY filtering settings
#define TPS43_REG_XY_STATIC_BETA            0x0633  /* 1 byte - XY static beta (See 5.9.2.2) */
#define TPS43_REG_ALP_COUNT_BETA            0x0634  /* 1 byte - ALP count beta (See 3.3.2) */
#define TPS43_REG_ALP1_LTA_BETA             0x0635  /* 1 byte - ALP1 LTA beta (See 3.4.2) */
#define TPS43_REG_ALP2_LTA_BETA             0x0636  /* 1 byte - ALP2 LTA beta */
#define TPS43_REG_XY_DYNAMIC_FILTER_BOTTOM  0x0637  /* 1 byte - XY dynamic filter bottom beta (See 5.9.2.1) */
#define TPS43_REG_XY_DYNAMIC_FILTER_LOWER   0x0638  /* 1 byte - XY dynamic filter lower speed */
#define TPS43_REG_XY_DYNAMIC_FILTER_UPPER   0x0639  /* 2 bytes - XY dynamic filter upper speed */

#define TPS43_REG_X_RESOLUTION              0x066E /* 2 bytes - X resolution */
#define TPS43_REG_Y_RESOLUTION              0x0670 /* 2 bytes - Y resolution */

#define TPS43_DEFAULT_READ_ADDRESS          0x0675 /* 2 bytes - default address used for reading if none is specified */
/* XY configuration */
// Read-write
#define TPS43_REG_XY_CONFIG_0       0x0669  /* 1 byte */

/* Gesture configuration */
// Read-write // Low-level gesture configuration
#define TPS43_REG_SINGLE_FINGER_GESTURES 0x06B7  /* 1 byte */
#define TPS43_REG_MULTI_FINGER_GESTURES  0x06B8  /* 1 byte */
#define TPS43_REG_TAP_TIME                   0x06B9  /* 2 bytes - Tap time [ms] */
#define TPS43_REG_TAP_DISTANCE               0x06BB  /* 2 bytes - Tap distance [pixels] */
#define TPS43_REG_HOLD_TIME                  0x06BD  /* 2 bytes - Hold time [ms] */
#define TPS43_REG_SWIPE_INITIAL_TIME         0x06BF  /* 2 bytes - Swipe initial time [ms] */
#define TPS43_REG_SWIPE_INITIAL_DISTANCE     0x06C1  /* 2 bytes - Swipe initial distance [pixels] */
#define TPS43_REG_SWIPE_CONSECUTIVE_TIME     0x06C3  /* 2 bytes - Swipe consecutive time [ms] */
#define TPS43_REG_SWIPE_CONSECUTIVE_DISTANCE 0x06C5  /* 2 bytes - Swipe consecutive distance [pixels] */
#define TPS43_REG_SWIPE_ANGLE                0x06C7  /* 1 byte  - Swipe angle [64tan(deg)] */
#define TPS43_REG_SCROLL_INITIAL_DISTANCE    0x06C8  /* 2 bytes - Scroll initial distance [pixels] */
#define TPS43_REG_SCROLL_ANGLE               0x06CA  /* 1 byte  - Scroll angle [64tan(deg)] */
#define TPS43_REG_ZOOM_INITIAL_DISTANCE      0x06CB  /* 2 bytes - Zoom initial distance [pixels] */
#define TPS43_REG_ZOOM_CONSECUTIVE_DISTANCE  0x06CD  /* 2 bytes - Zoom consecutive distance [pixels] */

/* Filter settings */
// Read-write
#define TPS43_REG_FILTER_SETTINGS   0x0632  /* 1 byte */


/* ============================================================ */
/* System information 0 (0x000F) - Status flags - 8 bits */
/* ============================================================ */

#define TPS43_SHOW_RESET            BIT(7)      /* Flag check on system reset */
#define TPS43_CHARGING_MODE_MASK    0x07        /* Charging mode (bits 0-3) */

/* ============================================================ */
/* System information 1 (0x0010) - Status flags - 8 bits */
/* ============================================================ */

#define TPS43_SWITCH_STATE          BIT(5)      /* SW_IN pin state */
#define TPS43_SNAP_TOGGLE           BIT(4)      /* Snap channel toggle */
#define TPS43_RR_MISSED             BIT(3)      /* Report rate missed */
#define TPS43_TOO_MANY_FINGERS      BIT(2)      /* Maximum touch points exceeded */
#define TPS43_PALM_DETECT           BIT(1)      /* Palm detected */
#define TPS43_TP_MOVEMENT           BIT(0)      /* Trackpad movement */


/* ============================================================ */
/* System control 0 (0x0431) - 8 bits */
/* ============================================================ */

#define TPS43_ACK_RESET             BIT(7)      /* Reset acknowledgment */
#define TPS43_AUTO_ATI              BIT(5)      /* Automatic ATI mode enable */

/* ============================================================ */
/* System control 1 (0x0432) - 8 bits */
/* ============================================================ */

#define TPS43_RESET                 BIT(1)
#define TPS43_SUSPEND               BIT(0)      /* Sleep mode */


/* ============================================================ */
/* System configuration 0 (0x058E) - 8 bits */
/* ============================================================ */

#define TPS43_SETUP_COMPLETE        BIT(6)      /* Setup complete flag */
#define TPS43_WDT_ENABLE            BIT(5)      /* Watchdog timer enable */
#define TPS43_REATI                 BIT(2)      /* ALP auto reattempt */


/* ============================================================ */
/* System configuration 1 (0x058F) - 8 bits */
/* ============================================================ */

#define TPS43_EVENT_MODE            BIT(0)
#define TPS43_GESTURE_EVENT         BIT(1)
#define TPS43_TP_EVENT              BIT(2)
#define TPS43_REATI_EVENT           BIT(3)
#define TPS43_TOUCH_EVENT           BIT(6)


/* ============================================================ */
/* XY configuration 0 (0x0669) - 8 bits */
/* ============================================================ */

#define TPS43_FLIP_X                BIT(0)      /* Flip X axis */
#define TPS43_FLIP_Y                BIT(1)      /* Flip Y axis */
#define TPS43_SWITCH_XY_AXIS        BIT(2)      /* Swap X and Y axes */


/* ============================================================ */
/* Gesture events 0 (0x000D) - 8 bits */
/* ============================================================ */

#define TPS43_SINGLE_TAP            BIT(0)      /* Single finger tap detected */
#define TPS43_PRESS_AND_HOLD        BIT(1)      /* Press and hold detected */
#define TPS43_SWIPE_UP              BIT(2)      /* Single finger swipe up */
#define TPS43_SWIPE_DOWN            BIT(3)      /* Single finger swipe down */
#define TPS43_SWIPE_LEFT            BIT(4)      /* Single finger swipe left */
#define TPS43_SWIPE_RIGHT           BIT(5)      /* Single finger swipe right */

/* ============================================================ */
/* Gesture events 1 (0x000E) - 8 bits */
/* ============================================================ */

#define TPS43_TWO_FINGER_TAP        BIT(0)      /* Two finger tap */
#define TPS43_SCROLL                BIT(1)      /* Scroll up */
#define TPS43_ZOOM                  BIT(2)      /* Zoom gesture */

/* ============================================================ */
/* Filter settings 0 (0x0632) - 8 bits */
/* ============================================================ */

#define TPS43_IIR_FILTER           BIT(0)      /* IIR filter */
#define TPS43_MAV_FILTER           BIT(1)      /* MAV filter */
#define TPS43_IIR_SELECT           BIT(2)      /* IIR select */
#define TPS43_ALP_COUNT_FILTER     BIT(3)      /* ALP count filter */


/* End communication window - MUST be called after each read (writing to invalid address 0xEEEE causes NACK) */
#define TPS43_REG_END_COMM_WINDOW   0xEEEE

struct tps43_config {
    struct i2c_dt_spec i2c_bus;
    struct gpio_dt_spec rdy_gpio;
    struct gpio_dt_spec rst_gpio;

    bool single_tap;
    bool press_and_hold;
    bool two_finger_tap;
    bool scroll;
    bool zoom;
    bool swipes;
    bool three_finger_swipe;
    int16_t three_finger_swipe_throttle_ms;
    bool invert_x;
    bool invert_y;
    bool switch_xy;
    bool invert_scroll_x;
    bool invert_scroll_y;

    int16_t sensitivity;
    int16_t scroll_sensitivity;
    int16_t zoom_sensitivity;

    bool enable_power_management;
    bool idle_sleep;

    uint8_t filter_settings;
    int16_t filter_dynamic_bottom;
    int16_t filter_dynamic_lower;
    int16_t filter_dynamic_upper;
    int16_t x_resolution;
    int16_t y_resolution;
    int16_t swipe_initial_distance;
    int16_t swipe_initial_time;
    int16_t swipe_angle;
    int16_t swipe_consecutive_distance;
    int16_t swipe_consecutive_time;
    int16_t scroll_initial_distance;
    int16_t scroll_angle;
    int16_t zoom_initial_distance;
    int16_t zoom_consecutive_distance;

    int16_t ati_target;
    int16_t ref_drift_limit;
    int16_t reati_lower_limit;
    int16_t reati_upper_limit;
    int16_t max_count_limit;
    int16_t ati_retry_time;
    int16_t report_rate_active;
    int16_t report_rate_idle_touch;
    int16_t report_rate_idle;
    int16_t report_rate_lp1;
    int16_t report_rate_lp2;
    int16_t timeout_active;
    int16_t timeout_idle_touch;
    int16_t timeout_idle;
    int16_t timeout_lp1;
    int16_t ref_update_time;
    int16_t tap_time;
    int16_t tap_distance;
    int16_t hold_time;
};

struct tps43_drv_data {
    const struct device *dev;
    struct k_sem lock;
    struct gpio_callback rdy_cb;
    struct k_work work;

    bool device_ready;
    bool initialized;
    bool drag_active;
    bool suspended;
    bool touching;
    int64_t last_three_finger_swipe_ms;
};

int tps43_set_sleep(const struct device *dev, bool sleep);

#ifdef __cplusplus
}
#endif

