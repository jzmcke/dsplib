
#define BLOB_OK          (0)
#define BLOB_ERR         (-1)

#ifdef BLOB_FILE
typedef struct blob_files_state_s blob_files_state;

extern int _blob_file_start(char *blob_name);
extern int _blob_file_float_a(char *var_name, float *p_var_val, int n);
extern int _blob_file_int_a(char *var_name, int *p_var_val, int n);
extern int _blob_file_unsigned_int_a(char *var_name, unsigned int *p_var_val, int n);
extern int _blob_file_flush();

/* Creates the blob file if not already created */
#define BLOB_START(blob_name)                                      _blob_file_start(blob_name)
/* Appends an array of float values to a blob */
#define BLOB_FLOAT_A(var_name, p_var_val, n)                       _blob_file_float_a(var_name, p_var_val, n)
/* Appends an array of int values to a blob */
#define BLOB_INT_A(var_name, p_var_val, n)                         _blob_file_int_a(var_name, p_var_val, n)
/* Appends an array of unsigned int values to a blob */
#define BLOB_UNSIGNED_INT_A(var_name, p_var_val, n)                _blob_file_unsigned_int_a(var_name, p_var_val, n)
/* Flushes the memory and saves to file. */
#define BLOB_FLUSH()                                               _blob_file_flush()
#elif BLOB_SOCKET

/* Blob socket format:
   int: n_blobs
   int: n_children
   int */
typedef struct blob_socket_state_s blob_socket_state;

extern int _blob_socket_start(char *blob_name);
extern int _blob_socket_float_a(char *var_name, float *p_var_val, int n);
extern int _blob_socket_int_a(char *var_name, int *p_var_val, int n);
extern int _blob_socket_unsigned_int_a(char *var_name, unsigned int *p_var_val, int n);
extern int _blob_socket_flush();

/* Creates the blob file if not already created */
#define BLOB_START(node_name)                                      _blob_socket_start(node_name)
/* Appends an array of float values to a blob */
#define BLOB_FLOAT_A(var_name, p_var_val, n)                       _blob_socket_float_a(var_name, p_var_val, n)
/* Appends an array of int values to a blob */
#define BLOB_INT_A(var_name, p_var_val, n)                         _blob_socket_int_a(var_name, p_var_val, n)
/* Appends an array of unsigned int values to a blob */
#define BLOB_UNSIGNED_INT_A(var_name, p_var_val, n)                _blob_socket_unsigned_int_a(var_name, p_var_val, n)
/* Flushes the memory and saves to file. */
#define BLOB_FLUSH()                                               _blob_socket_flush()
#endif