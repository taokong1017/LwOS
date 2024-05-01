TARGET     = lw.elf
BASE_DIR   = $(CURDIR)
BUILD_DIR  = $(BASE_DIR)/build
OUT_DIR    = $(BASE_DIR)/out
CONFIG_DIR = $(BASE_DIR)/config


include $(BASE_DIR)/.config
include $(BUILD_DIR)/compiler.mk
include $(BUILD_DIR)/global.def

define QEMU_RUN
	qemu-system-aarch64 -machine virt -smp 4 -m 512M -cpu cortex-a53 -nographic -kernel $(1)
endef

.PHONY: all menuconfig run dbg clean help

all:
	@echo "build all success"

run:
	$(call QEMU_RUN, $(OUT_DIR)/$(TARGET))

dbg:
	$(call QEMU_RUN, $(OUT_DIR)/$(TARGET)) -S -s

menuconfig:
	@python  $(CONFIG_DIR)/usr_config.py

clean:
	rm -rf $(OUT_DIR)

help:
	@echo "make all:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- -j"
	@echo "make clean:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- clean"
	@echo "make run:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- run"