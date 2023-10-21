#ifndef POWER_MEASURE_H
#define POWER_MEASURE_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "esp_err.h"

typedef struct {
    int32_t bus_voltage; //in mV
    int32_t shunt_voltage; // in uV
    int32_t bus_current; //in uA
} power_state_t;


esp_err_t power_measure_init();
esp_err_t power_measure_read(power_state_t *state);


#endif 


#ifdef __cplusplus
}
#endif