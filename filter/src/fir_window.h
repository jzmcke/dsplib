#define FIR_WINDOW_OK 		(0)
#define FIR_WINDOW_ERR 		(-1)

int
fir_window_design_low_pass
	(float 		 *p_filter_coeff
	,unsigned int fc
	,unsigned int fs
	,unsigned int n
	);

int
fir_window_design_high_pass
	(float 		 *p_filter_coeff
	,unsigned int fc
	,unsigned int fs
	,unsigned int n
	);