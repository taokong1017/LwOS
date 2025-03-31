PROJECT     ?= LwOS
SRC_ROOT    ?= $(CURDIR)
BUILD_ROOT  ?= $(SRC_ROOT)/build
TARGET      ?= $(BUILD_ROOT)/$(PROJECT).elf
SRC_DIRS    ?= arch kernel drivers samples components lib
CONFIG_FILE ?= .config
DEFAULT_CONFIG ?= config/arm64_qemu_defconfig

export PROJECT SRC_ROOT BUILD_ROOT SRC_DIRS CONFIG_FILE
.DEFAULT_GOAL := all

include scripts/pyenv.mk
include scripts/global.mk
include scripts/toolchain.mk
include scripts/logo.mk
include scripts/config.mk
include scripts/offsets.mk
include scripts/kbuild.mk
include scripts/analysis.mk

QEMU_RUN = qemu-system-aarch64 -machine virt,gic-version=2 -smp $(CONFIG_CPUS_MAX_NUM) -m 1024M -cpu cortex-a53 -nographic -kernel

.PHONY: all menuconfig run gdb clean help analyze

all: $(TARGET)
	@echo "Build completed successfully"

run:
	$(Q)$(QEMU_RUN) $(TARGET)

gdb:
	$(Q)$(QEMU_RUN) $(TARGET) -S -s

menuconfig:
	@test -f scripts/kconfig.py || (echo "Error: kconfig.py not found"; exit 1)
	$(Q)$(PYTHON_CMD) scripts/kconfig.py $(CONFIG_FILE)
	@echo "Configuration completed successfully"

clean:
	@echo "Remove directory and config file"
	$(Q)rm -rf $(BUILD_ROOT)
	@echo "Clean completed successfully"

help:
	@echo "make config:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- menuconfig"
	@echo "make all:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- -j"
	@echo "make clean:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- clean"
	@echo "make run:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- run"
	@echo "make gdb:		make CROSS_COMPILE=~/aarch64-none-elf/bin/aarch64-none-elf- gdb"