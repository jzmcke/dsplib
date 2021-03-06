#include "blob/include/blob.h"
#include "blob_core.h"
#include <string.h>
#include <assert.h>
#include <stdlib.h>

#define MAX_NODENAME_LEN (128)
#define MAX_CHILD_NODES (32)
#define MAX_BLOBS_PER_NODE (32)

#define BLOB_SOCKET_SERVER_ADDRESS "localhost"
#define BLOB_SOCKET_SERVER_PORT (8000)

typedef struct blob_node_s
{
    char                 p_name[MAX_NODENAME_LEN];  /* Name of this blob node. */
    int                  blob_idx;
    blob                *p_blob; /* For repeated calls inside a node. All blobs should be identical, except for their data.. */
    int                  n_children;
    struct blob_node_s    *ap_child_nodes[MAX_CHILD_NODES];
    struct blob_node_s    *p_parent_node;
    size_t               blob_size; /* Only valid after complete traverse */
} blob_node;

int _blob_socket_init(char *addr, int port);

int _blob_socket_start(char *node_name);
int _blob_socket_float_a(char *var_name, float *p_var_val, int n);
int _blob_socket_int_a(char *var_name, int *p_var_val, int n);
int _blob_socket_unsigned_int_a(char *var_name, unsigned int *p_var_val, int n);
int __blob_aggregate_data(blob_node *p_node, size_t *p_size);
int __blob_assemble_data(blob_node *p_node, unsigned char *p_data, size_t *p_size);
int _blob_node_close(blob_node **pp_blob_node);
int _blob_socket_flush();

struct blob_socket_state_s
{
    blob_node *p_cur_node;
};

typedef struct blob_socket_state_s blob_socket_state;

blob_socket_state blob_ss = {0};

int
_blob_socket_init(char *addr, int port)
{

}

int
_blob_node_close(blob_node **pp_blob_node)
{
    blob_node *p_blob_node = *pp_blob_node;
    for (int i=0; i<p_blob_node->n_children; i++)
    {
        /* Must free child nodes before attempting to free this node */
        assert(NULL == p_blob_node->ap_child_nodes[i]);
        return BLOB_ERR;
    }

    blob_close(&p_blob_node->p_blob);
    free(*pp_blob_node);
    return BLOB_OK;
}

int
_blob_socket_start(char *node_name)
{
    blob_node *p_parent_temp;
    if (NULL == blob_ss.p_cur_node)
    {
        blob_ss.p_cur_node = (blob_node*)calloc(sizeof(blob_node), 1);
        strcpy(blob_ss.p_cur_node->p_name, node_name);
    }
    else
    {   
        int b_found_node = 0;
        for (int i=0; i<blob_ss.p_cur_node->n_children; i++)
        {
            if (0 == strcmp(blob_ss.p_cur_node->ap_child_nodes[i]->p_name, node_name))
            {
                p_parent_temp = blob_ss.p_cur_node;
                blob_ss.p_cur_node = blob_ss.p_cur_node->ap_child_nodes[i];
                blob_ss.p_cur_node->p_parent_node = p_parent_temp;
                b_found_node = 1;
            }
        }
        if (!b_found_node)
        {
            p_parent_temp = blob_ss.p_cur_node;
            /* Create the node */
            blob_ss.p_cur_node->ap_child_nodes[blob_ss.p_cur_node->n_children] = (blob_node*)calloc(sizeof(blob_node), 1);
            
            /* Switch into the node */
            blob_ss.p_cur_node = blob_ss.p_cur_node->ap_child_nodes[blob_ss.p_cur_node->n_children];
            blob_ss.p_cur_node->p_parent_node = p_parent_temp;
            blob_ss.p_cur_node->p_parent_node->n_children++;
            strcpy(blob_ss.p_cur_node->p_name, node_name);
        }
    }
    return BLOB_OK;
}

int
_blob_socket_float_a(char *var_name, float *p_var_val, int n)
{
    if (NULL == blob_ss.p_cur_node->p_blob)
    {
        /* Create a new blob */
        blob_cfg cfg;
        cfg.protocol = BLOB_PROTOCOL_VERSION_FIRST;
        blob_init(&blob_ss.p_cur_node->p_blob, &cfg);
    }
    blob_float_a(blob_ss.p_cur_node->p_blob, var_name, p_var_val, n);
    return BLOB_OK;
}

int
_blob_socket_int_a(char *var_name, int *p_var_val, int n)
{
    if (NULL == blob_ss.p_cur_node->p_blob)
    {
        /* Create a new blob */
        blob_cfg cfg;
        cfg.protocol = BLOB_PROTOCOL_VERSION_FIRST;
        blob_init(&blob_ss.p_cur_node->p_blob, &cfg);
    }
    blob_int_a(blob_ss.p_cur_node->p_blob, var_name, p_var_val, n);
    return BLOB_OK;
}

int
_blob_socket_unsigned_int_a(char *var_name, unsigned int *p_var_val, int n)
{
    if (NULL == blob_ss.p_cur_node->p_blob)
    {
        /* Create a new blob */
        blob_cfg cfg;
        cfg.protocol = BLOB_PROTOCOL_VERSION_FIRST;
        blob_init(&blob_ss.p_cur_node->p_blob, &cfg);
    }
    blob_unsigned_int_a(blob_ss.p_cur_node->p_blob, var_name, p_var_val, n);
    return BLOB_OK;
}

int
_blob_socket_flush()
{
    if (NULL == blob_ss.p_cur_node->p_parent_node)
    {
        FILE *p_file;
        size_t total_size;
        unsigned char *p_full_tree_blob;
        size_t total_size_copied;

        /* This is the root node */
        __blob_aggregate_data(blob_ss.p_cur_node, &total_size);
        p_full_tree_blob = (unsigned char*)calloc(sizeof(unsigned char), total_size);
        memset(p_full_tree_blob, 0, total_size);
    
        total_size_copied = total_size;
        /* Now, serialise the data */
        __blob_assemble_data(blob_ss.p_cur_node, p_full_tree_blob, &total_size_copied);

        /* All data be filled */
        assert(total_size_copied == 0);

        /* For now, write to file */
        p_file = fopen("socket", "a");
        fwrite(p_full_tree_blob, total_size, 1, p_file);
        fclose(p_file);
        free(p_full_tree_blob);
        _blob_node_close(&blob_ss.p_cur_node);
    }
    else
    {
        size_t this_blob_size;
        unsigned char *p_data;
        if (NULL != blob_ss.p_cur_node->p_blob)
        {
            blob_get_data(blob_ss.p_cur_node->p_blob, &p_data, &this_blob_size);
        }
        else
        {
            blob_ss.p_cur_node->blob_size = 0;
        }
        
        
        blob_ss.p_cur_node->blob_size = this_blob_size;
        blob_ss.p_cur_node = blob_ss.p_cur_node->p_parent_node;
    }
    
    return BLOB_OK;
}

int
__blob_header_get_size(blob *p_blob, size_t *p_size)
{
    int *p_var_len;
    int *p_var_types;
    char *p_var_names;
    int n_vars;
    int n_repetitions;
    size_t total_size = 0;

    /* All blobs are the same, just with different data */
    blob_get_info(p_blob,
                  &p_var_len,
                  &p_var_types,
                  &p_var_names,
                  &n_vars,
                  &n_repetitions);

    total_size += n_vars * sizeof(int); /* Store the length of each variable */
    total_size += n_vars * sizeof(int); /* Store the types of each variable */
    total_size += n_vars * sizeof(char) * BLOB_MAX_VAR_NAME_LEN; /* Store the variable names */
    total_size += sizeof(int); /* Store the number of variables in the blob */
    total_size += sizeof(int); /* Store the number of repetititions of the variables in the blob */
    *p_size = total_size;
    return 0;
}

int
__blob_aggregate_data(blob_node *p_node, size_t *p_size)
{
    size_t total_size = 0;

    size_t tmp_size;
    unsigned char *p_data;
    if (NULL != p_node->p_blob)
    {
        blob_get_data(p_node->p_blob, &p_data, &tmp_size);
        total_size += tmp_size;
    }
    
    for (int i=0; i<p_node->n_children; i++)
    {
        size_t tmp_size = 0;
        unsigned char *p_data;
        __blob_aggregate_data(p_node->ap_child_nodes[i], &tmp_size);
        total_size += tmp_size;
    }

    if (NULL != p_node->p_blob)
    {
        __blob_header_get_size(p_node->p_blob, &tmp_size);
        total_size += tmp_size;
    }
    
    total_size += sizeof(int); /* node has blob ? */
    total_size += sizeof(unsigned int); /* number of children */
    total_size += sizeof(char) * MAX_NODENAME_LEN; /* name of the node */

    p_node->blob_size = total_size;
    *p_size = total_size;
    return 0;
}

int
__blob_assemble_data(blob_node *p_node, unsigned char *p_data, size_t *p_size)
{
    size_t total_size = 0;
    size_t size_left = *p_size;
    int n_initial_children = p_node->n_children;
    while (p_node->n_children > 0)
    {
        size_t tmp_size;
        __blob_assemble_data(p_node->ap_child_nodes[p_node->n_children-1], p_data, &size_left);
        _blob_node_close(&p_node->ap_child_nodes[p_node->n_children-1]);
        p_node->n_children--;
    }

    {
        int  *p_var_len;
        int  *p_var_types;
        char *p_var_names;
        int   n_vars;
        int   n_repetitions;
        unsigned char *p_blob_data;
        size_t blob_size;
        int b_node_has_blob;

        /* This node has no children. Copy blob contents over. */
        if (NULL != p_node->p_blob)
        {
            b_node_has_blob = 1;
            blob_get_data(p_node->p_blob, &p_blob_data, &blob_size);
            memcpy(p_data + size_left - blob_size, p_blob_data, blob_size);
            size_left -= blob_size;
            /* write the blob header */
            blob_get_info(p_node->p_blob,
                            &p_var_len,
                            &p_var_types,
                            &p_var_names,
                            &n_vars,
                            &n_repetitions);
            /* Copy the variable lengths into the buffer */
            memcpy(p_data + size_left - (n_vars * sizeof(int)), p_var_len, n_vars * sizeof(int));
            size_left -= (n_vars * sizeof(int));
            /* Copy the variable of types into the buffer */
            memcpy(p_data + size_left - (n_vars * sizeof(int)), p_var_types, n_vars * sizeof(int));
            size_left -= (n_vars * sizeof(int));
            /* Copy the variable names into the buffer */
            memcpy(p_data + size_left - (n_vars * sizeof(char) * BLOB_MAX_VAR_NAME_LEN), p_var_names, n_vars * sizeof(char) * BLOB_MAX_VAR_NAME_LEN);
            size_left -= (n_vars * sizeof(char) * BLOB_MAX_VAR_NAME_LEN);
            /* Copy the number of variables per blob */
            memcpy(p_data + size_left - sizeof(int), &n_vars, sizeof(int));
            size_left -= sizeof(int);
            /* Copy the number of repetitions of the variables in each blob */
            memcpy(p_data + size_left - sizeof(int), &n_repetitions, sizeof(int));
            size_left -= sizeof(int);
        }
        else
        {
            b_node_has_blob = 0;
            blob_size = 0;
        }
        
        /* Write if this node has a blob */
        memcpy(p_data + size_left - sizeof(int), &b_node_has_blob, sizeof(int));
        size_left -= sizeof(int);

        /* Write the number of children */
        memcpy(p_data + size_left - sizeof(int), &n_initial_children, sizeof(int));
        size_left -= sizeof(int);
        /* Write the name of the node */
        memcpy(p_data + size_left - (sizeof(char) * MAX_NODENAME_LEN), p_node->p_name, sizeof(char) * MAX_NODENAME_LEN);
        size_left -= (sizeof(char) * MAX_NODENAME_LEN);
    }
    *p_size = size_left;
    return 0;
}   
