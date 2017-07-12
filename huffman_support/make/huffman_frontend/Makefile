GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/huffman/src/huffman_encode.o
OBJS += $(GIT_ROOT)/stats/src/stats.o
OBJS += $(GIT_ROOT)/huffman/src/huffman_decode.o
OBJS += $(GIT_ROOT)/bitstream/src/bitstream.o
OBJS += $(GIT_ROOT)/huffman_support/frontend/huffman_frontend.o

INC = -I$(GIT_ROOT)

%.o: %.c
	$(CC) -c $< -o $@ $(INC)

$(GIT_ROOT)/huffman_support/make/huffman_frontend/huffman_frontend: $(OBJS)
	$(CC) -o $@ $^ $(INC)

clean :
	rm $(OBJS)