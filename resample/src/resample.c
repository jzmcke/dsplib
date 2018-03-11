#include "resample.h"
#include "dft.h"
#include <stdlib.h>
#include <math.h>


typedef struct resample_s {
	int fs_out;
	int type;
	int order; /* Number of side-lobes of a sinc reconstructor to use */
} resample_s;

int
resample_process(resample p_self, float *p_in, float *p_out, int fs_in, int n)
{
	int i;
	float t_sinc = 1.0*fs_in/fs_out;
	int max_sinc_samples = (int)(1.0*p_self->order/t_sinc)+1;
	int center = max_sinc_samples/2;
	float *p_sinc_fn;
	float upsample_ratio = fs_out/fs_in;
	p_sinc_fn = (float*)malloc(max_sinc_samples*sizeof(float));
	for (i=-max_sinc_samples/2; i<0; i++)
	{
		int t = t_sinc*i;
		p_sinc_fn[i+max_sinc_samples/2] = sin(2*M_PI*t)/(2*M_PI*t);
	}
	p_sinc_fn[center] = 1;

	for (i=1; i<max_sinc_samples/2; i++)
	{
		int t = t_sinc*i;
		p_sinc_fn[i+max_sinc_samples/2] = sin(2*M_PI*t)/(2*M_PI*t);
	}

	if (p_self->type == RESAMPLE_TYPE_SYNC)
	{
		if (fs_out > fs_in)
		{
			/* Upsample */
			for (i=0; i<(int)n*upsample_ratio; i++)
			{
				int j;
				int lower_term = 0;
				int upper_term = max_sinc_samples;
				float time;
				float out_sample = 0;

				time = 1.0*i/upsample_ratio;
				

				if (i<=center)
				{
					lower_term = center-i;
				}
				if (n-i <= center)
				{
					upper_term = center + (n-i);
				}
				for (j=lower_term; j<upper_term; j++)
				{
					out_sample += p_sinc_fn[j]*p_in[i+(j-center)];
				}
				p_out[i] = out_sample;
			}

		}
		else if (fs_out < fs_in)
		{
			/* Downsample */

		}
	}
	if (p_self->type == RESAMPLE_TYPE_FREQUENCY)
	{
		if (fs_out > fs_in)
		{
			/* Upsample */
			fft_forward_process(p_in,)
		}
		else
		{
			/* Downsample */
		}
	}
	free(p_sinc_fn);

	return RESAMPLE_OK;
}