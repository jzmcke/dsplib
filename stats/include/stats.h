#include <stdio.h>
#include <stdlib.h>

#define REGION_MAX 100000
#define REGION_NOT_FOUND -1
#define BITS_IN_BYTE 8
#define STATS_OK 0
#define STATS_ERROR -1

typedef struct mode_stats_s
{
	unsigned int *p_modes;
	float *p_mode_probs;
	unsigned int *p_modes_freq;
	unsigned int n_samples;
	unsigned int n_modes;
	unsigned int data_range;
	size_t byte_width;
} mode_stats;

int
stats_gen_rand_bin
	(mode_stats *p_stats
	,float *p_mode_probs
	,unsigned int n_modes
	,unsigned int data_range
	,unsigned int n_samples
	,char *p_name
	);

void
stats_print(mode_stats *p_stats);