#include "filter/include/filter.h"
#include "math.h"
#include "/usr/include/sndfile.h"
#include "dft/include/dft.h"
#include "filter_support/scripts/fir_filter_pvt.h"
#include "filter_support/scripts/iir_filter_pvt.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FILTER_FRONTEND_OK  (0)
#define FILTER_FRONTEND_ERR (-1)

#define BLOCK_SIZE_MS 20
#define FILTER_FRONTEND_TYPE_FIR  (0)
#define FILTER_FRONTEND_TYPE_IIR  (1)

typedef struct filter_frontend_cfg
{
	char 	        *p_in_name;
	char 	        *p_out_name;
    char            *p_tfm_name;
	unsigned int 	 N;
	int 	         type;
    float            gain_db;
} filter_frontend_cfg;

typedef struct filter_frontend_s
{
	SNDFILE *p_in_file;
    unsigned int     fs_in;
    unsigned int     block_size;
    float           *p_in;
    float           *p_in_rebuf;
    float           *p_bins;
    float           *p_filtered;
    float           *p_out;
    void            *p_filter_state;
	SNDFILE *p_out_file;
    unsigned int     fs_out;
	unsigned int 	 N;
	unsigned int     fc;
    float            gain_db;
    int              type;
} filter_frontend;

int
filter_frontend_rebuf
    (float *p_in
    ,float *p_out
    ,int N
    );

int
filter_frontend_unbuf
    (float *p_in
    ,float *p_out
    ,int N
    );

void
print_usage(void)
{
    printf("-----------------------------------------------------------------------------------------------------\n");
    printf("| Command       | Description                                                   | Default           |\n");
    printf("| --------------------------------------------------------------------------------------------------|\n");
    printf("| -i <in_file>  | Name of input wav file.                                       |                   |\n");
    printf("| -o <out_file> | Name of output wav file.                                      |                   |\n");
    printf("| -g <gain_db>  | Gain applied to the output .wav file after filtering.         | 0dB               |\n");
    printf("| --iir         | Force the use of an IIR filter.                               | fir               |\n");
    printf("-----------------------------------------------------------------------------------------------------\n");
}

int
filter_frontend_init(filter_frontend_cfg *p_cfg, filter_frontend **pp_self)
{
    *pp_self = (filter_frontend*)malloc(sizeof(filter_frontend));
	SF_INFO sf_in_info;
    SF_INFO sf_out_info;
	/* Read cfg structure */
	sf_in_info.format = 0;

    /* Open input for reading */
	(*pp_self)->p_in_file=sf_open(p_cfg->p_in_name,SFM_READ,&sf_in_info);
    if (NULL == (*pp_self)->p_in_file)
    {
        printf("Error opening in wav file\n");
        return -1;
    }
    
    (*pp_self)->fs_in = sf_in_info.samplerate;


    sf_out_info.samplerate = (*pp_self)->fs_in;
    sf_out_info.channels = 1;
    sf_out_info.format = SF_FORMAT_WAV|SF_FORMAT_FLOAT;

    /* Open output for write */
    (*pp_self)->p_out_file=sf_open(p_cfg->p_out_name,SFM_WRITE,&sf_out_info);
    if (NULL == (*pp_self)->p_out_file)
    {
        int err;

        err = sf_error((*pp_self)->p_out_file);
        if (err < 5)
        {
            printf("Error opening output wav file, return code %d\n",err);
        }
        else
        {
            printf("%s\n",sf_error_number(err));
        }
        
        return -1;
    }

    if (p_cfg->type == FILTER_FRONTEND_TYPE_FIR)
    {
        convolve_cfg cfg;
        cfg.M=sizeof(fir_filter)/(2*sizeof(fir_filter[0]));
        cfg.p_filt_coef = fir_filter;
        convolve_init((convolve**)&(*pp_self)->p_filter_state, &cfg);
    }
    else
    {
        iir_filter_cfg cfg;
        cfg.Mn = sizeof(iir_filter_n)/(2*sizeof(iir_filter_n[0]));
        cfg.Md = sizeof(iir_filter_d)/(2*sizeof(iir_filter_d[0]));
        cfg.p_filt_coef_n = iir_filter_n;
        cfg.p_filt_coef_d = iir_filter_d;
        iir_filter_init((iir_filter**)&(*pp_self)->p_filter_state, &cfg);
    }
    (*pp_self)->gain_db = p_cfg->gain_db;
    (*pp_self)->type = p_cfg->type;
    (*pp_self)->block_size = (unsigned int)(1.0*(*pp_self)->fs_in*BLOCK_SIZE_MS/1000);
    (*pp_self)->p_in = (float*)malloc(sizeof(float)*(*pp_self)->block_size);
    (*pp_self)->p_in_rebuf = (float*)malloc(sizeof(float)*2*(*pp_self)->block_size);
    (*pp_self)->p_bins = (float*)malloc(sizeof(float)*2*(*pp_self)->block_size);
    (*pp_self)->p_filtered = (float*)malloc(sizeof(float)*2*(*pp_self)->block_size + 2*(*pp_self)->N + 1);
    (*pp_self)->p_out = (float*)malloc(sizeof(float)*(*pp_self)->block_size);
	return FILTER_FRONTEND_OK;
}

void
filter_frontend_close(filter_frontend *p_frontend)
{
    if (p_frontend)
    {
        if (p_frontend->p_in_file)
        {
            sf_close(p_frontend->p_in_file);
        }
        if (p_frontend->p_out_file)
        {
            sf_close(p_frontend->p_out_file);
        }
    }
}

int 
filter_frontend_process(filter_frontend *p_self)
{
    int tick = 0;
    int i=0;
    int n_out;
    float gain_lin;
    gain_lin = powf(10,p_self->gain_db/20);
    while (p_self->block_size == sf_readf_float(p_self->p_in_file,p_self->p_in,p_self->block_size))
    {
        filter_frontend_rebuf(p_self->p_in,p_self->p_in_rebuf,p_self->block_size);

        if (p_self->type == FILTER_FRONTEND_TYPE_IIR)
        {
            iir_filter_process((iir_filter*)p_self->p_filter_state
                              ,p_self->p_in_rebuf
                              ,p_self->block_size
                              ,p_self->p_filtered
                              ,&n_out
                              );
        }
        else
        {
            convolve_overlap_add((convolve*)p_self->p_filter_state
                                ,p_self->p_in_rebuf
                                ,p_self->block_size
                                ,p_self->p_filtered
                                ,&n_out
                                );
        }
        

        filter_frontend_unbuf(p_self->p_filtered,p_self->p_out,p_self->block_size);

        for (i=0; i<p_self->block_size; i++)
        {
            p_self->p_out[i] *= gain_lin;
        }

        if (p_self->block_size != sf_writef_float(p_self->p_out_file,p_self->p_out,p_self->block_size))
        {
            printf("Error writing to output file.\n");
            printf("tick = %d\n",tick);
            return -1;
        }

        tick++;
    }
	return FILTER_FRONTEND_OK;
}

int
parse_args
    (filter_frontend_cfg *p_cfg
    ,int argc
    ,char **argv
    )
{
    p_cfg->p_out_name = "out.wav";
    p_cfg->gain_db = 0;
    p_cfg->type = FILTER_FRONTEND_TYPE_FIR;
    /* Skip function name */
    argc--;
    argv++;
    while (argc)
    {
        if (0 == strcmp(argv[0],"-i"))
        {
            p_cfg->p_in_name = argv[1];
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-o"))
        {
            p_cfg->p_out_name = argv[1];
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-g"))
        {
            p_cfg->gain_db = atof(argv[1]);
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "--iir"))
        {
            p_cfg->type = FILTER_FRONTEND_TYPE_IIR;
        }
        else if (0 == strcmp(argv[0], "-h"))
        {
            return -1;
        }
        else
        {
            fprintf(stderr, "Error parsing args\n");
            return -1;
        }
        argc--;
        argv++;
    }
    return 0;
}

int
filter_frontend_rebuf
    (float *p_in
    ,float *p_out
    ,int N)
{
    int i;
    for (i=0; i<N; i++)
    {
        p_out[2*i] = p_in[i];
        p_out[2*i+1] = 0;
    }
    return FILTER_FRONTEND_OK;
}

int
filter_frontend_unbuf
    (float *p_in
    ,float *p_out
    ,int N)
{
    int i;
    for (i=0; i<N; i++)
    {
        p_out[i] = p_in[2*i];
    }
    return FILTER_FRONTEND_OK;
}

int
main(int argc, char **argv)
{
	
	filter_frontend_cfg cfg;
	filter_frontend *p_filter_frontend;
	if (FILTER_FRONTEND_OK != parse_args(&cfg, argc, argv))
	{
		print_usage();
		return -1;
	}
	if (FILTER_FRONTEND_OK != filter_frontend_init(&cfg, &p_filter_frontend))
	{
        filter_frontend_close(p_filter_frontend);
		return -1;
	}

	if (FILTER_FRONTEND_OK != filter_frontend_process(p_filter_frontend))
	{
        filter_frontend_close(p_filter_frontend);
		return -1;
	}
    filter_frontend_close(p_filter_frontend);
	return 0;
}