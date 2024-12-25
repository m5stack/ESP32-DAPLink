#include "M5Unified.h"
#include "app_button_c_api.h"


extern "C" uint8_t is_button_a_press(void);
uint8_t is_button_a_press(void)
{
    if (M5.BtnA.isPressed()){
        return 1;
    }
    else {
        return 0;
    }
}

extern "C" uint8_t is_button_b_press(void);
uint8_t is_button_b_press(void)
{
    if (M5.BtnB.isPressed()){
        return 1;
    }
    else {
        return 0;
    }
}
extern "C" uint8_t is_button_c_press(void);
uint8_t is_button_c_press(void)
{
    if (M5.BtnC.isPressed()){
        return 1;
    }
    else {
        return 0;
    }
}
