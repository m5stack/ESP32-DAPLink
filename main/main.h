#pragma once

#include "driver/sdmmc_host.h"

typedef enum {
    STORAGE_SD = 0,
    STORAGE_FLASH,
    STORAGE_ERROR,
    STORAGE_MAX
} storage_status_t;

storage_status_t get_storage_status(void);
sdmmc_card_t* get_sdcard_status(void);
const char* get_base_path(void);
