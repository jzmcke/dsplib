#include <stdio.h>
#include "huffman/include/huffman_decode.h"

#define HUFFMAN_DECODE_OK 0
#define HUFFMAN_DECODE_ERROR -1
#define HUFFMAN_TABLE_MATCH (1)
#define HUFFMAN_TABLE_DIFF (0)
#define HUFFMAN_DECODER_MAX_CODEWORD_SIZE_BYTES (1024)

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
	bitstream *p_temp;
	unsigned int code;
	unsigned int symbol;
	unsigned int out_idx = 0;
	unsigned int b_end = 0;
	unsigned int b_match;
	unsigned int n_symbols;
	unsigned char p_temp_bytes[HUFFMAN_DECODER_MAX_CODEWORD_SIZE_BYTES] = {0}; 
	int n = 0;

	ret = bitstream_attach_array(&p_bitstream, p_in, BITSTREAM_BIG_ENDIAN);

	bitstream_attach_array(&p_temp, p_temp_bytes, BITSTREAM_BIG_ENDIAN);

	if (ret != BITSTREAM_OK)
	{
		printf("Failure to attach input array to bitstream");
		return HUFFMAN_DECODE_ERROR;
	}

	/* Read the number of symbols in the bitstream */
	bitstream_read_bits(p_bitstream,&n_symbols,HUFFMAN_HEADER_LENGTH_BITS);

	/* Keep decoding until finished */
	printf("n_symbols = %d\n",n_symbols);
	while ((n_symbols-n) > 0)
	{
		idx = 0;
		do
		{
			bitstream_clear(p_temp);
			n_bits = bitstream_get_n_bits_processed(p_table->p_entries[idx].p_code);
			bitstream_peek_substream(p_bitstream,p_temp,n_bits);
			b_match = huffman_decode_lookup_table(p_table,p_temp,&symbol);
			idx++;
		} while ((b_match != HUFFMAN_TABLE_MATCH) && idx < p_table->n_entries);

		bitstream_read_substream(p_bitstream,p_temp,n_bits);

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
	,bitstream *p_code
	,unsigned int *p_symbol
	)
{
	int n = 0;
	while (n<p_table->n_entries)
	{
		if (BITSTREAM_EQUAL == bitstream_is_equal(p_table->p_entries[n].p_code, p_code))
		{
			*p_symbol = p_table->p_entries[n].symbol;
			return HUFFMAN_TABLE_MATCH;
		}
		n++;
	}
	return HUFFMAN_TABLE_DIFF;
}