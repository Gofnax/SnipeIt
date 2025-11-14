#ifndef HAL_GPIO_H
#define HAL_GPIO_H

/* User Libraries */
#include "../hal_types.h"

// GPIODevice mode values
#define GPIO_INPUT 0
#define GPIO_OUTPUT 1

// GPIODevice pull values
#define GPIO_NO_PULL 0
#define GPIO_PULLUP 1
#define GPIO_PULLDOWN 2

// GPIODevice edge values
#define GPIO_EDGE_NONE 0
#define GPIO_EDGE_RISING 1
#define GPIO_EDGE_FALLING 2
#define GPIO_EDGE_BOTH 3

#endif