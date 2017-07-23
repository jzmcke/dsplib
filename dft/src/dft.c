#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "dft/include/dft.h"
#include "dft/include/util.h"

#define MAX_FFT_SAMPLES (524288)

int
_fft_core_process
    (float *p_in
    ,float *p_res
    ,int N
    ,int seperation
    ,float *p_scratch
    );

int
_ifft_core_process
    (float *p_in
    ,float *p_res
    ,int N
    ,int seperation
    ,float *p_scratch
    );

void
_dft_print_cplx
    (float *p_cplx);

int
_dft_re_cplx_mult
    (float re
    ,float *p_cplx
    ,float *p_out
    );

int
_dft_get_twiddle
    (float *p_twiddle
    ,int k
    ,int n
    ,int N
    );

int
_dft_cplx_cplx_mult_acc
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    );

int
_dft_cplx_cplx_mult
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    );

int
_dft_cplx_cplx_add
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    );

int
fft_forward_process
    (float *p_in
    ,float *p_out
    ,int N
    )
{
    float p_scratch[2*MAX_FFT_SAMPLES] = {0};
    _fft_core_process(p_in,p_out,N,1,p_scratch);
    for (int i=0; i<2*N; i+=2)
    {
        _dft_re_cplx_mult(1.0/sqrt(N),&p_out[i],&p_out[i]);
    }
    return DFT_OK;
}

int
_fft_core_process
    (float *p_in
    ,float *p_res
    ,int N
    ,int seperation
    ,float *p_scratch
    )
{
    if (N==1)
    {
        /* Accumulate output, assuming initialised to 0 */
        *p_res = *p_in;
        *(p_res+1) = *(p_in+1);
    }
    else
    {
        float *p_even_start = p_in;
        float *p_odd_start = p_in+2*seperation;
        float *p_ek = p_res;
        float *p_ok = p_res + 2*seperation;
        //float p_ek[2*MAX_FFT_SAMPLES] = {0};
        //float p_ok[2*MAX_FFT_SAMPLES] = {0};
        float p_twiddle[2] = {0};

        _fft_core_process(p_even_start,p_ek,N/2,2*seperation,p_scratch);
                
        _fft_core_process(p_odd_start,p_ok,N/2,2*seperation,p_scratch);

        for (int k=0; k < N/2; k++)
        {
            float p_temp_res1[2];
            float p_temp_res2[2];

            _dft_get_twiddle(p_twiddle,k,1,N);

            _dft_cplx_cplx_mult(&p_ok[2*k*(2*seperation)],p_twiddle,&p_ok[2*k*(2*seperation)]);
            /* Fill out the output */
            _dft_cplx_cplx_add(&p_ek[2*k*(2*seperation)],&p_ok[2*k*(2*seperation)],p_temp_res1);

            _dft_re_cplx_mult(-1,&p_ok[2*k*(2*seperation)],&p_ok[2*k*(2*seperation)]);

            _dft_cplx_cplx_add(&p_ek[2*k*(2*seperation)],&p_ok[2*k*(2*seperation)],p_temp_res2);
            
            p_scratch[2*k*seperation] = p_temp_res1[0];
            p_scratch[2*k*seperation+1] = p_temp_res1[1];
            p_scratch[2*(k+N/2)*seperation] = p_temp_res2[0];
            p_scratch[2*(k+N/2)*seperation+1] = p_temp_res2[1];
        }
        for (int k=0; k < N/2; k++)
        {
            p_res[2*k*seperation] = p_scratch[2*k*seperation];
            p_res[2*k*seperation+1] = p_scratch[2*k*seperation+1];
            p_res[2*(k+N/2)*seperation] = p_scratch[2*(k+N/2)*seperation];
            p_res[2*(k+N/2)*seperation+1] = p_scratch[2*(k+N/2)*seperation+1];
        }
    }

    return DFT_OK;
}

int
fft_inverse_process
    (float *p_in
    ,float *p_out
    ,int N
    )
{
    float p_scratch[2*MAX_FFT_SAMPLES] = {0};
    _ifft_core_process(p_in,p_out,N,1,p_scratch);
    for (int i=0; i<2*N; i+=2)
    {
        _dft_re_cplx_mult(1.0/sqrt(N),&p_out[i],&p_out[i]);
    }
    return DFT_OK;
}

int
_ifft_core_process
    (float *p_in
    ,float *p_res
    ,int N
    ,int seperation
    ,float *p_scratch
    )
{
    
    if (N==1)
    {
        /* Accumulate output, assuming initialised to 0 */
        *p_res = *p_in;
        *(p_res+1) = *(p_in+1);
    }
    else
    {
        float *p_even_start = p_in;
        float *p_odd_start = p_in+2*seperation;
        float *p_ek = p_res;
        float *p_ok = p_res + 2*seperation;
        float p_twiddle[2] = {0};

        _ifft_core_process(p_even_start,p_ek,N/2,2*seperation,p_scratch);
                
        _ifft_core_process(p_odd_start,p_ok,N/2,2*seperation,p_scratch);

        for (int k=0; k < N/2; k++)
        {
            float p_temp_res1[2];
            float p_temp_res2[2];

            _dft_get_twiddle(p_twiddle,k,-1,N);

            _dft_cplx_cplx_mult(&p_ok[2*k*(2*seperation)],p_twiddle,&p_ok[2*k*(2*seperation)]);
            /* Fill out the output */
            _dft_cplx_cplx_add(&p_ek[2*k*(2*seperation)],&p_ok[2*k*(2*seperation)],p_temp_res1);

            _dft_re_cplx_mult(-1,&p_ok[2*k*(2*seperation)],&p_ok[2*k*(2*seperation)]);

            _dft_cplx_cplx_add(&p_ek[2*k*(2*seperation)],&p_ok[2*k*(2*seperation)],p_temp_res2);
            
            p_scratch[2*k*seperation] = p_temp_res1[0];
            p_scratch[2*k*seperation+1] = p_temp_res1[1];
            p_scratch[2*(k+N/2)*seperation] = p_temp_res2[0];
            p_scratch[2*(k+N/2)*seperation+1] = p_temp_res2[1];
        }
        for (int k=0; k < N/2; k++)
        {
            p_res[2*k*seperation] = p_scratch[2*k*seperation];
            p_res[2*k*seperation+1] = p_scratch[2*k*seperation+1];
            p_res[2*(k+N/2)*seperation] = p_scratch[2*(k+N/2)*seperation];
            p_res[2*(k+N/2)*seperation+1] = p_scratch[2*(k+N/2)*seperation+1];
        }
    }

    return DFT_OK;
}

/* Performs a complex addition */
int
_dft_cplx_cplx_add
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    )
{
    *p_out = *p_in1 + *p_in2;
    *(p_out+1) = *(p_in1+1) + *(p_in2+1);
    return DFT_OK;
}

int
_dft_re_cplx_mult
    (float re
    ,float *p_cplx
    ,float *p_out
    )
{
    p_out[0] = re*p_cplx[0];
    p_out[1] = re*p_cplx[1];
    return DFT_OK;
}

/* Performs complex multiply-accumulate on an array of type {Re1, Im1} x {Re2, Im2} */
int
_dft_cplx_cplx_mult_acc
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
    return DFT_OK;
}

/* Performs complex multiplication on an array of type {Re1, Im1} x {Re2, Im2} */
int
_dft_cplx_cplx_mult
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
    return DFT_OK;
}

/* Currently performs sin/cos calculation. Can be optimised to do table lookup */
int
_dft_get_twiddle
    (float *p_twiddle
    ,int k
    ,int n
    ,int N
    )
{
    *p_twiddle = cos(-2.0*M_PI*k*n/N);
    *(p_twiddle+1) = sin(-2.0*M_PI*k*n/N);
    return DFT_OK;
}

int
dft_forward_process
    (float *p_in
    ,float *p_out
    ,int N
    )
{
    int k, n;
    float p_twiddle[2]; /* {Re, Im} */
    /* X[w] = 1/N * sum(x[n]*exp(-w*j*n/N))
     *      = 1/N * sum(x[n] * (cos(-w*n/N)+j*sin(-w*n/N)))
     */
    for (k=0; k<N; k++)
    {
        p_out[2*k] = 0;
        p_out[2*k+1] = 0;
        for (n=0; n<N; n++)
        {
            _dft_get_twiddle(p_twiddle,k,n,N);
            _dft_cplx_cplx_mult_acc(&p_in[2*n],p_twiddle,&p_out[2*k]);
        }
        p_out[2*k] /= sqrt(N);
        p_out[2*k+1] /= sqrt(N);
    }
    printf("Finished DFT\n");
    return DFT_OK;
}

int
dft_inverse_process
    (float *p_in
    ,float *p_out
    ,int N
    )
{
    int k, n;
    /* X[w] = 1/N * sum(x[n]*exp(w*j*n/N))
     *      = 1/N * sum(x[n] * (cos(w*n/N)+j*sin(w*n/N)))
     */
    for (n=0; n<N; n++)
    {
        p_out[2*n] = 0;
        p_out[2*n+1] = 0;
        for (k=0; k<N; k++)
        {
            /* Re*Re part */
            p_out[2*n] += p_in[2*k]*cos(2.0*M_PI*k*n/N);

            /* Re*Imag part */
            p_out[2*n+1] += p_in[2*k]*sin(2.0*M_PI*k*n/N);

            /* Imag*Re part */
            p_out[2*n+1] += p_in[2*k+1]*cos(2.0*M_PI*k*n/N);

            /* Imag*Imag part */
            p_out[2*n] += -p_in[2*k+1]*sin(2.0*M_PI*k*n/N);
        }
        p_out[2*n] /= sqrt(N);
        p_out[2*n+1] /= sqrt(N);
    }
    printf("Finished IDFT\n");
    return DFT_OK;
}

void
_dft_print_cplx
    (float *p_cplx
    )
{
    printf("{%f, %f}\n",*p_cplx, *(p_cplx+1));
}