#include "usb_desc.h"
#include "tusb_cdc_acm.h"
#include "usb_dap_bulk.h"

#ifdef USE_WINUSB
#define TUSB_DESC_WITH_MSC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN + TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN)
#define TUSB_DESC_WITHOUT_MSC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN)
#else
#define TUSB_DESC_WITH_MSC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#define TUSB_DESC_WITHOUT_MSC_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN)
#endif

static uint8_t const desc_hid_dap_report[] = {
    TUD_HID_REPORT_DESC_GENERIC_INOUT(CFG_TUD_HID_EP_BUFSIZE)};

static uint8_t const desc_configuration_with_msc[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_WITH_MSC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, STRID_CDC_INTERFACE, EDPT_CDC_NOTIFY, 8, EDPT_CDC_OUT, EDPT_CDC_IN, CFG_TUD_CDC_EP_BUFSIZE),
#ifdef USE_WINUSB
    // Interface number, string index, EP Out & IN address, EP size
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_BULK, STRID_BULK_INTERFACE, EDPT_BULK_OUT, EDPT_BULK_IN, TUD_OPT_HIGH_SPEED ? 512 : 64),
#else
    // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, STRID_HID_INTERFACE, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_dap_report), EDPT_HID_OUT, EDPT_HID_IN, CFG_TUD_HID_EP_BUFSIZE, 1),
#endif
    // Interface number, string index, EP Out & EP In address, EP size
    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, STRID_MSC_INTERFACE, EDPT_MSC_OUT, EDPT_MSC_IN, TUD_OPT_HIGH_SPEED ? 512 : 64)};

static char const *string_desc_arr_with_msc[] = {
    (const char[]){0x09, 0x04},              // 0: is supported language is English (0x0409)
    CONFIG_TINYUSB_DESC_MANUFACTURER_STRING, // 1: Manufacturer
    CONFIG_TINYUSB_DESC_PRODUCT_STRING,      // 2:  The value of this macro _must_ include the string "CMSIS-DAP". Otherwise debuggers will not recognizethe USB device
    CONFIG_TINYUSB_DESC_SERIAL_STRING,       // 3. SN
    CONFIG_TINYUSB_DESC_CDC_STRING,          // 4. CDC
#ifdef USE_WINUSB
    CONFIG_TINYUSB_DESC_HID_STRING,          // 5. BULK //TODO CONFIG
#else
    CONFIG_TINYUSB_DESC_HID_STRING,          // 5. HID  
#endif
    CONFIG_TINYUSB_DESC_MSC_STRING           // 6. MSC
};

static uint8_t const desc_configuration_without_msc[] = {
    // Config number, interface count, string index, total length, attribute, power in mA
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, TUSB_DESC_WITHOUT_MSC_TOTAL_LEN, TUSB_DESC_CONFIG_ATT_REMOTE_WAKEUP, 100),
    // Interface number, string index, EP notification address and size, EP data address (out, in) and size.
    TUD_CDC_DESCRIPTOR(ITF_NUM_CDC, STRID_CDC_INTERFACE, EDPT_CDC_NOTIFY, 8, EDPT_CDC_OUT, EDPT_CDC_IN, CFG_TUD_CDC_EP_BUFSIZE),
#ifdef USE_WINUSB
    // Interface number, string index, EP Out & IN address, EP size
    TUD_VENDOR_DESCRIPTOR(ITF_NUM_BULK, STRID_BULK_INTERFACE, EDPT_BULK_OUT, EDPT_BULK_IN, TUD_OPT_HIGH_SPEED ? 512 : 64),
#else
    // Interface number, string index, protocol, report descriptor len, EP In & Out address, size & polling interval
    TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, STRID_HID_INTERFACE, HID_ITF_PROTOCOL_NONE, sizeof(desc_hid_dap_report), EDPT_HID_OUT, EDPT_HID_IN, CFG_TUD_HID_EP_BUFSIZE, 1),
#endif
};

static char const *string_desc_arr_without_msc[] = {
    (const char[]){0x09, 0x04},              // 0: is supported language is English (0x0409)
    CONFIG_TINYUSB_DESC_MANUFACTURER_STRING, // 1: Manufacturer
    CONFIG_TINYUSB_DESC_PRODUCT_STRING,      // 2:  The value of this macro _must_ include the string "CMSIS-DAP". Otherwise debuggers will not recognizethe USB device
    CONFIG_TINYUSB_DESC_SERIAL_STRING,       // 3. SN
    CONFIG_TINYUSB_DESC_CDC_STRING,          // 4. CDC
#ifdef USE_WINUSB
    CONFIG_TINYUSB_DESC_HID_STRING,          // 5. BULK //TODO CONFIG
#else
    CONFIG_TINYUSB_DESC_HID_STRING,          // 5. HID
#endif
};


// WINUSB extra descriptor

#ifdef USE_WINUSB

#define BOS_TOTAL_LEN      (TUD_BOS_DESC_LEN + TUD_BOS_WEBUSB_DESC_LEN + TUD_BOS_MICROSOFT_OS_DESC_LEN)

#define MS_OS_20_DESC_LEN  0xB2


// BOS Descriptor is required for webUSB
uint8_t const desc_bos[] =
{
      // total length, number of device caps
  TUD_BOS_DESCRIPTOR(BOS_TOTAL_LEN, 2),

  // Vendor Code, iLandingPage
  TUD_BOS_WEBUSB_DESCRIPTOR(VENDOR_REQUEST_WEBUSB, 1),

  // Microsoft OS 2.0 descriptor
  TUD_BOS_MS_OS_20_DESCRIPTOR(MS_OS_20_DESC_LEN, VENDOR_REQUEST_MICROSOFT)
};

uint8_t const * tud_descriptor_bos_cb(void)
{
  return desc_bos;
}


uint8_t const desc_ms_os_20[] =
{
  // Set header: length, type, windows version, total length
  U16_TO_U8S_LE(0x000A), U16_TO_U8S_LE(MS_OS_20_SET_HEADER_DESCRIPTOR), U32_TO_U8S_LE(0x06030000), U16_TO_U8S_LE(MS_OS_20_DESC_LEN),

  // Configuration subset header: length, type, configuration index, reserved, configuration total length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_CONFIGURATION), 0, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A),

  // Function Subset header: length, type, first interface, reserved, subset length
  U16_TO_U8S_LE(0x0008), U16_TO_U8S_LE(MS_OS_20_SUBSET_HEADER_FUNCTION), ITF_NUM_BULK, 0, U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08),

  // MS OS 2.0 Compatible ID descriptor: length, type, compatible ID, sub compatible ID
  U16_TO_U8S_LE(0x0014), U16_TO_U8S_LE(MS_OS_20_FEATURE_COMPATBLE_ID), 'W', 'I', 'N', 'U', 'S', 'B', 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // sub-compatible

  // MS OS 2.0 Registry property descriptor: length, type
  U16_TO_U8S_LE(MS_OS_20_DESC_LEN-0x0A-0x08-0x08-0x14), U16_TO_U8S_LE(MS_OS_20_FEATURE_REG_PROPERTY),
  U16_TO_U8S_LE(0x0007), U16_TO_U8S_LE(0x002A), // wPropertyDataType, wPropertyNameLength and PropertyName "DeviceInterfaceGUIDs\0" in UTF-16
  'D', 0x00, 'e', 0x00, 'v', 0x00, 'i', 0x00, 'c', 0x00, 'e', 0x00, 'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00,
  'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00, 'G', 0x00, 'U', 0x00, 'I', 0x00, 'D', 0x00, 's', 0x00, 0x00, 0x00,
  U16_TO_U8S_LE(0x0050), // wPropertyDataLength
	//bPropertyData: “{975F44D9-0D08-43FD-8B3E-127CA8AFFF9D}”. //{CDB3B5AD-293B-4663-AA36-1AAE46463776}
//   '{', 0x00, '9', 0x00, '7', 0x00, '5', 0x00, 'F', 0x00, '4', 0x00, '4', 0x00, 'D', 0x00, '9', 0x00, '-', 0x00,
//   '0', 0x00, 'D', 0x00, '0', 0x00, '8', 0x00, '-', 0x00, '4', 0x00, '3', 0x00, 'F', 0x00, 'D', 0x00, '-', 0x00,
//   '8', 0x00, 'B', 0x00, '3', 0x00, 'E', 0x00, '-', 0x00, '1', 0x00, '2', 0x00, '7', 0x00, 'C', 0x00, 'A', 0x00,
//   '8', 0x00, 'A', 0x00, 'F', 0x00, 'F', 0x00, 'F', 0x00, '9', 0x00, 'D', 0x00, '}', 0x00, 0x00, 0x00, 0x00, 0x00

  '{', 0x00 ,'C', 0x00 ,'D', 0x00 ,'B', 0x00 ,'3', 0x00 ,'B', 0x00 ,'5', 0x00 ,'A', 0x00 ,'D', 0x00 ,'-', 0x00 ,
  '2', 0x00 ,'9', 0x00 ,'3', 0x00 ,'B', 0x00 ,'-', 0x00 ,'4', 0x00 ,'6', 0x00 ,'6', 0x00 ,'3', 0x00 ,'-', 0x00 ,
  'A', 0x00 ,'A', 0x00 ,'3', 0x00 ,'6', 0x00 ,'-', 0x00 ,'1', 0x00 ,'A', 0x00 ,'A', 0x00 ,'E', 0x00 ,'4', 0x00 ,
  '6', 0x00 ,'4', 0x00 ,'6', 0x00 ,'3', 0x00 ,'7', 0x00 ,'7', 0x00 ,'6', 0x00 ,'}', 0x00, 0x00, 0x00, 0x00, 0x00
};

TU_VERIFY_STATIC(sizeof(desc_ms_os_20) == MS_OS_20_DESC_LEN, "Incorrect size");
#endif 

static tusb_desc_device_t descriptor_config = {
    .bLength = sizeof(descriptor_config),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0210,
    .bDeviceClass = TUSB_CLASS_MISC,
    .bDeviceSubClass = MISC_SUBCLASS_COMMON,
    .bDeviceProtocol = MISC_PROTOCOL_IAD,
    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,
    .idVendor = CONFIG_TINYUSB_DESC_CUSTOM_VID,
    .idProduct = CONFIG_TINYUSB_DESC_CUSTOM_PID,
    .bcdDevice = CONFIG_TINYUSB_DESC_BCD_DEVICE,
    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,
    .bNumConfigurations = 0x01};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    return desc_hid_dap_report;
}

tusb_desc_device_t *get_device_descriptor(void)
{
    return &descriptor_config;
}

const char **get_string_descriptor(bool with_msc)
{
    return with_msc ? string_desc_arr_with_msc : string_desc_arr_without_msc;
}

int get_string_descriptor_count(bool with_msc)
{
    return with_msc ? (sizeof(string_desc_arr_with_msc) / sizeof(char *)) : (sizeof(string_desc_arr_without_msc) / sizeof(char *));
}

const uint8_t *get_configuration_descriptor(bool with_msc)
{
    return with_msc ? desc_configuration_with_msc : desc_configuration_without_msc;
}

bool tud_vendor_control_xfer_cb(uint8_t rhport, uint8_t stage, tusb_control_request_t const * request){
    return dap_bulk_ctrl_request(rhport, stage, request);
}

void tud_vendor_rx_cb(uint8_t itf){
    dap_bulk_rx_cb(itf);
}