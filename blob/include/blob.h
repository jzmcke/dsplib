
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
extern int _blob_socket_init(char *addr, int port);
extern int _blob_socket_retrieve_start(char *blob_name);
extern int _blob_socket_retrieve_float_a(char *var_name, const float **pp_var_val, int *p_n, int rep);
extern int _blob_socket_retrieve_int_a(char *var_name, const int **pp_var_val, int *p_n, int rep);
extern int _blob_socket_retrieve_unsigned_int_a(char *var_name, const unsigned int **pp_var_val, int *p_n, int rep);
extern int _blob_socket_retrieve_flush();

extern int _blob_socket_terminate();

/* Tie the blob to a network location */
#define BLOB_SOCKET_INIT(address, port)                            _blob_socket_init(address, port)
/* Terminate a socket connection */
#define BLOB_SOCKET_TERMINATE()                                    _blob_socket_terminate()


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
                           

#define BLOB_RECEIVE_START(node_name)                                      _blob_socket_retrieve_start(node_name)
/* Appends an array of float values to a blob */
#define BLOB_RECEIVE_FLOAT_A(var_name, pp_var_val, p_n, rep)               _blobsocket_retrieve_float_a(var_name, pp_var_val, p_n, rep)
/* Appends an array of int values to a blob */
#define BLOB_RECEIVE_INT_A(var_name, pp_var_val, p_n, rep)                 _blob_socket_retrieve_int_a(var_name, pp_var_val, p_n, rep)
/* Appends an array of unsigned int values to a blob */
#define BLOB_RECEIVE_UNSIGNED_INT_A(var_name, pp_var_val, p_n, rep)        _blob_socket_retrieve_unsigned_int_a(var_name, pp_var_val, p_n, rep)
/* Flushes the memory and saves to file. */
#define BLOB_RECEIVE_FLUSH()                                               _blob_socket_retrieve_flush()

#endif