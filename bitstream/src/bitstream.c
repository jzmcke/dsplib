#include "bitstream/include/bitstream.h"
#include <stdlib.h>
#include <stdio.h>
#define BITS_IN_BYTE 8
#define BITSTREAM_PRINT_LEN 1024


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
	,int n_bits
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
	int n = 0;
	int src_len = p_src->byte_pos+1;
	unsigned int init_byte_pos = p_src->byte_pos;
	unsigned int init_bit_pos = p_src->bit_pos;
	unsigned int init_tot_bits = p_src->n_tot_bits;
	unsigned int temp;
	p_src->byte_pos = 0;
	p_src->bit_pos = 0;

	while (n < (src_len-2))
	{
		bitstream_read_bits(p_src,&temp,BITS_IN_BYTE);
		bitstream_add_bits(p_dest,temp,BITS_IN_BYTE);
		printf("hi");
		n++;
	}
	bitstream_read_bits(p_src,&temp,init_bit_pos);
	bitstream_add_bits(p_dest,temp,init_bit_pos);
	p_src->byte_pos = init_byte_pos;
	p_src->bit_pos = init_bit_pos;
	p_src->n_tot_bits = init_tot_bits;
	return BITSTREAM_OK;
}

int
bitstream_read_substream
	(bitstream *p_stream
	,bitstream *p_read
	,int n_bits
	)
{
	unsigned int n_overhang = n_bits % BITS_IN_BYTE;
	int i;
	unsigned int temp;

	for (i=0; i<=(n_bits-BITS_IN_BYTE); i+=BITS_IN_BYTE)
	{
		bitstream_read_bits(p_stream,&temp,BITS_IN_BYTE);
		bitstream_add_bits(p_read,temp,BITS_IN_BYTE);
	}
	bitstream_read_bits(p_stream,&temp,n_overhang);
	bitstream_add_bits(p_read,temp,n_overhang);

}

int
bitstream_peek_bits
	(bitstream *p_stream
	,unsigned int *p_read_data
	,int n_bits
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
bitstream_flip_bits(unsigned char data,int n_bits)
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
	,int n_bits /* < 8 */
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
	//printf("data_bitstream = 0x%04x\n",data_bitstream);
	masked_data = (data_bitstream&mask)<<p_stream->bit_pos;	
	//printf("masked_data = 0x%04x\n",masked_data);
	p_stream->p_data[p_stream->byte_pos]^=masked_data;
	//printf("added byte = 0x%04x\n",p_stream->p_data[p_stream->byte_pos]);
	
	if (n_bits_overhang > 0)
	{
		mask_overhang = ((1<<n_bits)-1)- (1<<(n_bits-n_bits_overhang)-1);
		overhang_bits = (mask_overhang&data_bitstream)>>(n_bits-n_bits_overhang);
		//printf("overhang_bits = 0x%04x, mask = 0x%04x\n",overhang_bits, mask_overhang);
		p_stream->p_data[p_stream->byte_pos+1]^=overhang_bits;
		//printf("byte[%d] = 0x%04x\n",p_stream->byte_pos,p_stream->p_data[p_stream->byte_pos]&0xff);
	}
	p_stream->bit_pos = (p_stream->bit_pos + n_bits);
	if (p_stream->bit_pos >= BITS_IN_BYTE)
	{
		p_stream->bit_pos %= BITS_IN_BYTE;
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
bitstream_reset(bitstream *p_stream)
{
	p_stream->byte_pos = 0;
	p_stream->bit_pos = 0;
	p_stream->n_tot_bits = 0;
}

int
bitstream_clear(bitstream *p_stream)
{
	memset(p_stream->p_data,0,p_stream->byte_pos+1);
	p_stream->byte_pos = 0;
	p_stream->bit_pos = 0;
	p_stream->n_tot_bits = 0;
}

int
bitstream_peek_substream
	(bitstream *p_stream
	,bitstream *p_read
	,int n_bits
	)
{
	unsigned int init_byte_pos = p_stream->byte_pos;
	unsigned int init_bit_pos = p_stream->bit_pos;

	bitstream_read_substream(p_stream,p_read,n_bits);

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
bitstream_copy(bitstream *p_dest, bitstream *p_src)
{
	p_dest->n_tot_bits = p_src->n_tot_bits;
	p_dest->byte_pos = p_src->byte_pos;
	p_dest->bit_pos = p_src->bit_pos;
	p_dest->b_big_endian = p_src->b_big_endian;
	memcpy(p_dest->p_data,p_src->p_data,p_src->byte_pos+1);
	return BITSTREAM_OK;
}

int
bitstream_sprintf(char *p_dest, bitstream *p_bitstream, size_t n_bytes)
{
	int n = n_bytes-1;
	char a_temp[3]; /* 2 hex digits + 1 for \0 */
	sprintf(p_dest,"0x");

	if (n_bytes == BITSTREAM_PRINT_DEFAULT_LEN)
	{
		n = p_bitstream->byte_pos;
	}

	if (n < (p_bitstream->byte_pos))
	{
		printf("Attempting to print fewer bytes than present in stream\n");
		return BITSTREAM_ERROR;
	}
	while (n>=0)
	{
		if (n == p_bitstream->byte_pos)
		{
			if (p_bitstream->bit_pos!=0)
			{
				sprintf(a_temp,"%02x",p_bitstream->p_data[n]&((1<<p_bitstream->bit_pos)-1));
				strcat(p_dest,a_temp);
				p_dest += 2;
			}
		}
		else if (n < p_bitstream->byte_pos)
		{
			sprintf(a_temp,"%02x",p_bitstream->p_data[n]&0xff);
			strcat(p_dest,a_temp);
			p_dest += 2;
		}
		else
		{
			sprintf(a_temp,"%02x",0&0xff);
			strcat(p_dest,a_temp);
			p_dest += 2;
		}
		n--;
	}
}

void
bitstream_printf(bitstream *p_bitstream)
{
	char p_str[BITSTREAM_PRINT_LEN];
	bitstream_sprintf(p_str,p_bitstream,BITSTREAM_PRINT_DEFAULT_LEN);
	printf("%s\n",p_str);
}

void
bitstream_print_info(bitstream *p_bitstream)
{
	printf("----------------------------------------------\n");
	printf("byte_pos     | %d\n", p_bitstream->byte_pos);
	printf("bit_pos      | %d\n", p_bitstream->bit_pos);
	printf("n_tot_bits   | %d\n",p_bitstream->n_tot_bits);
	printf("b_big_endian | %d\n",p_bitstream->b_big_endian);
	printf("p_data       | ");
	bitstream_printf(p_bitstream);
}