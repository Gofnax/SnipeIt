#include "servo_fsm.h"

/* Standard library includes */
#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <time.h>

/* User library includes */
#include "ddl/servo/servo_config.h"
#include "ddl/servo/servo_types.h"
#include "src/hal/i2c/hal_i2c.h"
#include "util/log/log.h"

static void sleep_us(long us)
{
    struct timespec ts;
    ts.tv_sec  = us / 1000000L;
    ts.tv_nsec = (us % 1000000L) * 1000L;
    nanosleep(&ts, NULL);
}

static float clampf(float x, float lo, float hi)
{
    if (x < lo) return lo;
    if (x > hi) return hi;
    return x;
}

static eStatus pca9685_write8(uint8_t reg, uint8_t value)
{
    return hal_i2c_write_reg(eSERVO_I2C_DEVICE, reg, 1, &value, 1);
}

static eStatus pca9685_read8(uint8_t reg, uint8_t* out_value)
{
    if (out_value == NULL) return eSTATUS_NULL_PARAM;
    return hal_i2c_read_reg(eSERVO_I2C_DEVICE, reg, 1, out_value, 1);
}

static eStatus pca9685_set_pwm_freq(float freq_hz)
{
    // prescale = round(oscillator_freq / (4096 * desired_freq)) - 1
    float prescale_float = (PCA9685_OSC_HZ / (PCA9685_PWM_STEPS * freq_hz)) - 1.0f;
    int prescale_int = (int)(prescale_float + 0.5f);
    if (prescale_int < PCA9685_PRESCALE_MIN)
        prescale_int = PCA9685_PRESCALE_MIN;
    if (prescale_int > PCA9685_PRESCALE_MAX)
        prescale_int = PCA9685_PRESCALE_MAX;
    uint8_t prescale = (uint8_t)prescale_int;

    uint8_t mode1_old;
    eStatus status = pca9685_read8(REG_MODE1, &mode1_old);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    uint8_t mode1_sleep = (mode1_old & ~MODE1_RESTART) | MODE1_SLEEP;
    status = pca9685_write8(REG_MODE1, mode1_sleep);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    status = pca9685_write8(REG_PRE_SCALE, prescale);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    status = pca9685_write8(REG_MODE1, mode1_old);
    if (status != eSTATUS_SUCCESSFUL)
        return status;

    /* Oscillator can take up to 500 us to stabilise */
    sleep_us(500);

    /* Set RESTART and AI on top of the original mode bits */
    return pca9685_write8(REG_MODE1, (uint8_t)(mode1_old | MODE1_RESTART | MODE1_AI));
}

static eStatus pca9685_init(void)
{
    // Moved to 'servo.c' under 'ddl_servo_init()'
}

/* Writes the 12-bit ON/OFF counts for a single channel in one I2C burst,
 * using the AI (auto-increment) feature. Layout of the 4 data bytes matches
 * LEDn_ON_L, LEDn_ON_H, LEDn_OFF_L, LEDn_OFF_H. */
static eStatus pca9685_set_pwm(uint8_t channel, uint16_t on, uint16_t off)
{
    if (channel > 15) return eSTATUS_INVALID_VALUE;

    /* Mask to 12 bits; bit 4 of the H bytes is the full-ON / full-OFF
     * flag, which we deliberately leave as 0. */
    uint8_t data[4];
    data[0] = (uint8_t)( on        & 0xFF);
    data[1] = (uint8_t)((on  >> 8) & 0x0F);
    data[2] = (uint8_t)( off       & 0xFF);
    data[3] = (uint8_t)((off >> 8) & 0x0F);

    return hal_i2c_write_reg(eSERVO_I2C_DEVICE, LEDn_BASE(channel), 1,
                             data, sizeof(data));
}

/*
 * Reads the 4 channel bytes back and decodes them into 12-bit ON/OFF counts.
 * Either output pointer may be NULL if that half isn't needed.
 */
static eStatus pca9685_get_pwm(uint8_t channel,
                               uint16_t* out_on, uint16_t* out_off)
{
    if (channel > 15) return eSTATUS_INVALID_VALUE;

    uint8_t data[4] = {0};
    eStatus st = hal_i2c_read_reg(eSERVO_I2C_DEVICE, LEDn_BASE(channel), 1,
                                  data, sizeof(data));
    if (st != eSTATUS_SUCCESSFUL) return st;

    if (out_on)  *out_on  = (uint16_t)(data[0] | ((data[1] & 0x0F) << 8));
    if (out_off) *out_off = (uint16_t)(data[2] | ((data[3] & 0x0F) << 8));
    return eSTATUS_SUCCESSFUL;
}

/* ============================ Servo-angle API ============================ */

/* Number of PWM counts per microsecond at the configured refresh rate. */
static float counts_per_us(void)
{
    float period_us = 1000000.0f / PWM_FREQ_HZ;
    return (float)PCA9685_PWM_STEPS / period_us;
}

static uint16_t pulse_us_to_counts(float pulse_us)
{
    float counts = pulse_us * counts_per_us();
    if (counts < 0.0f)                         counts = 0.0f;
    if (counts > (float)(PCA9685_PWM_STEPS-1)) counts = PCA9685_PWM_STEPS - 1;
    return (uint16_t)(counts + 0.5f);
}

static float counts_to_pulse_us(uint16_t counts)
{
    return (float)counts / counts_per_us();
}

/*
 * Commands the servo on @channel to @angle_deg (clamped to 0..180).
 * Leading edge of the pulse is anchored at count 0 (ON=0); pulse width
 * is encoded in the OFF count. This matches what the Adafruit library
 * does in its setPWM(n, 0, pulse) examples and what the datasheet
 * illustrates in figure 7/example 1.
 */
eStatus servo_set_angle(uint8_t channel, float angle_deg)
{
    angle_deg = clampf(angle_deg, SERVO_MIN_ANGLE_DEG, SERVO_MAX_ANGLE_DEG);

    float pulse_us =
        SERVO_MIN_PULSE_US +
        (angle_deg / (SERVO_MAX_ANGLE_DEG - SERVO_MIN_ANGLE_DEG)) *
        (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US);

    return pca9685_set_pwm(channel, 0, pulse_us_to_counts(pulse_us));
}

/*
 * Reads back whatever angle was last commanded for @channel by decoding
 * the PCA9685's OFF register. Returns eSTATUS_NULL_PARAM if out_angle_deg
 * is NULL. If the channel has never been written (POR state = full-OFF),
 * the returned angle is clamped to SERVO_MIN_ANGLE_DEG and is meaningless.
 */
eStatus servo_get_angle(uint8_t channel, float* out_angle_deg)
{
    if (out_angle_deg == NULL) return eSTATUS_NULL_PARAM;

    uint16_t off_counts = 0;
    eStatus  st = pca9685_get_pwm(channel, NULL, &off_counts);
    if (st != eSTATUS_SUCCESSFUL) return st;

    float pulse_us = counts_to_pulse_us(off_counts);
    float angle    =
        ((pulse_us - SERVO_MIN_PULSE_US) /
         (SERVO_MAX_PULSE_US - SERVO_MIN_PULSE_US)) *
        (SERVO_MAX_ANGLE_DEG - SERVO_MIN_ANGLE_DEG);

    *out_angle_deg = clampf(angle, SERVO_MIN_ANGLE_DEG, SERVO_MAX_ANGLE_DEG);
    return eSTATUS_SUCCESSFUL;
}

/* ================================= main ================================= */

#if 1
int main(void)
{
    printf("PCA9685 experiment: addr=0x%02X, PWM=%.0f Hz\n",
           eSERVO_PCA_ADDRESS, PWM_FREQ_HZ);

    if (pca9685_init() != eSTATUS_SUCCESSFUL)
    {
        fprintf(stderr, "PCA9685 init failed\n");
        return 1;
    }
    printf("PCA9685 initialised.\n");

    float angle = 0.0f;

    while(angle <= 180.0f)
    {
        printf("\n--> Commanding both servos to %.1f deg\n", angle);

        if (servo_set_angle(eSERVO_CHANNEL1, angle) != eSTATUS_SUCCESSFUL)
            fprintf(stderr, "    servo1 (ch %d) set failed\n", eSERVO_CHANNEL1);
        if (servo_set_angle(eSERVO_CHANNEL2, angle) != eSTATUS_SUCCESSFUL)
            fprintf(stderr, "    servo2 (ch %d) set failed\n", eSERVO_CHANNEL2);

        /* Let the servos physically swing before we check. 500 ms is plenty
         * for a 9g servo over the whole 180 deg sweep. */
        sleep_us(500 * 1000);

        float reported = 0.0f;
        if (servo_get_angle(eSERVO_CHANNEL1, &reported) == eSTATUS_SUCCESSFUL)
            printf("    servo1 read-back: %.1f deg\n", reported);
        if (servo_get_angle(eSERVO_CHANNEL2, &reported) == eSTATUS_SUCCESSFUL)
            printf("    servo2 read-back: %.1f deg\n", reported);

        sleep_us(500 * 1000);
        angle += 10.0f;
    }

    printf("Reseting servos back to 0 degrees\n");
    if (servo_set_angle(eSERVO_CHANNEL1, 0.0f) != eSTATUS_SUCCESSFUL)
        printf("    servo1 (ch %d) set failed\n", eSERVO_CHANNEL1);
    if (servo_set_angle(eSERVO_CHANNEL2, 0.0f) != eSTATUS_SUCCESSFUL)
        printf("    servo2 (ch %d) set failed\n", eSERVO_CHANNEL2);
    printf("Done reseting servos\n");

    sleep_us(500 * 1000);

    /*  Park the chip in SLEEP on the way out so the servos stop twitching.
        We can remove this line if we want the servos to keep "holding weight". */
    (void)pca9685_write8(REG_MODE1, MODE1_SLEEP | MODE1_ALLCALL);

    hal_i2c_cleanup();
    printf("\nDone.\n");
    return 0;
}
#endif
