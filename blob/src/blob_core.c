#include "blob_core.h"
#include "bitstream/include/bitstream.h"
#include <stdlib.h>
#include <assert.h>

#define BLOB_MAX_VARS_PER_BLOB      (32)
#define BLOB_VAR_TYPE_INT           (0)
#define BLOB_VAR_TYPE_FLOAT         (1)
#define BLOB_VAR_TYPE_UNSIGNED_INT  (2)

struct blob_s
{
    int             n_vars_in_blob;
    char            aa_var_names[BLOB_MAX_VARS_PER_BLOB][BLOB_MAX_VAR_NAME_LEN];
    int             a_var_types[BLOB_MAX_VARS_PER_BLOB];
    int             a_var_len[BLOB_MAX_VARS_PER_BLOB];
    int             n_repetitions;
    unsigned char  *p_blob_data; /* Blob of memory. Grows with every new allocation that exceeds current size */
    unsigned char  *p_root_blob_data;
    size_t          base_blob_size;
    size_t          total_blob_size;
    unsigned int    a_var_data_offsets[BLOB_MAX_VARS_PER_BLOB]; /* data is allocated on first write. subsequent writes must be the same size. */
    int             var_idx; /* the variable the blob is expecting to be written next */
};

int
blob_init(blob **pp_blob, blob_cfg *p_cfg)
{
    *pp_blob = (blob*)calloc(sizeof(blob), 1);
    (*pp_blob)->n_vars_in_blob = 0;
    (*pp_blob)->var_idx = 0;
    (*pp_blob)->n_repetitions = 0;
    (*pp_blob)->base_blob_size = 0;
    (*pp_blob)->total_blob_size = 0;
    (*pp_blob)->p_blob_data = NULL;
    (*pp_blob)->p_root_blob_data = NULL;
    for (int i=0; i<BLOB_MAX_VARS_PER_BLOB; i++)
    {
        (*pp_blob)->a_var_types[i] = 0;
        (*pp_blob)->a_var_len[i] = 0;
        (*pp_blob)->a_var_data_offsets[i] = 0;
        (*pp_blob)->aa_var_names[i][0] = '\0';
    }
    return BLOB_OK;
}

int
blob_close(blob **pp_blob)
{
    blob *p_blob = *pp_blob;
    if (p_blob != NULL)
    {
        if (NULL != p_blob->p_root_blob_data)
        {
            free(p_blob->p_root_blob_data);
        }
        free(*pp_blob);
    }
    return 0;
}

int
blob_float_a(blob *p_blob, char *p_var_name, float *p_var_val, int n)
{
    unsigned char *p_var_data;
    if (  (p_blob->var_idx == p_blob->n_vars_in_blob)
        &&(0 != strcmp(p_var_name, p_blob->aa_var_names[0]))
        )
    {
        unsigned char *p_new_blob;

        /* This occurs only when we are building the first repetition */
        assert(p_blob->n_repetitions == 0);

        /* The variable is new */
        if ((p_blob->n_vars_in_blob + 1) > BLOB_MAX_VARS_PER_BLOB)
        {
            /* Exceeded max size. */
            return BLOB_ERR;
        }

        /* Allocate new, larger blob, copy old blob into it. */
        p_new_blob = (unsigned char*)calloc(p_blob->base_blob_size + sizeof(float)*n, 1);
        memcpy(p_new_blob, p_blob->p_root_blob_data, p_blob->base_blob_size);
        free(p_blob->p_root_blob_data); /* Apparently it's OK to free a NULL pointer.. so the first variable made is all g :) cool */

        p_blob->p_root_blob_data = p_new_blob;
        p_blob->p_blob_data = p_new_blob;
        p_blob->a_var_data_offsets[p_blob->n_vars_in_blob] = p_blob->base_blob_size;
        
        p_blob->base_blob_size += n * sizeof(float);
        p_blob->total_blob_size = p_blob->base_blob_size;
    
        strcpy(p_blob->aa_var_names[p_blob->n_vars_in_blob], p_var_name);
        p_blob->a_var_len[p_blob->n_vars_in_blob] = n;
        p_blob->a_var_types[p_blob->n_vars_in_blob] = BLOB_VAR_TYPE_FLOAT;
        p_blob->n_vars_in_blob++;

    }
    else if (  (p_blob->var_idx == p_blob->n_vars_in_blob)
             &&(0 == strcmp(p_var_name, p_blob->aa_var_names[0]))
             )
    {
        unsigned char *p_new_repetition;
        p_new_repetition = (unsigned char*)calloc(p_blob->total_blob_size + p_blob->base_blob_size, 1);
        memcpy(p_new_repetition, p_blob->p_root_blob_data, p_blob->total_blob_size);
        free(p_blob->p_root_blob_data);
        p_blob->p_root_blob_data = p_new_repetition;
        p_blob->p_blob_data = p_blob->p_root_blob_data + p_blob->total_blob_size;
        p_blob->total_blob_size += p_blob->base_blob_size;
        p_blob->n_repetitions++;
    }
    
    /* If we are over the end, point back to the start */
    p_blob->var_idx = p_blob->var_idx % p_blob->n_vars_in_blob; 

    if (   (0 != strcmp(p_var_name, p_blob->aa_var_names[p_blob->var_idx]))
        || (BLOB_VAR_TYPE_FLOAT != p_blob->a_var_types[p_blob->var_idx])
        || (n != p_blob->a_var_len[p_blob->var_idx])
        )
    {
        /* Unexpected variable write operation */
        return BLOB_ERR;
    }

    p_var_data = &p_blob->p_blob_data[p_blob->a_var_data_offsets[p_blob->var_idx]];
    for (int i=0; i<n; i++)
    {
        *((float*)&p_var_data[i*sizeof(float)]) = (float)p_var_val[i];
    }

    p_blob->var_idx++;
    return BLOB_OK;
}

int
blob_int_a(blob *p_blob, char *p_var_name, int *p_var_val, int n)
{
    unsigned char *p_var_data;
    
    if (  (p_blob->var_idx == p_blob->n_vars_in_blob)
        &&(0 != strcmp(p_var_name, p_blob->aa_var_names[0])))
    {
        unsigned char *p_new_blob;

        /* This occurs only when we are building the first repetition */
        assert(p_blob->n_repetitions == 0);

        /* The variable is new */
        if ((p_blob->n_vars_in_blob + 1) > BLOB_MAX_VARS_PER_BLOB)
        {
            /* Exceeded max size. */
            return BLOB_ERR;
        }

        /* Allocate new, larger blob, copy old blob into it. */
        p_new_blob = (unsigned char*)calloc(p_blob->base_blob_size + sizeof(int32_t)*n, 1);
        memcpy(p_new_blob, p_blob->p_blob_data, p_blob->base_blob_size);
        p_blob->a_var_data_offsets[p_blob->n_vars_in_blob] = p_blob->base_blob_size;
        free(p_blob->p_blob_data);
        p_blob->p_blob_data = p_new_blob;
        p_blob->p_root_blob_data = p_new_blob;
        p_blob->base_blob_size += n * sizeof(int32_t);
        p_blob->total_blob_size = p_blob->base_blob_size;

        strcpy(p_blob->aa_var_names[p_blob->n_vars_in_blob], p_var_name);
        p_blob->a_var_len[p_blob->n_vars_in_blob] = n;
        p_blob->a_var_types[p_blob->n_vars_in_blob] = BLOB_VAR_TYPE_INT;
        p_blob->n_vars_in_blob++;
    }
    else if (  (p_blob->var_idx == p_blob->n_vars_in_blob)
             &&(0 == strcmp(p_var_name, p_blob->aa_var_names[0]))
             )
    {
        unsigned char *p_new_repetition;
        p_new_repetition = (unsigned char*)calloc(p_blob->total_blob_size + p_blob->base_blob_size, 1);
        memcpy(p_new_repetition, p_blob->p_root_blob_data, p_blob->total_blob_size);
        free(p_blob->p_root_blob_data);
        p_blob->p_root_blob_data = p_new_repetition;
        p_blob->p_blob_data = p_blob->p_root_blob_data + p_blob->total_blob_size;
        p_blob->total_blob_size += p_blob->base_blob_size;
        p_blob->n_repetitions++;
    }
    
    /* If we are over the end, point back to the start */
    p_blob->var_idx = p_blob->var_idx % p_blob->n_vars_in_blob; 

    /* Don't suddenly change the variable order! Blob requires this to be constant. */
    if (   (0 != strcmp(p_var_name, p_blob->aa_var_names[p_blob->var_idx]))
        || (BLOB_VAR_TYPE_INT != p_blob->a_var_types[p_blob->var_idx])
        || (n != p_blob->a_var_len[p_blob->var_idx])
        )
    {
        /* Unexpected variable write operation */
        return BLOB_ERR;
    }

    p_var_data = &p_blob->p_blob_data[p_blob->a_var_data_offsets[p_blob->var_idx]];
    for (int i=0; i<n; i++)
    {
        *((int32_t*)&p_var_data[i*sizeof(int32_t)]) = (int32_t)p_var_val[i];
    }

    p_blob->var_idx++;
    return BLOB_OK;
}

int
blob_unsigned_int_a(blob *p_blob, char *p_var_name, unsigned int *p_var_val, int n)
{
    unsigned char *p_var_data;

    if (   (p_blob->var_idx == p_blob->n_vars_in_blob)
        && (0 != strcmp(p_var_name, p_blob->aa_var_names[0]))
        )
    {
        unsigned char *p_new_blob;

        /* This occurs only when we are building the first repetition */
        assert(p_blob->n_repetitions == 0);

        /* The variable is new */
        if ((p_blob->n_vars_in_blob + 1) > BLOB_MAX_VARS_PER_BLOB)
        {
            /* Exceeded max size. */
            return BLOB_ERR;
        }

        /* Allocate new, larger blob, copy old blob into it. */
        p_new_blob = (unsigned char*)calloc(p_blob->base_blob_size + sizeof(u_int32_t)*n, 1);
        memcpy(p_new_blob, p_blob->p_blob_data, p_blob->base_blob_size);
        p_blob->a_var_data_offsets[p_blob->n_vars_in_blob] = p_blob->base_blob_size;
        free(p_blob->p_blob_data);
        p_blob->p_blob_data = p_new_blob;
        p_blob->p_root_blob_data = p_new_blob;
        p_blob->base_blob_size += n * sizeof(u_int32_t);
        p_blob->total_blob_size = p_blob->base_blob_size;

        strcpy(p_blob->aa_var_names[p_blob->n_vars_in_blob], p_var_name);
        p_blob->a_var_len[p_blob->n_vars_in_blob] = n;
        p_blob->a_var_types[p_blob->n_vars_in_blob] = BLOB_VAR_TYPE_UNSIGNED_INT;
        p_blob->n_vars_in_blob++;
    }
    else if (  (p_blob->var_idx == p_blob->n_vars_in_blob)
             &&(0 == strcmp(p_var_name, p_blob->aa_var_names[0]))
             )
    {
        unsigned char *p_new_repetition;
        p_new_repetition = (unsigned char*)calloc(p_blob->total_blob_size + p_blob->base_blob_size, 1);
        memcpy(p_new_repetition, p_blob->p_root_blob_data, p_blob->total_blob_size);
        free(p_blob->p_root_blob_data);
        p_blob->p_root_blob_data = p_new_repetition;
        p_blob->p_blob_data = p_blob->p_root_blob_data + p_blob->total_blob_size;
        p_blob->total_blob_size += p_blob->base_blob_size;
        p_blob->n_repetitions++;
    }
    
    /* If we are over the end, point back to the start */
    p_blob->var_idx = p_blob->var_idx % p_blob->n_vars_in_blob; 

    if (   (0 != strcmp(p_var_name, p_blob->aa_var_names[p_blob->var_idx]))
        || (BLOB_VAR_TYPE_UNSIGNED_INT != p_blob->a_var_types[p_blob->var_idx])
        || (n != p_blob->a_var_len[p_blob->var_idx])
        )
    {
        /* Unexpected variable write operation */
        return BLOB_ERR;
    }

    p_var_data = &p_blob->p_blob_data[p_blob->a_var_data_offsets[p_blob->var_idx]];
    for (int i=0; i<n; i++)
    {
        *((u_int32_t*)&p_var_data[i*sizeof(u_int32_t)]) = p_var_val[i];
    }

    p_blob->var_idx++;
    return BLOB_OK;
}

void
blob_get_data(blob *p_blob, unsigned char **pp_data, size_t *base_blob_size)
{
    *pp_data = p_blob->p_root_blob_data;
    *base_blob_size = p_blob->total_blob_size;
}

void
blob_get_info(blob *p_blob,
              int  **pp_var_len,
              int  **pp_var_types,
              char **pp_var_names,
              int  *n_vars,
              int  *n_repetitions)
{
    char (*pointer)[BLOB_MAX_VAR_NAME_LEN] = p_blob->aa_var_names;
    *pp_var_len = p_blob->a_var_len;
    *pp_var_types = p_blob->a_var_types;
    *pp_var_names = (char*)p_blob->aa_var_names;
    *n_vars = p_blob->n_vars_in_blob;
    *n_repetitions = p_blob->n_repetitions;
}

