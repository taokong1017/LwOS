# File: scripts/toolchain.mk

# Basic toolchain configuration
CROSS_COMPILE ?= ~/aarch64-none-elf/bin/aarch64-none-elf-
CC      = $(CROSS_COMPILE)gcc
CPP      = $(CROSS_COMPILE)cpp
AS      = $(CROSS_COMPILE)gcc
LD      = $(CROSS_COMPILE)ld
AR      = $(CROSS_COMPILE)ar
OBJCOPY = $(CROSS_COMPILE)objcopy
SIZE    = $(CROSS_COMPILE)size
OBJDUMP = $(CROSS_COMPILE)objdump
NM      = $(CROSS_COMPILE)nm
READELF = $(CROSS_COMPILE)readelf

# Architecture related configuration
ARCH ?= ARM64
CPU  ?= cortex-a53

# Common compilation flags
COMMON_FLAGS = -O2 -g -Wall -Wunused-function -Wunused-variable -Wno-frame-address \
	-fno-builtin -nostdinc -nostdlib -pipe
CFLAGS      += $(COMMON_FLAGS)
ASFLAGS     += $(COMMON_FLAGS)
LDFLAGS     += -static

# Architecture optimization configuration
ifeq ($(ARCH),ARM64)
	CFLAGS  += -mgeneral-regs-only -mno-omit-leaf-frame-pointer -mlittle-endian \
		-I$(SRC_ROOT)/arch/arm64/include
	ASFLAGS += -I$(SRC_ROOT)/arch/arm64/include -D__ASSEMBLY__ 
	LDFLAGS += -EL --no-warn-rwx-segments 
endif

# Verify toolchain commands
$(foreach cmd,$(CC) $(CPP) $(LD) $(AR) $(OBJCOPY) $(SIZE) $(OBJDUMP) $(NM) $(READELF),\
	$(if $(shell command -v $(cmd) >/dev/null 2>&1 || echo 1), \
		$(error "Cannot find required toolchain command: $(cmd)")))

# Toolchain version check
TOOLCHAIN_MIN_VERSION := 8.0.0

$(eval TOOLCHAIN_VERSION := $(shell $(CC) -dumpversion 2>/dev/null))

ifeq ($(TOOLCHAIN_VERSION),)
	$(error "Cannot determine toolchain version. Please check if $(CC) is available")
else ifneq ($(shell printf '%s\n%s\n' "$(TOOLCHAIN_MIN_VERSION)" "$(TOOLCHAIN_VERSION)" | sort -V | head -n1),$(TOOLCHAIN_MIN_VERSION))
	$(error "GCC $(TOOLCHAIN_MIN_VERSION) or higher is required, but found $(TOOLCHAIN_VERSION)")
endif
