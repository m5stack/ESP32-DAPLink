#ifndef USB_BULK_H
#define USB_BULK_H

#include "tinyusb.h"

enum
{
  VENDOR_REQUEST_WEBUSB = 1,
  VENDOR_REQUEST_MICROSOFT = 2
};

extern uint8_t const desc_ms_os_20[];

#endif 