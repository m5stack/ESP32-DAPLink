#include "power_measure.h"

#include "driver/i2c.h"
#include "esp_log.h"
#include <stdio.h>

static const char *TAG = "i2c";

#define I2C_MASTER_SCL_IO 45 /*!< GPIO number used for I2C master clock */
#define I2C_MASTER_SDA_IO 46 /*!< GPIO number used for I2C master data  */
#define I2C_MASTER_NUM                                                         \
  0 /*!< I2C master i2c port number, the number of i2c peripheral interfaces   \
       available will depend on the chip */
#define I2C_MASTER_FREQ_HZ 400000   /*!< I2C master clock frequency */
#define I2C_MASTER_TX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_RX_BUF_DISABLE 0 /*!< I2C master doesn't need buffer */
#define I2C_MASTER_TIMEOUT_MS 1000

#define INA219_ADDR 0x40 /*!< Slave address of the INA219 sensor */

static int i2c_master_port = I2C_MASTER_NUM;

static esp_err_t ina219_register_read(uint8_t reg_addr, int16_t *data) {
  uint8_t buff[2];
  esp_err_t err = i2c_master_write_read_device(
      I2C_MASTER_NUM, INA219_ADDR, &reg_addr, 1, &buff, 2,
      I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
  if (err == ESP_OK) {
    *data = (buff[0] << 8) | buff[1];
  }
  return err;
}

esp_err_t power_measure_init() {
  i2c_config_t conf = {
      .mode = I2C_MODE_MASTER,
      .sda_io_num = I2C_MASTER_SDA_IO,
      .scl_io_num = I2C_MASTER_SCL_IO,
      .sda_pullup_en = GPIO_PULLUP_ENABLE,
      .scl_pullup_en = GPIO_PULLUP_ENABLE,
      .master.clk_speed = I2C_MASTER_FREQ_HZ,
  };

  i2c_param_config(i2c_master_port, &conf);

  return i2c_driver_install(i2c_master_port, conf.mode,
                            I2C_MASTER_RX_BUF_DISABLE,
                            I2C_MASTER_TX_BUF_DISABLE, 0);
}

esp_err_t power_measure_read(power_state_t *state) {

  int16_t data[2] = {0};
  ESP_ERROR_CHECK(ina219_register_read(0x01, &data[0]));
  ESP_ERROR_CHECK(ina219_register_read(0x02, &data[1]));

  //ESP_LOGI(TAG, "0x01: %04x, 0x02: %04x", data[0], data[1]);

  state->bus_voltage = (data[1] >> 3) * 4;
  state->shunt_voltage = data[0] * 10;
  state->bus_current = state->shunt_voltage * 0.2;

  return ESP_OK;
}