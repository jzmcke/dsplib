#include <stdlib.h>
#include <stdio.h>

int
file_read_cplx_data
    (char *p_name
    ,float **pp_data
    ,float *p_num_data
    )
{
    float d1, d2;
    FILE *p_file;
    p_file = fopen(p_name, "r");
    if (NULL == p_file)
    {
        fprintf(stderr,"Error opening file for read\n");
        return -1;
    }

    while (2 == fscanf(p_file,"%f,%f\n",&d1,&d2))
        (*p_num_data)++;

    fclose(p_file);

    *pp_data = (float *)malloc(2*sizeof(float)*(*p_num_data));

    p_file = fopen(p_name, "r");
    if (NULL == p_file)
    {
        fprintf(stderr,"Error re-opening file for read\n");
        return -1;
    }
    for (int i=0; i<(*p_num_data); i++)
    {
        fscanf(p_file,"%f,%f\n",&p_num_data[2*i],&p_num_data[2*i+1]);
    }
    return 0;
}

int
file_write_cplx_data
    (char *p_name
    ,float *p_data
    ,int num_data
    )
{
    FILE *p_file;
    p_file = fopen(p_name,"w+");

    if (NULL == p_file)
    {
        fprintf(stderr,"Error opening file for read\n");
        return -1;
    }
    for (int i=0; i<num_data; i++)
    {
        fprintf(p_file,"%f,%f\n",p_data[2*i], p_data[2*i+1]);
    }
    fclose(p_file);
    return 0;
}
