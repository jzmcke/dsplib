#define IIR_FILTER_ERR (-1)
#define IIR_FILTER_OK	 (0)

typedef struct iir_filter_s iir_filter;

typedef struct iir_filter_cfg_s
{
	float *p_filt_coef_n;
	float *p_filt_coef_d;
	int    Mn;
	int    Md;
} iir_filter_cfg;

int
iir_filter_init
	(iir_filter **pp_self
	,iir_filter_cfg *p_cfg
	);

int
iir_filter_process
	(iir_filter *p_self
	,float *p_input
	,int 	Nx
	,float *p_output 
	,int   *p_n_out /* In/out. Number of available/written samples in p_output*/
	);
