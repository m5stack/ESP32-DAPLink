#pragma once

#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include "sdkconfig.h"

void lfgx_write_pixels(void* param, uint32_t len);
void lfgx_set_addr_window(int32_t x, int32_t y, int32_t w, int32_t h);
void lfgx_start_write(void);
void lfgx_end_write(void);
uint8_t m5gfx_get_touch(uint16_t* x, uint16_t* y);
void beep(void);

#ifdef __cplusplus
}

#endif
