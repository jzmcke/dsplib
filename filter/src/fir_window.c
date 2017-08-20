#include "filter/src/fir_window.h"
#include <math.h>
#include <stdio.h>

#define MATHS_PI 3.14159265358979323846

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
	if (ord%2 != 0)
	{
		/* Odd length */
		for (i=-(ord-1)/2; i<=(ord-1)/2; i++)
		{
			if (i==0)
			{
				p_filter_coeff[2*(ord+1)/2] = 2.0*M_PI*fc/ord;
				continue;
			}
			p_filter_coeff[2*(i+(ord-1)/2)] = sin(2.0*M_PI*fc*i)/(M_PI*i*fs);
			p_filter_coeff[2*(i+(ord-1)/2)+1] = 0;
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
	if (ord%2 != 0)
	{
		/* Odd length */
		for (i=-(ord-1)/2; i<=(ord-1)/2; i++)
		{
			if (i==0)
			{
				p_filter_coeff[2*(ord+1)/2] = -2.0*M_PI*fc/ord;
				continue;
			}
			p_filter_coeff[2*(i+(ord-1)/2)] = -1.0*sinf(2.0*M_PI*fc*i)/(M_PI*i*fs);
			p_filter_coeff[2*(i+(ord-1)/2)+1] = 0;
		}
		return FIR_WINDOW_OK;
	}
	else
	{
		return FIR_WINDOW_ERR;
	}
	
}

