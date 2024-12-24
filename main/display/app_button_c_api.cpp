#include "M5Unified.h"
#include "app_button_c_api.h"


extern "C" uint8_t is_button_press(void);
uint8_t is_button_press(void)
{
    if (M5.BtnA.isPressed()){
        return 1;
    }
    else {
        return 0;
    }
}
