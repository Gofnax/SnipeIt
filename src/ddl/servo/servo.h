#ifndef DDL_SERVO_H
#define DDL_SERVO_H

/* User library includes */
#include "ddl/ddl_frame.h"
#include "util/fsm/fsm.h"
#include "status.h"

/**
 * @brief   Initialize the PCA9685 PWM driver.
 * @param   frame A pointer to the DDLFrame that will hold all our sensor data.
 * @returns A value from @ref eStatus.
 * @retval  eSTATUS_SUCCESSFUL      successful execution
 * @retval  eSTATUS_NULL_PARAM      frame is NULL or the ServoObject is misconfigured
 * @retval  eSTATUS_SYSTEM_ERROR    thread or queue initalization failed
 */
eStatus ddl_servo_init(DDLFrame* frame);

#endif