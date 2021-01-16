CC = gcc 
OBJS = $(GIT_ROOT)/blob/src/blob_core.o
OBJS += $(GIT_ROOT)/blob/src/blob_socket.o
OBJS += $(GIT_ROOT)/blob_support/frontend/test_blob.o
INC = -I$(GIT_ROOT)

DEBUG_OBJS = ${OBJS:.o=_debug.o}

CFLAGS = -Wall -std=gnu99

CFLAGS_RELEASE = ${CFLAGS}
CFLAGS_RELEASE += -O3

CFLAGS_DEBUG = ${CFLAGS}
CFLAGS_DEBUG += -g

%.o: %.c
	$(CC) $(CFLAGS_RELEASE) -c -DBLOB_SOCKET=1 $< -o $@ $(INC)

%_debug.o: %.c
	$(CC) $(CFLAGS_DEBUG) -c -DBLOB_SOCKET=1 $< -o $@ $(INC)

debug: $(DEBUG_OBJS)
	mkdir -p bin
	$(CC) -g -DBLOB_SOCKET=1 -o bin/test_blob_debug $^ $(INC)

release: $(OBJS)
	mkdir -p bin
	$(CC) -DBLOB_SOCKET=1 -o bin/test_blob_release $^ $(INC)

clean :
	rm $(OBJS)
	rm $(DEBUG_OBJS)
	rm -r bin
	
