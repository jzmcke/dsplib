/* DFT definitions */
#define DFT_OK 			(0)
#define DFT_ERROR 		(-1)
#define DFT_MAX_SAMPLES (524288)

int
dft_forward_process
    (float *p_in
    ,float *p_out
    ,int N
    );

int
dft_inverse_process
    (float *p_in
    ,float *p_out
    ,int N
    );

int
fft_forward_process
    (float *p_in
    ,float *p_out
    ,int N
    );

int
fft_inverse_process
    (float *p_in
    ,float *p_out
    ,int N
    );