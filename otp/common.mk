HIKEY_DIR=$(HOME)/ndss/hikey
LLVM_BUILD_PATH=$(HOME)/ndss/llvm_test/build_release
ARCH=arm64
CROSS_COMPILE=$(HIKEY_DIR)/toolchains/aarch64/bin/aarch64-linux-gnu-
LLVM_BIN_PATH=$(LLVM_BUILD_PATH)/bin
LLVM_LIB_PATH=$(LLVM_BUILD_PATH)/lib
INSTALL_PATH=$(HIKEY_DIR)/patches_hikey/rootfs
SLIB_PATH=$(HIKEY_DIR)/s-lib
SLIB = $(SLIB_PATH)/slib.a
SRC_PATH=..
SRC_FILE	+=	otp_ginseng.c
SRC_S_FILE += sha1-armv8.S

BC_FILE		=$(subst .c,.bc,$(SRC_FILE))
S_FILE 		=$(subst .c,.S,$(SRC_FILE))
O_FILE 		=$(subst .c,.o,$(SRC_FILE))
EXE_FILE	=$(subst .c,,$(SRC_FILE))

INCLUDES=-I$(HIKEY_DIR)/toolchains/aarch64/aarch64-linux-gnu/libc/usr/include\
	 		-I$(HIKEY_DIR)/s-lib\
	 		-I$(HIKEY_DIR)/arm-trusted-firmware/include/bl31
CLANG_OPT=-emit-llvm\
	 		-Xclang -load -Xclang $(LLVM_LIB_PATH)/libGinsengPass.so\
	 		$(INCLUDES)\
	 		-target aarch64\
	 		-Werror=implicit-function-declaration
LLC_OPT=-regalloc=ginsengfast -march=aarch64
SRCS=$(SRC_PATH)/$(SRC_FILE)

CLANG 	 = $(LLVM_BIN_PATH)/clang
LLVM-DIS = $(LLVM_BIN_PATH)/llvm-dis
LLC 	 = $(LLVM_BIN_PATH)/llc
AS       = $(CROSS_COMPILE)as
GCC       = $(CROSS_COMPILE)gcc

slib:
	HIKEY_DIR=$(HIKEY_DIR) make -C $(SLIB_PATH)

# .c -> .bc
c2bc: $(BC_FILE)
	@echo "C2BC is done"

.PHONY: $(BC_FILE)
$(BC_FILE):
	@echo "Building " $@"..."
	$(CLANG) $(CLANG_OPT) $(SRC_PATH)/$(subst .bc,.c,$@) $(CLANG_DEBUG) -c -o $@

# .bc -> .S
bc2S: $(S_FILE)
	@echo "BC2S is done"

.PHONY: $(S_FILE)
$(S_FILE):
	@echo "Building " $@"..."
	$(LLC) $(OPT_LEVEL) $(LLC_OPT) $(subst .S,.bc,$@) -o $@

# .S -> exe
S2exe: slib $(EXE_FILE)
	@echo "S2EXE is done"

.PHONY: $(EXE_FILE)
$(EXE_FILE):
	@echo "Building " $@"..."
	$(AS) -o $@.o $@.S
	$(GCC) $@.o $(SRC_PATH)/$(SRC_S_FILE) $(SLIB) -o $@


c2exe: c2bc bc2S S2exe

all: c2exe

.PHONY: clean
clean:
	rm -f $(BC_FILE) $(S_FILE) $(O_FILE) $(EXE_FILE) 
