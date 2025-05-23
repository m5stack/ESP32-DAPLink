/*
* Copyright 2024 NXP
* NXP Confidential and Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "events_init.h"
#include <stdio.h>
#include "lvgl.h"

#include "event_task.h"
#include "http_client.h"

#if LV_USE_FREEMASTER
#include "freemaster_client.h"
#endif

#include "esp_log.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "esp_vfs.h"
#include "esp_vfs_fat.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "app_display_c_api.h"

#define DIR_NUM_MAX 10 

static uint8_t is_flash_begin = 0;

static const char *TAG = "lvgl_event";

static void list_algorithm()
{
    const char *dir_alg = "/data/algorithm";
	uint32_t file_index = 0;

    ESP_LOGI(TAG, "Opening dir %s", dir_alg);

    DIR *dir_t1 = opendir(dir_alg);

	if (dir_t1) {
		int count = 0;
		const char* names[DIR_NUM_MAX];

		ESP_LOGI(TAG, "DIR: %s\n",dir_alg);
		lv_dropdown_clear_options(guider_ui.screen_ddlist_1);
		while(1) {
			struct dirent* de = readdir(dir_t1);
			if (!de) {
				break;
			}

			ESP_LOGI(TAG, "\t%s\n", de->d_name);
			char *tmp = NULL;
			if ((tmp = strstr(de->d_name, "\n"))) {
				*tmp = '\0';
			}		
			lv_dropdown_add_option(guider_ui.screen_ddlist_1, de->d_name, file_index);
			names[count] = de->d_name;
			++count;
			++file_index;
		}
	}
	closedir((DIR*)dir_alg);
}

static void list_program()
{
    const char *dir_alg = "/data/program";
	uint32_t file_index = 0;

    ESP_LOGI(TAG, "Opening dir %s", dir_alg);

    DIR *dir_t1 = opendir(dir_alg);

	if (dir_t1) {
		int count = 0;
		const char* names[DIR_NUM_MAX];

		ESP_LOGI(TAG, "DIR: %s\n",dir_alg);
		lv_dropdown_clear_options(guider_ui.screen_ddlist_2);
		while(1) {
			struct dirent* de = readdir(dir_t1);
			if (!de) {
				break;
			}

			ESP_LOGI(TAG, "\t%s\n", de->d_name);
			char *tmp = NULL;
			if ((tmp = strstr(de->d_name, "\n"))) {
				*tmp = '\0';
			}		
			lv_dropdown_add_option(guider_ui.screen_ddlist_2, de->d_name, file_index);
			names[count] = de->d_name;
			++count;
			++file_index;
		}
	}
	closedir((DIR*)dir_alg);
}

uint8_t get_flash_begin_status(void)
{
	return is_flash_begin;
}

void set_flash_begin_status(uint8_t status)
{
	is_flash_begin = status;
}

static lv_timer_t *daplink_update_data_task;
static void screen_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_SCREEN_LOADED:
	{
		list_algorithm();
		list_program();		

		uint64_t bytes_total = 0, bytes_free = 0;
		long int bytes_total_mb = 0, bytes_free_mb = 0;		
		esp_vfs_fat_info(CONFIG_TINYUSB_MSC_MOUNT_PATH, &bytes_total, &bytes_free);	
		bytes_total_mb = bytes_total / 1048576;
		bytes_free_mb = bytes_free / 1048576;		
		lv_label_set_text_fmt(guider_ui.screen_label_5, 
		"Flash\nTotal:%ldMB\nFree:%ldMB",
		bytes_total_mb, bytes_free_mb);				
		ESP_LOGI(TAG, "FAT FS: %" PRIu64 " kB total, %" PRIu64 " kB free", bytes_total / 1024, bytes_free / 1024);		
		daplink_update_data_task = lv_timer_create(screen_timer_cb, 50, &guider_ui);
		break;
	}
	case LV_EVENT_SCREEN_UNLOADED:
	{
		lv_timer_del(daplink_update_data_task);
		break;
	}
	default:
		break;
	}
}
static void screen_btn_1_event_handler (lv_event_t *e)
{
	lv_event_code_t code = lv_event_get_code(e);

	switch (code) {
	case LV_EVENT_CLICKED:
	{
		// start program
		beep();
		ESP_LOGI(TAG, "hello");
		start_swd_flash();
		set_flash_begin_status(1);
		lv_obj_set_style_bg_color(guider_ui.screen_btn_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
		lv_label_set_text(guider_ui.screen_btn_1_label, "Idle");
		lv_bar_set_value(guider_ui.screen_bar_1, 0, LV_ANIM_OFF);
		break;
	}
	default:
		break;
	}
}
void events_init_screen(lv_ui *ui)
{
	lv_obj_add_event_cb(ui->screen, screen_event_handler, LV_EVENT_ALL, NULL);
	lv_obj_add_event_cb(ui->screen_btn_1, screen_btn_1_event_handler, LV_EVENT_ALL, NULL);
}

void events_init(lv_ui *ui)
{

}
