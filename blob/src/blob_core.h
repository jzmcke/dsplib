#include <stdio.h>

#define BLOB_PROTOCOL_VERSION_FIRST (1)
#define BLOB_MAX_VAR_NAME_LEN       (128)

#define BLOB_ERR                    (-1)
#define BLOB_OK                     (0)

typedef struct blob_s blob;

typedef struct blob_cfg_s
{
    unsigned int protocol;
} blob_cfg;

int
blob_close(blob **pp_blob);

int
blob_init(blob **pp_blob, blob_cfg *p_cfg);

int
blob_float_a(blob *p_blob, char *p_var_name, float *p_var_val, int n);

int
blob_int_a(blob *p_blob, char *p_var_name, int *p_var_val, int n);

int
blob_unsigned_int_a(blob *p_blob, char *p_var_name, unsigned int *p_var_val, int n);

void
blob_get_data(blob *p_blob, unsigned char **pp_data, size_t *blob_size);

void
blob_get_info(blob *p_blob, int **pp_blob_len, int **pp_var_types, char **pp_var_names, int *n_vars, int *n_repititions);