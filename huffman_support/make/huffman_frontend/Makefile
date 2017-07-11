CC=gcc
OBJS=huffman_encode.o stats.o huffman_decode.o bitstream.o huffman_frontend.o

%.o: %.c
	gcc -c $< -o $@

huffman_frontend: $(OBJS)
	gcc -o $@ $^