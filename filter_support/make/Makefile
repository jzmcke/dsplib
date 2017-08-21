GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/filter/src/convolve.o
OBJS += $(GIT_ROOT)/filter/src/fir_window.o
OBJS += $(GIT_ROOT)/dft/src/dft.o
OBJS += $(GIT_ROOT)/cplx_math/src/cplx_math.o
OBJS += $(GIT_ROOT)/filter_support/frontend/filter_frontend.o

RELEASE_OBJS = ${OBJS:.o=_release.o}
DEBUG_OBJS = ${OBJS:.o=_debug.o}

CFLAGS = -Wall -std=gnu99

CFLAGS_DEBUG = ${CFLAGS}
CFLAGS_DEBUG += -g

CFLAGS_RELEASE = ${CFLAGS}
CFLAGS_RELEASE += -O3

LD_DIRS=-L/usr/local/lib
LD_LIBS=-lsndfile -lm

INC = -I$(GIT_ROOT)

%_release.o: %.c
	$(CC) ${CFLAGS_RELEASE} -c $< -o $@ $(INC)

%_debug.o: %.c
	$(CC) ${CFLAGS_DEBUG} -c $< -o $@ $(INC)

release: $(RELEASE_OBJS)
	$(CC) ${CFLAGS_RELEASE} -o filter_frontend_release $^ $(LD_DIRS) $(LD_LIBS) $(INC)

debug: $(DEBUG_OBJS)
	$(CC) ${CFLAGS_DEBUG} -o filter_frontend_debug $^ $(LD_DIRS) $(LD_LIBS) $(INC)

clean:
	rm $(OBJS) $(DEBUG_OBJS) $(RELEASE_OBJS) filter_frontend_release filter_frontend_debug
