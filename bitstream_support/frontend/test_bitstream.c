#include "bitstream/include/bitstream.h"

#define BITSTREAM_FRONTEND_MAX_STR_LEN 1024
#define TEST_BITSTREAM_MAX_BYTES 1024
#define BITS_IN_BYTE 8

void
test_print(unsigned int endianness)
{
	unsigned char a_char_array[TEST_BITSTREAM_MAX_BYTES] = {0};
	char p_str[BITSTREAM_FRONTEND_MAX_STR_LEN];
	unsigned char i;
	bitstream *p_bitstream;

	bitstream_attach_array(&p_bitstream, a_char_array, endianness);
	for (i=0; i<8; i++)
	{
		bitstream_add_bits(p_bitstream,i,BITS_IN_BYTE);
	}

	bitstream_sprintf(p_str, p_bitstream, 16);
	printf("%s\n",p_str);
}

void
test_concat(void)
{
	unsigned char a_char_array_1[TEST_BITSTREAM_MAX_BYTES] = {0};
	char p_str[BITSTREAM_FRONTEND_MAX_STR_LEN];
	bitstream *p_bitstream_dest;
	unsigned char a_char_array_2[TEST_BITSTREAM_MAX_BYTES] = {0};
	bitstream *p_bitstream_src;
	unsigned char i;

	bitstream_attach_array(&p_bitstream_dest,a_char_array_1,BITSTREAM_BIG_ENDIAN);
	bitstream_attach_array(&p_bitstream_src,a_char_array_2,BITSTREAM_LITTLE_ENDIAN);
	for (i=0; i<5; i++)
	{
		bitstream_add_bits(p_bitstream_dest,i,i);
		bitstream_add_bits(p_bitstream_src,i,BITS_IN_BYTE);
	}
	bitstream_concatenate
		(p_bitstream_dest
		,p_bitstream_src
		);
	bitstream_sprintf(p_str,p_bitstream_dest,16);
	printf("%s\n",p_str);
}

void
test_substreams(void)
{
	unsigned char a_char_array_1[TEST_BITSTREAM_MAX_BYTES] = {0};
	char p_str[BITSTREAM_FRONTEND_MAX_STR_LEN];
	bitstream *p_bitstream;
	unsigned char a_char_array_2[TEST_BITSTREAM_MAX_BYTES] = {0};
	bitstream *p_bitstream_peek;
	unsigned char i;


	bitstream_attach_array(&p_bitstream,a_char_array_1,BITSTREAM_LITTLE_ENDIAN);
	bitstream_attach_array(&p_bitstream_peek,a_char_array_2,BITSTREAM_LITTLE_ENDIAN);

	for (i = 0; i < 255; i++)
	{
		bitstream_add_bits(p_bitstream,i,BITS_IN_BYTE);
	}
	bitstream_reset(p_bitstream);
	for (i=0; i < 16; i++)
	{
		bitstream_peek_substream
			(p_bitstream
			,p_bitstream_peek
			,i*BITS_IN_BYTE
			);
		bitstream_sprintf(p_str,p_bitstream_peek,-1);
		printf("%d: %s\n",i,p_str);
		bitstream_clear(p_bitstream_peek);
	}
}

void
test_compare(void)
{
	unsigned char a_char_array_1[TEST_BITSTREAM_MAX_BYTES] = {0};
	char p_str[BITSTREAM_FRONTEND_MAX_STR_LEN];
	bitstream *p_bitstream_1;
	unsigned char a_char_array_2[TEST_BITSTREAM_MAX_BYTES] = {0};
	bitstream *p_bitstream_2;
	unsigned char i;

	bitstream_attach_array(&p_bitstream_1,a_char_array_1,BITSTREAM_LITTLE_ENDIAN);
	bitstream_attach_array(&p_bitstream_2,a_char_array_2,BITSTREAM_LITTLE_ENDIAN);
	for (i = 0; i < 8; i++)
	{
		bitstream_add_bits(p_bitstream_1,i,i);
		bitstream_add_bits(p_bitstream_2,i,i);
	}

	printf("Equal = %d\n",BITSTREAM_EQUAL == bitstream_is_equal(p_bitstream_1,p_bitstream_2));
	bitstream_add_bits(p_bitstream_2,3,BITS_IN_BYTE);
	printf("Not Equal = %d\n",BITSTREAM_NOT_EQUAL == bitstream_is_equal(p_bitstream_1,p_bitstream_2));

	bitstream_copy(p_bitstream_1,p_bitstream_2);
	printf("Equal = %d\n",BITSTREAM_EQUAL == bitstream_is_equal(p_bitstream_1,p_bitstream_2));

	bitstream_sprintf(p_str,p_bitstream_1,-1);
	printf("%d: %s\n",i,p_str);
	bitstream_sprintf(p_str,p_bitstream_2,-1);
	printf("%d: %s\n",i,p_str);

}

void 
test_add_int(void)
{
	unsigned char a_char_array[TEST_BITSTREAM_MAX_BYTES] = {0};
	unsigned int test = 256;
	unsigned int read;
	bitstream *p_bitstream;

	printf("test = 0x%08x\n",test);
	bitstream_attach_array(&p_bitstream,a_char_array,BITSTREAM_LITTLE_ENDIAN);
	bitstream_add_int(p_bitstream,test);
	printf("Little endian case:\n");
	bitstream_printf(p_bitstream);
	bitstream_reset(p_bitstream);
	bitstream_read_int(p_bitstream,&read);
	printf("read = %d\n",read);
	printf("Equal = %d\n", read == test);
	bitstream_clear(p_bitstream);
	

	bitstream_attach_array(&p_bitstream,a_char_array,BITSTREAM_BIG_ENDIAN);
	bitstream_add_int(p_bitstream,test);
	printf("Big endian case:\n:");
	bitstream_printf(p_bitstream);
	bitstream_reset(p_bitstream);
	bitstream_read_int(p_bitstream,&read);
	printf("read = %d\n",read);
	printf("Equal = %d\n", read == test);


}

int
main(int argc, char **argv)
{
	unsigned int i;

	printf("------ Test print ------\n");
	for (i=0; i<2; i++)
	{
		test_print(i);
	}
	printf("----- Test concat -----\n");
	test_concat();

	printf("----- Test substreams -----\n");
	test_substreams();

	printf("----- Test compare -----\n");
	test_compare();

	printf("----- Test add int -----\n");
	test_add_int();

	return 0;
}
