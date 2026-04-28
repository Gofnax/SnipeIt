#include "servo.h"

/* Standard library includes */
#include <stddef.h>

/* User library includes */
#include "util/active_object/active_object.h"
#include "ddl/servo/servo_config.h"
#include "ddl/servo/servo_fsm.h"
#include "osal/osal.h"

static ServoObject servo_aobj;

eStatus ddl_servo_init(DDLFrame* frame)
{
    eStatus status = hal_i2c_init();
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    status = hal_i2c_set_address(eSERVO_I2C_DEVICE, eSERVO_PCA_ADDRESS);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    /* MODE2: totem-pole, non-inverting, change-on-STOP */
    status = pca9685_write8(REG_MODE2, MODE2_OUTDRV);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    /* MODE1: clear SLEEP so oscillator runs, keep ALLCALL for compatibility.
     * AI and RESTART get set inside pca9685_set_pwm_freq. */
    status = pca9685_write8(REG_MODE1, MODE1_ALLCALL);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    sleep_us(500);

    return pca9685_set_pwm_freq(PWM_FREQ_HZ);
}