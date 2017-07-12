GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/bitstream/src/bitstream.o
OBJS += $(GIT_ROOT)/bitstream_support/frontend/test_bitstream.o
INC = -I$(GIT_ROOT)

%.o: %.c
	$(CC) -c $< -o $@ $(INC)

$(GIT_ROOT)/bitstream_support/make/test_bitstream/test_bitstream: $(OBJS)
	$(CC) -o $@ $^ $(INC)

clean :
	rm $(OBJS)