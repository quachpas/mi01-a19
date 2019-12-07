CC ?= gcc
CFLAGS ?= -O0 -g 
LDFLAGS = -export-dynamic
PKGCONFIG = $(shell which pkg-config)
GTK_INC = $(shell $(PKGCONFIG) --cflags gtk+-3.0)
GTK_LIB = $(shell $(PKGCONFIG) --libs gtk+-3.0)

BINARY = tp_image
C_SRCS = main.c process_image_c.c
ASM_SRCS = process_image_asm.S process_image_simd.S

OBJS = $(ASM_SRCS:.S=.o) $(C_SRCS:.c=.o)

all: $(BINARY)

%.o: %.c
	$(CC) -c -o $@ $(CFLAGS) $(GTK_INC) $<

%.o: %.S
	$(CC) -c -o $@ $(CFLAGS) $<

$(BINARY): $(OBJS) 
	$(CC) -o $@ $(LDFLAGS) $^ $(GTK_LIB)

clean:
	rm -f $(OBJS) $(BINARY)
