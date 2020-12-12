GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/blob/src/blob_core.o
OBJS += $(GIT_ROOT)/blob/src/blob_socket.o
OBJS += $(GIT_ROOT)/blob_support/frontend/test_blob.o
INC = -I$(GIT_ROOT)

%.o: %.c
	$(CC) -g -c -DBLOB_SOCKET=1 $< -o $@ $(INC)

$(GIT_ROOT)/blob_support/make/test_blob/test_blob: $(OBJS)
	$(CC) -g -DBLOB_SOCKET=1 -o $@ $^ $(INC)

clean :
	rm $(OBJS)
	rm test_blob
	
