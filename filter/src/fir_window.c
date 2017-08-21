#include "filter/src/fir_window.h"
#include "cplx_math/include/cplx_math.h"
#include <math.h>
#include <stdio.h>

int
fir_window_design_low_pass
	(float 		 *p_filter_coeff
	,unsigned int fc
	,unsigned int fs
	,unsigned int n
	)
{
	int i;
	int ord = (int)n;
	float power;
	float power_acc = 0;
	float dc_level;
	if (ord%2 != 0)
	{
		/* Odd length */
		for (i=-(ord-1)/2; i<=(ord-1)/2; i++)
		{
			if (i==0)
			{
				p_filter_coeff[2*(ord-1)/2] = 2.0*fc/fs;
				p_filter_coeff[2*(ord-1)/2+1] = 0;
			}
			else
			{
				p_filter_coeff[2*(i+(ord-1)/2)] = sin(2.0*M_PI*fc*i/fs)/(M_PI*i);
				p_filter_coeff[2*(i+(ord-1)/2)+1] = 0;	
			}
			power = cplx_square(&p_filter_coeff[2*(i+(ord-1)/2)]);
			power_acc += power;
		}
		dc_level = sqrt(power_acc);
		for (i=-(ord-1)/2; i<=(ord-1)/2; i++)
		{
			p_filter_coeff[2*(i+(ord-1)/2)] *= 1.0/dc_level;
			p_filter_coeff[2*(i+(ord-1)/2)+1] *= 1.0/dc_level;
		}

		return FIR_WINDOW_OK;
	}
	else
	{
		return FIR_WINDOW_ERR;
	}
	
}

int
fir_window_design_high_pass
	(float 		 *p_filter_coeff
	,unsigned int fc
	,unsigned int fs
	,unsigned int n
	)
{
	int i;
	int ord = (int)n;
	float power;
	float power_acc = 0;
	float dc_level;
	if (ord%2 != 0)
	{
		/* Odd length */
		for (i=-(ord-1)/2; i<=(ord-1)/2; i++)
		{
			if (i==0)
			{
				p_filter_coeff[2*(ord-1)/2] = 1-2.0*fc/fs;
				p_filter_coeff[2*(ord-1)/2+1] = 0;
			}
			else
			{
				p_filter_coeff[2*(i+(ord-1)/2)] = -1.0*sin(2.0*M_PI*fc*i/fs)/(M_PI*i);
				p_filter_coeff[2*(i+(ord-1)/2)+1] = 0;
			}
			power = cplx_square(&p_filter_coeff[2*(i+(ord-1)/2)]);
			power_acc += power;
		}
		dc_level = sqrt(power_acc);
		printf("dc_level = %f",dc_level);
		for (i=-(ord-1)/2; i<=(ord-1)/2; i++)
		{
			p_filter_coeff[2*(i+(ord-1)/2)] *= 1.0/dc_level;
			p_filter_coeff[2*(i+(ord-1)/2)+1] *= 1.0/dc_level;
		}
		return FIR_WINDOW_OK;
	}
	else
	{
		return FIR_WINDOW_ERR;
	}
	
}

