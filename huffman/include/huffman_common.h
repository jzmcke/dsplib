#ifndef HUFFMAN_COMMON_H
#define HUFFMAN_COMMON_H

#include "bitstream/include/bitstream.h"

#define HUFFMAN_HEADER_LENGTH_BITS (8)

typedef struct huffman_entry_s
{
	unsigned int symbol;
	bitstream *p_code;
	unsigned int freq;
} huffman_entry;

typedef struct huffman_table_s
{
	huffman_entry *p_entries;
	unsigned int n_entries;
} huffman_table;

#endif