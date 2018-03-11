#include "filter/src/iir_filter.h"
#include "cplx_math/include/cplx_math.h"
#include "util/include/util.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

struct iir_filter_s
{
	float *p_input_state;
	float *p_output_state;
	int    output_state_head;
	float *p_filt_coef_n;
	float *p_filt_coef_d;
	int    Mn;
	int    Md; /* excludes y[n] coefficient, assumes == 1 */
};

int
inc(int cur_idx, int inc, int mod)
{
	return ((cur_idx+inc)%mod+mod)%mod;
}

int
iir_filter_init
	(iir_filter **pp_self
	,iir_filter_cfg *p_cfg
	)
{
	int i;
	*pp_self = (iir_filter*)malloc(sizeof(iir_filter));

	(*pp_self)->p_input_state = (float*)calloc(2*p_cfg->Mn,sizeof(float));
	(*pp_self)->p_output_state = (float*)calloc(2*p_cfg->Md,sizeof(float));
	(*pp_self)->output_state_head = 0;
	(*pp_self)->p_filt_coef_n = p_cfg->p_filt_coef_n;
	(*pp_self)->p_filt_coef_d = p_cfg->p_filt_coef_d;
	for (i=0; i<p_cfg->Md; i++)
	{
		(*pp_self)->p_filt_coef_d[2*i] *= -1.0;
		(*pp_self)->p_filt_coef_d[2*i+1] *= -1.0;
	}
	(*pp_self)->Mn = p_cfg->Mn;
	(*pp_self)->Md = p_cfg->Md; 
	return IIR_FILTER_OK;
}

int
iir_filter_process
	(iir_filter *p_self
	,float *p_input
	,int 	Nx
	,float *p_output
	,int   *p_n_out
	)
{
	int i, j;
	//printf("New frame\n");
	for (i=0; i<p_self->Mn; i++)
	{
		//printf("Output state: \n");
		//util_print_cplx_sig(p_self->p_output_state,p_self->Md);
		//printf("Summation over X - Time %d\n",i);
		float res[2] = {0};	
		for (j=i; j>=0; j--)
		{
			//printf("j=%d\n",j);
			cplx_cplx_mult_acc(&p_input[2*(i-j)],&p_self->p_filt_coef_n[2*j],res);
			//printf("p_input[%d] = \n",2*(i-j));
			//util_print_cplx_sig(&p_input[2*(i-j)],1);
			//printf("p_self->p_filt_coef_n[%d] = \n",2*j);
			//util_print_cplx_sig(&p_self->p_filt_coef_n[2*j],1);
			//printf("res = \n");
			//util_print_cplx_sig(res,1);
		}
		//printf("Summation over Y - Time %d\n",i);
		for (j=0; j<p_self->Md; j++)
		{
			//printf("j=%d\n",j);
			cplx_cplx_mult_acc(&p_self->p_output_state[2*inc(p_self->output_state_head,-j,p_self->Md)]
							  ,&p_self->p_filt_coef_d[2*j]
							  ,res
							  );

			//printf("p_self->p_output_state[%d] = \n",2*(inc(p_self->output_state_head,-j,p_self->Md)));
			//util_print_cplx_sig(&p_self->p_output_state[2*inc(p_self->output_state_head,-j,p_self->Md)],1);
			//printf("p_self->p_filt_coef_d[%d] = \n",2*j);
			//util_print_cplx_sig(&p_self->p_filt_coef_d[2*j],1);
			//printf("res = \n");
			//util_print_cplx_sig(res,1);
		}

		cplx_cplx_add(&p_self->p_input_state[2*i],res,res);
		p_output[2*i] = res[0];
		p_output[2*i+1] = res[1];
		p_self->output_state_head = inc(p_self->output_state_head,1,p_self->Md);
		p_self->p_output_state[2*p_self->output_state_head] = res[0];
		p_self->p_output_state[2*p_self->output_state_head+1] = res[1];
	}

	for (i=p_self->Mn; i<Nx; i++)
	{
		float res[2] = {0};
		for (j=0; j<=p_self->Mn; j++)
		{
			cplx_cplx_mult_acc(&p_input[2*(i-j)],&p_self->p_filt_coef_n[2*j],res);
		}
		for (j=0; j<p_self->Md; j++)
		{
			cplx_cplx_mult_acc(&p_self->p_output_state[2*inc(p_self->output_state_head,-j,p_self->Md)]
							  ,&p_self->p_filt_coef_d[2*j]
							  ,res
							  );
		}
		p_output[2*i] = res[0];
		p_output[2*i+1] = res[1];
		p_self->output_state_head = inc(p_self->output_state_head,1,p_self->Md);
		p_self->p_output_state[2*p_self->output_state_head] = res[0];
		p_self->p_output_state[2*p_self->output_state_head+1] = res[1];
		
	}

	for (i=Nx; i<(p_self->Mn + Nx); i++)
	{
		float res[2] = {0};
		for (j=p_self->Mn; j>=i-Nx; j--)
		{
			cplx_cplx_mult_acc(&p_input[2*(i-j)],&p_self->p_filt_coef_n[2*j],res);
		}
		p_self->p_input_state[2*(i-Nx)] = res[0];
		p_self->p_input_state[2*(i-Nx)+1] = res[1];
	}

	*p_n_out = Nx;
	return IIR_FILTER_OK;
}