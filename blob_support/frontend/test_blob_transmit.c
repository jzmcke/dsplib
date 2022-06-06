#include "blob/include/blob.h"
#include <assert.h>
#include <math.h>
#include <unistd.h>

int
main(int argc, char **argv)

{
    int up = 31;
    int down = 72;
    int forward_backward, left_right;
    int i = 0;
    BLOB_SOCKET_INIT("192.168.50.115", 8000);
    
    while (1)
    {
        int forward = 0;
        int backward = 0;
        int left = 0;
        int right = 0;
        int stop = 0;
        char c;

        c = getchar();

        if (c == 'w')
        {
            forward = 1;
        }
        else if (c == 's')
        {
            backward = 1;
        }
        else if (c == 'x')
        {
            stop = 1;
        }
        else if (c == 'a')
        {
            left = 1;
        }
        else if (c == 'd')
        {
            right = 1;
        }
        printf("Sending forward_backward: %d\n", forward_backward);
        BLOB_START("main");
        BLOB_INT_A("forward", &forward, 1);
        BLOB_INT_A("backward", &backward, 1);
        BLOB_INT_A("left", &left, 1);
        BLOB_INT_A("right", &right, 1);
        BLOB_INT_A("stop", &stop, 1);
        BLOB_FLUSH();
        usleep(100000); // 20ms
    }
    
    BLOB_SOCKET_TERMINATE();
    return 0;
}
