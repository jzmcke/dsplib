#define CONVOLVE_ERR (-1)
#define CONVOLVE_OK	 (0)

typedef struct convolve_s convolve;

typedef struct convolve_cfg_s
{
	float *p_filt_coef;
	int    M;
} convolve_cfg;

int
convolve_init
	(convolve **pp_self
	,convolve_cfg *p_cfg
	);

int
convolve_overlap_add
	(convolve *p_self
	,float *p_input
	,int 	Nx
	,float *p_output 
	,int   *p_n_out /* In/out. Number of available/written samples in p_output*/
	);