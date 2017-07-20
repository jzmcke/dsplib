#ifndef HUFFMAN_DECODE_H
#define HUFFMAN_DECODE_H

#include "huffman/include/huffman_common.h"

int
huffman_decode_process
	(huffman_table *p_table
	,unsigned char *p_in
	,size_t n_data
	,unsigned int *p_out
	,unsigned int *p_n_out
	);

int
huffman_decode_lookup_table
	(huffman_table *p_table
	,bitstream *p_code
	,unsigned int *p_symbol
	);

#endif