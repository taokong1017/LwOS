TARGET     = LwOS.elf
BASE_DIR   = $(CURDIR)
CONFIG_DIR = $(BASE_DIR)/config
SUB_DIRS   =
LINKER     = $(BASE_DIR)/sample/linker/lwos.lds

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

define ALL_OBJS
	$(shell find $(1) -name "*.o")
endef

CFLAGS := -I$(BASE_DIR)/sample/include

include scripts/Makefile.compiler
include scripts/Makefile.define
include scripts/Kbuild.include
include scripts/Makefile.lib
build := -f scripts/Makefile.build obj
clean := -f scripts/Makefile.clean obj

define MAKE_LDS
	$(CPP) $(cpp_flags) -D__ASSEMBLY__  $(2) |grep -v "^#" > $(1)
endef

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

.PHONY: all menuconfig run dbg clean help obj $(LINKER)

all: obj $(LINKER)
	$(Q)$(LD) $(LDFLAGS) -T $(LINKER) -e 0x4000000 -o $(TARGET) $(strip $(call ALL_OBJS, $(srctree)))
	@echo "build all success"

obj:
	$(Q)$(call MAKE_CMD, $(SUB_DIRS))
	@echo "build obj success"

$(LINKER): %.lds: %.lds.S
	$(Q)$(call MAKE_LDS, $@, $<);
	@echo "build linker success"

run:
	$(Q)$(call QEMU_RUN, $(OUT_DIR)/$(TARGET))

dbg:
	$(Q)$(call QEMU_RUN, $(OUT_DIR)/$(TARGET)) -S -s

menuconfig:
	$(Q)python  $(CONFIG_DIR)/usr_config.py

clean:
	$(Q)$(RM) -rf $(TARGET)
	$(Q)$(RM) -rf $(BASE_DIR)/sample/linker/lwos.lds $(BASE_DIR)/sample/linker/.lwos.lds*
	$(Q)$(call MAKE_CLEAN_CMD, $(SUB_DIRS))

help:
	@echo "make all:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- -j"
	@echo "make clean:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- clean"
	@echo "make run:	make CROSS_COMPILE=aarch64-none-elf/bin/aarch64-none-elf- run"