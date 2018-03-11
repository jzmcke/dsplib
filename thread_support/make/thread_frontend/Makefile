GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/thread_support/frontend/thread_frontend.o

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
	$(CC) ${CFLAGS_RELEASE} -o thread_frontend_release $^ $(INC) -pthread

debug: $(DEBUG_OBJS)
	$(CC) ${CFLAGS_DEBUG} -o thread_frontend_debug $^ $(INC) -pthread

clean:
	rm $(OBJS) $(DEBUG_OBJS)