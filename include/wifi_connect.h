#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "esp_event.h"

struct {
    struct arg_lit *connect;
    struct arg_lit *disconnect;
    struct arg_lit *ip;
    struct arg_end *end;
} wificonnect_args;
int get_wificonnect(int argc, char **argv);
void reg_get_wificonnect(void);

esp_err_t event_handler(void *ctx, system_event_t *event);

void connect(void);
void disconnect(void);
void print_ip(void);

#ifdef __cplusplus
}
#endif