TARGET     = lw.elf
BASE_DIR   = $(CURDIR)
BUILD_DIR  = $(BASE_DIR)/build
CONFIG_DIR = $(BASE_DIR)/config
SUB_DIRS   =

ifeq ($(V),1)
	export quiet =
	export Q =
else
	export quiet = quiet_
	export Q = @
endif
export srctree = $(BASE_DIR)

define QEMU_RUN
	qemu-system-aarch64 -machine virt -smp 4 -m 512M -cpu cortex-a53 -nographic -kernel $(1)
endef

build := -f scripts/Makefile.build obj
clean := -f scripts/Makefile.clean obj
define MAKE_CMD
	$(Q)for dir in $(1); do\
		$(MAKE) $(build)=$$dir; \
	done
endef

define MAKE_CLEAN_CMD
	$(Q)for dir in $(1); do\
		$(MAKE) $(clean)=$$dir; \
	done
endef

.PHONY: all menuconfig run dbg clean help

all:
	$(Q)$(call MAKE_CMD, $(SUB_DIRS))
	@echo "build all success"

run:
	$(Q)$(call QEMU_RUN, $(OUT_DIR)/$(TARGET))

dbg:
	$(Q)$(call QEMU_RUN, $(OUT_DIR)/$(TARGET)) -S -s

menuconfig:
	$(Q)python  $(CONFIG_DIR)/usr_config.py

clean:
	$(Q)$(call MAKE_CLEAN_CMD, $(SUB_DIRS))

help:
	@echo "make all:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- -j"
	@echo "make clean:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- clean"
	@echo "make run:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- run"