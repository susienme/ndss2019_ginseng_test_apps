ARCH=arm64
CROSS_COMPILE=../../toolchains/aarch64/bin/aarch64-linux-gnu-

CC      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AR      = $(CROSS_COMPILE)ar
NM      = $(CROSS_COMPILE)nm
OBJCOPY = $(CROSS_COMPILE)objcopy
OBJDUMP = $(CROSS_COMPILE)objdump
READELF = $(CROSS_COMPILE)readelf

OBJS =
# OBJS = main.o
ASMS = callingConvention.S
ASMTOBIN = callingConvention_test

#CFLAGS += -Wall -I../ta/include -I$(TEEC_EXPORT)/include -I./include
CFLAGS += -Wall

#Add/link other required libraries here
#LDADD += -lteec -L$(TEEC_EXPORT)/lib

BINARY=
# BINARY=regs
ASSEMBLY=callingConvention
ASSEMBLYTOBIN=callingConvention_test

#.PHONY: callingConvention

.PHONY: all
# all: $(ASSEMBLYTOBIN)
all: $(BINARY) $(ASSEMBLY) $(ASSEMBLYTOBIN)
# all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) $(LDADD) -o $@ $<

$(ASSEMBLY): $(ASMS)
	$(CC) -S -o $@.S $@.c

$(ASSEMBLYTOBIN): $(ASMTOBIN).S
	$(CC) -o $@ $@.S

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)
