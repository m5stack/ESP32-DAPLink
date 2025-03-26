#pragma once
#include<stdio.h>
 
#ifdef __cplusplus
extern "C"{
#endif

typedef enum {
    PROG_STATUS_IDLE = 0,
    PROG_STATUS_BUSY
} prog_status_t;
 
void update_prog_progress_and_status(void);
void start_swd_flash(void);
int get_rx_data_len(void);
int get_prog_progress(void);
prog_status_t get_prog_status(void);
void set_prog_progress(int progress);
void set_prog_status(prog_status_t status);
 
#ifdef __cplusplus
}
#endif
