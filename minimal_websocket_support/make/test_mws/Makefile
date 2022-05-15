CC = gcc 
OBJS = $(GIT_ROOT)/minimal_websocket_support/frontend/test_mws.o
OBJS += $(GIT_ROOT)/minimal_websocket/src/minimal_websocket.o
INC = -I$(GIT_ROOT) -I/usr/local/opt/openssl/include

LD_DIRS=-L/usr/local/opt/openssl/lib -L/usr/local/opt/libwebsockets/lib
LD_LIBS=-lwebsockets -lssl -lcrypto

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
	$(CC) -g -DBLOB_SOCKET=1 -o bin/test_mws_debug $^ $(INC) $(LD_DIRS) $(LD_LIBS)

release: $(OBJS)
	mkdir -p bin
	$(CC) -DBLOB_SOCKET=1 -o bin/test_mws_release $^ $(INC) $(LD_DIRS) $(LD_LIBS)

clean :
	rm $(OBJS)
	rm $(DEBUG_OBJS)
	rm -r bin
	
