#ifndef BUZZ_H
#define BUZZ_H

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>

void buzz_init(void);
void buzz_beep(uint32_t duration_ms);



#ifdef __cplusplus
}
#endif


#endif 