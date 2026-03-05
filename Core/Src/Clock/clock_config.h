/*
 * clock_config.h - H735G-DK port
 */
#ifndef SRC_CLOCK_CLOCK_CONFIG_H_
#define SRC_CLOCK_CLOCK_CONFIG_H_

#include "stm32h7xx_hal.h"

void SystemClock_Config(void);
void MPU_Config(void);
void CPU_CACHE_Enable(void);

#endif /* SRC_CLOCK_CLOCK_CONFIG_H_ */