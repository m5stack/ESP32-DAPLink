#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

uint8_t is_button_press(void);

#ifdef __cplusplus
}

#endif
