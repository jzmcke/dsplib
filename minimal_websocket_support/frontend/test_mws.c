#include "minimal_websocket/include/minimal_websocket.h"
#include <unistd.h>
#include <stdio.h>

int
main(int argc, char **argv)
{
    minimal_websocket_cfg cfg;
    char addr[] = "localhost";
    int port = 8000;
    unsigned char i=1;
    minimal_websocket *p_mws;

    cfg.addr = addr;
    cfg.port = port;
    cfg.max_tx_size = 16384;
    cfg.max_rx_size = 16384;
    minimal_websocket_init(&p_mws, &cfg);
    while (1)
    {
        minimal_websocket_set_send_data(p_mws, &i, 1);
        minimal_websocket_service(p_mws);
        i = (i+1) % 128;
        printf("%hhx\n", i);
        usleep(20000);
    }

    return 0;
}
