#include "filter/include/filter.h"
#include "/usr/include/sndfile.h"
#include "dft/include/dft.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define FILTER_FRONTEND_OK  (0)
#define FILTER_FRONTEND_ERR (-1)

#define SAMPLE_RATE 32000
#define BLOCK_SIZE_MS 20
#define FILTER_TYPE_LOWPASS  (0)
#define FILTER_TYPE_HIGHPASS (1)

typedef struct filter_frontend_cfg
{
	char 	        *p_in_name;
	char 	        *p_out_name;
    char            *p_tfm_name;
	unsigned int 	 N;
	unsigned int     fc;
	int 	         b_type;
} filter_frontend_cfg;

typedef struct filter_frontend_s
{
	SNDFILE *p_in_file;
    SNDFILE *p_tfm_file;
    unsigned int     fs_in;
    unsigned int     block_size;
    float           *p_in;
    float           *p_in_rebuf;
    float           *p_bins;
    float           *p_filtered;
    float           *p_out;
    convolve        *p_convolve;
    float           *p_filter;
	SNDFILE *p_out_file;
    unsigned int     fs_out;
	unsigned int 	 N;
	unsigned int     fc;
	int 	         b_type;
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
    printf("| -t <tfm_file> | Name of transform wav file.                                   |                   |\n");
    printf("| -fc <HZ>      | Filter cutoff frequency.                                      |                   |\n");
    printf("| -N <order>    | Filter order.                                                 |                   |\n");
    printf("| -t <type>     | \"high\" or \"low\"                                               |                   |\n");
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

    /* Open the transform file */
    (*pp_self)->p_tfm_file=sf_open(p_cfg->p_tfm_name,SFM_WRITE,&sf_out_info);
    if (NULL == (*pp_self)->p_tfm_file)
    {
        int err;
        err = sf_error((*pp_self)->p_tfm_file);
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

    (*pp_self)->N = p_cfg->N;
    (*pp_self)->p_filter = (float*)malloc(sizeof(float)*2*(*pp_self)->N);
    (*pp_self)->fc = p_cfg->fc;
    (*pp_self)->b_type = p_cfg->b_type;

    if ((*pp_self)->b_type == FILTER_TYPE_LOWPASS)
    {
        fir_window_design_low_pass
                    ((*pp_self)->p_filter
                    ,(*pp_self)->fc
                    ,(*pp_self)->fs_in
                    ,(*pp_self)->N
                    );
    }
    else
    {
        printf("Hi\n");
        fir_window_design_high_pass
                    ((*pp_self)->p_filter
                    ,(*pp_self)->fc
                    ,(*pp_self)->fs_in
                    ,(*pp_self)->N
                    );
    }
    
    {
        convolve_cfg cfg;
        cfg.M=(*pp_self)->N;
        cfg.p_filt_coef = (*pp_self)->p_filter;
        convolve_init(&(*pp_self)->p_convolve,&cfg);
    }
    

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
    int n_out;
    while (p_self->block_size == sf_readf_float(p_self->p_in_file,p_self->p_in,p_self->block_size))
    {

        filter_frontend_rebuf(p_self->p_in,p_self->p_in_rebuf,p_self->block_size);

        convolve_overlap_add(p_self->p_convolve,p_self->p_in_rebuf,p_self->block_size,p_self->p_filtered,&n_out);

        filter_frontend_unbuf(p_self->p_filtered,p_self->p_out,p_self->block_size);

        /*fft_forward_process(p_self->p_in_rebuf,p_self->p_bins,p_self->block_size); */

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
    p_cfg->p_tfm_name = "tfm.wav";
    p_cfg->N = 11;
    p_cfg->b_type = FILTER_TYPE_LOWPASS;
    p_cfg->fc = 1000;
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
        else if (0 == strcmp(argv[0], "-fc"))
        {
            p_cfg->fc = (unsigned int)atoi(argv[1]);
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-N"))
        {
            p_cfg->N = (unsigned int)atoi(argv[1]);
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-t"))
        {
        	char *p_type;
            p_type = argv[1];
            if (strcmp(p_type,"low") == 0)
            {
            	p_cfg->b_type = FILTER_TYPE_LOWPASS;
            }
            else if (strcmp(p_type,"high") == 0)
            {
            	p_cfg->b_type = FILTER_TYPE_HIGHPASS;
            }
            else
            {
            	printf("Unknown filter type %s.", p_type);
            	return -1;
            }
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "--tfm"))
        {
            p_cfg->p_tfm_name = argv[1];
            argc--;
            argv++;
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