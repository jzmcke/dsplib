CC=gcc

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

LD_DIRS=-L/usr/include
LD_DIRS += -L/usr/local/Cellar/libsndfile/1.0.30_1/include
LD_LIBS=-lsndfile -lm

INC=-I$(GIT_ROOT)

%_release.o: %.c
	$(CC) $(RELEASE_CFLAGS) -c $< -o $@ $(INC)

%_debug.o: %.c
	$(CC) $(DEBUG_CFLAGS) -c $< -o $@ $(INC)

release: $(RELEASE_OBJS)
	mkdir -p bin
	$(CC) $(RELEASE_OBJS) ${RELEASE_CFLAGS} -o bin/dft_frontend_release $(LD_DIRS) $(LD_LIBS) $(INC)

debug: $(DEBUG_OBJS)
	mkdir -p bin
	$(CC) $(DEBUG_OBJS) ${DEBUG_CFLAGS} -o bin/dft_frontend_debug $(LD_DIRS) $(LD_LIBS) $(INC)

clean:
	rm -f $(DEBUG_OBJS) $(RELEASE_OBJS)
	rm -rf bin
