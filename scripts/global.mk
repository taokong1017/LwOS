# File: scripts/global.mk

# Initialize global variables
EXTRA_CFLAGS :=
EXTRA_AFLAGS :=
MAKEFLAGS += --max-load=$(shell nproc)
HEADERS   := \
	-I$(SRC_ROOT)/arch/arm64/include \
	-I$(SRC_ROOT)/drivers/serial/include \
	-I$(SRC_ROOT)/sample/include \
	-I$(SRC_ROOT)/lib/libc/include \
	-I$(SRC_ROOT)/kernel/include \
	-I$(SRC_ROOT)/components/rpmsg/include \
	-I$(SRC_ROOT)/components/shell/include \
	-I$(SRC_ROOT)/components/shell/include \
	-I$(BUILD_ROOT)/generated/include \
	-I$(SRC_ROOT) \
	-I$(BUILD_ROOT)
CFLAGS    += $(HEADERS)
ASFLAGS   += $(HEADERS)

# Add verbose output control
ifeq ($(V),1)
	Q =
	QUIET =
else
	Q = @
	QUIET = quiet_
endif
