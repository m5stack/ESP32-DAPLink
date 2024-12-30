#include "app_display_c_api.h"
#include "M5GFX.h"

M5GFX display;

extern "C" void lfgx_init(void);
void lfgx_init(void)
{
    display.initBus();
    display.initDMA();
    display.begin();
}

extern "C" void lfgx_write_pixels(void* param, uint32_t len);
void lfgx_write_pixels(void* param, uint32_t len)
{
    display.writePixels((lgfx::rgb565_t *)param, len);
}

extern "C" void lfgx_set_addr_window(int32_t x, int32_t y, int32_t w, int32_t h);
void lfgx_set_addr_window(int32_t x, int32_t y, int32_t w, int32_t h)
{
    display.setAddrWindow(x, y, w, h);
}

extern "C" void lfgx_start_write(void);
void lfgx_start_write(void)
{
    display.startWrite();
}  

extern "C" void lfgx_end_write(void);
void lfgx_end_write(void)
{
    display.endWrite();
}  

extern "C" uint8_t m5gfx_get_touch(uint16_t* x, uint16_t* y);
uint8_t m5gfx_get_touch(uint16_t* x, uint16_t* y)
{
    return display.getTouch(x, y);
}
