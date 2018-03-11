CC=gcc
GIT_ROOT=/home/joe/dsplib

OBJS=$(GIT_ROOT)/dft/src/dft.o
OBJS+=$(GIT_ROOT)/util/src/util.o
OBJS+=$(GIT_ROOT)/dft/src/file.o
OBJS+=$(GIT_ROOT)/dft_support/frontend/dft_frontend.o
OBJS+=$(GIT_ROOT)/cplx_math/src/cplx_math.o

DEBUG_OBJS = ${OBJS:.o=_debug.o}
RELEASE_OBJS = ${OBJS:.o=_release.o}

CFLAGS = -Wall -std=gnu99

DEBUG_CFLAGS = $(CFLAGS)
DEBUG_CFLAGS += -g

RELEASE_CFLAGS = $(CFLAGS)
RELEASE_CFLAGS += -O3

LD_DIRS=-L/usr/local/lib
LD_LIBS=-lsndfile -lm

INC=-I$(GIT_ROOT)

%_release.o: %.c
	$(CC) $($RELEASE_CFLAGS) -c $< -o $@ $(INC)

%_debug.o: %.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@ $(INC)

release: $(RELEASE_OBJS)
	$(CC) $(RELEASE_OBJS) ${RELEASE_CFLAGS} -o dft_frontend_release $(LD_DIRS) $(LD_LIBS) $(INC)

debug: $(DEBUG_OBJS)
	$(CC) $(DEBUG_OBJS) ${REBUG_CFLAGS} -o dft_frontend_debug $(LD_DIRS) $(LD_LIBS) $(INC)

clean:
	rm $(DEBUG_OBJS) $(RELEASE_OBJS) dft_frontend_release dft_frontend_debug