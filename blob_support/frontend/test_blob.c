#include "blob/include/blob.h"
#include <assert.h>

#define NUM_IN_FIRST_ARRAY  (4)
#define NUM_IN_SECOND_ARRAY (7)
#define NUM_IN_THIRD_ARRAY (170)

void
func_top(void)
{
    int ret;
    int a_array_first[NUM_IN_FIRST_ARRAY];
    int a_array_second[NUM_IN_SECOND_ARRAY];
    float a_array_third[NUM_IN_THIRD_ARRAY];
    
    static int start = 0;
    BLOB_START("top");
    for (int i=0; i<NUM_IN_FIRST_ARRAY; i++)
    {
        a_array_first[i] = start + i;
    }
    for (int i=0; i<NUM_IN_SECOND_ARRAY; i++)
    {
        a_array_second[i] = start + i;
    }
    for (int i=0; i<NUM_IN_THIRD_ARRAY; i++)
    {
        a_array_third[i] = (float)(start + i);
    }
    ret = BLOB_INT_A("first_array", a_array_first, NUM_IN_FIRST_ARRAY);
    assert(ret==0);
    ret = BLOB_INT_A("second_array", a_array_second, NUM_IN_SECOND_ARRAY);
    assert(ret==0);
    BLOB_FLUSH();
    start = start + 100;
}

void
func_mid(void)
{
    int ret;
    int integer_val = -1;
    unsigned int unsigned_int_val = 4;
    float a_array_first[NUM_IN_FIRST_ARRAY];

    BLOB_START("top_second");
    for (int i=0; i<NUM_IN_FIRST_ARRAY; i++)
    {
        a_array_first[i] = i+1;
    }
    BLOB_START("mid");
    ret = BLOB_INT_A("integer_val", &integer_val, 1);
    assert(ret ==  0);
    ret = BLOB_UNSIGNED_INT_A("unsigned_integer_val", &unsigned_int_val, 1);
    assert(ret ==  0);
    ret = BLOB_FLOAT_A("float_array", a_array_first, NUM_IN_FIRST_ARRAY);
    assert(ret ==  0);
    BLOB_FLUSH();
    BLOB_FLUSH();
}

int
main(int argc, char **argv)
{
    int i = 0;
    BLOB_START("main");
    for (i=0; i<10; i++)
    {
        BLOB_INT_A("iteration", &i, 1);
        func_top();
        func_mid();
        func_top();
        func_top();
    }
    
    BLOB_FLUSH();
    return 0;
}
