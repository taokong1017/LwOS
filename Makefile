PROJECT    = LwOS
TARGET     = $(PROJECT).elf
BASE_DIR   = $(CURDIR)
CONFIG_DIR = $(BASE_DIR)/config
SUB_DIRS   = arch kernel drivers samples
LINKER     = $(BASE_DIR)/samples/linker/lwos.lds
CONFIG     :=

CROSS_COMPILE :=
LOGO       := kernel/src/logo.c
CPU_NUM    := 2

ifeq ($(V),1)
	export quiet =
	export Q =
else
	export quiet = quiet_
	export Q = @
endif
export srctree = $(BASE_DIR)

QEMU_RUN = qemu-system-aarch64 -machine virt,gic-version=2 -smp $(CPU_NUM) -m 1024M -cpu cortex-a53 -nographic -kernel

define ALL_OBJS
	$(shell find $(1) -name "*.o")
endef

-include .config
include scripts/Makefile.compiler
include scripts/Makefile.define
include scripts/Kbuild.include
include scripts/Makefile.lib
BUILD := -f scripts/Makefile.build obj
CLEAN := -f scripts/Makefile.clean obj

define MAKE_LDS
	$(CPP) $(cpp_flags) -D__ASSEMBLY__  $(2) |grep -v "^#" > $(1)
endef

define MAKE_CMD
	$(Q)for dir in $(1); do\
		$(MAKE) $(BUILD)=$$dir; \
	done
endef

define MAKE_CLEAN_CMD
	$(Q)for dir in $(1); do\
		$(MAKE) $(CLEAN)=$$dir; \
	done
endef

.PHONY: all check menuconfig run dbg clean help obj defconfig gen $(LINKER)

all: obj $(LINKER)
	$(Q)$(LD) $(LDFLAGS) -T $(LINKER) -e __start -o $(TARGET) -Map=$(PROJECT).map \
		$(strip $(filter-out %/offsets.o, $(call ALL_OBJS, $(srctree))))
	$(Q)$(OBJDUMP) -d $(TARGET) > $(PROJECT).sym
	$(Q)$(READELF) -e $(TARGET) > $(PROJECT).stat
	@echo "build all success"

check:
ifeq ($(CROSS_COMPILE),)
	$(error CROSS_COMPILE is not set)
endif
ifeq ($(wildcard .config),)
	$(error excute make menuconfig or make defconfig ***)
endif

obj: gen 
	$(Q)$(call MAKE_CMD, $(SUB_DIRS))
	@echo "build obj success"

$(LOGO):
	$(Q)python3 scripts/logo_gen.py -o $(LOGO)

gen: check $(LOGO)
ifeq ($(CONFIG_ARM64), y)
	$(Q)$(CC) $(CFLAGS) -c $(BASE_DIR)/arch/arm64/src/offsets.c -o $(BASE_DIR)/arch/arm64/src/offsets.o
	$(Q)python3 scripts/gen_offset_header.py -i $(BASE_DIR)/arch/arm64/src/offsets.o -o \
		$(BASE_DIR)/arch/arm64/include/offsets.h
	$(Q)$(RM) $(BASE_DIR)/arch/arm64/src/offsets.o
endif

$(LINKER): %.lds: %.lds.S check
	$(Q)$(call MAKE_LDS, $@, $<);
	@echo "build linker success"

run:
	$(Q)$(QEMU_RUN) $(TARGET)

dbg:
	$(Q)$(QEMU_RUN) $(TARGET) -S -s

menuconfig:
	$(Q)python3  $(CONFIG_DIR)/usr_config.py
	$(Q)make clean

defconfig: 
	$(Q)if [ -e $(CONFIG_DIR)/$(CONFIG) ]; then \
		python3 $(CONFIG_DIR)/usr_config.py defconfig $(CONFIG_DIR)/$(CONFIG); \
	else	\
		echo "*** $(CONFIG_DIR)/$(CONFIG) does not exist ***"; \
	fi
	$(Q)make clean

clean:
	$(Q)$(RM) $(BASE_DIR)/samples/linker/lwos.lds $(BASE_DIR)/samples/linker/.lwos.lds*
	$(Q)$(RM) $(BASE_DIR)/arch/arm64/include/offsets.h
	$(Q)$(call MAKE_CLEAN_CMD, $(SUB_DIRS))
	$(Q)$(RM) $(TARGET) $(LOGO)
	$(Q)$(RM) $(shell find $(SUB_DIRS) -name "*.o*")

help:
	@echo "make config:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- menuconfig"
	@echo "make defconfig:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- CONFIG=aarch64_defconfig defconfig"
	@echo "make all:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- -j"
	@echo "make clean:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- clean"
	@echo "make run:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- run"
	@echo "make dbg:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- dbg"