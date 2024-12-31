/*
 * Copyright (c) 2023-2023, lihongquan
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Change Logs:
 * Date           Author       Notes
 * 2023-9-8      lihongquan   add license declaration
 */

#include <stdint.h>
#include "esp_log.h"
#include <errno.h>
#include <dirent.h>
#include <esp_wifi.h>
#include <esp_event.h>
#include <nvs_flash.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"
#include "DAP_config.h"
#include "DAP.h"
#include "esp_netif.h"
#include "web_handler.h"
#include "esp_http_server.h"
#include "web_server.h"
#include "programmer.h"
#include "protocol_examples_common.h"
#include "driver/gpio.h"
#include "power_measure.h"
#include "buzz.h"

#include "M5Unified.h"
#include "M5GFX.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "lvgl.h"
#include "gui_guider.h"
#include "app_display_c_api.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include <sys/unistd.h>
#include <sys/stat.h>
#include "sdmmc_cmd.h"
#include "driver/sdmmc_host.h"

#include "main.h"

#define EXAMPLE_ESP_WIFI_SSID      "ATOMS3R-DAP"
#define EXAMPLE_ESP_WIFI_PASS      "12345678"
#define EXAMPLE_ESP_WIFI_CHANNEL   1
#define EXAMPLE_MAX_STA_CONN       1

#define EXAMPLE_LVGL_TICK_PERIOD_MS     2               /*!< LVGL tick period in ms */
#define EXAMPLE_LVGL_TASK_MAX_DELAY_MS  500
#define EXAMPLE_LVGL_TASK_MIN_DELAY_MS  1
#define EXAMPLE_LVGL_TASK_STACK_SIZE    (8 * 1024)
#define EXAMPLE_LVGL_TASK_PRIORITY      2

#define LV_TICK_PERIOD_MS 2

#define SDSPI_CS GPIO_NUM_4
#define SDSPI_SCK GPIO_NUM_18
#define SDSPI_MOSI GPIO_NUM_23
#define SDSPI_MISO GPIO_NUM_38

#define EXAMPLE_MAX_CHAR_SIZE    64

static const char *TAG = "main";
static httpd_handle_t http_server = NULL;
TaskHandle_t kDAPTaskHandle = NULL;
SemaphoreHandle_t xGuiSemaphore;
TaskHandle_t task_lvgl_handle    = NULL;
// Mount path for the partition
const char *base_path = "/data";
// Handle of the wear levelling library instance
static wl_handle_t s_wl_handle = WL_INVALID_HANDLE;
static storage_status_t storage_mount_status = STORAGE_SD;
static sdmmc_card_t *card;

extern "C" void tcp_server_task(void *pvParameters);
extern "C" void DAP_Thread(void *pvParameters);

extern "C" void ui_main(void);

static void disconnect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    web_server_stop((httpd_handle_t *)arg);
}

static void connect_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    web_server_init((httpd_handle_t *)arg);
}

storage_status_t get_storage_status(void)
{
    return storage_mount_status;
}

sdmmc_card_t* get_sdcard_status(void)
{
    return card;
}

const char* get_base_path(void)
{
    return base_path;
}

static esp_err_t mount_flash_as_fat(void)
{
    // To mount device we need name of device partition, define base_path
    // and allow format partition in case if it is new one and was not formatted before
    esp_vfs_fat_mount_config_t mount_config = {0};
    mount_config.max_files = 4;
    mount_config.format_if_mount_failed = false;
    mount_config.allocation_unit_size = CONFIG_WL_SECTOR_SIZE;

    esp_err_t err = esp_vfs_fat_spiflash_mount_rw_wl(base_path, "storage", &mount_config, &s_wl_handle);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Failed to mount FATFS (%s)", esp_err_to_name(err));
        return err;
    } 

    // Print FAT FS size information
    uint64_t bytes_total, bytes_free;
    esp_vfs_fat_info(base_path, &bytes_total, &bytes_free);
    ESP_LOGI(TAG, "FAT FS: %" PRIu64 " kB total, %" PRIu64 " kB free", bytes_total / 1024, bytes_free / 1024);

    return err; 
}

 extern "C"  void wifi_init_softap(void)
{
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_ap();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_START, connect_handler, &http_server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_AP_STOP, disconnect_handler, &http_server));

    wifi_config_t wifi_config;
    esp_wifi_get_config(WIFI_IF_AP, &wifi_config);
    // wifi_config.ap.ssid = EXAMPLE_ESP_WIFI_SSID;
    memcpy(wifi_config.ap.ssid, EXAMPLE_ESP_WIFI_SSID, strlen(EXAMPLE_ESP_WIFI_SSID));
    wifi_config.ap.ssid_len = strlen(EXAMPLE_ESP_WIFI_SSID);
    wifi_config.ap.channel = EXAMPLE_ESP_WIFI_CHANNEL;
    // wifi_config.ap.password = EXAMPLE_ESP_WIFI_PASS;
    memcpy(wifi_config.ap.password, EXAMPLE_ESP_WIFI_PASS, strlen(EXAMPLE_ESP_WIFI_PASS));
    wifi_config.ap.max_connection = EXAMPLE_MAX_STA_CONN;
    wifi_config.ap.pmf_cfg.required = true;

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "wifi_init_softap finished. SSID:%s password:%s channel:%d",
             EXAMPLE_ESP_WIFI_SSID, EXAMPLE_ESP_WIFI_PASS, EXAMPLE_ESP_WIFI_CHANNEL);
}

bool example_lvgl_lock(int timeout_ms)
{
    const TickType_t timeout_ticks = (timeout_ms == -1) ? portMAX_DELAY : pdMS_TO_TICKS(timeout_ms);
    return xSemaphoreTake(xGuiSemaphore, timeout_ticks) == pdTRUE;
}

void example_lvgl_unlock(void)
{
    xSemaphoreGive(xGuiSemaphore);
}

static void example_lvgl_port_task(void *arg)
{
    uint32_t task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
    setup_ui(&guider_ui);
    while (1) {
        /* Lock the mutex due to the LVGL APIs are not thread-safe */
        if (example_lvgl_lock(-1)) {
            M5.update();
            task_delay_ms = lv_timer_handler();
            /* Release the mutex */
            example_lvgl_unlock();
        }
        if (task_delay_ms > EXAMPLE_LVGL_TASK_MAX_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MAX_DELAY_MS;
        } else if (task_delay_ms < EXAMPLE_LVGL_TASK_MIN_DELAY_MS) {
            task_delay_ms = EXAMPLE_LVGL_TASK_MIN_DELAY_MS;
        }
        vTaskDelay(pdMS_TO_TICKS(task_delay_ms));
    }
}

static void lv_tick_task(void *arg)
{
    (void)arg;

    lv_tick_inc(LV_TICK_PERIOD_MS);
}

 extern "C" void app_main(void)
{
    esp_err_t ret;

    M5.begin();
    M5.Display.begin();
    M5.Power.begin();
    M5.Speaker.begin();
    M5.Speaker.setVolume(100);

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_LOGI(TAG, "Mounting FAT filesystem");
    ESP_LOGI(TAG, "Using SPI peripheral");

    sdmmc_host_t host = SDSPI_HOST_DEFAULT();

    host.max_freq_khz = 80000;
    host.command_timeout_ms = 10000;

    spi_bus_config_t bus_cfg = {
        .mosi_io_num = SDSPI_MOSI,
        .miso_io_num = SDSPI_MISO,
        .sclk_io_num = SDSPI_SCK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = 80000,
    };

    host.slot = VSPI_HOST;

    sdspi_device_config_t slot_config = SDSPI_DEVICE_CONFIG_DEFAULT();
    slot_config.gpio_cs = SDSPI_CS;
    slot_config.host_id = (spi_host_device_t)host.slot;    

    // Options for mounting the filesystem.
    // If format_if_mount_failed is set to true, SD card will be partitioned and
    // formatted in case when mounting fails.
    esp_vfs_fat_sdmmc_mount_config_t mount_config = {0};
    mount_config.max_files = 5;
    mount_config.format_if_mount_failed = false;
    mount_config.allocation_unit_size = 16 * 1024;    

    ESP_LOGI(TAG, "Mounting SDSPI");
    ret = esp_vfs_fat_sdspi_mount(base_path, &host, &slot_config, &mount_config, &card);  

    if (ret != ESP_OK) {
        if (ret == ESP_FAIL) {
            ESP_LOGE(TAG, "Failed to mount filesystem. "
                     "If you want the card to be formatted, set the EXAMPLE_FORMAT_IF_MOUNT_FAILED menuconfig option.");
        } else {
            ESP_LOGE(TAG, "Failed to initialize the card (%s). "
                     "Make sure SD card lines have pull-up resistors in place.", esp_err_to_name(ret));
        }
        ESP_LOGI(TAG, "Mounting flash fat");
        ret = mount_flash_as_fat();

        if (ret != ESP_OK) {
            storage_mount_status = STORAGE_ERROR;
            ESP_LOGE(TAG, "Failed mount flash fat");
        }
        else {
            storage_mount_status = STORAGE_FLASH;
            ESP_LOGI(TAG, "Flash fat Filesystem mounted");
        }
    }
    else {
        storage_mount_status = STORAGE_SD;
        ESP_LOGI(TAG, "SDSPI Filesystem mounted");
        // Card has been initialized, print its properties
        sdmmc_card_print_info(stdout, card);        
    }   

    wifi_init_softap();

    DAP_Setup();

    programmer_init();

    // DAP handle task
    xTaskCreate(DAP_Thread, "DAP_Task", 2048, NULL, 10, &kDAPTaskHandle);
    
    //ui main
    lv_init();
    lv_port_disp_init(); 
    lv_port_indev_init();
    ESP_LOGI(TAG, "Install LVGL tick timer");
    xGuiSemaphore = xSemaphoreCreateMutex();
    const esp_timer_create_args_t periodic_timer_args = {
        .callback = &lv_tick_task, .name = "periodic_gui"};
    esp_timer_handle_t periodic_timer;
    ESP_ERROR_CHECK(esp_timer_create(&periodic_timer_args, &periodic_timer));
    ESP_ERROR_CHECK(
        esp_timer_start_periodic(periodic_timer, LV_TICK_PERIOD_MS * 1000));

    xTaskCreate(example_lvgl_port_task, "LVGL", EXAMPLE_LVGL_TASK_STACK_SIZE, NULL, EXAMPLE_LVGL_TASK_PRIORITY, 
    &task_lvgl_handle);     
}
