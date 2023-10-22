#ifndef USB_DAP_BULK_H
#define USB_DAP_BULK_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "tinyusb.h"

enum
{
  VENDOR_REQUEST_WEBUSB = 1,
  VENDOR_REQUEST_MICROSOFT = 2
};

extern uint8_t const desc_ms_os_20[];

bool dap_bulk_ctrl_request(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request);
void dap_bulk_rx_cb(uint8_t itf);

#ifdef __cplusplus
extern "C"
}
#endif

#endif 