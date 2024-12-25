#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

uint8_t is_button_a_press(void);
uint8_t is_button_b_press(void);
uint8_t is_button_c_press(void);

#ifdef __cplusplus
}

#endif
