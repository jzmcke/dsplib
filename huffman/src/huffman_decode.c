#include <stdio.h>
#include "huffman_decode.h"

#define HUFFMAN_DECODE_OK 0
#define HUFFMAN_DECODE_ERROR -1
#define HUFFMAN_TABLE_MATCH (1)
#define HUFFMAN_TABLE_DIFF (0)

int
huffman_decode_process
	(huffman_table *p_table
	,char *p_in
	,size_t n_data
	,unsigned int *p_out
	,unsigned int *p_n_out
	)
{
	int ret;
	int idx;
	int n_bits;
	bitstream *p_bitstream;
	unsigned int code;
	unsigned int symbol;
	unsigned int out_idx = 0;
	unsigned int b_end = 0;
	unsigned int b_match;
	unsigned int n_symbols;
	int n = 0;

	ret = bitstream_attach_array(&p_bitstream, p_in, BITSTREAM_BIG_ENDIAN);
	if (ret != BITSTREAM_OK)
	{
		printf("Failure to attach input array to bitstream");
		return HUFFMAN_DECODE_ERROR;
	}

	/* Read the number of sumbols in the bitstream */
	bitstream_read_bits(p_bitstream,&n_symbols,HUFFMAN_HEADER_LENGTH_BITS);

	/* Keep decoding until finished */
	while ((n_symbols-n) > 0)
	{
		idx = 0;
		do
		{
			n_bits = bitstream_get_n_bits_processed(p_table->p_entries[idx].p_code);
			bitstream_peek_bits(p_bitstream,&code,n_bits);
			b_match = huffman_decode_lookup_table(p_table,code,n_bits,&symbol);
			idx++;
		} while (!b_match && idx < p_table->n_entries);
		bitstream_read_bits(p_bitstream,&code,n_bits);
		p_out[out_idx++] = symbol;
		if (idx >= p_table->n_entries)
		{
			return HUFFMAN_DECODE_ERROR;
		}
		n++;
	}

	*p_n_out = out_idx;
	printf("Decoder process\n");
	return HUFFMAN_DECODE_OK;
}

int
huffman_decode_lookup_table
	(huffman_table *p_table
	,unsigned int code
	,unsigned int n_bits
	,unsigned int *p_symbol
	)
{
	int n = 0;
	while (n<p_table->n_entries)
	{
		if ((p_table->p_entries[n].code == code)&&(n_bits==bitstream_get_n_bits_processed(p_table->p_entries[n].p_code)))
		{
			*p_symbol = p_table->p_entries[n].symbol;
			return HUFFMAN_TABLE_MATCH;
		}
		n++;
	}
	return HUFFMAN_TABLE_DIFF;
}