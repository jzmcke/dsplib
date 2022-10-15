#ifndef BLOB_H
#define BLOB_H

#define BLOB_OK          (0)
#define BLOB_ERR         (-1)

/* Add the relevant define into the main.c/main.cpp, or makefile, of your project. */
// #define BLOB_ESP32_WEBSOCKETS
// #define BLOB_WEBSOCKETS
// #define BLOB_FILE

typedef struct blob_s blob;
blob *p_g_blob;

#ifdef BLOB_ESP32_WEBSOCKETS
    extern int _blob_espws_init(blob_comm_cfg*,const char*, int);
    extern int _blob_espws_terminate(blob_comm_cfg*);
    #define BLOB_INIT(address, port)                                   _blob_espws_init(&g_blob_ccfg, address, port); blob_init(&g_blob_ccfg)
    #define BLOB_TERMINATE()                                           _blob_espws_terminate(&g_blob_ccfg)
#elif BLOB_WEBSOCKETS
    extern int _blob_minws_init(blob_comm_cfg*, const char*, int);
    extern int _blob_minws_terminate(blob_comm_cfg*);
    #define BLOB_INIT(address, port)                                   _blob_minws_init(&blob_comm_cfg, address, port); blob_init(&blob_comm_cfg)
    #define BLOB_TERMINATE()                                           _blob_minws_terminate(&blob_comm_cfg)
#elif BLOB_FILE
   #define BLOB_INIT()                                                (void)
   #define BLOB_TERMINATE()                                           (void)
#endif



/* Creates the blob file if not already created */
#define BLOB_START(node_name)                                      blob_start(p_g_blob, node_name)
/* Appends an array of float values to a blob */
#define BLOB_FLOAT_A(var_name, p_var_val, n)                       blob_float_a(p_g_blob, var_name, p_var_val, n)
/* Appends an array of int values to a blob */
#define BLOB_INT_A(var_name, p_var_val, n)                         blob_int_a(p_g_blob, var_name, p_var_val, n)
/* Appends an array of unsigned int values to a blob */
#define BLOB_UNSIGNED_INT_A(var_name, p_var_val, n)                blob_unsigned_int_a(p_g_blob, var_name, p_var_val, n)
/* Returns the data and transmits it via the connection callback */
#define BLOB_FLUSH()                                               blob_flush(p_g_blob)

/* Retreives data from the connection and disassembles */
#define BLOB_RECEIVE_START(node_name)                                      blob_retrieve_start(p_g_blob, node_name)
/* Appends an array of float values to a blob */
#define BLOB_RECEIVE_FLOAT_A(var_name, pp_var_val, p_n, rep)               blob_retrieve_float_a(p_g_blob, var_name, pp_var_val, p_n, rep)
/* Appends an array of int values to a blob */
#define BLOB_RECEIVE_INT_A(var_name, pp_var_val, p_n, rep)                 blob_retrieve_int_a(p_g_blob, var_name, pp_var_val, p_n, rep)
/* Appends an array of unsigned int values to a blob */
#define BLOB_RECEIVE_UNSIGNED_INT_A(var_name, pp_var_val, p_n, rep)        blob_retrieve_unsigned_int_a(p_g_blob, var_name, pp_var_val, p_n, rep)
/* Flushes the memory. */
#define BLOB_RECEIVE_FLUSH()                                               blob_retrieve_flush(p_g_blob)

#endif
