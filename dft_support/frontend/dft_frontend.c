#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <math.h>

#ifdef HAVE_SNDFILE
#include "sndfile.h"
#endif
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
#ifdef HAVE_SNDFILE
    SNDFILE *s1, *s2;
    SF_INFO wav_cfg;
#endif

    ret = parse_args(argc,argv,&cfg);

    if (0 != ret)
    {  
        print_usage();
        return -1;
    }
    sig_len = (unsigned int)(cfg.t_secs*SAMPLE_RATE);

    unsigned int n_fft = 1;
    while (n_fft < sig_len) n_fft <<= 1;

#ifdef HAVE_SNDFILE
    wav_cfg.frames = cfg.t_secs;
    wav_cfg.samplerate = SAMPLE_RATE;
    wav_cfg.channels = 1;
    wav_cfg.format = SF_FORMAT_WAV|SF_FORMAT_FLOAT;
    wav_cfg.sections = 1;
    wav_cfg.seekable = 0;
#endif

    p_in = calloc(2*n_fft,sizeof(float));
    p_dft = calloc(2*n_fft,sizeof(float));
    p_inv = calloc(2*n_fft,sizeof(float));
    p_write_buf = calloc(n_fft,sizeof(float));

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

    sprintf(p_write_name, "%s_in", cfg.p_output_name);
    file_write_cplx_data
        (p_write_name
        ,p_in
        ,sig_len
        );

#ifdef HAVE_SNDFILE
    sprintf(p_write_name, "%s_in.wav", cfg.p_output_name);
    s1 = sf_open(p_write_name, SFM_WRITE, &wav_cfg);

    for (int i=0; i<sig_len; i++)
    {
        /* Get real part only */
        p_write_buf[i] = p_in[2*i];
    }

    sf_writef_float(s1,p_write_buf,sig_len);
    sf_close(s1);
#endif

    printf("FFT size: %u (padded from %u)\n", n_fft, sig_len);
    fft_forward_process(p_in,p_dft,n_fft);

    sprintf(p_write_name, "%s_dft", cfg.p_output_name);
    file_write_cplx_data
        (p_write_name
        ,p_dft
        ,n_fft
        );

    fft_inverse_process(p_dft,p_inv,n_fft);


    sprintf(p_write_name, "%s_inv", cfg.p_output_name);
    file_write_cplx_data
        (p_write_name
        ,p_inv
        ,n_fft
        );

    for (int i=0; i<sig_len; i++)
    {
        /* Get real part only */
        p_write_buf[i] = p_inv[2*i];
    }

#ifdef HAVE_SNDFILE
    sprintf(p_write_name, "%s_inv.wav", cfg.p_output_name);
    s2 = sf_open(p_write_name, SFM_WRITE, &wav_cfg);
    sf_writef_float(s2,p_write_buf,sig_len);
    sf_close(s2);
#endif

    free(p_in);
    free(p_dft);
    free(p_inv);
    free(p_write_buf);

    return 0;
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
