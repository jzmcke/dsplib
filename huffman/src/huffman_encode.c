#include <stdio.h>
#include <stdlib.h>
#include "huffman_encode.h"

#define HUFFMAN_ENCODE_OK 0
#define HUFFMAN_ENCODE_ERROR -1
#define HUFFMAN_ENCODER_MAX_CODEWORD_SIZE_BYTES (64) /* Yes, I mean 64 bytes */
#define BITS_IN_INT (32)

struct symbol_freq_elem_s
{
	unsigned int symbol;
	unsigned int freq;
	struct symbol_freq_elem_s *p_to_leaf_node1;
	struct symbol_freq_elem_s *p_to_leaf_node2;
};

void
huffman_encode_free_freq_elem(symbol_freq_elem **pp_elem);

int
huffman_encode_process
	(huffman_table *p_table
	,unsigned int *p_in
	,unsigned int n_data
	,char *p_out
	,size_t *p_n_out
	)
{
	int ret;
	bitstream *p_stream;
	int i;
	ret = bitstream_attach_array(&p_stream,p_out,BITSTREAM_BIG_ENDIAN);
	if (ret == BITSTREAM_ERROR)
	{
		printf("Error opening bitstream\n");
		return HUFFMAN_ENCODE_ERROR;
	}

	/* Write the header to the huffman bitstream */

	/* Symbol number */
	ret = bitstream_add_bits(p_stream,n_data,8);
	if (ret == BITSTREAM_ERROR)
	{	
		printf("Error adding header to bitstream\n");
		return HUFFMAN_ENCODE_ERROR;
	}

	/* TODO: Add table */

	for (i=0; i<n_data; i++)
	{
		int n;
		int table_idx = HUFFMAN_ENCODE_ERROR;
		for (n=0; n<p_table->n_entries; n++)
		{
			if (p_table->p_entries[n].symbol == p_in[i])
			{
				table_idx = n;
			}
		}
		if (table_idx == HUFFMAN_ENCODE_ERROR)
		{
			printf("Cannot locate codeword in Huffman table\n");
			return HUFFMAN_ENCODE_ERROR;
		}
		ret = bitstream_concatenate(p_stream,p_table->p_entries[table_idx].p_code);
		if (ret == BITSTREAM_ERROR)
		{
			printf("Error adding bits to bitstream\n");
			return HUFFMAN_ENCODE_ERROR;
		}
	}
	*p_n_out = bitstream_get_num_bytes(p_stream); /* Includes header info */
	return HUFFMAN_ENCODE_OK;
}

void
huffman_encode_print_map
	(symbol_freq_elem *p_symbol_freq_map
	,int n_elem
	);

void
huffman_encode_table_print_freq_elem
	(symbol_freq_elem *p_elem);

void
huffman_encode_table_print_layer
	(symbol_freq_elem **pp_elem
	,int n
	);

int
huffman_encode_traverse_tree
	(huffman_table *p_table
	,symbol_freq_elem **pp_elem
	,bitstream *p_bs_code_word
	);

int
huffman_encode_get_table
	(huffman_table **pp_table
	,unsigned int *p_symbols
	,unsigned int n_symbols
	)
{
	int i;
	int idx;
	int num_unique_symbols = 0;
	symbol_freq_elem *p_symbol_freq_map;
	symbol_freq_elem **pp_current_layer_nodes;
	unsigned int n_layer_nodes;
	bitstream *p_bs_zeros;
	char p_zeros[HUFFMAN_ENCODER_MAX_CODEWORD_SIZE_BYTES] = {0};

	bitstream_attach_array(&p_bs_zeros,p_zeros,BITSTREAM_BIG_ENDIAN);

	p_symbol_freq_map = (symbol_freq_elem*)calloc(n_symbols,sizeof(symbol_freq_elem));
	pp_current_layer_nodes = (symbol_freq_elem**)calloc(n_symbols,sizeof(symbol_freq_elem*));
	*pp_table = (huffman_table*)calloc(1,sizeof(huffman_table));
	(*pp_table)->p_entries = calloc(n_symbols,sizeof(huffman_entry));
	(*pp_table)->n_entries = 0;

	for (i=0; i<n_symbols; i++)
	{
		symbol_freq_elem *p_elem;
		/* Search through the symbol_freq_map for the current symbol, add the entry if non-existent */
		p_elem = p_symbol_freq_map;
		idx = 0;

		while ( (p_elem->freq > 0) && (p_elem->symbol != p_symbols[i]))
		{
			p_elem++;
			idx++;
		}
		p_elem->freq++;
		p_elem->symbol = p_symbols[i];
		p_elem->p_to_leaf_node1 = NULL;
		p_elem->p_to_leaf_node2 = NULL;

		if ((idx+1) > num_unique_symbols)
		{
			num_unique_symbols = idx+1;
		}

		/* Ensure list remains ordered with respect to frequency */
		while ((p_elem->freq > (p_elem-1)->freq) && (p_elem != p_symbol_freq_map))
		{
				symbol_freq_elem t;
				t = *(p_elem-1);
				*(p_elem-1) = *(p_elem);
				*(p_elem) = t;
				p_elem--;
		}
	}

	for (i=0; i<num_unique_symbols; i++)
	{
		pp_current_layer_nodes[i] = p_symbol_freq_map+i;
	}
	for (i=num_unique_symbols; i<n_symbols; i++)
	{
		free(pp_current_layer_nodes[i]);
		pp_current_layer_nodes[i] = NULL;
	}

	n_layer_nodes = num_unique_symbols;
	while (n_layer_nodes != 1)
	{
		int m;
		symbol_freq_elem *p_new_node;
		symbol_freq_elem *p_min_node1;
		symbol_freq_elem *p_min_node2;
		/* Find the two minimum nodes */

		p_new_node = (symbol_freq_elem*)malloc(sizeof(symbol_freq_elem));

		/* Minimum nodes are always at the bottom of the list */
		p_min_node1 = pp_current_layer_nodes[n_layer_nodes-1];
		pp_current_layer_nodes[n_layer_nodes-1] = NULL;
		p_min_node2 = pp_current_layer_nodes[n_layer_nodes-2];
		pp_current_layer_nodes[n_layer_nodes-2] = p_new_node;	
		
		p_new_node->freq = p_min_node1->freq + p_min_node2->freq;
		p_new_node->symbol = -1; /* Not relevant */
		p_new_node->p_to_leaf_node1=p_min_node1;
		p_new_node->p_to_leaf_node2=p_min_node2;

		n_layer_nodes -= 1;
		
		/* Sort the list of pointers such that the content of the pointers is ordered by frequency */
		m = n_layer_nodes-1;

		while ((m!=0) && (pp_current_layer_nodes[m]->freq > pp_current_layer_nodes[m-1]->freq))
		{
			symbol_freq_elem *t;
			t = pp_current_layer_nodes[m-1];
			pp_current_layer_nodes[m-1] = pp_current_layer_nodes[m];
			pp_current_layer_nodes[m] = t;
			m--;
		}

	}
	
	huffman_encode_traverse_tree
		(*pp_table
		,pp_current_layer_nodes
		,p_bs_zeros
		);

	free(p_symbol_freq_map);

	return HUFFMAN_ENCODE_OK;
}

void
huffman_encode_table_print_freq_elem
	(symbol_freq_elem *p_elem)
{
	printf("{symbol: %10d, freq: %6d, p_to_leaf_node1: %016llx, p_to_leaf_node2: 0x%016llx}\n",p_elem->symbol, p_elem->freq,(unsigned long long)p_elem->p_to_leaf_node1,(unsigned long long)p_elem->p_to_leaf_node2);
}

void
huffman_encode_print_map
	(symbol_freq_elem *p_symbol_freq_map
	,int n_elem
	)
{
	int i;
	for (i=0; i<n_elem; i++)
	{
		huffman_encode_table_print_freq_elem((p_symbol_freq_map + i));
	}
}

void
huffman_encode_table_print_layer
	(symbol_freq_elem **pp_elem
	,int n
	)
{
	int i;
	for (i=0; i<n;i++)
	{
		if (pp_elem[i]!= NULL)
		{
			huffman_encode_table_print_freq_elem(pp_elem[i]);
		}
		else
		{
			printf("NULL\n");
		}
	}
}

int
huffman_encode_traverse_tree
	(huffman_table *p_table
	,symbol_freq_elem **pp_elem
	,bitstream *p_bs_code_word
	)
{
	int ret;
	bitstream *p_bs_codeword_1;
	bitstream *p_bs_codeword_2;
	unsigned char *p_data_1;
	unsigned char *p_data_2;
	if ((*pp_elem) == NULL)
	{
		return HUFFMAN_ENCODE_ERROR;
	}
	if ( ((*pp_elem)->p_to_leaf_node1 == NULL) && ((*pp_elem)->p_to_leaf_node2 == NULL) )
	{
		int m;
		/* Create a table entry mapping the data of this node to the codeword in the table */
		p_table->p_entries[p_table->n_entries].symbol = (*pp_elem)->symbol;
		p_table->p_entries[p_table->n_entries].p_code = p_bs_code_word;
		p_table->p_entries[p_table->n_entries].freq = (*pp_elem)->freq;
		p_table->n_entries++;

		m=p_table->n_entries-1;
		while ((m!=0) && (bitstream_get_n_bits_processed(p_table->p_entries[m].p_code) < bitstream_get_n_bits_processed(p_table->p_entries[m-1].p_code)))
		{
			huffman_entry t;
			t = p_table->p_entries[m-1];
			p_table->p_entries[m-1] = p_table->p_entries[m];
			p_table->p_entries[m] = t;
			m--;
		}
		
	}
	else
	{
		p_data_1 = (unsigned char*)malloc(sizeof(unsigned char)*HUFFMAN_ENCODER_MAX_CODEWORD_SIZE_BYTES);
		p_data_2 = (unsigned char*)malloc(sizeof(unsigned char)*HUFFMAN_ENCODER_MAX_CODEWORD_SIZE_BYTES);
		bitstream_attach_array(&p_bs_codeword_1,p_data_1,BITSTREAM_BIG_ENDIAN);
		bitstream_attach_array(&p_bs_codeword_2,p_data_2,BITSTREAM_BIG_ENDIAN);

		bitstream_copy(p_bs_codeword_1, p_bs_code_word, (size_t)bitstream_get_num_bytes(p_bs_code_word));
		bitstream_copy(p_bs_codeword_2, p_bs_code_word, (size_t)bitstream_get_num_bytes(p_bs_code_word));

		bitstream_add_bits(p_bs_codeword_1,0x00,1);
		ret = huffman_encode_traverse_tree(p_table,&(*pp_elem)->p_to_leaf_node1,p_bs_codeword_1);
		if (ret != HUFFMAN_ENCODE_OK)
		{
			printf("Error processing element:\n");
			return HUFFMAN_ENCODE_ERROR;
		}

		bitstream_add_bits(p_bs_codeword_2,0x01,1);
		ret = huffman_encode_traverse_tree(p_table,&(*pp_elem)->p_to_leaf_node2,p_bs_codeword_2);
		if (ret != HUFFMAN_ENCODE_OK)
		{
			printf("Error processing element:\n");
			return HUFFMAN_ENCODE_ERROR;
		}
	}
	
	return HUFFMAN_ENCODE_OK;
}

int
huffman_entry_get_n_bits(huffman_entry *p_entry)
{
	return bitstream_get_n_bits_processed(p_entry->p_code);
}

void
huffman_encode_free_freq_elem(symbol_freq_elem **pp_elem)
{
	printf("Freeing: 0x%016llx\n",(unsigned long long)(*pp_elem));
	huffman_encode_table_print_freq_elem((*pp_elem));
	free(*pp_elem);
}

void
huffman_encode_print_huffman_table(huffman_table *p_table)
{
	int i;
	printf(" Symbol             | Code-word          | Bits consumed      \n");
	for (i=0; i<p_table->n_entries; i++)
	{
		char a_bs[HUFFMAN_ENCODER_MAX_CODEWORD_SIZE_BYTES];
		bitstream_sprintf(a_bs,p_table->p_entries[i].p_code,8);
		printf(" %8u           | 0x%s             | %d               \n",p_table->p_entries[i].symbol,a_bs,bitstream_get_n_bits_processed(p_table->p_entries[i].p_code));
	}
}