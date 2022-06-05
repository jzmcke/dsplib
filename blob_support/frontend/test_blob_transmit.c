#include "blob/include/blob.h"
#include <assert.h>
#include <math.h>
#include <unistd.h>

int
main(int argc, char **argv)
{
    int up = 31;
    int down = 72;
    BLOB_SOCKET_INIT("172.21.131.23", 8000);
    
    while (1)
    {
        BLOB_START("main");
        BLOB_INT_A("up", &up, 1);
        BLOB_INT_A("down", &down, 1);
        BLOB_FLUSH();
        usleep(20000); // 20ms
    }
    
    BLOB_SOCKET_TERMINATE();
    return 0;
}
