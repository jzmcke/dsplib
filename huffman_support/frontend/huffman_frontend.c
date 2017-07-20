#include <stdio.h>
#include <stdlib.h>
#include "stats/include/stats.h"
#include <string.h>
#include "huffman/include/huffman_encode.h"
#include "huffman/include/huffman_decode.h"
#include "huffman/include/huffman_common.h"

#define DATA_RANGE ((1<<16)-1)
#define BITS_IN_BYTE 8
#define HUFFMAN_FRONTEND_MAX_NUM_MODES 100


typedef struct huff_test_s
{
    int b_dump_input_stats;
    int b_dump_compression_stats;

    /* Generate input file */
    char *p_gen_name;   
    float *p_mode_probs;
    unsigned int n_modes;
    unsigned int n_samples;
    unsigned int data_range;
    /* OR provide input file */
    char *p_input_name;             /* Pointer to input file */
    unsigned int byte_width;        /* Number of bytes per sample in p_binary_file */

    FILE *p_enc_file;
    FILE *p_dec_file;
} huff_test;

int
parse_args
    (huff_test *p_test
    ,int argc
    ,char **argv
    );

void
print_usage(void);

int
main(int argc, char **argv)
{
    int i;
    unsigned char *p_enc = NULL;
    unsigned int *p_dec = NULL;
    size_t n_enc; /* bytes */
    unsigned int n_dec;
    unsigned char *p_file_data = NULL;
    unsigned int *p_symbols = NULL;
    unsigned int mask = 0x0;
    unsigned int byte_width;
    size_t n_read;
    float av_bits_per_symbol;
    huffman_table *p_table;
    FILE *p_file;
    mode_stats stats = {0};
    huff_test test_args;
    int ret;

    ret = parse_args(&test_args,argc,argv);
    if (ret == -1)
    {
        print_usage();
        return -1;
    }

    p_enc = (unsigned char*)calloc(test_args.n_samples,sizeof(unsigned char));

    if (test_args.p_input_name != NULL)
    {
        byte_width = test_args.byte_width;
        p_file = fopen(test_args.p_input_name,"r");
    }
    else
    {
        stats_gen_rand_bin
            (&stats
            ,test_args.p_mode_probs
            ,test_args.n_modes
            ,test_args.data_range
            ,test_args.n_samples
            ,test_args.p_gen_name
            );
        byte_width = stats.byte_width;
        stats_print(&stats);
        p_file = fopen(test_args.p_gen_name,"r");
        
    }

    p_file_data = (unsigned char *)calloc(test_args.n_samples,byte_width);

    /* Convert binary file into raw symbol array */
    
    p_symbols = (unsigned int*)malloc(test_args.n_samples*sizeof(unsigned int));

    p_dec = (unsigned int*)malloc(test_args.n_samples*sizeof(unsigned int));

    n_read = fread(p_file_data,byte_width,test_args.n_samples,p_file);
    if (n_read != test_args.n_samples)
    {
        printf("Expected number of samples not equal to number of samples read\n");
        return -1;
    }

    mask = (1<<BITS_IN_BYTE) - 1;


    for (i=0; i<test_args.n_samples; i++)
    {
        int j;
        unsigned int data = 0;
        int base_sample_idx = i*byte_width;
        for (j = 0; j < byte_width; j++)
        {
            data ^= (((unsigned int) p_file_data[base_sample_idx+j])&mask) << (j * BITS_IN_BYTE);
        }
        p_symbols[i] = data;
    }

    fclose(p_file);


    huffman_encode_get_table(&p_table,p_symbols,test_args.n_samples);

    huffman_encode_print_huffman_table(p_table);

    av_bits_per_symbol = 0;
    for (i=0; i<p_table->n_entries; i++)
    {
        float prob;
        int n_bits;
        prob = 1.0*p_table->p_entries[i].freq/test_args.n_samples;
        n_bits = huffman_entry_get_n_bits(&p_table->p_entries[i]);
        
        av_bits_per_symbol += 1.0*prob*n_bits;
    }
    printf("Av bits per symbol = %f\n",av_bits_per_symbol);
    printf("Compression ratio  = %f\n",1.0*av_bits_per_symbol/(byte_width*BITS_IN_BYTE));

    huffman_encode_process(p_table,p_symbols,test_args.n_samples,p_enc,&n_enc);

    fwrite(p_enc,1,n_enc,test_args.p_enc_file);
    fclose(test_args.p_enc_file);
    huffman_decode_process(p_table,p_enc,n_enc,p_dec,&n_dec);

    for (i=0; i<n_dec; i++)
    {
        fwrite(&p_dec[i],byte_width,1,test_args.p_dec_file);
    }
    fclose(test_args.p_dec_file);

    return 0;
}

void
print_usage(void)
{
    printf("-----------------------------------------------------------------------------------------------------\n");
    printf("| Command       | Description                                                   | Default           |\n");
    printf("| --------------------------------------------------------------------------------------------------|\n");
    printf("| -i <in.bin>   | Provide an uncompressed binary file.                          |                   |\n");
    printf("| -g <gen.bin>  | Name of statistics file to generate. This is an alternative   |                   |\n");
    printf("|               | input to -i.                                                  |                   |\n");
    printf("| -e <enc.bin>  | File-name of the huffman encoded bitstream.                   |                   |\n");
    printf("| -d <dec.bin>  | File-name of the decompressed bistream (should be identical   |                   |\n");
    printf("|               | to the input file.                                            |                   |\n");
    printf("| -bw <width>   | If supplying an input file, input the symbol byte boundaries. |                   |\n");
    printf("| -m <m1,m2..mn>| For file generation only. Supply a list of probabilities so   |                   |\n");
    printf("|               | that a random set of symbols can be generated and assigned    |                   |\n");
    printf("|               | these probabilities.                                          |                   |\n");
    printf("| -r <range>    | The range over which the random symbols will be generated.    |                   |\n");
    printf("|               | They will be generated according to the distribution using -m.|                   |\n");
    printf("| -n <samples>  | Number of samples to generate if using -g to supply input.    |                   |\n");
    printf("-----------------------------------------------------------------------------------------------------\n");
}

int
parse_args(huff_test *p_test, int argc, char **argv)
{
    unsigned int n_args_processed = 0;
    char *p_current_arg;
    char *p_input_name = NULL;
    char *p_enc_name = "enc.bin";
    char *p_dec_name = "dec.bin";
    char *p_mode_list = NULL;
    unsigned int byte_width = -1;
    char *p_gen_name = NULL;
    unsigned int n_gen_samples = -1;
    unsigned int data_range = -1;
    char *p_token;
    unsigned int n_modes = 0;
    /* Skip binary name */
    argv++;
    while (n_args_processed < (argc-1))
    {
        p_current_arg = argv[0];
        if (strcmp(p_current_arg,"-i") == 0)
        {
            argv++;
            n_args_processed++;
            p_input_name = argv[0];

        }
        else if (strcmp(p_current_arg,"-g") == 0)
        {
            argv++;
            n_args_processed++;
            p_gen_name = argv[0];

        }
        else if (strcmp(p_current_arg,"-e") == 0)
        {
            argv++;
            n_args_processed++;
            p_enc_name = argv[0];
        }
        else if (strcmp(p_current_arg,"-d") == 0)
        {
            n_args_processed++;
            argv++;
            p_dec_name = argv[0];
        }
        else if (strcmp(p_current_arg,"-bw") == 0)
        {
            n_args_processed++;
            argv++;
            byte_width = atoi(argv[0]);
        }
        else if (strcmp(p_current_arg,"-m") == 0)
        {
            n_args_processed++;
            argv++;
            p_mode_list = argv[0];
        }
        else if (strcmp(p_current_arg,"-n") == 0)
        {
            n_args_processed++;
            argv++;
            n_gen_samples = atoi(argv[0]);
        }
        else if (strcmp(p_current_arg,"-r") == 0)
        {
            n_args_processed++;
            argv++;
            data_range = atoi(argv[0]);
        }
        else if (strcmp(p_current_arg,"-h") == 0)
        {
            return -1;
        }
        else
        {
            printf("Unknown argument supplied\n");
            return -1;
        }
        n_args_processed++;
        argv++;
    }
    if (p_input_name != NULL)
    {

        if (byte_width == -1)
        {
            printf("When supplying a file, the sample width in bytes of each symbol must be specified\n");
            return -1;
        }
        if (p_gen_name != NULL)
        {
            printf("Cannot generate a binary file and provide one\n");
            return -1;
        }
    }
    else if (p_input_name == NULL)
    {
        if (p_gen_name == NULL)
        {
            printf("Must supply either the name of a binary to generate, or the path to an existing binary\n");
            return -1;
        }
        if (   (n_modes != 0)
             &&(p_mode_list == NULL)
            )
        {
            printf("Must supply a list of probability modes\n");
            return -1;
        }
        if (n_gen_samples == -1)
        {
            printf("Must supply the number of symbols to randomly generate\n");
            return -1;
        }
        if (data_range == -1)
        {
            printf("Must supply the range of data over which to uniformly generate data\n");
            return -1;
        }
    }

    if (p_input_name == NULL)
    {
        p_test->p_mode_probs = (float*)malloc(sizeof(float)*HUFFMAN_FRONTEND_MAX_NUM_MODES);
        if (p_test->p_mode_probs == NULL)
        {
            printf("Error allocating space for mode probabilities\n");
            return -1;
        }
        p_token = strtok(p_mode_list,",\0 ");
        while (p_token != NULL)
        {
            p_test->p_mode_probs[n_modes] = atof(p_token);
            n_modes++;
            p_token = strtok(NULL,",");
        }
    }
    p_test->p_enc_file = fopen(p_enc_name,"w");
    if (!p_test->p_enc_file)
    {
        printf("Error opening encoded file\n");
        return -1;
    }

    p_test->p_dec_file = fopen(p_dec_name,"w");
    if (!p_test->p_dec_file)
    {
        printf("Error opening decoded file\n");
        return -1;
    }

    p_test->data_range = data_range;
    p_test->byte_width = byte_width;
    p_test->n_modes = n_modes;
    p_test->n_samples = n_gen_samples;
    p_test->p_input_name = p_input_name;
    p_test->p_gen_name = p_gen_name;
    
    return 0;
}