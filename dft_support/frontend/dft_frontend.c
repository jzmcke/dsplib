#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#include "sndfile.h"
#include "util/include/util.h"
#include "dft/include/dft.h"
#include "dft/include/file.h"

#define NUM_SECONDS 0.1
#define SAMPLE_RATE 32000
#define SAMPLE_PERIOD 1.0/SAMPLE_RATE

typedef struct dft_cfg_s
{
    char *p_type;
    char *p_output_name;
    float f;
    float t_secs;
} dft_cfg;

int
parse_args
    (int argc
    ,char **argv
    ,dft_cfg *p_cfg
    );

void
print_usage(void);

int
main
    (int argc
    ,char **argv
    )
{
    unsigned int sig_len;

    float *p_in;
    float *p_dft;
    float *p_inv;
    float *p_write_buf;

    char p_write_name[1024];
    int ret;
    dft_cfg cfg;
    SNDFILE *s1, *s2;
    SF_INFO wav_cfg;

    ret = parse_args(argc,argv,&cfg);

    if (0 != ret)
    {  
        print_usage();
        return -1;
    }
    sig_len = (unsigned int)(cfg.t_secs*SAMPLE_RATE);

    wav_cfg.frames = cfg.t_secs;
    wav_cfg.samplerate = SAMPLE_RATE;
    wav_cfg.channels = 1;
    wav_cfg.format = SF_FORMAT_WAV|SF_FORMAT_FLOAT;
    wav_cfg.sections = 1;
    wav_cfg.seekable = 0;

    p_in = calloc(2*sig_len,sizeof(float));
    p_dft = calloc(2*sig_len,sizeof(float));
    p_inv = calloc(2*sig_len,sizeof(float));
    p_write_buf = calloc(sig_len,sizeof(float));

    /* Delta function */
    if (0 == strcmp(cfg.p_type,"delta"))
    {
        p_in[0] = 1;
        p_in[1] = 1;
    }
    else if (0 == strcmp(cfg.p_type,"sin"))
    {
        float k = cfg.f*SAMPLE_PERIOD;
        for (int n=0; n<sig_len; n++)
        {
            p_in[2*n] = cos(2*M_PI*k*n);
        }
    }
    else if (0 == strcmp(cfg.p_type,"test"))
    {
        for (int n = 0; n<sig_len;n++)
        {
            p_in[2*n] = n;
            p_in[2*n+1] = 0;
        }
    }

    strcpy(p_write_name,cfg.p_output_name);

    file_write_cplx_data
        (strcat(p_write_name,"_in")
        ,p_in
        ,sig_len
        );

    s1 = sf_open(strcat(p_write_name,".wav"), SFM_WRITE, &wav_cfg);

    for (int i=0; i<sig_len; i++)
    {
        /* Get real part only */
        p_write_buf[i] = p_in[2*i];
    }

    sf_writef_float(s1,p_write_buf,sig_len);
    
    fft_forward_process(p_in,p_dft,sig_len);

    strcpy(p_write_name,cfg.p_output_name);
    file_write_cplx_data
        (strcat(p_write_name,"_dft")
        ,p_dft
        ,sig_len
        );

    fft_inverse_process(p_dft,p_inv,sig_len);


    strcpy(p_write_name,cfg.p_output_name);

    for (int i=0; i<sig_len; i++)
    {
        /* Get real part only */
        p_write_buf[i] = p_inv[2*i];
    }

    file_write_cplx_data
        (strcat(p_write_name,"_inv")
        ,p_inv
        ,sig_len
        );

    s2 = sf_open(strcat(p_write_name,".wav"), SFM_WRITE, &wav_cfg);
    sf_writef_float(s2,p_write_buf,sig_len);

}

void
print_usage(void)
{
    printf("-----------------------------------------------------------------------------------------------------\n");
    printf("| Command       | Description                                                   | Default           |\n");
    printf("| --------------------------------------------------------------------------------------------------|\n");
    printf("| -t <type>     | Type of input \"delta\" or \"sin\"                                |                   |\n");
    printf("| -f <freq>     | Frequency (Hz) of the sin wave if type sin.                   |                   |\n");
    printf("| -o <enc.bin>  | Base output name. Prints text files of transform pair and a   |                   |\n");
    printf("|               | .wav file.                                                    |                   |\n");
    printf("| -l <secs>     | Time in seconds of the generated tone.                        |                   |\n");
    printf("-----------------------------------------------------------------------------------------------------\n");
}

int
parse_args
    (int argc
    ,char **argv
    ,dft_cfg *p_cfg
    )
{
    /* Skip function name */
    argc--;
    argv++;
    while (argc)
    {
        if (0 == strcmp(argv[0],"-t"))
        {
            p_cfg->p_type = argv[1];
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-o"))
        {
            p_cfg->p_output_name = argv[1];
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-f"))
        {
            p_cfg->f = atof(argv[1]);
            argc--;
            argv++;
        }
        else if (0 == strcmp(argv[0], "-l"))
        {
            p_cfg->t_secs = atof(argv[1]);
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
