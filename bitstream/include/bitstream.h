#ifndef BITSTREAM_H
#define BISTREAM_H

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define BITSTREAM_OK 0
#define BITSTREAM_ERROR (-1)
#define BITSTREAM_BIG_ENDIAN (1)
#define BITSTREAM_LITTLE_ENDIAN (0)
#define BITSTREAM_NOT_EQUAL (-1)
#define BITSTREAM_EQUAL (0)
#define BITSTREAM_MAX_STR_LEN (64*BITS_IN_BYTE)
#define BITSTREAM_PRINT_DEFAULT_LEN (-1)

typedef struct bitstream_s bitstream;

int
bitstream_add_bits
	(bitstream *p_stream
	,unsigned char data
	,int n_bits
	);

/* It is the users job to ensure p_data is sufficiently long to hold
   all bits added to it.
*/
int
bitstream_attach_array
	(bitstream **pp_stream
	,unsigned char *p_data
	,int b_big_endian
	);

int
bitstream_peek_substream
	(bitstream *p_stream
	,bitstream *p_read
	,int n_bits
	);

int
bitstream_read_substream
	(bitstream *p_stream
	,bitstream *p_read
	,int n_bits
	);

int
bitstream_concatenate
	(bitstream *p_dest
	,bitstream *p_src
	);

size_t
bitstream_get_num_bytes(bitstream *p_stream);

int
bitstream_is_equal
	(bitstream *p1
	,bitstream *p2
	);

int
bitstream_peek_bits
	(bitstream *p_stream
	,unsigned int *p_read_data
	,int n_bits
	);

int
bitstream_read_bits
	(bitstream *p_stream
	,unsigned int *p_read_data
	,int n_bits
	);

int
bitstream_get_n_bits_processed(bitstream *p_stream);

unsigned char
bitstream_flip_bits(unsigned char data,int n_bits);

char *
bitstream_get_array(bitstream *p_stream);

int
bitstream_copy(bitstream *p_dest, bitstream *p_src);

int
bitstream_sprintf(char *p_dest, bitstream *p_bitstream, size_t n_bytes);

void
bitstream_printf(bitstream *p_bitstream);

void
bitstream_print_info(bitstream *p_bitstream);

int
bitstream_reset(bitstream *p_stream);

int
bitstream_clear(bitstream *p_stream);

#endif