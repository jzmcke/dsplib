#include "cplx_math/include/cplx_math.h"
#include <stdio.h>

/* Performs a complex addition */
int
cplx_cplx_add
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    )
{
    *p_out = *p_in1 + *p_in2;
    *(p_out+1) = *(p_in1+1) + *(p_in2+1);
    return CPLX_MATH_OK;
}

int
re_cplx_mult
    (float re
    ,float *p_cplx
    ,float *p_out
    )
{
    p_out[0] = re*p_cplx[0];
    p_out[1] = re*p_cplx[1];
    return CPLX_MATH_OK;
}

/* Performs complex multiply-accumulate on an array of type {Re1, Im1} x {Re2, Im2} */
int
cplx_cplx_mult_acc
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    )
{
    float a_temp1[2];
    float a_temp2[2];
    a_temp1[0] = *p_in1; /* Needed to support case p_in1 == p_out */
    a_temp1[1] = *(p_in1+1); /* Needed to support case p_in1 == p_out */
    a_temp2[0] = *p_in2;
    a_temp2[1] = *(p_in2+1);

    /* Re*Re part */
    *p_out += (a_temp1[0])*(a_temp2[0]);

    /* Re*Imag part */
    (*(p_out+1)) += (a_temp1[0])*(a_temp2[1]);

    /* Imag*Re part */
    (*(p_out+1)) += (a_temp1[1])*(a_temp2[0]);

    /* Imag*Imag part */
    *p_out += -(a_temp1[1])*(a_temp2[1]);
    return CPLX_MATH_OK;
}

/* Performs complex multiplication on an array of type {Re1, Im1} x {Re2, Im2} */
int
cplx_cplx_mult
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    )
{
    float a_temp1[2];
    float a_temp2[2];
    a_temp1[0] = *p_in1; /* Needed to support case p_in1 == p_out */
    a_temp1[1] = *(p_in1+1); /* Needed to support case p_in1 == p_out */
    a_temp2[0] = *p_in2;
    a_temp2[1] = *(p_in2+1);

    /* Re*Re part */
    *p_out = (a_temp1[0])*(a_temp2[0]);

    /* Re*Imag part */
    (*(p_out+1)) = (a_temp1[0])*(a_temp2[1]);

    /* Imag*Re part */
    (*(p_out+1)) += (a_temp1[1])*(a_temp2[0]);

    /* Imag*Imag part */
    *p_out += -(a_temp1[1])*(a_temp2[1]);
    return CPLX_MATH_OK;
}

void
print_cplx
    (float *p_cplx
    )
{
    printf("{%f, %f}\n",*p_cplx, *(p_cplx+1));
}