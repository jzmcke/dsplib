GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/huffman/src/huffman_encode.o
OBJS += $(GIT_ROOT)/stats/src/stats.o
OBJS += $(GIT_ROOT)/huffman/src/huffman_decode.o
OBJS += $(GIT_ROOT)/bitstream/src/bitstream.o
OBJS += $(GIT_ROOT)/huffman_support/frontend/huffman_frontend.o

DEBUG_OBJS = ${OBJS:.o=_debug.o}

CFLAGS = -Wall -std=gnu99

CFLAGS_DEBUG = ${CFLAGS}
CFLAGS_DEBUG += -g

CFLAGS_RELEASE = ${CFLAGS}
CFLAGS_RELEASE += -O3

INC = -I$(GIT_ROOT)

%.o: %.c
	$(CC) ${CFLAGS_RELEASE} -c $< -o $@ $(INC)

%_debug.o: %.c
	$(CC) ${CFLAGS_DEBUG} -c $< -o $@ $(INC)

release: $(OBJS)
	$(CC) ${CFLAGS_RELEASE} -o huffman_frontend_release $^ $(INC)

debug: $(DEBUG_OBJS)
	$(CC) ${CFLAGS_DEBUG} -o huffman_frontend_debug $^ $(INC)

clean:
	rm $(OBJS) $(DEBUG_OBJS)