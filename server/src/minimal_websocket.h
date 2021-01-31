#include <stdlib.h>

typedef struct minimal_websocket_s minimal_websocket;

typedef struct minimal_websocket_cfg
{
    char *addr;
    int port;
    void *main_thread;
} minimal_websocket_cfg;

int
minimal_websocket_init(minimal_websocket **pp_self, minimal_websocket_cfg *p_cfg);

void
minimal_websocket_set_data(minimal_websocket *p_self, unsigned char *p_data, size_t n_bytes);