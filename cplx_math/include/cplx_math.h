#define CPLX_MATH_OK (0)
#define CPLX_MATH_ERR (-1)

int
cplx_cplx_mult_acc
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    );

int
cplx_cplx_mult
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    );

int
cplx_cplx_add
    (float *p_in1
    ,float *p_in2
    ,float *p_out
    );

void
print_cplx
    (float *p_cplx);

int
re_cplx_mult
    (float re
    ,float *p_cplx
    ,float *p_out
    );

float
cplx_square
    (float *p_in
    );