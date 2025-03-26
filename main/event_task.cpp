#include "event_task.h"
#include "esp_log.h"
#include "events_init.h"
#include "http_client.h"
#include "programmer.h"
#include "cJSON.h"

static const char *TAG = "event_task";

static uint8_t flash_status = 0;
static uint8_t last_flash_status = 0;
static TickType_t flash_counter = 0;

static void update_prog_progress_and_status_locally(void)
{
    char buffer[100] = {0};
    int encode_len = 0;

    programmer_get_status(buffer, 100, encode_len);
    ESP_LOGI(TAG, "%s", buffer);

    cJSON *root = NULL;
    root = cJSON_Parse(buffer);
    if (root) {
        cJSON *p = cJSON_GetObjectItem(root, "progress");
        if (p) {
            ESP_LOGI(TAG, "progress:%d", p->valueint);
            set_prog_progress(p->valueint);
        }
        cJSON *s = cJSON_GetObjectItem(root, "status");
        if (s) {
            ESP_LOGI(TAG, "status:%s", s->valuestring);
            if (strcmp(s->valuestring, "idle") == 0) {
                set_prog_status(PROG_STATUS_IDLE);
            }
            else if (strcmp(s->valuestring, "busy") == 0) {
                set_prog_status(PROG_STATUS_BUSY);
            }
        }
    }  
    cJSON_Delete(root); 
}

void screen_timer_cb(lv_timer_t *t) 
{
    flash_status = get_flash_begin_status();
    if (flash_status != last_flash_status) {
        if (flash_status) {
            flash_counter = xTaskGetTickCount();
        }
        last_flash_status = flash_status;
    }
    if (flash_status) {
        update_prog_progress_and_status_locally();
        prog_status_t s = get_prog_status();
        int p = get_prog_progress();
        if (s == PROG_STATUS_IDLE && p == 0) {
            if ((xTaskGetTickCount() - flash_counter) * portTICK_PERIOD_MS > 5000) {
                lv_obj_set_style_bg_color(guider_ui.screen_btn_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
                lv_label_set_text(guider_ui.screen_btn_1_label, "Idle");
                ESP_LOGI(TAG, "TIMEOUT");
                set_flash_begin_status(0);
            }
        }
        else if (s == PROG_STATUS_IDLE && (p > 0 && p < 100)) {
            if ((xTaskGetTickCount() - flash_counter) * portTICK_PERIOD_MS > 10000) {
                lv_obj_set_style_bg_color(guider_ui.screen_btn_1, lv_color_hex(0xff0000), LV_PART_MAIN|LV_STATE_DEFAULT);
                lv_label_set_text(guider_ui.screen_btn_1_label, "Failed");
                ESP_LOGI(TAG, "Failed");
                set_flash_begin_status(0);
            }
        }
        else {
            lv_bar_set_value(guider_ui.screen_bar_1, p, LV_ANIM_OFF);
            if (p >= 100) {
                lv_obj_set_style_bg_color(guider_ui.screen_btn_1, lv_color_hex(0x00660b), LV_PART_MAIN|LV_STATE_DEFAULT);
                lv_label_set_text(guider_ui.screen_btn_1_label, "Done");
                ESP_LOGI(TAG, "DONE");
                set_flash_begin_status(0);                
            }
            else {
                lv_obj_set_style_bg_color(guider_ui.screen_btn_1, lv_color_hex(0x000247), LV_PART_MAIN|LV_STATE_DEFAULT);
                lv_label_set_text(guider_ui.screen_btn_1_label, "Busy");
            }
        }
    }
    
    lv_label_set_text_fmt(guider_ui.screen_label_4, "%d,%d", get_prog_progress(), get_prog_status());
}