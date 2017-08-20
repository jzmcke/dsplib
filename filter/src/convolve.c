#include "filter/src/convolve.h"
#include "cplx_math/include/cplx_math.h"
#include <stdlib.h>

struct convolve_s
{
	float *p_state;
	float *p_filt_coef;
	int    M;
};

int
convolve_init
	(convolve **pp_self
	,convolve_cfg *p_cfg
	)
{
	*pp_self = (convolve*)malloc(sizeof(convolve));

	(*pp_self)->p_state = (float*)calloc(p_cfg->M,sizeof(float));
	(*pp_self)->p_filt_coef = p_cfg->p_filt_coef;
	(*pp_self)->M = p_cfg->M;
	return CONVOLVE_OK;
}

int
convolve_overlap_add
	(convolve *p_self
	,float *p_input
	,int 	Nx
	,float *p_output
	,int   *p_n_out
	)
{
	int i, j;
	for (i=0; i<p_self->M; i++)
	{
		float res[2] = {0};
		for (j=i; j>=0; j--)
		{
			cplx_cplx_mult_acc(&p_input[2*(i-j)],&p_self->p_filt_coef[2*j],res);
		}
		p_output[2*i] = res[0];
		p_output[2*i+1] = res[1];
	}

	for (i=p_self->M; i<Nx; i++)
	{
		float res[2] = {0};
		for (j=p_self->M; j>=0; j--)
		{
			cplx_cplx_mult_acc(&p_input[2*(i-j)],&p_self->p_filt_coef[2*j],res);
		}
		p_output[2*i] = res[0];
		p_output[2*i+1] = res[1];
	}

	for (i=Nx; i<(p_self->M + Nx); i++)
	{
		float res[2] = {0};
		for (j=p_self->M; j>=i-Nx; j--)
		{
			cplx_cplx_mult_acc(&p_input[2*(i-j)],&p_self->p_filt_coef[2*j],res);
		}
		p_output[2*i] = res[0];
		p_output[2*i+1] = res[1];
	}
	*p_n_out = p_self->M + Nx;
	return CONVOLVE_OK;
}