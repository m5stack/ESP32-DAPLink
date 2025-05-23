/*
 * SPDX-FileCopyrightText: 2021-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: CC0-1.0
 */

// This demo UI is adapted from LVGL official example:
// https://docs.lvgl.io/master/widgets/extra/meter.html#simple-meter

#include "esp_log.h"
#include "esp_netif.h"
#include "lvgl.h"
#include "power_measure.h"

static lv_obj_t *meter;
static lv_obj_t *btn;
static lv_disp_rot_t rotation = LV_DISP_ROT_270;

static char *TAG = "ui";
lv_timer_t *lvgl_pwr_timer;
lv_obj_t *lv_pwr_label;
lv_obj_t *lv_wifi_label;

void lvgl_pwr_timer_cb(lv_timer_t *tmr) {
  power_state_t state = {0};
  power_measure_read(&state);
  lv_label_set_text_fmt(lv_pwr_label, "%0.2f V / %0.2f mA",
                        state.bus_voltage / 1000.0f,
                        state.bus_current / 1000.0f);
  // ESP_LOGI(TAG, "power: bus_voltage: %ld mV, shunt_voltage %ld uV,
  // bus_current: %ld uA",
  // state.bus_voltage, state.shunt_voltage, state.bus_current);
}

static void set_value(void *indic, int32_t v) {
  lv_meter_set_indicator_end_value(meter, indic, v);
}

static void btn_cb(lv_event_t *e) {
  lv_disp_t *disp = lv_event_get_user_data(e);
  rotation++;
  if (rotation > LV_DISP_ROT_270) {
    rotation = LV_DISP_ROT_NONE;
  }
  lv_disp_set_rotation(disp, rotation);
}

static void btn_program_cb(lv_event_t *e) {
  
}

void example_lvgl_demo_ui(lv_disp_t *disp) {
  lv_obj_t *scr = lv_disp_get_scr_act(disp);

  lv_pwr_label = lv_label_create(scr);
  lv_label_set_text(lv_pwr_label, "Power: 0 mW");
  lv_obj_align(lv_pwr_label, LV_ALIGN_LEFT_MID, 0, 0);

  lv_wifi_label = lv_label_create(scr);
  lv_label_set_text(lv_pwr_label, "IP: - ");
  lv_obj_align_to(lv_wifi_label, lv_pwr_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

  esp_netif_t *netif = NULL;
  esp_netif_ip_info_t ip;
  for (int i = 0; i < esp_netif_get_nr_of_ifs(); ++i) {
    netif = esp_netif_next(netif);
    esp_netif_get_ip_info(netif, &ip);
    lv_label_set_text_fmt(lv_wifi_label, "IP: " IPSTR, IP2STR(&ip.ip));
  }

  btn = lv_btn_create(scr);
  lv_obj_t *lbl = lv_label_create(btn);
  lv_label_set_text_static(lbl, LV_SYMBOL_REFRESH " ROTATE");
  lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 30, -10);
  /*Button event*/
  lv_obj_add_event_cb(btn, btn_cb, LV_EVENT_CLICKED, disp);

  btn = lv_btn_create(scr);
  lv_wifi_label = lv_label_create(btn);
  lv_label_set_text_static(lv_wifi_label, LV_SYMBOL_REFRESH " Program");
  lv_obj_align(btn, LV_ALIGN_BOTTOM_LEFT, 150, -10);
  /*Button event*/
  lv_obj_add_event_cb(btn, btn_program_cb, LV_EVENT_CLICKED, disp);

  lvgl_pwr_timer = lv_timer_create(lvgl_pwr_timer_cb, 500, 0);
}
