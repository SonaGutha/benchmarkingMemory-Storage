CC=gcc
CFLAGS=-O3 -pthread -g
TARGET=hashgen
SRCS=hashgen.c blake3.c blake3_dispatch.c blake3_portable.c \
     blake3_sse2_x86-64_unix.S blake3_sse41_x86-64_unix.S \
     blake3_avx2_x86-64_unix.S blake3_avx512_x86-64_unix.S
OBJS=$(SRCS:.c=.o)

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.S
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(OBJS) $(TARGET)
