#include <stdio.h>

void
util_print_cplx_sig
    (float *p_data
    ,int N
    )
{
    printf("RE: ");
    for (int i=0; i<2*N; i+=2)
    {
        printf("%f ",p_data[i]);
    }

    printf("\nIM: ");
    for (int i=0; i<2*N; i+=2)
    {
        printf("%f ",p_data[i+1]);
    }
    printf("\n");
}
