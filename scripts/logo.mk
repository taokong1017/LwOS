# File: scripts/logo.mk

LOGO_HEADER   := $(BUILD_ROOT)/generated/include/logo.h

# Command to generate logo header file
$(LOGO_HEADER):
	$(Q)mkdir -p $(@D)
	$(Q)$(PYTHON_CMD) $(SRC_ROOT)/scripts/gen_logo.py -o $(LOGO_HEADER)
