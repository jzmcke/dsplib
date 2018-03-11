GIT_ROOT = /home/joe/dsplib
CC = gcc
OBJS = $(GIT_ROOT)/filter/src/convolve.o
OBJS += $(GIT_ROOT)/filter/src/iir_filter.o
OBJS += $(GIT_ROOT)/filter/src/fir_window.o
OBJS += $(GIT_ROOT)/dft/src/dft.o
OBJS += $(GIT_ROOT)/cplx_math/src/cplx_math.o
OBJS += $(GIT_ROOT)/util/src/util.o
OBJS += $(GIT_ROOT)/filter_support/frontend/filter_frontend.o

HEADER = $(GIT_ROOT)/filter_support/scripts/fir_filter_pvt.h
HEADER += $(GIT_ROOT)/filter_support/scripts/iir_filter_pvt.h

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

$(GIT_ROOT)/filter_support/frontend/filter_frontend_release.o: $(HEADER)
$(GIT_ROOT)/filter_support/frontend/filter_frontend_release.o: $(GIT_ROOT)/filter_support/frontend/filter_frontend.c
	$(CC) ${CFLAGS_RELEASE} -c $< -o $@ $(INC)

$(GIT_ROOT)/filter_support/frontend/filter_frontend_debug.o: $(HEADER)
$(GIT_ROOT)/filter_support/frontend/filter_frontend_debug.o: $(GIT_ROOT)/filter_support/frontend/filter_frontend.c
	$(CC) ${CFLAGS_DEBUG} -c $< -o $@ $(INC)

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
