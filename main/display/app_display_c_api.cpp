#include "app_display_c_api.h"
#include "M5Unified.h"


extern "C" void lfgx_write_pixels(void* param, uint32_t len);
void lfgx_write_pixels(void* param, uint32_t len)
{
    M5.Display.writePixels((lgfx::rgb565_t *)param, len);
}

extern "C" void lfgx_set_addr_window(int32_t x, int32_t y, int32_t w, int32_t h);
void lfgx_set_addr_window(int32_t x, int32_t y, int32_t w, int32_t h)
{
    M5.Display.setAddrWindow(x, y, w, h);
}

extern "C" void lfgx_start_write(void);
void lfgx_start_write(void)
{
    M5.Display.startWrite();
}  

extern "C" void lfgx_end_write(void);
void lfgx_end_write(void)
{
    M5.Display.endWrite();
}  

extern "C" uint8_t m5gfx_get_touch(uint16_t* x, uint16_t* y);
uint8_t m5gfx_get_touch(uint16_t* x, uint16_t* y)
{
    return M5.Display.getTouch(x, y);
}
