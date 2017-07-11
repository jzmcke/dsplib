#include "huffman_common.h"

typedef struct symbol_freq_elem_s symbol_freq_elem;

int
huffman_encode_process
	(huffman_table *p_table
	,unsigned int *p_in
	,unsigned int n_data
	,char *p_out
	,size_t *p_n_out
	);

int
huffman_encode_get_table
	(huffman_table **p_table
	,unsigned int *p_symbols
	,unsigned int n_symbols
	);

void
huffman_encode_print_huffman_table(huffman_table *p_table);

int
huffman_entry_get_n_bits(huffman_entry *p_entry);
