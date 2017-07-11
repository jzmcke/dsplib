#include "stats.h"
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

unsigned int
get_region(unsigned int sample, unsigned int *p_region_bounds, unsigned int n_modes);

/* Returns indices of data modes */
/* Maximum data range 2^32 */
int
stats_gen_rand_bin
	(mode_stats *p_stats
	,float *p_mode_probs
	,unsigned int n_modes
	,unsigned int data_range
	,unsigned int n_samples
	,char *p_name
	)
{
	int i;
	FILE *p_file;
	unsigned int *p_region_bounds;
	unsigned int *p_modes_freq;
	unsigned int *p_modes;
	time_t t;
	size_t byte_num = 4;
	unsigned int byte_bound = (1<<24);

	/* Seed rand() with time */
	srand((unsigned) time(&t));

	/* Allocate and randomise the locations of the modes */
	p_modes = (unsigned int*)malloc(n_modes*sizeof(unsigned int));
	p_modes_freq = (unsigned int*)malloc(n_modes*sizeof(unsigned int));
	for (i=0;i<n_modes;i++)
	{
		/* Continually set it until the indiex is unique */
		int b_mode_present = 1;
		while (b_mode_present)
		{
			int j = i-1;
			b_mode_present = 0;
			while(j>=0)
			{
				if (p_modes[i] == p_modes[j])
				{
					b_mode_present = 1;
				}
				j--;
			}
			p_modes[i] = rand()%data_range;
		}
	}

	p_file = fopen(p_name,"w+");

	if (NULL == p_file)
	{
		return STATS_ERROR;
	}
	p_region_bounds = (unsigned int*)malloc(n_modes*sizeof(unsigned int));
	p_region_bounds[0] = (unsigned int)(p_mode_probs[0]*REGION_MAX);
	for (i=1; i<n_modes; i++)
	{
		p_region_bounds[i]=p_region_bounds[i-1]+p_mode_probs[i]*REGION_MAX;
		if (p_region_bounds[i] > REGION_MAX)
		{
			return STATS_ERROR;
			printf("Mode probabilities sum to a value greater than 1.0.\n");
		}
	}

	/* Get the byte length of the uncompressed code words */
	while (data_range<byte_bound)
	{
		byte_bound >>= BITS_IN_BYTE;
		byte_num--;
	}

	for (i=0; i<n_samples; i++)
	{
		unsigned int sample;
		unsigned int region;
		sample = rand()%REGION_MAX;
		region = get_region(sample,p_region_bounds,n_modes);

		if (region != REGION_NOT_FOUND)
		{
			fwrite(p_modes+region,byte_num,1,p_file);
			p_modes_freq[region]++;
		}
		else
		{
			int rand_byte = rand()%data_range;
			fwrite(&rand_byte,byte_num,1,p_file);
		}
	}
	fclose(p_file);

	/* Update stats */
	p_stats->p_modes = p_modes;
	p_stats->p_mode_probs = p_mode_probs;
	p_stats->p_modes_freq = p_modes_freq;
	p_stats->n_samples=n_samples;
	p_stats->n_modes=n_modes;
	p_stats->data_range=data_range;
	p_stats->byte_width = byte_num;

	return STATS_OK;
}

unsigned int
get_region(unsigned int sample, unsigned int *p_region_bounds, unsigned int n_modes)
{
	int i;
	for (i=0; i<n_modes;i++)
	{
		if (sample < p_region_bounds[i])
		{
			return i;
		}
	}
	return REGION_NOT_FOUND;
}

void
stats_print(mode_stats *p_stats)
{
	int i;
	printf("\n------------------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------------------\n");
	printf("Number of modes: %d\n",p_stats->n_modes);
	printf("Data Range: %d\n", p_stats->data_range);
	printf("Byte width: %u\n", (unsigned int)p_stats->byte_width);
	printf("Total samples: %d\n", p_stats->n_samples);
	for (i=0; i<p_stats->n_modes; i++)
	{
		printf("Mode (%d,%d): p(%d) = %f, freq(%d) = %d, p_true(%d) = %f\n"
			  ,i
			  ,p_stats->p_modes[i]
			  ,p_stats->p_modes[i]
			  ,p_stats->p_mode_probs[i]
			  ,p_stats->p_modes[i]
			  ,p_stats->p_modes_freq[i]
			  ,p_stats->p_modes[i]
			  ,1.0*p_stats->p_modes_freq[i]/p_stats->n_samples
			  );
	}
	printf("\n------------------------------------------------------------------------------------\n");
	printf("------------------------------------------------------------------------------------\n");
}