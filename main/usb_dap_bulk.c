#include "usb_dap_bulk.h"
#include "dap_configuration.h"
#include "esp_log.h"
#include "DAP.h"


#define URL "example.tinyusb.org/webusb-serial/index.html"
#define TAG "UBLK"

const tusb_desc_webusb_url_t desc_url = {.bLength = 3 + sizeof(URL) - 1,
                                         .bDescriptorType =
                                             3,        // WEBUSB URL type
                                         .bScheme = 1, // 0: http, 1: https
                                         .url = URL};

//--------------------------------------------------------------------+
// WebUSB use vendor class
//--------------------------------------------------------------------+

// Invoked when a control transfer occurred on an interface of this class
// Driver response accordingly to the request and the transfer stage
// (setup/data/ack) return false to stall control endpoint (e.g unsupported
// request)
bool dap_bulk_ctrl_request(uint8_t rhport, uint8_t stage,
                           tusb_control_request_t const *request) {

  ESP_LOGI(TAG, "Vendor Control Xfer");
  // nothing to with DATA & ACK stage
  if (stage != CONTROL_STAGE_SETUP)
    return true;

  switch (request->bmRequestType_bit.type) {
  case TUSB_REQ_TYPE_VENDOR:
    switch (request->bRequest) {
    case VENDOR_REQUEST_WEBUSB:
      // match vendor request in BOS descriptor
      // Get landing page url
      return tud_control_xfer(rhport, request, (void *)(uintptr_t)&desc_url,
                              desc_url.bLength);

    case VENDOR_REQUEST_MICROSOFT:
      if (request->wIndex == 7) {
        // Get Microsoft OS 2.0 compatible descriptor
        uint16_t total_len;
        memcpy(&total_len, desc_ms_os_20 + 8, 2);

        return tud_control_xfer(rhport, request,
                                (void *)(uintptr_t)desc_ms_os_20, total_len);
      } else {
        return false;
      }

    default:
      break;
    }
    break;

  case TUSB_REQ_TYPE_CLASS:
    if (request->bRequest == 0x22) {
      // // Webserial simulate the CDC_REQUEST_SET_CONTROL_LINE_STATE (0x22) to
      // connect and disconnect. web_serial_connected = (request->wValue != 0);

      // // Always lit LED if connected
      // if ( web_serial_connected )
      // {
      //   board_led_write(true);
      //   blink_interval_ms = BLINK_ALWAYS_ON;

      //   tud_vendor_write_str("\r\nWebUSB interface connected\r\n");
      //   tud_vendor_write_flush();
      // }else
      // {
      //   blink_interval_ms = BLINK_MOUNTED;
      //}

      // response with status OK
      return tud_control_status(rhport, request);
    }
    break;

  default:
    break;
  }

  // stall unknown request
  return false;
}

void dap_bulk_rx_cb(uint8_t itf) {
  uint8_t in[DAP_PACKET_SIZE];
  uint8_t out[DAP_PACKET_SIZE];

  uint32_t read = tud_vendor_n_read(itf, in, sizeof(in));
  //ESP_LOGI(TAG, "dap_bulk recevied size: %ld", read);
  if (read > 0) {
    uint32_t res = DAP_ProcessCommand(in, out);
    res = res & 0xFFFF;
    //ESP_LOGI(TAG, "dap_bulk response size: %ld", res);
    tud_vendor_n_write(itf,out, res);
    tud_vendor_n_flush(itf);
  }
}