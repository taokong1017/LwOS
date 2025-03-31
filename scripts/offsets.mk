# File: scripts/offset.mk

# Define the generated header file path
OFFSET_HEADER := $(BUILD_ROOT)/generated/include/offsets.h
OFFSET_OBJ    := $(BUILD_ROOT)/generated/include/offsets.o
OFFSET_DEPEND := $(BUILD_ROOT)/generated/include/\.offsets.d
OFFSET_SRC    := $(SRC_ROOT)/arch/arm64/src/offsets.c

# Rule to generate config.h from config file
$(OFFSET_HEADER): $(CONFIG_HEADER) $(OFFSET_SRC)
	@mkdir -p $(@D)
	$(Q)$(CC) $(CFLAGS) -MT $@ -MMD -MP -MF $(@D)/.$(basename $(@F)).d -c $(OFFSET_SRC) -o $(OFFSET_OBJ)
	$(Q)$(PYTHON_CMD) $(SRC_ROOT)/scripts/gen_offset_header.py -i $(OFFSET_OBJ) -o $(OFFSET_HEADER)
	$(Q)rm -rf $(OFFSET_OBJ) $(OFFSET_DEPEND)
