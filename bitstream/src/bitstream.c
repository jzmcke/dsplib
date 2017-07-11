#include "bitstream.h"
#include <stdlib.h>
#include <stdio.h>
#define BITS_IN_BYTE 8


struct bitstream_s
{
	char *p_data;
	unsigned int n_tot_bits;
	unsigned int byte_pos;
	unsigned int bit_pos;
	int b_big_endian;
};

int
bitstream_attach_array
	(bitstream **pp_stream
	,unsigned char *p_data
	,int b_big_endian
	)
{
	(*pp_stream) = (bitstream *)malloc(sizeof(bitstream));
	(*pp_stream)->p_data = p_data;
	(*pp_stream)->byte_pos = 0;
	(*pp_stream)->bit_pos = 0;
	(*pp_stream)->n_tot_bits = 0;
	(*pp_stream)->b_big_endian = b_big_endian;
	return BITSTREAM_OK;
}

int
bitstream_read_bits
	(bitstream *p_stream
	,unsigned int *p_read_data
	,unsigned int n_bits
	)
{
	int ret;
	ret = bitstream_peek_bits(p_stream,p_read_data,n_bits);
	if (ret != BITSTREAM_OK)
	{
		printf("Failure peeking bits\n");
		return BITSTREAM_ERROR;
	}
	if ((p_stream->bit_pos+n_bits) >= BITS_IN_BYTE)
	{
		p_stream->byte_pos++;
	}
	p_stream->bit_pos = (p_stream->bit_pos+n_bits)%BITS_IN_BYTE;
	p_stream->n_tot_bits += n_bits;
	return BITSTREAM_OK;
}


int
bitstream_concatenate
	(bitstream *p_dest
	,bitstream *p_src
	)
{
	unsigned int n_bits_overhang;
	unsigned int n = 0;

	while (n < p_src->byte_pos)
	{
		bitstream_add_bits(p_dest,p_src->p_data[n],BITS_IN_BYTE);
		n++;
	}
	bitstream_add_bits(p_dest,p_src->p_data[n],p_src->bit_pos);
	return BITSTREAM_OK;
}


int
bitstream_peek_bits
	(bitstream *p_stream
	,unsigned int *p_read_data
	,unsigned int n_bits
	)
{
	int n_bits_overhang;
	unsigned int read_data;
	unsigned int n_valid_bits;
	unsigned int valid_bit_mask;
	unsigned int overhang_bits = 0x00;
	unsigned int mask_overhang;
	n_bits_overhang = n_bits + p_stream->bit_pos - BITS_IN_BYTE;
	if (n_bits_overhang > BITS_IN_BYTE)
	{
		printf("n_bits_overhang = %d\n",n_bits_overhang);
		printf("Attempting to read too many bits at once. Maximum 8 allowed\n");
		return BITSTREAM_ERROR;
	}
	n_valid_bits = n_bits;
	if (n_bits_overhang > 0)
	{
		n_valid_bits = n_bits - n_bits_overhang;
		mask_overhang = ((1<<n_bits_overhang)-1);
		overhang_bits = p_stream->p_data[p_stream->byte_pos+1]&mask_overhang;
		overhang_bits = overhang_bits << n_valid_bits;
	}

	valid_bit_mask = ((1<<n_valid_bits)-1)<<p_stream->bit_pos;
	read_data = (valid_bit_mask&p_stream->p_data[p_stream->byte_pos]) >> p_stream->bit_pos;
	read_data ^= overhang_bits;
	read_data &= ((1<<n_bits)-1);

	if (p_stream->b_big_endian)
	{
		read_data = bitstream_flip_bits(read_data,n_bits);
	}

	*p_read_data = read_data;
	return BITSTREAM_OK;
}

unsigned char
bitstream_flip_bits(unsigned char data,unsigned int n_bits)
{
	int n;
	unsigned char data_bitstream = data;
	unsigned char output = 0x00;
	n = 0;
	while (n<n_bits)
	{
		output^=((data_bitstream&0x01)<<(n_bits-1-n));
		data_bitstream>>=1;
		n++;
	}
	return output;
}

int
bitstream_add_bits
	(bitstream *p_stream
	,unsigned char data /* data no larger than 8 bits */
	,unsigned int n_bits /* < 8 */
	)
{
	int mask = ((1<<n_bits)-1);
	int n_bits_overhang;
	unsigned int masked_data;
	unsigned int overhang_bits;
	unsigned int mask_overhang;
	unsigned char data_bitstream = data;
	if (p_stream->b_big_endian)
	{

		data_bitstream = bitstream_flip_bits(data_bitstream,n_bits);
	}
	//printf("\n-------------------------------\n");
	//printf("bitpos = %d, bytepos = %d\n",p_stream->bit_pos,p_stream->byte_pos);
	n_bits_overhang = n_bits + p_stream->bit_pos - BITS_IN_BYTE;
	//printf("n_bits = %d, n_bits_overhang = %d\n", n_bits, n_bits_overhang);
	if (n_bits_overhang > BITS_IN_BYTE)
	{
		printf("Too many bits added to bitstream at once. Maximum 8 allowed.\n");
		return BITSTREAM_ERROR;
	}
	masked_data = (data_bitstream&mask)<<p_stream->bit_pos;	
	p_stream->p_data[p_stream->byte_pos]^=masked_data;
	p_stream->bit_pos += n_bits;
	if (n_bits_overhang > 0)
	{
		mask_overhang = ((1<<n_bits)-1)- (1<<(n_bits-n_bits_overhang)-1);
		overhang_bits = (mask_overhang&data_bitstream)>>(n_bits-n_bits_overhang);
		//printf("overhang_bits = 0x%04x, mask = 0x%04x\n",overhang_bits, mask_overhang);
		p_stream->p_data[p_stream->byte_pos+1]^=overhang_bits;
		p_stream->bit_pos=n_bits_overhang;
		//printf("byte[%d] = 0x%04x\n",p_stream->byte_pos,p_stream->p_data[p_stream->byte_pos]&0xff);
		p_stream->byte_pos++;
	}
	//printf("byte[%d] = 0x%04x\n",p_stream->byte_pos,p_stream->p_data[p_stream->byte_pos]&0xff);
	p_stream->n_tot_bits += n_bits;
	return BITSTREAM_OK;
}

size_t
bitstream_get_num_bytes(bitstream *p_stream)
{
	return (size_t)(p_stream->byte_pos+1);
}
int
bitstream_get_n_bits_processed(bitstream *p_stream)
{
	return p_stream->n_tot_bits;
}

int
bitstream_peek_substream
	(bitstream *p_stream
	,bitstream *p_read
	,int n_bits
	)
{
	unsigned int n_overhang = n_bits % BITS_IN_BYTE;
	int i;
	unsigned int temp;
	unsigned int init_byte_pos = p_stream->byte_pos;
	unsigned int init_bit_pos = p_stream->bit_pos;
	for (i=0; i<(n_bits-BITS_IN_BYTE); i+=BITS_IN_BYTE)
	{
		bitstream_read_bits(p_stream,&temp,BITS_IN_BYTE);
		bitstream_add_bits(p_read,&temp,BITS_IN_BYTE);
	}
	bitstream_read_bits(p_stream,&temp,n_overhang);
	bitstream_add_bits(p_read,&temp,n_overhang);
	p_stream->byte_pos = init_byte_pos;
	p_stream->bit_pos = init_bit_pos;
}

int
bitstream_is_equal
	(bitstream *p1
	,bitstream *p2
	)
{
	int i;
	int b_is_equal = 1;
	unsigned int mask;
	if (p1->n_tot_bits != p2->n_tot_bits)
	{
		return BITSTREAM_NOT_EQUAL;
	}
	for (i=0; i<p1->byte_pos; i++)
	{
		if (p1->p_data[i] != p2->p_data[i])
		{
			return BITSTREAM_NOT_EQUAL;
		}
	}
	mask = (1<<p1->bit_pos)-1;
	if (p1->p_data[p1->byte_pos] != p2->p_data[p2->byte_pos])
	{
		return BITSTREAM_NOT_EQUAL;
	}
	return BITSTREAM_EQUAL;
}

char *
bitstream_get_array(bitstream *p_stream)
{
	return p_stream->p_data;
}

int
bitstream_copy(bitstream *p_dest, bitstream *p_src, size_t n_bytes)
{
	p_dest->n_tot_bits = p_src->n_tot_bits;
	p_dest->byte_pos = p_src->byte_pos;
	p_dest->bit_pos = p_src->bit_pos;
	p_dest->b_big_endian = p_src->b_big_endian;
	memcpy(p_dest->p_data,p_src->p_data,n_bytes);
	return BITSTREAM_OK;
}

int
bitstream_sprintf(char *p_dest, bitstream *p_bitstream, size_t n_bytes)
{
	int n = n_bytes;
	char a_temp[3]; /* 2 hex digits + 1 for \0 */
	sprintf(p_dest,"0x");
	if (n_bytes < (p_bitstream->byte_pos+1))
	{
		printf("Attempting to print fewer bytes than allocated in stream\n");
		return BITSTREAM_ERROR;
	}
	while (n>=0)
	{
		if (n == p_bitstream->byte_pos)
		{
			sprintf(a_temp,"%02x",p_bitstream->p_data[n]&((1<<p_bitstream->bit_pos)-1));
			strcat(p_dest,a_temp);
		}
		else if (n < p_bitstream->byte_pos)
		{
			sprintf(a_temp,"%02x",p_bitstream->p_data[n]);
			strcat(p_dest,a_temp);
		}
		else
		{
			sprintf(a_temp,"%02x",0);
			strcat(p_dest,a_temp);
		}
		p_dest += 2;
		n--;
	}
}