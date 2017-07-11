#include "huffman_common.h"

int
huffman_decode_process
	(huffman_table *p_table
	,char *p_in
	,size_t n_data
	,unsigned int *p_out
	,unsigned int *p_n_out
	);

int
huffman_decode_lookup_table
	(huffman_table *p_table
	,unsigned int code
	,unsigned int n_bits
	,unsigned int *p_symbol
	);